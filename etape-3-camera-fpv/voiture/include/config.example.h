// Copier ce fichier en config.h et remplir les valeurs.
// config.h ne doit jamais être publié (il contient le mot de passe Wi-Fi).
#pragma once

// La voiture crée son propre réseau Wi-Fi : connecte ton téléphone dessus
// puis ouvre http://192.168.4.1 dans le navigateur.
#define WIFI_NOM_VOITURE        "VoitureRC"
#define WIFI_MOTDEPASSE_VOITURE "change-moi-123"   // 8 caractères minimum
#define CANAL_WIFI              1                  // 1 à 13

// Vitesse maximale envoyée aux moteurs (0 à 255).
// Avec 2 accus 18650 (7,4 V) et des moteurs TT prévus pour 6 V, rester vers 200.
#define VITESSE_MAX          200
// Part du manche gauche/droite dans le virage (en %). Baisser si la voiture tourne trop sec.
#define SENSIBILITE_VIRAGE   70
// En dessous de ce pourcentage, le manche est considéré au centre.
#define ZONE_MORTE           8

// Mettre à 1 si un moteur tourne à l'envers (plus simple que de ressouder).
#define INVERSER_GAUCHE      0
#define INVERSER_DROITE      0

// Sécurité : la voiture s'arrête si elle ne reçoit plus d'ordre pendant ce délai.
#define DELAI_SECURITE_MS    300

// Télécommande ESP-NOW (étape 2) : même nombre dans la voiture et la télécommande.
// La voiture ignore les paquets portant un autre identifiant (télécommande d'un copain...).
#define ID_APPAIRAGE         48151623

// Adresse fixe de l'ESP32-CAM sur le réseau de la voiture (même valeur dans le config.h de la caméra).
// Prise hors de la plage DHCP pour ne pas tomber sur l'adresse du téléphone.
#define IP_CAMERA            "192.168.4.200"
