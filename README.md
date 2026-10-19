# Voiture télécommandée ESP32

Une petite voiture deux roues motrices pilotée d'abord depuis le téléphone, puis avec une vraie télécommande maison, et enfin en vue embarquée (FPV) grâce à une ESP32-CAM. Le projet avance par étapes, du plus simple au plus complet ; chaque dossier est un projet PlatformIO autonome et peut faire l'objet d'une vidéo.

| Étape | Dossier | Ce que ça fait | Cartes |
| --- | --- | --- | --- |
| 1 | [etape-1-telephone](etape-1-telephone) | La voiture crée son Wi-Fi, on la pilote avec un joystick sur la page web du téléphone | ESP32 DevKit V1 |
| 2 | [etape-2-telecommande](etape-2-telecommande) | Télécommande maison à joystick en ESP-NOW, le téléphone marche toujours | 2 × ESP32 DevKit V1 |
| 3 | [etape-3-camera-fpv](etape-3-camera-fpv) | Vidéo en direct de la caméra embarquée sur le téléphone, LED de la caméra en phare | + ESP32-CAM |
| Bonus | [bonus](bonus) | Anti-collision à ultrasons, jauge de batterie, phares et klaxon | |

Le code compile avec PlatformIO (framework Arduino) mais n'a pas encore été testé sur une vraie voiture.

Sécurité commune à toutes les étapes : la voiture s'arrête d'elle-même si elle ne reçoit plus d'ordre pendant 300 ms (téléphone verrouillé, télécommande éteinte, hors de portée).

## Liste d'achat par étape

Prix approximatifs ; chaque étape réutilise ce qui a été acheté avant. Les liens ouvrent une recherche, pas un produit précis.

### Étape 1 (environ 30 à 40 €)

| Pièce | Prix approx. | Chercher |
| --- | --- | --- |
| ESP32 DevKit V1 (ESP32-WROOM-32, 30 broches) | 5 à 7 € | [Amazon](https://www.amazon.fr/s?k=ESP32+DevKit+V1+WROOM-32) · [AliExpress](https://fr.aliexpress.com/w/wholesale-ESP32-DevKit-V1.html) |
| Châssis 2WD (plaque, 2 moteurs TT jaunes, 2 roues, roue folle, support de piles) | 8 à 12 € | [Amazon](https://www.amazon.fr/s?k=chassis+voiture+robot+2WD+moteur+TT) · [AliExpress](https://fr.aliexpress.com/w/wholesale-2WD-smart-car-chassis.html) |
| Driver moteurs TB6612FNG | 2 à 4 € | [Amazon](https://www.amazon.fr/s?k=TB6612FNG) · [AliExpress](https://fr.aliexpress.com/w/wholesale-TB6612FNG.html) |
| 2 accus 18650 de marque (Samsung, Sony, LG, Molicel) | 8 à 12 € | [Amazon](https://www.amazon.fr/s?k=accu+18650+Samsung+25R) |
| Support 2 × 18650 avec interrupteur | 2 à 3 € | [Amazon](https://www.amazon.fr/s?k=support+2+18650+interrupteur) · [AliExpress](https://fr.aliexpress.com/w/wholesale-2x18650-battery-holder-switch.html) |
| Convertisseur abaisseur MP1584EN (réglé sur 5 V) | 1 à 2 € | [Amazon](https://www.amazon.fr/s?k=MP1584EN) · [AliExpress](https://fr.aliexpress.com/w/wholesale-MP1584EN.html) |
| 2 condensateurs céramique 100 nF (antiparasites moteurs) | moins d'1 € | [AliExpress](https://fr.aliexpress.com/w/wholesale-100nF-ceramic-capacitor.html) |
| Mini breadboard, fils Dupont | 3 à 4 € | [Amazon](https://www.amazon.fr/s?k=mini+breadboard+fils+dupont) |
| Chargeur d'accus 18650 (si tu n'en as pas) | 10 à 15 € | [Amazon](https://www.amazon.fr/s?k=chargeur+18650) |

Pourquoi le TB6612FNG plutôt que le L298N vendu avec beaucoup de kits : le L298N perd environ 2 V et chauffe, alors que le TB6612 est petit, efficace et accepte directement la logique 3,3 V de l'ESP32.

### Étape 2 (environ 15 à 20 € en plus)

| Pièce | Prix approx. | Chercher |
| --- | --- | --- |
| Second ESP32 DevKit V1 (la télécommande) | 5 à 7 € | [Amazon](https://www.amazon.fr/s?k=ESP32+DevKit+V1+WROOM-32) |
| Module joystick KY-023 | 1 à 2 € | [Amazon](https://www.amazon.fr/s?k=joystick+KY-023) · [AliExpress](https://fr.aliexpress.com/w/wholesale-KY-023-joystick.html) |
| Shield batterie 18650 (« 18650 battery shield », sorties 5 V et 3,3 V, charge USB) | 2 à 4 € | [AliExpress](https://fr.aliexpress.com/w/wholesale-18650-battery-shield-V3.html) |
| 1 accu 18650 de marque | 4 à 6 € | |
| Plaque de prototypage ou petit boîtier pour la télécommande | 2 à 4 € | |

### Étape 3 (environ 10 à 14 € en plus)

| Pièce | Prix approx. | Chercher |
| --- | --- | --- |
| ESP32-CAM AI-Thinker (caméra OV2640) | 6 à 8 € | [Amazon](https://www.amazon.fr/s?k=ESP32-CAM+AI-Thinker) · [AliExpress](https://fr.aliexpress.com/w/wholesale-ESP32-CAM.html) |
| Carte de programmation ESP32-CAM-MB | 2 à 4 € | [AliExpress](https://fr.aliexpress.com/w/wholesale-ESP32-CAM-MB.html) |
| Condensateur 470 à 1000 µF, 10 V ou plus | moins d'1 € | |
| Option : caméra OV2640 grand angle (120° ou 160°), plus immersive | 3 à 5 € | [AliExpress](https://fr.aliexpress.com/w/wholesale-OV2640-wide-angle.html) |

La carte ESP32-CAM-MB et l'ESP32-CAM de la boîte aux lettres peuvent servir pour les essais, mais il en faut une dédiée à la voiture.

### Bonus

| Pièce | Prix approx. |
| --- | --- |
| Capteur à ultrasons HC-SR04 (ou HC-SR04P en 3,3 V) | 1 à 2 € |
| Résistances 100 kΩ et 33 kΩ (jauge de batterie), 1 kΩ et 2 kΩ (pont pour l'écho du HC-SR04) | moins d'1 € |
| 2 LED blanches + 2 LED rouges et leurs résistances, ou une barrette NeoPixel | 1 à 3 € |
| Buzzer actif 3,3 V | moins d'1 € |

## Mise en route

Chaque dossier a son propre README avec le câblage et la mise en route. Dans tous les cas : copier `include/config.example.h` en `include/config.h`, remplir les valeurs, puis `pio run -t upload` depuis le dossier du programme. Le fichier `config.h` est exclu du dépôt.
