# Documentation ZSWatch — architecture MVC

Ce dossier décrit l’organisation du firmware **ZSWatch** (application Zephyr + LVGL) selon une séparation inspirée du **modèle MVC**, adaptée à un système embarqué multi-thread.

## Documents

| Fichier | Contenu |
|--------|---------|
| [acheminement-flux.md](acheminement-flux.md) | Flux de données et d’événements de bout en bout (capteurs → écran, boutons → navigation) |
| [vue.md](vue.md) | Couche **Vue** (`src/vue/ui/`) : LVGL, écrans, `view_model_data_t`, façade `ui.*` |
| [controller.md](controller.md) | Couche **Contrôleur** (`src/controller/`) : entrées, threads, BLE |
| [model.md](model.md) | Couche **Modèle** (`src/model/`) : capteurs, traitements, RTC |
| [projet-global.md](projet-global.md) | Vue d’ensemble du projet : `main`, build CMake, priorités de threads, fichiers racine |

## Arborescence logique

```
App/ZSWatch/src/
├── main.c                 # Point d’entrée, démarrage des modules
├── model/                 # Données et matériel « métier »
│   ├── sensors/
│   ├── processing/
│   └── rtc/
├── vue/ui/                # Affichage LVGL (écrans, styles, orchestrateur me_vue)
└── controller/            # Entrées utilisateur, tâches, connectivité
    ├── input/
    ├── thread/
    └── ble/
```

Pour le détail du rôle de chaque fichier, ouvrir les documents ci-dessus.
