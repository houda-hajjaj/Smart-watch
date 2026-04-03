# Couche Contrôleur — `src/controller/`

Le **contrôleur** regroupe les **entrées utilisateur** (boutons), les **threads** qui orchestrent l’échantillonnage, le BLE, le RTC, la puissance, et l’UI, ainsi que le **service Bluetooth**. Il fait le lien entre le **modèle** (capteurs, algorithmes, RTC) et la **vue** (LVGL), en s’exécutant en parallèle sur le RTOS Zephyr.

## Arborescence

```
controller/
├── input/          # GPIO / boutons
├── thread/         # Threads applicatifs
└── ble/            # Stack BLE + GATT
```

---

## `controller/input/`

| Fichier | Rôle |
|---------|------|
| `buttons.h` | API publique : init, masque d’événements (`BUTTON_EVENT_*`), `buttons_consume_events()` |
| `buttons.c` | Configuration **GPIO** (`sw0`, `sw1` via devicetree), **debounce**, callbacks IRQ. Met à jour **`power_thread`** (réveil / activité). En mode normal, SW0 peut positionner **`BUTTON_EVENT_NEXT_SCENE`** pour la navigation ; SW1 gère le réveil selon le mode éco. |

Les boutons **ne** appellent **pas** LVGL directement : ils posent des bits lus par le **`ui_thread`**.

---

## `controller/thread/`

| Fichier | Rôle |
|---------|------|
| `data_init_thread.h` / `.c` | **Premier thread métier** : initialise **`MotionSensor`**, **`EnvSensor`**, **`MagSensor`**, **`WatchRTC`**, remplit **`DataInitContext`**, signale la fin avec `data_init_thread_wait_ready`. Tous les threads qui utilisent le matériel attendent cette étape. |
| `sampling_thread.h` / `.c` | **Boucle d’échantillonnage** : timer → lecture capteurs → appels **`processing/*`** (activité, pas, distance, calories, compas, météo, altimètre, étages, chute, etc.) → agrégation dans **`SamplingData`** (mutex). Met à jour **`ble_service_update`**, notifie **`power_thread`**. |
| `ble_thread.h` / `.c` | Thread auxiliaire BLE : **LED** de statut, synchronisation avec connexion / mises à jour ; s’appuie sur **`ble_service`**, **`data_init`**, **`sampling_thread`**, **`compass`**, **`rtc`**. |
| `rtc_thread.h` / `.c` | Boucle périodique : lecture **`watch_rtc_get`** et log (debug série). L’**affichage** de l’heure sur l’écran est fait dans **`ui_thread`**, pas ici. |
| `power_thread.h` / `.c` | **Gestionnaire d’inactivité** : après un délai sans activité, passe en **mode éco** (`power_thread_is_eco_mode`). Les boutons et le sampling peuvent appeler **`power_thread_notify_activity`** pour réinitialiser le timer. |
| `ui_thread.h` / `.c` | **Thread principal LVGL** : attend **`data_init_thread_wait_done`**, appelle **`ui_init_with_config`** → **`ME_VUE`**, boucle **`ui_process`**, consomme **`buttons`**, rafraîchit **`view_model_data_t`** et appelle **`ui_update`**, gère l’écran de veille si éco. **Priorité** et **période** définies en tête de fichier. |

### Dépendances typiques des threads

- **`sampling_thread`** : `#include` massifs vers **`processing/`** et **`ble_thread.h`**, **`data_init_thread.h`**, **`power_thread.h`**.
- **`ui_thread`** : agrège **`rtc`**, **`sampling_thread`**, **`ble_service`**, **`buttons`**, **`power_thread`**, **`ui/ui.h`**.

---

## `controller/ble/`

| Fichier | Rôle |
|---------|------|
| `ble_service.h` | Contrat **GATT** : structure **`ble_sensor_data`** (température, humidité, pression, accéléro, magnéto, pas, texte RTC, cap, direction), fonctions **`ble_service_init`**, **`ble_service_update`**, **`ble_service_is_connected`**, **`ble_service_get_rssi_dbm`**, etc. |
| `ble_service.c` | Implémentation : registre les services, notifications, état de connexion ; peut inclure **`rtc`** et **`data_init_thread`** pour l’horodatage exposé en BLE. |

Le **thread BLE** (`ble_thread.c`) orchestre l’activité autour de ce service ; le **sampling** alimente les données ; l’**UI** lit seulement l’état pour l’affichage.

---

## Position du « contrôleur » dans MVC

- **Coordination** : threads + timers + mutex.
- **Adaptation** : transforme **`SamplingData`** + RTC + BLE en **`view_model_data_t`** dans **`ui_thread_refresh_model`**.
- **Entrée** : boutons → événements → **`ME_VUE_show_next_screen`** (sans passer par des callbacks LVGL dans l’ISR).

Pour la chronologie exacte au démarrage, voir [projet-global.md](projet-global.md) et [acheminement-flux.md](acheminement-flux.md).
