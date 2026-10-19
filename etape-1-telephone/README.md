# Étape 1 : piloter la voiture depuis le téléphone

L'ESP32 crée son propre réseau Wi-Fi. Le téléphone s'y connecte, ouvre `http://192.168.4.1` et affiche un gros joystick tactile. Pas d'appli à installer, pas de box Internet : ça marche aussi en pleine nature.

Le code compile avec PlatformIO (ESP32 DevKit V1, framework Arduino) mais n'a pas encore été testé sur une vraie voiture.

## Câblage

| De | Vers |
| --- | --- |
| Batterie + (après l'interrupteur du support 2 × 18650) | VM du TB6612 **et** IN+ du MP1584 |
| Batterie - | GND commun (TB6612, MP1584 IN-, ESP32) |
| OUT+ du MP1584 (réglé sur 5 V) | Broche **VIN** (ou 5V) de l'ESP32 |
| OUT- du MP1584 | GND |
| 3V3 de l'ESP32 | VCC du TB6612 (logique) |
| GPIO27 / GPIO26 / GPIO25 | AIN1 / AIN2 / PWMA (moteur gauche) |
| GPIO33 / GPIO32 / GPIO14 | BIN1 / BIN2 / PWMB (moteur droit) |
| GPIO13 | STBY |
| AO1 / AO2 | Moteur gauche |
| BO1 / BO2 | Moteur droit |

**Règle le MP1584 sur 5 V au multimètre avant d'y brancher l'ESP32.** Il sort souvent réglé bien plus haut.

Soude un condensateur céramique de 100 nF directement entre les deux bornes de chaque moteur : ça évite que les parasites des balais fassent redémarrer l'ESP32.

Pendant les essais, roues en l'air, tu peux alimenter l'ESP32 par USB tout en laissant la batterie sur le TB6612 (GND toujours commun).

## Mise en route

1. Copie `include/config.example.h` en `include/config.h` et change au moins le mot de passe Wi-Fi (8 caractères minimum).
2. Branche l'ESP32 en USB, lance `pio run -t upload` puis `pio device monitor`.
3. Sur le téléphone, connecte-toi au réseau `VoitureRC` puis ouvre `http://192.168.4.1`.
4. Si le téléphone propose de quitter ce réseau « sans Internet », réponds de rester connecté (et coupe les données mobiles si la page ne charge pas).
5. Pousse le joystick vers le haut : les deux roues doivent avancer. Si une roue recule, mets `INVERSER_GAUCHE` ou `INVERSER_DROITE` à 1 dans `config.h`. Si gauche et droite sont inversées, échange les fils des deux moteurs sur le TB6612.

## Points à vérifier ensemble

- **Vitesse** : `VITESSE_MAX` vaut 200 sur 255, soit environ 6 V moyens pour des moteurs TT prévus pour 3 à 6 V. Monte à 255 si tu acceptes de les user plus vite.
- **Virages** : la voiture tourne comme un char (une roue plus vite que l'autre). Si elle tourne trop sec, baisse `SENSIBILITE_VIRAGE`.
- **Sécurité** : verrouille le téléphone pendant que la voiture roule : elle doit s'arrêter en moins d'une demi-seconde, et le moniteur série affiche `[SECURITE]`.
- **Redémarrages** : si l'ESP32 redémarre au démarrage brusque des moteurs, c'est une chute de tension. Vérifie les condensateurs 100 nF, et ajoute un condensateur de 470 µF entre VM et GND du TB6612.
