#include "trame.h"
#include <SoftwareSerial.h>
static const uint8_t RXPin = 2, TXPin = 3;
static const uint32_t GPSBaud = 9600;
const byte tailleMessageMax = 82; //La norme fixe 82 caractères (checksum et CR/LF inclus).
char message[tailleMessageMax + 1]; // +1 car on doit avoir un caractère de fin de chaîne en C, le '\0'
const char marqueurDeFin = '\n';

SoftwareSerial uartGps(RXPin, TXPin);

bool messageRecu() {
  static byte idx = 0;

  while (uartGps.available()) {          // on vide tout ce qui est arrivé
    char c = uartGps.read();

    if (c == '\r') continue;             // on ignore CR
    if (c == '\n') {                     // LF termine la phrase
      message[idx] = '\0';
      idx = 0;
      return true;                       // une phrase complète prête
    }
    if (idx < tailleMessageMax) message[idx++] = c;
    // sinon, on déborde : on jette le surplus
  }
  return false;                          // rien de complet pour l'instant
}

Trame gps(nullptr);           // ← une seule instance, jamais recréée


void setup() {
  Serial.begin(115200);
  uartGps.begin(9600);
}

void traiterMessage() {

  // --- 1) GPRMC : date, heure, position, vitesse, route ---
  Serial.println(F("[AFFICHAGE RMC]"));
  char bufRMC[11];
  // Date
  snprintf(bufRMC, sizeof(bufRMC), "%02u/%02u/%04u",
           gps.jour, gps.mois, 2000 + gps.annee);
  Serial.println(bufRMC);
  // Heure locale
  uint8_t hl = (gps.heure + Trame::UTC_OFFSET) % 24;
  snprintf(bufRMC, sizeof(bufRMC), "%02u:%02u:%02u",
           hl, gps.minute, gps.seconde);
  Serial.println(bufRMC);
  // Pos
  Serial.print(F("Lat: "));  Serial.print(gps.latitude, 6);
  Serial.print(F(" "));      Serial.println(gps.latitude_direction);
  Serial.print(F("Long: ")); Serial.print(gps.longitude, 6);
  Serial.print(F(" "));      Serial.println(gps.longitude_direction);
  // Vitesse & route
  Serial.print(F("Vitesse km/h: ")); Serial.println(gps.vitesse);
  Serial.print(F("Route : "));      Serial.println(gps.route);

  // --- 2) GGA
  Serial.println(F("[AFFICHAGE GGA]"));
  // Qualité du fix et nb satellites
  char bufGGA[23];
  snprintf(bufGGA, sizeof(bufGGA),
           "FixQ:%u Sats:%u HDOP: ",
           gps.fixQuality,
           gps.numSat);
  Serial.print(bufGGA);
  Serial.println(gps.hdop);
  // Altitude
  Serial.print(F("Alt: "));
  Serial.print(gps.altitude);
  Serial.print(F(" "));
  Serial.println(gps.altitudeUnit);

  // Geoid Separation
  Serial.print(F("Geoid: "));
  Serial.print(gps.geoidSeparation);
  Serial.print(F(" "));
  Serial.println(gps.geoidUnit);

  // --- 2) GSA
  Serial.println(F("[AFFICHAGE GSA]"));
  char bufGSA[15];
  snprintf(bufGSA, sizeof(bufGSA),
           "Mode:%c  Fix:%u",
           gps.selMode, gps.fixType);
  Serial.println(bufGSA);
  Serial.print(F("PDOP: "));  Serial.println(gps.pdop);
  Serial.print(F("HDOP: "));  Serial.println(gps.hdop_gsa);
  Serial.print(F("VDOP: "));  Serial.println(gps.vdop);


  // Sat IDs
  Serial.print("SatIDs:");
  for (uint8_t i = 0; i < 12; ++i) {
    if (gps.satID[i]) {
      Serial.print(F(" "));
      Serial.print(gps.satID[i]);
    }
  }
  Serial.println(F("\n"));
}
void loop() {

  if (!messageRecu()) return;
  gps.setSentence(message);       // on remplace juste la phrase
  if (!gps.extrait())   return;
  if (!gps.estValide()) return;
  traiterMessage();
}
