# Couche Modèle — `src/model/`

Le **modèle** regroupe tout ce qui relève des **données brutes** (capteurs), des **traitements** (algorithmes sur ces mesures), et du **temps réel** (**RTC**). Ces modules sont en **C pur**, sans LVGL : ils sont appelés depuis les threads du **contrôleur** (surtout **`sampling_thread`** et **`data_init_thread`**).

## Arborescence

```
model/
├── sensors/     # Accès matériel aux capteurs
├── processing/  # Algorithmes et estimateurs
└── rtc/         # Horloge temps réel
```

---

## `model/sensors/`

| Fichier | Rôle |
|---------|------|
| `motion_sensor.h` / `.c` | Abstraction **accéléromètre** (mouvement, pas côté driver) : init, lecture, configuration pour le pipeline de mouvement. |
| `mag_sensor.h` / `.c` | Abstraction **magnétomètre** : lectures pour le compas. |
| `env_sensor.h` / `.c` | Abstraction **environnement** (température, humidité, pression selon la carte). |

Ces fichiers encapsulent les appels **`sensor`** Zephyr et les structures `sensor_value` pour le reste de l’application.

---

## `model/processing/`

Modules de **traitement du signal** et **logique métier** appliqués aux échantillons. Chaque paire `.h` / `.c` couvre un domaine fonctionnel.

| Fichier | Rôle (résumé) |
|---------|----------------|
| `activity.h` / `.c` | Détection / classification d’**activité** à partir des capteurs mouvement. |
| `step_counter.h` / `.c` | **Comptage de pas** (pédomètre). |
| `distance.h` / `.c` | Estimation de **distance** parcourue. |
| `calories.h` / `.c` | Estimation des **calories** ; peut dépendre de `activity.h` pour les types. |
| `compass.h` / `.c` | Calcul du **cap** / orientation à partir du magnéto (et fusion éventuelle). |
| `fusion.h` / `.c` | **Fusion** de capteurs (ex. vecteurs accéléro / magnéto). |
| `weather.h` / `.c` | Logique **tendance météo** simplifiée (champ `weather_trend` dans le modèle vue). |
| `altimeter.h` / `.c` | **Altitude** estimée (typiquement à partir de la pression). |
| `floor_counter.h` / `.c` | Comptage d’**étages** montés/descendus. |
| `fall_detector.h` / `.c` | Détection de **chute**. |

Le **`sampling_thread`** inclut explicitement ces en-têtes et enchaîne les mises à jour dans **`SamplingData`**.

---

## `model/rtc/`

| Fichier | Rôle |
|---------|------|
| `rtc.h` | Type **`WatchRTC`**, API **`watch_rtc_get`**, **`watch_rtc_print`**, init si présente. |
| `rtc.c` | Implémentation sur le driver **`zephyr/drivers/rtc.h`** : lecture struct **`rtc_time`**. |

Utilisé par **`data_init_thread`** (contexte partagé), **`rtc_thread`** (logs), **`ui_thread`** (heure affichée), et par **`ble_service`** / **`ble_thread`** pour l’horodatage exposé en BLE.

---

## Interface vers le reste de l’app

- **Aucun** fichier sous `model/` n’inclut `lvgl.h`.
- Les sorties « visibles » passent par des structures intermédiaires :
  - **`DataInitContext`** (handles + RTC) ;
  - **`SamplingData`** + champs dérivés du **`processing`** ;
  - puis **`view_model_data_t`** (copie dans **`ui_thread`**) pour l’affichage.

Pour le cheminement complet, voir [acheminement-flux.md](acheminement-flux.md).
