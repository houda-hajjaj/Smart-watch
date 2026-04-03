# Projet ZSWatch — vue d’ensemble

**ZSWatch** est une application **Zephyr RTOS** pour carte embarquée (ex. nRF5340), avec interface **LVGL** sur écran, capteurs **ST IKS01A3** (ou équivalent selon overlay), **BLE** pour télémetrie, et architecture organisée en **Modèle / Vue / Contrôleur** sous `src/`.

## Point d’entrée : `src/main.c`

```c
buttons_init();
data_init_thread_start();
ui_thread_start();
k_sleep(K_MSEC(100));
sampling_thread_start();
ble_thread_start();
rtc_thread_start();
power_thread_start();
// boucle idle
```

- **`main`** ne contient pas la logique métier : il **démarre** les modules dans un ordre compatible avec les dépendances (boutons → init données → UI → puis sampling, BLE, RTC, power).
- **`data_init`** doit avoir fini avant que **`ui_thread`** n’initialise LVGL (attente interne dans `ui_thread_entry`).
- Délai de 100 ms : laisse le temps aux threads de s’amorcer avant le sampling intensif.

---

## Build (CMake)

| Élément | Rôle |
|---------|------|
| `CMakeLists.txt` (racine app) | `find_package(Zephyr)`, `FILE(GLOB_RECURSE app_sources src/*.c)`, `target_sources(app PRIVATE ...)`, **`target_include_directories`** listant `src`, `src/model/...`, `src/controller/...`, `src/vue/...` pour résoudre les `#include` du style `thread/...`, `ui/...`, `processing/...`. |
| `prj.conf` | Options Kconfig Zephyr : LVGL, Bluetooth, capteurs, stack size, etc. |
| `app.overlay` | **Devicetree** : nœuds choisis (`zephyr_display`), alias GPIO (`sw0`, `sw1`, `led0`), etc. |

Le **glob** sur `src/*.c` compile automatiquement tout fichier C sous `model/`, `vue/`, `controller/`, y compris les écrans.

---

## Threads et priorités (indicatif)

Les priorités exactes sont dans chaque `*_thread.c`. En général :

- **`ui_thread`** : priorité **élevée** (affichage fluide), période courte (~10 ms), **LVGL** uniquement ici.
- **`sampling_thread`** : travail capteurs + processing, priorité intermédiaire.
- **`ble_thread`**, **`rtc_thread`** : tâches de fond ou périodiques plus lentes.
- **`power_thread`** : pas forcément un thread bloquant long ; la logique éco peut être consultée depuis d’autres threads.

**Règle d’or :** ne pas appeler **`lv_*`** hors du **`ui_thread`**.

---

## Cartographie MVC dans ce dépôt

| Couche | Chemin | Responsabilité |
|--------|--------|----------------|
| **Modèle** | `src/model/` | Capteurs, algorithmes, RTC |
| **Vue** | `src/vue/ui/` | LVGL, écrans, `view_model_data_t`, `ME_VUE` |
| **Contrôleur** | `src/controller/` | Boutons, threads, BLE |

Le fichier **`main.c`** est le **bootstrap** ; le **contrôleur** principal pour l’UI est **`ui_thread.c`**, qui assemble modèle + BLE en **données affichables**.

---

## Documents associés

- [README.md](README.md) — index de la documentation  
- [acheminement-flux.md](acheminement-flux.md) — flux détaillé  
- [vue.md](vue.md), [controller.md](controller.md), [model.md](model.md) — rôles fichier par fichier  

---

## Fichiers hors `src/` utiles au projet

| Fichier | Usage |
|---------|--------|
| `west.yml` (niveau workspace) | Manifeste NCS / Zephyr (si présent à la racine du workspace) |
| `README` projet | Instructions build `west build` côté équipe |

Pour compiler : `west build` depuis l’application, avec `-b` la carte cible et variables d’environnement **Zephyr / NCS** configurées selon la doc Nordic.
