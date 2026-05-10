#ifndef ASTRO_CONSTANTS_H
#define ASTRO_CONSTANTS_H

#include <math.h>

// Se M_PI non è definito dal sistema, lo definiamo noi per sicurezza
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// =========================================================================
// 1. COSTANTI ASTRONOMICHE GENERICHE E SOLE
// =========================================================================
#define G_CONST           6.67259e-20       // Universal gravity constant [km^3/(kg*s^2)]
#define AU_KM             149597870.691     // Astronomical Unit [km]
#define C_LIGHT           299792.458        // Speed of light in vacuum [km/s]
#define G0_ACCEL          9.80665           // Standard free fall (g0) [m/s^2]
#define R_EARTH_MOON      384400.0          // Mean distance Earth-Moon [km]
#define OBLIQ_ECLIPTIC    (84381.412 / 3600.0 * M_PI / 180.0) // Obliquity of ecliptic (J2000) [rad]
#define DAYS_JULIAN_YEAR  365.25            // Earth days in a Julian year

#define R_SUN             6.955e5           // Sun mean radius [km]
#define MU_SUN            1.32712440017987e11 // Sun planetary constant (mu) [km^3/s^2]
#define SOLAR_FLUX_1AU    1367.0            // Energy flux density of the Sun at 1 AU [W/m^2]
#define L_SUN             3.828e26          // Solar luminosity [W]

// =========================================================================
// 2. COSTANTI PLANETARIE (MU = G * M) [km^3/s^2]
// =========================================================================
#define MU_MERCURY        2.203208e4
#define MU_VENUS          3.24858599e5
#define MU_EARTH          3.98600433e5
#define MU_MARS           4.2828314e4
#define MU_JUPITER        1.26712767863e8
#define MU_SATURN         3.79406260630e7
#define MU_URANUS         5.79454900700e6
#define MU_NEPTUNE        6.83653406400e6
#define MU_PLUTO          9.81601000000e2
#define MU_MOON           4902.801

// =========================================================================
// 3. RAGGIO MEDIO DEI PIANETI [km]
// =========================================================================
#define R_MERCURY         2439.7
#define R_VENUS           6051.8
#define R_EARTH           6371.01
#define R_MARS            3389.9
#define R_JUPITER         69911.0
#define R_SATURN          58232.0
#define R_URANUS          25362.0
#define R_NEPTUNE         24624.0
#define R_PLUTO           1151.0
#define R_MOON            1738.0

// =========================================================================
// 4. COEFFICIENTI ARMONICI GRAVITAZIONALI (J2) [-]
// =========================================================================
#define J2_MERCURY        (50.3 * 1e-6)
#define J2_VENUS          (4.458 * 1e-6)
#define J2_EARTH          0.1082626925638815e-2
#define J2_MARS           (1960.45 * 1e-6)
#define J2_JUPITER        (14696.5735 * 1e-6)
#define J2_SATURN         (16290.573 * 1e-6)
#define J2_URANUS         (3510.68 * 1e-6)
#define J2_NEPTUNE        (3408.43 * 1e-6)
#define J2_PLUTO          0.0   // Not present in NASA fact sheets
#define J2_MOON           (202.7 * 1e-6)

// =========================================================================
// 5. SCHIACCIAMENTO PLANETARIO (Oblateness) [-]
// =========================================================================
#define OBLAT_MERCURY     0.0009
#define OBLAT_VENUS       0.00001
#define OBLAT_EARTH       0.00335
#define OBLAT_MARS        0.00589
#define OBLAT_JUPITER     0.06487
#define OBLAT_SATURN      0.09796
#define OBLAT_URANUS      0.02293
#define OBLAT_NEPTUNE     0.01708
#define OBLAT_PLUTO       0.0
#define OBLAT_MOON        0.0012

// =========================================================================
// 6. PERIODO DI ROTAZIONE SIDERALE [hours]
// =========================================================================
#define ROT_MERCURY       1407.6
#define ROT_VENUS         -5832.6
#define ROT_EARTH         23.9345
#define ROT_MARS          24.6229
#define ROT_JUPITER       9.9250
#define ROT_SATURN        10.656
#define ROT_URANUS        -17.24
#define ROT_NEPTUNE       16.11
#define ROT_PLUTO         -153.2928
#define ROT_MOON          655.720

// =========================================================================
// 7. INCLINAZIONE ASSIALE (Axial tilt) [deg]
// =========================================================================
#define TILT_MERCURY      0.034
#define TILT_VENUS        177.36
#define TILT_EARTH        23.44
#define TILT_MARS         25.19
#define TILT_JUPITER      3.13
#define TILT_SATURN       26.73
#define TILT_URANUS       97.77
#define TILT_NEPTUNE      28.32
#define TILT_PLUTO        119.51
#define TILT_MOON         6.68

// =========================================================================
// 8. IRRAGGIAMENTO SOLARE (Solar Irradiance) [W/m^2]
// =========================================================================
#define IRR_MERCURY       9082.7
#define IRR_VENUS         2601.3
#define IRR_EARTH         1361.0
#define IRR_MARS          586.2
#define IRR_JUPITER       50.26
#define IRR_SATURN        14.82
#define IRR_URANUS        3.69
#define IRR_NEPTUNE       1.508
#define IRR_PLUTO         0.873
#define IRR_MOON          1361.0

#endif // ASTRO_CONSTANTS_H