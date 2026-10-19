// Voiture télécommandée - étape 2 : la télécommande maison (ESP32 DevKit V1 + joystick KY-023)
//
// Fonctionnement :
//  1. Au démarrage, on mesure la position de repos du joystick (ne pas le toucher).
//  2. Toutes les 30 ms, on lit le joystick et on diffuse la position en ESP-NOW
//     sur le canal Wi-Fi de la voiture, avec l'identifiant ID_APPAIRAGE.
//  3. La LED de la carte reste allumée tant que la voiture répond, clignote sinon.
//
// Câblage du KY-023 : +5V -> 3V3 (surtout pas 5 V : l'ADC de l'ESP32 monte à 3,3 V max),
// GND -> GND, VRx -> GPIO34, VRy -> GPIO35, SW -> GPIO32.

#include <Arduino.h>
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "config.h"
#include "paquet.h"

// ADC1 uniquement : l'ADC2 ne fonctionne pas quand le Wi-Fi est actif
#define BROCHE_X       34
#define BROCHE_Y       35
#define BROCHE_BOUTON  32
#define BROCHE_LED     2     // LED bleue de la carte

#define PERIODE_ENVOI_MS  30

const uint8_t ADRESSE_DIFFUSION[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

int centreX = 2048, centreY = 2048;
volatile uint32_t dernierRetourMs = 0;
uint32_t dernierEnvoiMs = 0;

int lireMoyenne(int broche, int n) {
  long somme = 0;
  for (int i = 0; i < n; i++) somme += analogRead(broche);
  return somme / n;
}

// Valeur brute de l'ADC (0 à 4095) vers -100..100 autour du centre mesuré au démarrage
int8_t versPourcentage(int brut, int centre) {
  int v;
  if (brut >= centre) v = (long)(brut - centre) * 100 / max(1, 4095 - centre);
  else                v = (long)(brut - centre) * 100 / max(1, centre);
  v = constrain(v, -100, 100);
  if (abs(v) < ZONE_MORTE) v = 0;
  return v;
}

#if ESP_IDF_VERSION_MAJOR >= 5
void paquetRecu(const esp_now_recv_info_t *info, const uint8_t *donnees, int longueur) {
#else
void paquetRecu(const uint8_t *mac, const uint8_t *donnees, int longueur) {
#endif
  if (longueur != sizeof(PaquetRetour)) return;
  PaquetRetour r;
  memcpy(&r, donnees, sizeof(r));
  if (r.id == ID_APPAIRAGE) dernierRetourMs = millis();
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[DEMARRAGE] Télécommande voiture RC");

  pinMode(BROCHE_BOUTON, INPUT_PULLUP);
  pinMode(BROCHE_LED, OUTPUT);
  analogSetAttenuation(ADC_11db);   // plage 0 à ~3,1 V

  centreX = lireMoyenne(BROCHE_X, 64);
  centreY = lireMoyenne(BROCHE_Y, 64);
  Serial.printf("[JOYSTICK] Centre mesuré : X=%d Y=%d\n", centreX, centreY);

  // ESP-NOW sans se connecter à un réseau : on se cale juste sur le canal de la voiture
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  esp_wifi_set_channel(CANAL_WIFI, WIFI_SECOND_CHAN_NONE);

  if (esp_now_init() != ESP_OK) {
    Serial.println("[ESPNOW] ERREUR d'initialisation, redémarrage");
    delay(2000);
    ESP.restart();
  }
  esp_now_register_recv_cb(paquetRecu);

  esp_now_peer_info_t pair = {};
  memcpy(pair.peer_addr, ADRESSE_DIFFUSION, 6);
  pair.channel = CANAL_WIFI;
  pair.ifidx = WIFI_IF_STA;
  pair.encrypt = false;
  esp_now_add_peer(&pair);

  Serial.printf("[ESPNOW] Prêt sur le canal %d\n", CANAL_WIFI);
}

void loop() {
  if (millis() - dernierEnvoiMs >= PERIODE_ENVOI_MS) {
    dernierEnvoiMs = millis();

    PaquetPilotage p;
    p.id = ID_APPAIRAGE;
    p.x = versPourcentage(analogRead(BROCHE_X), centreX);
    p.y = versPourcentage(analogRead(BROCHE_Y), centreY);
    if (INVERSER_X) p.x = -p.x;
    if (INVERSER_Y) p.y = -p.y;
    p.boutons = digitalRead(BROCHE_BOUTON) == LOW ? 1 : 0;
    esp_now_send(ADRESSE_DIFFUSION, (const uint8_t *)&p, sizeof(p));

    static uint32_t dernierLogMs = 0;
    if (millis() - dernierLogMs > 500) {
      dernierLogMs = millis();
      Serial.printf("[JOYSTICK] x=%4d y=%4d bouton=%d liaison=%s\n", p.x, p.y, p.boutons,
                    millis() - dernierRetourMs < 500 ? "OK" : "perdue");
    }
  }

  // LED fixe = la voiture répond ; clignotante = pas de liaison
  bool liaison = millis() - dernierRetourMs < 500;
  digitalWrite(BROCHE_LED, liaison ? HIGH : (millis() / 250) % 2);
}
