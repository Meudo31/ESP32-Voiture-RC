// Copier ce fichier en config.h et remplir les valeurs.
// config.h ne doit jamais être publié (il contient le mot de passe Wi-Fi).
#pragma once

// Réseau créé par la voiture : mêmes valeurs que dans le config.h de la voiture
#define WIFI_NOM_VOITURE        "VoitureRC"
#define WIFI_MOTDEPASSE_VOITURE "change-moi-123"
#define IP_CAMERA               "192.168.4.200"

// Taille et qualité de l'image : plus petit = plus fluide et moins de retard.
// FRAMESIZE_QVGA (320x240), FRAMESIZE_VGA (640x480), FRAMESIZE_SVGA (800x600)
#define TAILLE_IMAGE     FRAMESIZE_VGA
#define QUALITE_JPEG     12     // 10 (belle image) à 30 (image légère)

// 1 si la caméra est montée tête en bas sur la voiture
#define RETOURNER_IMAGE  0

// Intensité de la LED flash utilisée comme phare (0 à 255). Elle chauffe : rester modéré.
#define INTENSITE_PHARE  80
