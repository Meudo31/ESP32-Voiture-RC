// Voiture télécommandée - étape 3 : la caméra embarquée (ESP32-CAM AI-Thinker)
//
// Fonctionnement :
//  1. L'ESP32-CAM se connecte au réseau Wi-Fi de la voiture avec une adresse fixe (IP_CAMERA).
//  2. Elle diffuse la vidéo en MJPEG sur http://IP_CAMERA:81/flux (un seul spectateur à la fois).
//  3. http://IP_CAMERA/phare?etat=1 allume la LED flash (GPIO4) comme phare, etat=0 l'éteint.
//  4. Si le Wi-Fi de la voiture disparaît (voiture éteinte), elle se reconnecte toute seule.
//
// La page de pilotage de la voiture (http://192.168.4.1) affiche ce flux et le bouton phare.

#include <Arduino.h>
#include <WiFi.h>
#include "esp_camera.h"
#include "esp_http_server.h"
#include "config.h"

// ---------- Broches ----------
#define BROCHE_LED    4
#define CANAL_LED     7   // canal LEDC libre (la caméra utilise le canal 0)

// Brochage caméra AI-Thinker
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22

#define FRONTIERE "trame-voiture-rc"

httpd_handle_t serveurCommande = NULL;
httpd_handle_t serveurFlux = NULL;

// ---------- LED / phare ----------
void phareInit() {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcAttach(BROCHE_LED, 5000, 8);
#else
  ledcSetup(CANAL_LED, 5000, 8);
  ledcAttachPin(BROCHE_LED, CANAL_LED);
#endif
}

void phareRegler(int intensite) {
#if ESP_ARDUINO_VERSION_MAJOR >= 3
  ledcWrite(BROCHE_LED, intensite);
#else
  ledcWrite(CANAL_LED, intensite);
#endif
}

// ---------- Caméra ----------
bool initCamera() {
  camera_config_t config = {};
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = TAILLE_IMAGE;
  config.jpeg_quality = QUALITE_JPEG;
  config.fb_count = 2;
  config.fb_location = CAMERA_FB_IN_PSRAM;
  config.grab_mode = CAMERA_GRAB_LATEST;   // toujours l'image la plus récente : moins de retard

  if (esp_camera_init(&config) != ESP_OK) return false;

  sensor_t *capteur = esp_camera_sensor_get();
  capteur->set_vflip(capteur, RETOURNER_IMAGE);
  capteur->set_hmirror(capteur, RETOURNER_IMAGE);
  return true;
}

// ---------- Serveur web ----------
esp_err_t gererFlux(httpd_req_t *req) {
  httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=" FRONTIERE);
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  Serial.println("[FLUX] Spectateur connecté");

  char entete[96];
  uint32_t images = 0, debutMs = millis();
  esp_err_t res = ESP_OK;
  while (res == ESP_OK) {
    camera_fb_t *image = esp_camera_fb_get();
    if (!image) {
      Serial.println("[FLUX] ERREUR de capture");
      res = ESP_FAIL;
      break;
    }
    size_t n = snprintf(entete, sizeof(entete),
                        "\r\n--" FRONTIERE "\r\nContent-Type: image/jpeg\r\nContent-Length: %u\r\n\r\n",
                        (unsigned)image->len);
    res = httpd_resp_send_chunk(req, entete, n);
    if (res == ESP_OK) res = httpd_resp_send_chunk(req, (const char *)image->buf, image->len);
    esp_camera_fb_return(image);

    if (++images % 100 == 0) {
      Serial.printf("[FLUX] %.1f images/s\n", images * 1000.0f / (millis() - debutMs));
    }
  }
  Serial.println("[FLUX] Spectateur parti");
  return res;
}

esp_err_t gererPhare(httpd_req_t *req) {
  char requete[32], valeur[4];
  int intensite = 0;
  if (httpd_req_get_url_query_str(req, requete, sizeof(requete)) == ESP_OK &&
      httpd_query_key_value(requete, "etat", valeur, sizeof(valeur)) == ESP_OK) {
    intensite = atoi(valeur) ? INTENSITE_PHARE : 0;
  }
  phareRegler(intensite);
  Serial.printf("[PHARE] %s\n", intensite ? "allumé" : "éteint");
  httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
  return httpd_resp_sendstr(req, intensite ? "allume" : "eteint");
}

void demarrerServeurs() {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  config.server_port = 80;
  httpd_uri_t uriPhare = {"/phare", HTTP_GET, gererPhare, NULL};
  if (httpd_start(&serveurCommande, &config) == ESP_OK) {
    httpd_register_uri_handler(serveurCommande, &uriPhare);
  }

  // Le flux bloque son serveur tant qu'on regarde : il a donc le sien, sur le port 81
  config.server_port = 81;
  config.ctrl_port += 1;
  httpd_uri_t uriFlux = {"/flux", HTTP_GET, gererFlux, NULL};
  if (httpd_start(&serveurFlux, &config) == ESP_OK) {
    httpd_register_uri_handler(serveurFlux, &uriFlux);
  }
}

// ---------- Wi-Fi ----------
void connecterWifi() {
  Serial.printf("[WIFI] Connexion au réseau de la voiture \"%s\"", WIFI_NOM_VOITURE);
  WiFi.begin(WIFI_NOM_VOITURE, WIFI_MOTDEPASSE_VOITURE);
  uint32_t debut = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - debut < 15000) {
    delay(300);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.printf("[WIFI] Connectée, flux sur http://%s:81/flux\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("[WIFI] Voiture introuvable, nouvel essai plus tard");
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\n[DEMARRAGE] Caméra voiture RC, étape 3");

  phareInit();
  phareRegler(0);

  if (!initCamera()) {
    Serial.println("[CAMERA] ERREUR d'initialisation, redémarrage dans 5 s");
    delay(5000);
    ESP.restart();
  }
  Serial.println("[CAMERA] Prête");

  IPAddress ip, passerelle(192, 168, 4, 1), masque(255, 255, 255, 0);
  ip.fromString(IP_CAMERA);
  WiFi.mode(WIFI_STA);
  WiFi.config(ip, passerelle, masque);
  WiFi.setSleep(false);   // indispensable pour un flux fluide
  WiFi.setAutoReconnect(true);
  connecterWifi();

  demarrerServeurs();
}

void loop() {
  static uint32_t derniereVerifMs = 0;
  if (millis() - derniereVerifMs > 5000) {
    derniereVerifMs = millis();
    if (WiFi.status() != WL_CONNECTED) {
      phareRegler(0);
      WiFi.disconnect();
      connecterWifi();
    }
  }
  delay(10);
}
