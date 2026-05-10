#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include "astro_constants.h"

// Funzione di supporto per il calcolo della norma (lunghezza) di un vettore 3D
double norm3(double x, double y, double z) {
    return sqrt(x*x + y*y + z*z);
}

// Funzione di supporto per il prodotto scalare
double dot3(double x1, double y1, double z1, double x2, double y2, double z2) {
    return (x1*x2 + y1*y2 + z1*z2);
}

// ==========================================
// FUNZIONE ESPORTATA PER PYTHON
// ==========================================
void calcola_flusso_eclissi(
    int num_steps, 
    double* r_sc,   // Input: Array 1D posizioni MAVEN [Nx3]
    double* r_sun,  // Input: Array 1D posizioni Sole  [Nx3]
    double* flux,   // Output: Array 1D Flusso Solare  [N]
    int* is_eclipse // Output: Array 1D Eclissi (1=Si, 0=No) [N]
) {
    for (int i = 0; i < num_steps; i++) {
        // Estrai coordinate per questo istante di tempo
        double sc_x = r_sc[i*3 + 0], sc_y = r_sc[i*3 + 1], sc_z = r_sc[i*3 + 2];
        double s_x = r_sun[i*3 + 0], s_y = r_sun[i*3 + 1], s_z = r_sun[i*3 + 2];

        // 1. CALCOLO FLUSSO SOLARE
        double v_sun_mav_x = s_x - sc_x;
        double v_sun_mav_y = s_y - sc_y;
        double v_sun_mav_z = s_z - sc_z;
        
        // Distanza in metri
        double dist_sun_maven_m = norm3(v_sun_mav_x, v_sun_mav_y, v_sun_mav_z) * 1000.0;
        
        // Equazione del flusso: L / (4 * pi * d^2)
        flux[i] = L_SUN / (4.0 * M_PI * dist_sun_maven_m * dist_sun_maven_m);

        // 2. MODELLO D'OMBRA CONICO (ECLISSI)
        double dist_sc_sun = norm3(v_sun_mav_x, v_sun_mav_y, v_sun_mav_z); // = norm(r_sun - r_sc)
        double dist_sc_mars = norm3(sc_x, sc_y, sc_z);                     // = norm(r_sc)

        double term_sun = R_SUN / dist_sc_sun;
        double ang_sun = asin(term_sun > 1.0 ? 1.0 : term_sun);

        double term_mars = R_MARS / dist_sc_mars;
        double ang_mars = asin(term_mars > 1.0 ? 1.0 : term_mars);

        // Versori
        double u_mars_x = -sc_x / dist_sc_mars;
        double u_mars_y = -sc_y / dist_sc_mars;
        double u_mars_z = -sc_z / dist_sc_mars;

        double u_sun_x = v_sun_mav_x / dist_sc_sun;
        double u_sun_y = v_sun_mav_y / dist_sc_sun;
        double u_sun_z = v_sun_mav_z / dist_sc_sun;

        double cos_sep = dot3(u_mars_x, u_mars_y, u_mars_z, u_sun_x, u_sun_y, u_sun_z);
        if (cos_sep > 1.0) cos_sep = 1.0;
        if (cos_sep < -1.0) cos_sep = -1.0;
        
        double theta = acos(cos_sep);

        // is_sunlight = theta > (ang_sun + ang_mars)
        int is_sunlight = (theta > (ang_sun + ang_mars)) ? 1 : 0;
        
        // Se non è alla luce del sole, è in eclissi (penombra o ombra)
        is_eclipse[i] = (is_sunlight == 1) ? 0 : 1;
    }
}