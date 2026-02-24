#include "altimeter.h"
#include <math.h>

#define KPA_TO_HPA 10.0

void altimeter_init(Altimeter *alt)
{
    alt->altitude = 0.0;
    alt->reference_pressure = 1013.25; // valeur par défaut (non utilisée tant que has_reference est false)
    alt->has_reference = false;
}

void altimeter_set_reference(Altimeter *alt, double pressure_kpa)
{
    alt->reference_pressure = pressure_kpa * KPA_TO_HPA;
    alt->has_reference = true;
    // on peut aussi mettre à jour l'altitude tout de suite
    altimeter_update(alt, pressure_kpa);
}

void altimeter_update(Altimeter *alt, double pressure_kpa)
{
    if (!alt->has_reference) {
        // Si pas de référence, on utilise la formule avec la pression standard,
        // mais l'altitude sera absolue (peu fiable)
        double pressure_hpa = pressure_kpa * KPA_TO_HPA;
        alt->altitude = 44330.0 * (1.0 - pow(pressure_hpa / 1013.25, 1.0/5.255));
        return;
    }

    double pressure_hpa = pressure_kpa * KPA_TO_HPA;
    // altitude relative à la référence
    alt->altitude = 44330.0 * (1.0 - pow(pressure_hpa / alt->reference_pressure, 1.0/5.255));
}

double altimeter_get_altitude(const Altimeter *alt)
{
    return alt->altitude;
}

bool altimeter_has_reference(const Altimeter *alt)
{
    return alt->has_reference;
}