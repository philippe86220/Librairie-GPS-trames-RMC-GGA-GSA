# TrameNMEA – Décodage minimaliste des trames GPS RMC / GGA / GSA

Librairie C++ pour cartes **Arduino AVR** (ATmega328, Arduino UNO…) permettant  
d’extraire rapidement les informations clés des phrases NMEA `$GPRMC`,
`$GPGGA` et `$GPGSA`.

> **Objectif** : fournir un parseur léger 
> sans dépendance externe, capable de tourner sur un Nano / Uno.

---

## Sommaire
- [Installation](#installation)
- [Connexion matérielle](#connexion-matérielle)
- [Exemple minimal](#exemple-minimal)
- [API](#api)
- [Structure interne](#structure-interne)
- [Licence et crédits](#licence-et-crédits)

---

## Installation

1. **Via le Library Manager**  (après publication)  
   `Sketch ▸ Include Library ▸ Manage Libraries… ▸ TrameNMEA`

2. **Manuellement**  
   - Téléchargez / clonez ce dépôt.  
   - Copiez le dossier **`TrameNMEA`** dans *Documents/Arduino/libraries*.

---

## Connexion matérielle

| GPS module pin | Arduino pin |
|----------------|------------|
| TX (GPS)       | **D2** (SoftwareSerial RX) |
| RX (GPS)       | **D3** (SoftwareSerial TX) |
| VCC / GND      | 3.3 V / GND |

Assurez‑vous que le module émet en 9600 bauds (valeur par défaut de la plupart
des récepteurs).

Type de module utilisé pour la mise en oeure de la librairie :
https://www.gotronic.fr/art-module-gps-tel0094-25732.htm

## Exemple minimal
```cpp
#include <trame.h>
#include <SoftwareSerial.h>

constexpr uint8_t  RX = 2, TX = 3;
SoftwareSerial gpsUART(RX, TX);

constexpr byte NMEA_MAX = 82;
char sentence[NMEA_MAX + 1];
Trame gps(nullptr);

void setup() {
  Serial.begin(115200);
  gpsUART.begin(9600);
}

bool getSentence() {
  static byte i = 0;
  while (gpsUART.available()) {
    char c = gpsUART.read();
    if (c == '\r')              continue;
    if (c == '\n') {
      sentence[i] = '\0';
      i = 0;
      return true;
    }
    if (i < NMEA_MAX) sentence[i++] = c;
  }
  return false;
}

void loop() {
  if (getSentence()) gps.setSentence(sentence);
  if (gps.extrait() && gps.estValide()) {
    Serial.print(gps.jour); Serial.print(F(":")); Serial.print(gps.mois); Serial.print(F(":")); Serial.println(2000 + gps.annee);
    uint8_t hl = (gps.heure + Trame::UTC_OFFSET) % 24;
    Serial.print(hl); Serial.print(F(":")); Serial.print(gps.minute); Serial.print(F(":")); Serial.println(gps.seconde);
    Serial.print(F("Latitude : ")); Serial.println(gps.latitude, 6);
    Serial.print(F("Longitude: ")); Serial.print(gps.longitude, 6);
    Serial.println(F("\n"));
  }
}
```
## API

| Méthode                       | Effet                                                     |
| -----------------------------  | -------------------------------------------------------- |
| `Trame(char* uneTrame)`        | Constructeur (une seule instance recommandée).           |
| `void setSentence(char* nmea)` | Pointeur vers la nouvelle phrase NMEA.                   |
| `bool extrait()`               | Identifie et parse ; renvoie `false` si échec.           |
| `bool estValide() const`       | `true` si `valid == 'A'` **et** `fixQuality > 0`.        |
| `Type type() const`            | Enum `GPRMC / GPGGA / GPGSA / INCONNU`.                  |


## Champs publics mis à jour

- latitude, longitude, vitesse, route
- jour, mois, annee, heure, minute, seconde
- fixQuality, numSat, hdop, altitude, geoidSeparation
- selMode, fixType, satID[12], pdop, hdop_gsa, vdop

## Structure interne
```text
buffers statiques  ─┬─ _bufRMC[12][20]
                    ├─ _bufGGA[14][20]
                    └─ _bufGSA[17][20]

Trame::extrait()  → détecte type → choisit buffer → découpe → parseRMC/GGA/GSA
```


## Licence et crédits
MIT Licence – voir LICENSE.
Certaines parties de la conception et de la documentation ont été réalisées
avec l’assistance de ChatGPT (OpenAI).
Les tests, validations et décisions finales ont été effectués manuellement.


---


