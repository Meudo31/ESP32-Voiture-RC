// Voiture télécommandée - étape 1 : pilotage depuis le téléphone
//
// Fonctionnement :
//  1. L'ESP32 crée un réseau Wi-Fi (point d'accès) nommé WIFI_NOM_VOITURE.
//  2. Le téléphone s'y connecte et ouvre http://192.168.4.1 : une page avec un joystick.
//  3. La page envoie la position du joystick par WebSocket, 20 fois par seconde.
//  4. L'ESP32 la convertit en vitesse pour chaque moteur (pilotage façon char d'assaut).
//  5. Sécurité : sans ordre reçu pendant DELAI_SECURITE_MS, la voiture s'arrête.

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "config.h"
#include "moteurs.h"
#include "page.h"

WebServer serveurWeb(80);
WebSocketsServer serveurWs(81);

uint32_t dernierOrdreMs = 0;
bool arretSecurite = true;

// Joystick (x, y de -100 à 100) vers vitesses gauche/droite
void piloter(int x, int y) {
  x = constrain(x, -100, 100);
  y = constrain(y, -100, 100);
  if (abs(x) < ZONE_MORTE) x = 0;
  if (abs(y) < ZONE_MORTE) y = 0;
  x = x * SENSIBILITE_VIRAGE / 100;

  int gauche = constrain(y + x, -100, 100);
  int droite = constrain(y - x, -100, 100);
  moteursCommande(gauche * VITESSE_MAX / 100, droite * VITESSE_MAX / 100);

  dernierOrdreMs = millis();
  if (arretSecurite) {
    arretSecurite = false;
    Serial.println("[PILOTE] Ordres reçus, voiture active");
  }
}

void evenementWs(uint8_t client, WStype_t type, uint8_t *donnees, size_t longueur) {
  switch (type) {
    case WStype_CONNECTED:
      Serial.printf("[WS] Téléphone %u connecté\n", client);
      serveurWs.sendTXT(client, "Voiture prête");
      break;
    case WStype_DISCONNECTED:
      Serial.printf("[WS] Téléphone %u déconnecté\n", client);
      moteursStop();
      break;
    case WStype_TEXT: {
      char texte[16];
      size_t n = min(longueur, sizeof(texte) - 1);
      memcpy(texte, donnees, n);
      texte[n] = '\0';
      int x, y;
      if (sscanf(texte, "%d,%d", &x, &y) == 2) piloter(x, y);
      break;
    }
    default:
      break;
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[DEMARRAGE] Voiture RC, étape 1");

  moteursInit();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NOM_VOITURE, WIFI_MOTDEPASSE_VOITURE, CANAL_WIFI);
  WiFi.setSleep(false);   // moins de latence
  Serial.printf("[WIFI] Réseau \"%s\" créé, page sur http://%s\n",
                WIFI_NOM_VOITURE, WiFi.softAPIP().toString().c_str());

  serveurWeb.on("/", []() { serveurWeb.send_P(200, "text/html", PAGE_PILOTAGE); });
  serveurWeb.begin();

  serveurWs.begin();
  serveurWs.onEvent(evenementWs);
}

void loop() {
  serveurWeb.handleClient();
  serveurWs.loop();

  if (!arretSecurite && millis() - dernierOrdreMs > DELAI_SECURITE_MS) {
    moteursStop();
    arretSecurite = true;
    Serial.println("[SECURITE] Plus d'ordre reçu, arrêt des moteurs");
  }
}
