#include <Arduino.h>
#include "moteurs.h"
#include "config.h"

// PWM à 20 kHz : au-dessus de l'audible, les moteurs ne sifflent pas
#define FREQ_PWM        20000
#define RESOLUTION_PWM  8
#define CANAL_GAUCHE    0
#define CANAL_DROIT     1

// L'API LEDC a changé entre les versions 2 et 3 du cœur Arduino ESP32
static void pwmBrancher(int broche, int canal) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(broche, FREQ_PWM, RESOLUTION_PWM);
#else
  ledcSetup(canal, FREQ_PWM, RESOLUTION_PWM);
  ledcAttachPin(broche, canal);
#endif
}

static void pwmEcrire(int broche, int canal, int valeur) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(broche, valeur);
#else
  ledcWrite(canal, valeur);
#endif
}

static void piloterCanal(int in1, int in2, int brochePwm, int canal, int vitesse) {
  vitesse = constrain(vitesse, -255, 255);
  if (vitesse > 0) {
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
  } else if (vitesse < 0) {
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
  } else {
    // IN1 = IN2 = LOW : roue libre
    digitalWrite(in1, LOW);
    digitalWrite(in2, LOW);
  }
  pwmEcrire(brochePwm, canal, abs(vitesse));
}

void moteursInit() {
  int sorties[] = {BROCHE_AIN1, BROCHE_AIN2, BROCHE_BIN1, BROCHE_BIN2, BROCHE_STBY};
  for (int b : sorties) {
    pinMode(b, OUTPUT);
    digitalWrite(b, LOW);
  }
  pwmBrancher(BROCHE_PWMA, CANAL_GAUCHE);
  pwmBrancher(BROCHE_PWMB, CANAL_DROIT);
  moteursStop();
  digitalWrite(BROCHE_STBY, HIGH);
}

void moteursCommande(int gauche, int droite) {
  if (INVERSER_GAUCHE) gauche = -gauche;
  if (INVERSER_DROITE) droite = -droite;
  piloterCanal(BROCHE_AIN1, BROCHE_AIN2, BROCHE_PWMA, CANAL_GAUCHE, gauche);
  piloterCanal(BROCHE_BIN1, BROCHE_BIN2, BROCHE_PWMB, CANAL_DROIT, droite);
}

void moteursStop() {
  moteursCommande(0, 0);
}
