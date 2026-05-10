import streamlit as st
import numpy as np
import ctypes
import os
import requests
import re
import subprocess
import pandas as pd
from datetime import datetime

# ==========================================
# 0. AUTO-COMPILAZIONE PER STREAMLIT CLOUD
# ==========================================
# Se siamo su Linux (Streamlit Cloud) compila i file C in automatico
if os.name != 'nt': 
    librerie = [
        ('maven_mesh.so', 'maven_mesh.c'),
        ('solar_math.so', 'solar_math.c'),
        ('thermal_solver.so', 'thermal_solver.c')
    ]
    for lib_so, file_c in librerie:
        if not os.path.exists(lib_so):
            print(f"Compilazione di {file_c} in corso su Streamlit Cloud...")
            # Lancia il comando: gcc -shared -fPIC -o nome.so nome.c -lm
            subprocess.run(['gcc', '-shared', '-fPIC', '-o', lib_so, file_c, '-lm'], check=True)

# ==========================================
# 1. CARICAMENTO DELLE 3 LIBRERIE C
# ==========================================
def load_c_lib(name):
    path = os.path.join(os.path.dirname(__file__), f'{name}.dll' if os.name == 'nt' else f'{name}.so')
    return ctypes.CDLL(path)

try:
    mesh_c = load_c_lib('maven_mesh')
    solar_c = load_c_lib('solar_math')
    therm_c = load_c_lib('thermal_solver')
except OSError as e:
    st.error(f"⚠️ Errore caricamento librerie C. Assicurati di aver compilato le 3 DLL nella stessa cartella: {e}")
    st.stop()

# ==========================================
# 2. FUNZIONI PYTHON (Orchestrazione NASA)
# ==========================================
def scarica_ephemeris(comando, centro, t_start, t_stop, step):
    # Proteggiamo l'URL sostituendo gli spazi con '%20' (es. "10 m" -> "10%20m")
    step_safe = step.replace(" ", "%20")
    url = f"https://ssd.jpl.nasa.gov/api/horizons.api?format=text&COMMAND='{comando}'&EPHEM_TYPE=VECTORS&CENTER='{centro}'&OUT_UNITS='KM-S'&START_TIME='{t_start}'&STOP_TIME='{t_stop}'&STEP_SIZE='{step_safe}'&VEC_TABLE='2'"
    
    try:
        # Aggiunto timeout per evitare blocchi infiniti se il server NASA è lento
        risposta = requests.get(url, timeout=30)
        testo = risposta.text
        
        # Controllo di sicurezza: la NASA ha trovato i dati?
        if "$$SOE" not in testo:
            # Stampiamo a schermo il vero motivo dell'errore (es. data sbagliata)
            st.error(f"Errore server NASA per l'oggetto {comando}. Risposta grezza:\n{testo[:300]}")
            return np.array([])
            
        dati = testo[testo.index("$$SOE")+5 : testo.index("$$EOE")]
        
        # Regex migliorata: " \s*=\s* " significa che l'uguale può essere 
        # attaccato o staccato, a Python non importerà!
        pattern = r"X\s*=\s*([-\d\.E\+]+)\s*Y\s*=\s*([-\d\.E\+]+)\s*Z\s*=\s*([-\d\.E\+]+)\s*VX\s*=\s*([-\d\.E\+]+)\s*VY\s*=\s*([-\d\.E\+]+)\s*VZ\s*=\s*([-\d\.E\+]+)"
        matches = re.findall(pattern, dati)
        
        return np.array(matches, dtype=np.float64)
        
    except Exception as e:
        st.error(f"Errore di connessione a JPL Horizons: {e}")
        return np.array([])

# ==========================================
# 3. INTERFACCIA STREAMLIT
# ==========================================
st.set_page_config(page_title="MAVEN Thermal Analysis", page_icon="🛰️", layout="wide")
st.title("🔥 MAVEN: Solutore Termico ad Alte Prestazioni (Motore C)")

# Layout in due colonne
col1, col2 = st.columns([1, 3])

with col1:
    st.subheader("1. Simulazione")
    d_start = st.date_input("Inizio", datetime(2014, 10, 4))
    d_stop = st.date_input("Fine", datetime(2014, 10, 6))
    mesh_level = st.selectbox("Dettaglio Mesh", [1, 2], help="1=Veloce, 2=Dettagliata")
    dt_min = st.number_input("Time-step ODE (minuti)", value=10.0, step=1.0)
    
    st.subheader("2. Riscaldatore Batteria")
    q_max = st.number_input("Potenza Max (W)", value=50.0)
    t_set = st.number_input("Setpoint ON (°C)", value=-10.0)
    
    esegui = st.button("🚀 Avvia Solutore C", type="primary", use_container_width=True)

