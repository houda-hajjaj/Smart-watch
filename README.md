# ZSWatch BLE Sensor Project

This project implements a Bluetooth Low Energy (BLE) smartwatch prototype using an **nRF5340 DK** and an **ST IKS01A3 shield**. It runs on **Zephyr RTOS** (nRF Connect SDK v3.1.0) and demonstrates periodic sensor data transmission over BLE using standard GATT services.

##  Features
- **Hardware**: nRF5340 DK (application core) + X-NUCLEO-IKS01A3 shield (LSM6DSO, LIS2MDL, HTS221, LPS22HH).
- **Sensor drivers**: Periodic reading of temperature, humidity, pressure, acceleration, and magnetic field.
- **BLE implementation**:
  - Environmental Sensing Service (ESS, UUID 0x181A) with standard characteristics:
    - Temperature (0x2A6E)
    - Humidity (0x2A6F)
    - Pressure (0x2A6D)
    - 3‑axis accelerometer (0x2AA1)
    - 3‑axis magnetometer (0x2AA0)
  - Notifications enabled for all characteristics.
  - Device name: "ZSWatch".
- **Robust communication**: Increased BLE buffers, proper CCC handling, and dual‑core setup (network core runs `hci_ipc`).

##  Status
- Sensors initialise correctly (I²C).
- BLE advertising and connection functional.
- Notifications sent every 2 seconds with scaled sensor values.
- Tested with nRF Connect for Mobile – data appears in real‑time after subscribing.

# Journal de bord — Projet ZSWatch

Voici un journal succinct des trois premières semaines de développement.

## Semaine 1 — Initialiser le projet et la configuration

- Création de l'arborescence du projet et initialisation du dépôt.
- Configuration de l'environnement Zephyr / nRF Connect SDK, ajout des fichiers `CMakeLists.txt` et `prj.conf` pour la cible `nrf5340dk`.
- Mise en place d'une première compilation avec `west build` pour vérifier la toolchain et résoudre les dépendances.
- Résultat : build initial réussi, configuration de base validée.

## Semaine 2 — Terminer les capteurs et l'USART

- Intégration des pilotes de capteurs (altimeter, compass, mag_sensor, env_sensor, motion_sensor, step_counter) et configuration I²C/SPI.
- Mise en place des routines de lecture périodique et des conversions d'unités pour température, pression, humidité, accélération et magnétisme.
- Implémentation d'un canal de debug via USART pour journaliser les lectures et faciliter le débogage matériel.
- Tests matériels : lectures stables, calibrations initiales appliquées.

## Semaine 3 — BLE

- Implémentation des services BLE principaux (Environmental Sensing Service et caractéristiques associées).
- Activation des notifications pour l'envoi périodique des mesures vers un client connecté.
- Ajustements des buffers BLE et gestion des CCC pour fiabiliser les notifications.
- Test avec nRF Connect (mobile) : découverte, connexion et réception des notifications OK.

## Remarques et prochaines étapes

- Intégrer l'affichage (LVGL) et une interface utilisateur simple.
- Ajouter une gestion de l'heure (RTC) et des enregistrements historiques.
- Envisager un service personnalisé combinant plusieurs capteurs pour simplifier le client BLE.

Fichier mis à jour : [README.md](README.md)
