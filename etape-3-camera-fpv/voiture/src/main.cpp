// Voiture télécommandée - étape 3 : la voiture, avec la vidéo de la caméra sur la page
//
// Le code est celui de l'étape 2 ; seule la page web (page.h) change : elle affiche le flux
// de l'ESP32-CAM (adresse fixe IP_CAMERA sur le réseau de la voiture) au-dessus du joystick,
// avec un bouton pour allumer la LED de la caméra comme phare.
//
// Fonctionnement :
//  1. Réseau Wi-Fi WIFI_NOM_VOITURE et page de pilotage sur http://192.168.4.1.
//  2. En plus, la voiture écoute en ESP-NOW les paquets de la télécommande (même canal Wi-Fi).
//     Pas besoin de connaître l'adresse MAC : la télécommande diffuse ses paquets et la voiture
//     ne garde que ceux qui portent son ID_APPAIRAGE.
//  3. La voiture répond 5 fois par seconde pour que la télécommande allume sa LED « liaison OK ».
//  4. Le dernier ordre reçu gagne, quelle que soit sa source.
//  5. Sécurité : sans ordre reçu pendant DELAI_SECURITE_MS, la voiture s'arrête.

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <esp_now.h>
#include "config.h"
#include "moteurs.h"
#include "page.h"
#include "paquet.h"

WebServer serveurWeb(80);
WebSocketsServer serveurWs(81);

const uint8_t ADRESSE_DIFFUSION[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint32_t dernierOrdreMs = 0;
bool arretSecurite = true;

// Rempli par le callback ESP-NOW (tâche Wi-Fi), lu dans loop()
volatile bool paquetEnAttente = false;
volatile int8_t telecommandeX = 0, telecommandeY = 0;
volatile uint8_t telecommandeBoutons = 0;
uint32_t derniereTelecommandeMs = 0;
uint32_t dernierRetourMs = 0;

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

// ---------- Téléphone (WebSocket) ----------
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

// ---------- Télécommande (ESP-NOW) ----------
// La signature du callback a changé avec ESP-IDF 5 (cœur Arduino 3)
#if ESP_IDF_VERSION_MAJOR >= 5
void paquetRecu(const esp_now_recv_info_t *info, const uint8_t *donnees, int longueur) {
#else
void paquetRecu(const uint8_t *mac, const uint8_t *donnees, int longueur) {
#endif
  if (longueur != sizeof(PaquetPilotage)) return;
  PaquetPilotage p;
  memcpy(&p, donnees, sizeof(p));
  if (p.id != ID_APPAIRAGE) return;
  telecommandeX = p.x;
  telecommandeY = p.y;
  telecommandeBoutons = p.boutons;
  paquetEnAttente = true;
}

bool initEspNow() {
  if (esp_now_init() != ESP_OK) return false;
  esp_now_register_recv_cb(paquetRecu);

  esp_now_peer_info_t pair = {};
  memcpy(pair.peer_addr, ADRESSE_DIFFUSION, 6);
  pair.channel = CANAL_WIFI;
  pair.ifidx = WIFI_IF_AP;   // la voiture est en point d'accès
  pair.encrypt = false;
  return esp_now_add_peer(&pair) == ESP_OK;
}

void traiterTelecommande() {
  if (!paquetEnAttente) return;
  paquetEnAttente = false;

  if (derniereTelecommandeMs == 0 || millis() - derniereTelecommandeMs > 1000) {
    Serial.println("[ESPNOW] Télécommande détectée");
  }
  derniereTelecommandeMs = millis();
  piloter(telecommandeX, telecommandeY);

  static uint8_t anciensBoutons = 0;
  if ((telecommandeBoutons & 1) && !(anciensBoutons & 1)) {
    Serial.println("[ESPNOW] Bouton du joystick appuyé (libre pour les bonus : phares, klaxon...)");
  }
  anciensBoutons = telecommandeBoutons;
}

void envoyerRetour() {
  if (millis() - dernierRetourMs < 200) return;
  dernierRetourMs = millis();
  PaquetRetour r = {ID_APPAIRAGE};
  esp_now_send(ADRESSE_DIFFUSION, (const uint8_t *)&r, sizeof(r));
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[DEMARRAGE] Voiture RC, étape 3");

  moteursInit();

  WiFi.mode(WIFI_AP);
  WiFi.softAP(WIFI_NOM_VOITURE, WIFI_MOTDEPASSE_VOITURE, CANAL_WIFI);
  WiFi.setSleep(false);   // moins de latence
  Serial.printf("[WIFI] Réseau \"%s\" créé sur le canal %d, page sur http://%s\n",
                WIFI_NOM_VOITURE, CANAL_WIFI, WiFi.softAPIP().toString().c_str());

  if (initEspNow()) {
    Serial.println("[ESPNOW] Prêt, en attente de la télécommande");
  } else {
    Serial.println("[ESPNOW] ERREUR d'initialisation, seul le téléphone fonctionnera");
  }

  serveurWeb.on("/", []() { serveurWeb.send_P(200, "text/html", PAGE_PILOTAGE); });
  serveurWeb.begin();

  serveurWs.begin();
  serveurWs.onEvent(evenementWs);
}

void loop() {
  serveurWeb.handleClient();
  serveurWs.loop();
  traiterTelecommande();

  // Répondre seulement si une télécommande a parlé récemment
  if (derniereTelecommandeMs && millis() - derniereTelecommandeMs < 2000) envoyerRetour();

  if (!arretSecurite && millis() - dernierOrdreMs > DELAI_SECURITE_MS) {
    moteursStop();
    arretSecurite = true;
    Serial.println("[SECURITE] Plus d'ordre reçu, arrêt des moteurs");
  }
}