if esegui:
    # Parametri di calcolo
    dt_sec = dt_min * 60.0
    step_str = f"{int(dt_min)} m"
    
    # --- FASE 1: AMBIENTE ---
    with st.spinner("1/3 Download Effemeridi NASA Horizons..."):
        d1 = d_start.strftime("%Y-%m-%d")
        d2 = d_stop.strftime("%Y-%m-%d")
        r_v_sc = scarica_ephemeris('-202', '500@499', d1, d2, step_str)
        r_v_sun = scarica_ephemeris('10', '500@499', d1, d2, step_str)
        
        if len(r_v_sc) == 0:
            st.error("Errore download NASA. Controlla le date.")
            st.stop()
            
        n_steps = min(len(r_v_sc), len(r_v_sun))
        r_sc = r_v_sc[:n_steps, 0:3].flatten()
        v_sc = r_v_sc[:n_steps, 3:6].flatten()
        r_sun = r_v_sun[:n_steps, 0:3].flatten()
        
        flux = np.zeros(n_steps, dtype=np.float64)
        ecl = np.zeros(n_steps, dtype=np.int32)
        
        # FIRMA E CHIAMATA SOLAR MATH
        solar_c.calcola_flusso_eclissi.argtypes = [
            ctypes.c_int, np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.float64),
            np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.int32)
        ]
        solar_c.calcola_flusso_eclissi(n_steps, r_sc, r_sun, flux, ecl)

    # --- FASE 2: GEOMETRIA ---
    with st.spinner("2/3 Costruzione Matrici Spaziali (Mesh)..."):
        MAX_NODES = 500
        v_out = np.zeros(MAX_NODES * 24, dtype=np.float64)
        nv_out = np.zeros(MAX_NODES, dtype=np.int32)
        n_out = np.zeros(MAX_NODES * 3, dtype=np.float64)
        t_out = np.zeros(MAX_NODES, dtype=np.int32)
        
        # FIRMA E CHIAMATA MESH
        mesh_c.generate_maven_mesh_c.argtypes = [
            ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double, ctypes.c_double,
            ctypes.c_double, ctypes.c_double, ctypes.c_int,
            np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.int32), 
            np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.int32)
        ]
        mesh_c.generate_maven_mesh_c.restype = ctypes.c_int
        
        n_nodes = mesh_c.generate_maven_mesh_c(
            2.0, 2.0, 2.0, 1.5, 2.0, 1.0, np.deg2rad(15), 1.0, mesh_level,
            v_out, nv_out, n_out, t_out
        )

    # --- FASE 3: TERMODINAMICA ---
    with st.spinner("3/3 Integrazione ODE in C..."):
        temp_hist = np.zeros(n_steps * n_nodes, dtype=np.float64)
        
        # FIRMA E CHIAMATA THERMAL SOLVER
        therm_c.solve_thermal_ode.argtypes = [
            ctypes.c_int, np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.int32),
            np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.int32),
            ctypes.c_int, ctypes.c_double,
            np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.float64),
            np.ctypeslib.ndpointer(np.int32), np.ctypeslib.ndpointer(np.float64), np.ctypeslib.ndpointer(np.float64),
            ctypes.c_double, ctypes.c_double, ctypes.c_double, np.ctypeslib.ndpointer(np.float64)
        ]
        
        therm_c.solve_thermal_ode(
            n_nodes, v_out, nv_out, n_out, t_out,
            n_steps, dt_sec, flux, r_sun, ecl, r_sc, v_sc,
            q_max, t_set + 273.15, 5.0, temp_hist
        )
        
        # Convertiamo Kelvin in Celsius e rimodelliamo in matrice 2D [Tempi x Nodi]
        T_celsius = temp_hist.reshape((n_steps, n_nodes)) - 273.15

    # --- FASE 4: RISULTATI ---
    with col2:
        st.success(f"Analisi Completata! ({n_steps} passi integrati su {n_nodes} nodi in frazioni di secondo)")
        
        st.subheader("🌡️ Analisi Termica Batterie (EaglePicher LP33165)")
        
        # Nodi Batteria (i tipi 4 e 5 definiti nel file C)
        indici_bat = np.where(t_out[:n_nodes] >= 4)[0]
        
        if len(indici_bat) > 0:
            T_bat_media = np.mean(T_celsius[:, indici_bat], axis=1)
            
            # Creiamo il DataFrame per i grafici
            asse_tempi = pd.date_range(start=d_start, periods=n_steps, freq=f'{int(dt_min)}min')
            
            df_plot = pd.DataFrame({
                "Temp. Media Batterie": T_bat_media,
                "Min Operativo (-20°C)": -20.0,
                "Max Operativo (+40°C)": 40.0,
                "Setpoint Heater": t_set
            }, index=asse_tempi)
            
            st.line_chart(df_plot, color=["#1f77b4", "#d62728", "#d62728", "#2ca02c"])
            
            # Report di Sicurezza
            t_min = np.min(T_bat_media)
            t_max = np.max(T_bat_media)
            
            col_a, col_b = st.columns(2)
            col_a.metric("Temperatura Minima", f"{t_min:.1f} °C")
            col_b.metric("Temperatura Massima", f"{t_max:.1f} °C")
            
            if t_min < -20:
                st.error(f"❌ STATO: BATTERIA SOTTO LIMITE DI {abs(-20 - t_min):.1f}°C. Aumenta la potenza max dell'Heater!")
            elif t_min < t_set:
                st.warning("⚠️ STATO: MARGINALE - Il riscaldatore sta lavorando, sei vicino al limite inferiore.")
            else:
                st.success("✅ STATO: OK - Batteria in sicurezza termica all'interno dei limiti.")