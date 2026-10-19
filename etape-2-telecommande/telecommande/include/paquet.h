// Format des messages ESP-NOW, identique dans la voiture et la télécommande.
#pragma once
#include <stdint.h>

// Télécommande -> voiture, environ 30 fois par seconde
struct __attribute__((packed)) PaquetPilotage {
  uint32_t id;        // ID_APPAIRAGE
  int8_t   x;         // -100 (gauche) à 100 (droite)
  int8_t   y;         // -100 (arrière) à 100 (avant)
  uint8_t  boutons;   // bit 0 : bouton du joystick enfoncé
};

// Voiture -> télécommande, pour allumer la LED « liaison OK »
struct __attribute__((packed)) PaquetRetour {
  uint32_t id;
};
