#ifndef ALTIMETER_H
#define ALTIMETER_H

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    double altitude;           // altitude en mètres
    double reference_pressure; // pression de référence en hPa (celle où altitude = 0)
    bool has_reference;        // true si la référence a été définie
} Altimeter;

/**
 * @brief Initialise l'altimètre.
 * @param alt Pointeur vers la structure Altimeter.
 */
void altimeter_init(Altimeter *alt);

/**
 * @brief Définit la pression de référence à partir de la pression actuelle.
 * @param alt Pointeur vers la structure Altimeter.
 * @param pressure_kpa Pression actuelle en kPa.
 */
void altimeter_set_reference(Altimeter *alt, double pressure_kpa);

/**
 * @brief Met à jour l'altitude à partir de la pression.
 * @param alt Pointeur vers la structure Altimeter.
 * @param pressure_kpa Pression en kPa (double)
 */
void altimeter_update(Altimeter *alt, double pressure_kpa);

/**
 * @brief Retourne l'altitude en mètres (relative à la référence).
 * @param alt Pointeur vers la structure Altimeter.
 * @return Altitude en mètres, ou 0 si la référence n'est pas définie.
 */
double altimeter_get_altitude(const Altimeter *alt);

/**
 * @brief Indique si la référence a été définie.
 */
bool altimeter_has_reference(const Altimeter *alt);

#endif /* ALTIMETER_H */