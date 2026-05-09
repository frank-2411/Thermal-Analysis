import streamlit as st

# ==========================================
# 1. IL TUO MOTORE DI CALCOLO
# ==========================================
def le_tue_formule(spessore, materiale, n_mesh):
    """
    Sostituisci questa logica con i tuoi codici esistenti.
    """
    # Dati fittizi per l'esempio
    conducibilita = {"Alluminio": 237, "Rame": 401, "Titanio": 21.9, "Acciaio": 50}
    k = conducibilita.get(materiale, 0)
    
    # Prevenzione divisione per zero
    if spessore <= 0:
        return "Errore: lo spessore deve essere maggiore di zero.", False
        
    # Calcolo fittizio
    risultato_termico = (k * n_mesh) / spessore 
    
    # Restituiamo il testo del risultato e un flag (True) per indicare che è andato a buon fine
    return f"Flusso termico calcolato: **{risultato_termico:.2f} W/mK**\n\n*(Dati usati - k: {k}, Mesh: {n_mesh})*", True

# ==========================================
# 2. INTERFACCIA WEB (STREAMLIT)
# ==========================================
# Configurazione base della pagina web
st.set_page_config(page_title="Analisi Termica", page_icon="🌡️", layout="centered")

st.title("🌡️ Analisi Termica")
st.markdown("Inserisci i parametri di configurazione per avviare il solutore.")

# Organizziamo gli input su due colonne per renderlo più bello graficamente
col1, col2 = st.columns(2)

with col1:
    # Streamlit converte automaticamente l'input in float (numeri decimali)
    spessore_val = st.number_input("Spessore (mm):", min_value=0.01, value=1.00, step=0.1)
    
    # Menu a tendina
    materiali_disponibili = ["Alluminio", "Rame", "Titanio", "Acciaio"]
    materiale_val = st.selectbox("Materiale:", materiali_disponibili)

with col2:
    # Streamlit converte automaticamente l'input in int (numeri interi)
    mesh_val = st.number_input("Numero di Mesh:", min_value=1, value=10, step=1)

st.divider() # Linea di separazione

# Pulsante per avviare il calcolo
if st.button("Esegui Analisi", type="primary"):
    # Mostra uno spinner di caricamento (utile se i tuoi calcoli richiedono qualche secondo)
    with st.spinner("Calcolo in corso..."):
        
        # Chiamiamo la tua funzione
        messaggio, successo = le_tue_formule(spessore_val, materiale_val, mesh_val)
        
        # Mostriamo il risultato graficamente
        if successo:
            st.success("Analisi completata con successo!")
            st.info(messaggio)
        else:
            st.error(messaggio)