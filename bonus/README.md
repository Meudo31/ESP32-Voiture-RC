# Bonus

Idées pour enrichir la voiture une fois l'étape 3 terminée. Chacune tient dans une courte vidéo ou un Short. Ces bonus ne sont pas encore codés : on les écrira ensemble quand tu y seras.

## Anti-collision à ultrasons

Un HC-SR04 à l'avant mesure la distance. En dessous de 20 cm, la voiture refuse d'avancer (elle peut toujours reculer ou tourner).

| De | Vers |
| --- | --- |
| HC-SR04 VCC | 5 V (sortie du MP1584) |
| HC-SR04 Trig | GPIO4 |
| HC-SR04 Echo | GPIO16 **via un pont diviseur** 1 kΩ (côté Echo) / 2 kΩ (côté GND), car l'écho sort en 5 V |
| HC-SR04 GND | GND |

Le HC-SR04P fonctionne en 3,3 V et se passe du pont diviseur.

## Jauge de batterie

Un pont diviseur 100 kΩ (côté batterie +) / 33 kΩ (côté GND) ramène les 8,4 V maximum de la batterie à environ 2,1 V, lisibles sur GPIO34. La voiture peut alors afficher la tension sur la page du téléphone, et ralentir sous 6,6 V pour protéger les accus.

## Phares, feux de recul et klaxon

- Deux LED blanches à l'avant, deux rouges à l'arrière (GPIO17, GPIO18, avec résistances de 220 Ω), ou une barrette NeoPixel pour des effets de couleur.
- Feux de recul allumés automatiquement quand la voiture recule.
- Le bouton du joystick de la télécommande (déjà transmis à la voiture à l'étape 2) commande les phares ou un buzzer actif sur GPIO19.

## Châssis 4 roues motrices ou roues mecanum

Un châssis 4WD se pilote avec le même code : les deux moteurs de chaque côté se branchent en parallèle sur la même sortie du TB6612. Avec quatre moteurs TT, le TB6612 arrive à sa limite (1,2 A par voie) : pour des moteurs plus gros, passer à deux drivers ou à un DRV8833 par côté. Des roues mecanum permettent même de se déplacer en crabe, mais demandent un driver par roue et un autre calcul de mélange.
