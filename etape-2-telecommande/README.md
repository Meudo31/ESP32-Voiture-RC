# Étape 2 : la télécommande maison (ESP-NOW)

Une vraie télécommande avec un joystick, sans téléphone. Elle parle à la voiture en **ESP-NOW**, le protocole radio direct d'Espressif : pas de connexion Wi-Fi à établir, une latence de quelques millisecondes et une portée meilleure que la page web. Le pilotage au téléphone de l'étape 1 marche toujours, le dernier ordre reçu gagne.

Deux programmes dans ce dossier :

- [voiture](voiture) : le code de l'étape 1, plus la réception ESP-NOW. À téléverser à la place de l'étape 1.
- [telecommande](telecommande) : le second ESP32 avec le joystick.

Le code compile avec PlatformIO mais n'a pas encore été testé sur de vraies cartes.

## Câblage de la télécommande

| De | Vers |
| --- | --- |
| KY-023 broche +5V | **3V3** de l'ESP32 (pas 5 V : l'ADC de l'ESP32 ne supporte que 3,3 V) |
| KY-023 GND | GND |
| KY-023 VRx | GPIO34 |
| KY-023 VRy | GPIO35 |
| KY-023 SW (bouton) | GPIO32 |
| Shield 18650, sortie 3,3 V | 3V3 de l'ESP32 |
| Shield 18650, GND | GND |

La voiture se câble exactement comme à l'étape 1.

## Mise en route

1. Dans `voiture` et dans `telecommande`, copie `include/config.example.h` en `include/config.h`. `CANAL_WIFI` et `ID_APPAIRAGE` doivent être **identiques** des deux côtés. Change l'identifiant pour un nombre à toi, surtout si un copain construit la même voiture.
2. Téléverse `voiture` sur la voiture et `telecommande` sur le second ESP32 (`pio run -t upload` dans chaque dossier).
3. Allume la télécommande **sans toucher au joystick** : elle mesure sa position de repos au démarrage.
4. Allume la voiture. La LED bleue de la télécommande passe de clignotante à fixe : la liaison est établie.
5. Pousse le joystick vers l'avant. Si la voiture recule ou tourne à l'envers, utilise `INVERSER_X` / `INVERSER_Y` dans le `config.h` de la télécommande. Si avant/arrière commande les virages, le joystick est monté tourné de 90° : échange les fils VRx et VRy.

## Comment ça marche

La télécommande diffuse ses paquets à tout le monde (adresse FF:FF:FF:FF:FF:FF) : pas besoin de recopier d'adresse MAC. La voiture ignore tout paquet qui ne porte pas son `ID_APPAIRAGE`, et répond 5 fois par seconde pour allumer la LED de la télécommande. Le bouton du joystick est transmis à la voiture, qui l'affiche pour l'instant dans le moniteur série : il servira aux bonus (phares, klaxon).

## Points à vérifier ensemble

- **Portée** : à mesurer dehors, en ligne droite, jusqu'à ce que la LED clignote. On peut comparer avec la portée au téléphone pour la vidéo.
- **Centre du joystick** : les KY-023 ne sont pas parfaits. Si la voiture avance seule joystick lâché, augmente `ZONE_MORTE` dans la télécommande.
- **Autonomie de la télécommande** : avec un 18650 de 2500 mAh, compter une bonne dizaine d'heures.
