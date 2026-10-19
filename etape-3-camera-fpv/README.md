# Étape 3 : la caméra embarquée (FPV)

Une ESP32-CAM posée à l'avant de la voiture filme la route. Le téléphone affiche la vidéo en direct au-dessus du joystick : on peut piloter sans voir la voiture. La LED flash de la caméra sert de phare, avec un bouton sur la page.

Deux programmes dans ce dossier :

- [voiture](voiture) : le code de l'étape 2 (téléphone + télécommande), avec une page web qui affiche la vidéo et le bouton phare.
- [camera](camera) : l'ESP32-CAM, qui se connecte au Wi-Fi de la voiture avec une adresse fixe et diffuse la vidéo.

Le code compile avec PlatformIO mais n'a pas encore été testé sur de vraies cartes.

## Câblage

| De | Vers |
| --- | --- |
| OUT+ du MP1584 (5 V, le même que pour l'ESP32 de la voiture) | Broche **5V** de l'ESP32-CAM |
| GND | GND de l'ESP32-CAM |
| Condensateur 470 à 1000 µF | Entre 5V et GND, au plus près de l'ESP32-CAM |

L'ESP32-CAM n'a besoin d'aucun fil vers la voiture : elles communiquent par Wi-Fi. Le MP1584 fournit jusqu'à 3 A, largement assez pour les deux cartes.

## Mise en route

1. Dans `camera`, copie `include/config.example.h` en `include/config.h`. Le nom du réseau, le mot de passe et `IP_CAMERA` doivent être les mêmes que dans le `config.h` de la voiture.
2. Pose l'ESP32-CAM sur la carte ESP32-CAM-MB, branche-la en USB et lance `pio run -t upload` dans `camera`. Si le téléversement échoue, maintiens le bouton IO0 de la carte MB en appuyant sur RST.
3. Téléverse `voiture` sur l'ESP32 de la voiture (son `config.h` contient maintenant `IP_CAMERA`).
4. Allume la voiture, puis la caméra. Dans le moniteur série de la caméra : `[WIFI] Connectée, flux sur http://192.168.4.200:81/flux`.
5. Sur le téléphone, ouvre `http://192.168.4.1` en mode portrait : la vidéo s'affiche en haut, le joystick en bas.

## Points à vérifier ensemble

- **Retard de l'image** : c'est le point clé pour piloter en FPV. Si l'image a du retard ou saccade, passe `TAILLE_IMAGE` en `FRAMESIZE_QVGA` ou monte `QUALITE_JPEG` vers 20. Le moniteur série de la caméra affiche le nombre d'images par seconde.
- **Image à l'envers** : selon le montage, mets `RETOURNER_IMAGE` à 1.
- **Un seul spectateur** : la caméra ne diffuse qu'à un téléphone à la fois. C'est voulu, pour garder un maximum d'images par seconde.
- **Portée** : la vidéo passe par le Wi-Fi de la voiture et décroche avant la télécommande ESP-NOW. Pour aller plus loin, il existe des ESP32-CAM avec connecteur d'antenne externe (il faut alors déplacer une petite résistance sur la carte).
- **Chaleur** : la LED flash chauffe vite à pleine puissance. `INTENSITE_PHARE` est limité à 80 sur 255.
