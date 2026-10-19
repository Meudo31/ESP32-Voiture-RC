// Pilotage des deux moteurs via un driver TB6612FNG.
#pragma once

// Broches ESP32 DevKit V1 vers le TB6612FNG
#define BROCHE_AIN1  27   // moteur gauche, sens
#define BROCHE_AIN2  26
#define BROCHE_PWMA  25   // moteur gauche, vitesse
#define BROCHE_BIN1  33   // moteur droit, sens
#define BROCHE_BIN2  32
#define BROCHE_PWMB  14   // moteur droit, vitesse
#define BROCHE_STBY  13   // HIGH = driver actif

void moteursInit();
// gauche et droite de -255 (pleine marche arrière) à 255 (pleine marche avant)
void moteursCommande(int gauche, int droite);
void moteursStop();
