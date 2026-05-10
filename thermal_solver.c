#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "astro_constants.h"

// Costanti termodinamiche
#define SIGMA 5.67037e-8
#define T_SPACE 3.0

// Funzioni vettoriali di supporto
double norm3(double x, double y, double z) { return sqrt(x*x + y*y + z*z); }
double dot3(double x1, double y1, double z1, double x2, double y2, double z2) { return x1*x2 + y1*y2 + z1*z2; }
void cross3(double ax, double ay, double az, double bx, double by, double bz, double* out) {
    out[0] = ay*bz - az*by;
    out[1] = az*bx - ax*bz;
    out[2] = ax*by - ay*bx;
}

// ==========================================
// FUNZIONE PRINCIPALE: SOLUTORE ODE CON SUB-STEPPING
// ==========================================
void solve_thermal_ode(
    int num_nodes, double* vertices, int* num_verts, double* normals, int* types,
    int num_steps, double dt, 
    double* S_flux, double* sun_vec, int* is_eclipse, double* r_sc, double* v_sc,
    double Q_heater_max, double T_heat_set, double T_heat_db,
    double* out_Temp_history
) {
    double* Area = (double*)malloc(num_nodes * sizeof(double));
    double* C_cap = (double*)malloc(num_nodes * sizeof(double));
    double* alpha = (double*)malloc(num_nodes * sizeof(double));
    double* eps = (double*)malloc(num_nodes * sizeof(double));
    double* Q_int = (double*)malloc(num_nodes * sizeof(double));
    double* T_current = (double*)malloc(num_nodes * sizeof(double));
    double* dTdt = (double*)malloc(num_nodes * sizeof(double));
    double* centers = (double*)malloc(num_nodes * 3 * sizeof(double));
    double* G_cond = (double*)calloc(num_nodes * num_nodes, sizeof(double));

    // 1. INIZIALIZZAZIONE
    for (int i = 0; i < num_nodes; i++) {
        T_current[i] = 20.0 + 273.15; // Partiamo da 20°C
        Q_int[i] = 0.0;
        
        int nv = num_verts[i];
        double cx = 0, cy = 0, cz = 0, area = 0;
        int v_idx = i * 24;
        for(int v=0; v<nv; v++) {
            cx += vertices[v_idx + v*3]; cy += vertices[v_idx + v*3 + 1]; cz += vertices[v_idx + v*3 + 2];
        }
        centers[i*3] = cx/nv; centers[i*3+1] = cy/nv; centers[i*3+2] = cz/nv;
        
        if (nv == 4) { 
            double dx = vertices[v_idx+3]-vertices[v_idx], dy = vertices[v_idx+4]-vertices[v_idx+1], dz = vertices[v_idx+5]-vertices[v_idx+2];
            double dx2 = vertices[v_idx+6]-vertices[v_idx+3], dy2 = vertices[v_idx+7]-vertices[v_idx+4], dz2 = vertices[v_idx+8]-vertices[v_idx+5];
            area = norm3(dx,dy,dz) * norm3(dx2,dy2,dz2);
        } else { area = M_PI * 1.0 * 1.0; } // HGA
        Area[i] = area;

        // Parametri fisici
        if (types[i] == 1) { 
            alpha[i] = 0.15; eps[i] = 0.05; C_cap[i] = (2700 * area * 0.05) * 897; Q_int[i] = 0.5;
        } else if (types[i] == 2) { 
            alpha[i] = 0.70; eps[i] = 0.85; C_cap[i] = (2000 * area * 0.005) * 700;
        } else if (types[i] == 3) { 
            alpha[i] = 0.20; eps[i] = 0.85; C_cap[i] = (1600 * area * 0.01) * 1100;
        } else { 
            alpha[i] = 0.0; eps[i] = 0.0; C_cap[i] = (18.0 / 6.0) * 900.0; 
        }
    }

    // 2. MATRICE CONDUTTANZE
    for (int i = 0; i < num_nodes; i++) {
        for (int j = 0; j < num_nodes; j++) {
            if (i == j) continue;
            double dist = norm3(centers[i*3]-centers[j*3], centers[i*3+1]-centers[j*3+1], centers[i*3+2]-centers[j*3+2]);
            if (types[i] == 1 && types[j] == 1 && dist < 1.2) G_cond[i*num_nodes+j] = 5.0; 
            if (types[i] == 2 && types[j] == 2 && dist < 1.5) G_cond[i*num_nodes+j] = 3.0; 
            if ((types[i] == 1 && types[j] == 2) && dist < 1.0) G_cond[i*num_nodes+j] = 0.5; 
            if (types[i] >= 4 && types[j] == 1 && dist < 2.0) G_cond[i*num_nodes+j] = 5.0; 
            if (types[i] == types[j] && types[i] >= 4 && dist < 1.0) G_cond[i*num_nodes+j] = 15.0; 
        }
    }

    // 3. LOOP FISICO
    for (int step = 0; step < num_steps; step++) {
        double s_flux = S_flux[step];
        int ecl = is_eclipse[step];
        
        // Calcolo assetto vettoriale
        double sx = sun_vec[step*3], sy = sun_vec[step*3+1], sz = sun_vec[step*3+2];
        double rx = r_sc[step*3], ry = r_sc[step*3+1], rz = r_sc[step*3+2];
        double vx = v_sc[step*3], vy = v_sc[step*3+1], vz = v_sc[step*3+2];
        
        double h[3]; cross3(rx, ry, rz, vx, vy, vz, h);
        double norm_h = norm3(h[0], h[1], h[2]); h[0]/=norm_h; h[1]/=norm_h; h[2]/=norm_h;
        double Zb[3] = {sx, sy, sz};
        double Xb[3]; cross3(Zb[0], Zb[1], Zb[2], h[0], h[1], h[2], Xb);
        double norm_X = norm3(Xb[0], Xb[1], Xb[2]);
        if (norm_X < 1e-6) { Xb[0] = 1; Xb[1] = 0; Xb[2] = 0; } else { Xb[0]/=norm_X; Xb[1]/=norm_X; Xb[2]/=norm_X; }

        // --- LA MAGIA DEL SUB-STEPPING ---
        // Spezziamo il dt (es. 600 secondi) in gradini di massimo 2 secondi
        double dt_max = 2.0; 
        int sub_steps = (int)ceil(dt / dt_max);
        double dt_sim = dt / sub_steps;

        for (int sub = 0; sub < sub_steps; sub++) {
            
            // Calcolo flussi per ogni nodo
            for (int i = 0; i < num_nodes; i++) {
                double q_sol = 0, q_rad = 0, q_cond = 0, q_ex = Q_int[i];

                if (types[i] >= 4) { // Batteria
                    double I_bat = ecl ? 2.67 : 12.0; 
                    q_ex += (I_bat * I_bat * 0.016) / 6.0; 
                    
                    double diff_T = T_heat_set - T_current[i];
                    double factor = diff_T / T_heat_db;
                    if (factor > 1.0) factor = 1.0; if (factor < 0.0) factor = 0.0;
                    q_ex += Q_heater_max * factor;
                } else { // Bus, SA, HGA
                    q_rad = eps[i] * Area[i] * SIGMA * (pow(T_current[i], 4) - pow(T_SPACE, 4));
                    if (!ecl) {
                        double cos_t = (types[i] == 2) ? 1.0 : 0.25; 
                        q_sol = s_flux * alpha[i] * Area[i] * cos_t;
                    }
                }

                for (int j = 0; j < num_nodes; j++) {
                    q_cond += G_cond[i*num_nodes+j] * (T_current[j] - T_current[i]);
                }

                dTdt[i] = (q_sol - q_rad + q_cond + q_ex) / C_cap[i];
            }

            // Applica il mini-passo di temperatura
            for (int i = 0; i < num_nodes; i++) {
                T_current[i] += dTdt[i] * dt_sim;
            }
        }
        
        // Finiti i sub-steps, salviamo il punto da inviare a Python
        for (int i = 0; i < num_nodes; i++) {
            out_Temp_history[step * num_nodes + i] = T_current[i];
        }
    }

    free(Area); free(C_cap); free(alpha); free(eps); free(Q_int); 
    free(T_current); free(dTdt); free(centers); free(G_cond);
}
