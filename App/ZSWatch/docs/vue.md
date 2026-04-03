# Couche Vue — `src/vue/ui/`

La **Vue** regroupe tout ce qui concerne **LVGL** : écrans, styles, type de données « affichable », et l’orchestrateur **`ME_VUE`** (« ma vue »). Elle ne parle pas directement aux capteurs ; elle reçoit un **`view_model_data_t`** déjà rempli par le **`ui_thread`** (voir [acheminement-flux.md](acheminement-flux.md)).

## Rôle global

| Fichier | Rôle |
|---------|------|
| `ui.h` / `ui.c` | **Façade** : `ui_init_with_config`, `ui_process`, `ui_update` → délèguent à `ME_VUE_*` et `lv_timer_handler` |
| `components/me_vue.h` | Types **`view_model_data_t`**, **`view_screen_id_t`**, **`view_transition_t`**, **`view_config_t`**, API **`ME_VUE_*`** |
| `components/me_vue.c` | **Cœur navigation** : init splash → home, transitions, `ME_VUE_update`, dispatch `default_view_event_cb` |
| `components/view_events.h` | **IDs d’événements** (`VIEW_EVENT_NAV_*`, swipe, boutons logiques) et **`view_event_data_t`** |
| `components/screens.h` | API **`view_screens_*`** : factory d’écrans, lookup par id, mise à jour depuis le modèle |
| `components/screens.c` | Implémentation : création / destruction des écrans, **`view_screens_update`**, routage vers les bons écrans |
| `components/screens_internal.h` | Détails internes partagés entre `screens.c` et les fichiers sous `screens/` |
| `components/styles.h` / `styles.c` | Couleurs / styles LVGL réutilisables |
| `screens/screen_splash.c` | Écran de démarrage |
| `screens/screen_home.c` | Accueil |
| `screens/screen_weather.c` | Données type météo / tendance |
| `screens/screen_compass.c` | Compas / cap |
| `screens/screen_activity.c` | Activité, pas, calories, etc. |
| `screens/screen_ble.c` | État BLE / RSSI |
| `filelist.txt` | Liste des sources (référence type SquareLine) |
| `CMakeLists.txt` | Définition optionnelle `add_library(ui)` (le build principal utilise un glob à la racine) |
| `README.md` | Note sur l’arborescence type `ui/` |
| `project.info` | Métadonnées projet SquareLine / Studio |

---

## Détail des composants principaux

### `me_vue.h` / `me_vue.c`

- **`view_model_data_t`** : structure **plate** (entiers, flottants, booléens) représentant **ce qui doit apparaître** à l’écran (heure, batterie, pas, température, BLE, etc.). C’est le **contrat** entre le thread UI et les écrans.
- **`ME_VUE_init`** : initialise LVGL côté logique vue, appelle `view_screens_init`, affiche le **splash** puis programme le passage **Home** après un délai.
- **`ME_VUE_process`** : applique les **écrans en attente** (transitions différées).
- **`ME_VUE_update`** : propage le modèle vers `view_screens_update`.
- **`ME_VUE_show_screen` / `ME_VUE_show_next_screen`** : navigation programmée ou « écran suivant » en boucle.
- **`default_view_event_cb`** : si aucun callback n’est fourni à l’init, réagit aux **événements de navigation** (home, capteurs, activité, BLE, swipe).

### `screens.c` et `screens_internal.h`

- Centralisent la **création** des `lv_obj_t` racine par identifiant (`VIEW_SCREEN_*`).
- **`view_screens_update`** dispatche les champs de **`view_model_data_t`** vers les bons labels / objets de chaque écran actif.

### Fichiers `screens/*.c`

- Chaque fichier contient typiquement une fonction **`screen_<nom>_create`** appelée depuis `screens.c` / `screens_internal.h`.
- Utilisent **`screens_internal.h`** et **`styles.h`** pour rester cohérents visuellement.
- Les includes relatifs `../components/` sont inchangés par la structure de dossiers.

### `ui.c` — façade

- **`ui_process`** : appelle **`lv_timer_handler()`** (LVGL) puis **`ME_VUE_process()`** — à exécuter **périodiquement** depuis le **`ui_thread`** uniquement.
- **`ui_update`** : simple relais vers **`ME_VUE_update`**.

---

## Ce que la Vue ne fait pas

- Elle ne lit **pas** les capteurs Zephyr directement.
- Elle ne gère **pas** le Bluetooth (sauf affichage d’états fournis par le contrôleur).
- Elle ne crée **pas** de threads : tout LVGL est piloté par **`ui_thread`**.

Pour le flux complet vers l’écran, voir [acheminement-flux.md](acheminement-flux.md). Pour le remplissage de `view_model_data_t`, voir [controller.md](controller.md) (`ui_thread.c`).
