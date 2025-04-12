#include "GPSManager.h"

GPSManager::GPSManager()
    : gpsSerial(nullptr)
    , last_serial_check(0)
    , serial_bytes_received(0)
    , last_serial_bytes_count(0)
{
}

void GPSManager::begin(HardwareSerial& serial, unsigned long baud) {
    gpsSerial = &serial;
    gpsSerial->begin(baud);
    
    // Warte kurz und prüfe dann, ob Daten empfangen werden
    unsigned long start_time = millis();
    uint8_t bytes_received = 0;
    
    while (millis() - start_time < 1000) {
        if (gpsSerial->available()) {
            gpsSerial->read(); // Verwerfe die Daten
            bytes_received++;
        }
        if (bytes_received > 10) {
            break;
        }
    }
}

void GPSManager::update() {
    // Lese GPS-Daten
    while (gpsSerial && gpsSerial->available()) {
        char c = gpsSerial->read();
        //handleNMEASentence(c);
        // Verarbeite das Zeichen mit TinyGPSPlus
        if (gps.encode(c)) {
            // Neue Daten wurden verarbeitet
            updateNavSatFixData();
        }
        
        serial_bytes_received++;
    }
    
    // Prüfe alle x Millisekunden, ob Daten empfangen werden
    if (millis() - last_serial_check > 200) {
        checkSerialData();
        last_serial_check = millis();
    }
}

bool GPSManager::hasValidFix() const {
    return gps.location.isValid();
}

const NavSatFixData& GPSManager::getNavSatFixData() const {
    return navSatData;
}

void GPSManager::updateNavSatFixData() {
    // Status setzen
    if (gps.location.isValid()) {
        navSatData.status = NavSatFixData::STATUS_FIX;
    } else {
        navSatData.status = NavSatFixData::STATUS_NO_FIX;
    }
    
    // Positionsdaten
    if (gps.location.isValid()) {
        navSatData.latitude = gps.location.lat();
        navSatData.longitude = gps.location.lng();
    }
    
    // Höhe
    if (gps.altitude.isValid()) {
        navSatData.altitude = gps.altitude.meters();
    }
    
    // Geschwindigkeit
    if (gps.speed.isValid()) {
        navSatData.speed_kmph = gps.speed.kmph();
    }
    
    // Kurs
    if (gps.course.isValid()) {
        navSatData.course_deg = gps.course.deg();
    }
    
    // Satelliten
    if (gps.satellites.isValid()) {
        navSatData.satellites = gps.satellites.value();
    }
    
    // HDOP-Wert für Genauigkeitsberechnung
    if (gps.hdop.isValid()) {
        navSatData.hdop = gps.hdop.value() / 100.0; // TinyGPSPlus gibt HDOP in Hundertstel zurück
    } else {
        // Wenn HDOP nicht verfügbar ist, verwenden wir einen Standardwert basierend auf der Anzahl der Satelliten
        float hdop_value = 5.0; // Standardwert
        
        if (gps.satellites.isValid()) {
            // Einfache Schätzung: Je mehr Satelliten, desto besser die Genauigkeit
            if (gps.satellites.value() >= 10) hdop_value = 1.0;
            else if (gps.satellites.value() >= 8) hdop_value = 1.5;
            else if (gps.satellites.value() >= 6) hdop_value = 2.0;
            else if (gps.satellites.value() >= 4) hdop_value = 3.0;
        }
        
        navSatData.hdop = hdop_value;
    }
    
    // Kovarianzmatrix aktualisieren
    navSatData.updateCovariance();
    
    // Zeitstempel
    if (gps.date.isValid() && gps.time.isValid()) {
        navSatData.year = gps.date.year();
        navSatData.month = gps.date.month();
        navSatData.day = gps.date.day();
        navSatData.hour = gps.time.hour();
        navSatData.minute = gps.time.minute();
        navSatData.second = gps.time.second();
    }
}

void GPSManager::checkSerialData() {
    uint32_t bytes_since_last_check = serial_bytes_received - last_serial_bytes_count;
    last_serial_bytes_count = serial_bytes_received;
    
    if (bytes_since_last_check == 0) {
        // Keine Daten empfangen - mögliches Verbindungsproblem
        navSatData.status = NavSatFixData::STATUS_NO_FIX;
    } else {
        // Daten werden empfangen, aber prüfe ob wir gültige NMEA-Daten bekommen
        if (gps.charsProcessed() > 0 && !gps.location.isValid()) {
            // NMEA-Daten werden empfangen, aber noch kein Fix
            navSatData.status = NavSatFixData::STATUS_NO_FIX;
        }
    }
}

void GPSManager::handleNMEASentence(char c) {
    // Store character in buffer
    if (c == '$') {
      // Start of a new sentence
      nmea_idx = 0;
    }
    
    if (nmea_idx < sizeof(nmea_buffer) - 1) {
      nmea_buffer[nmea_idx++] = c;
    }
    
    if (c == '\n') {
      // End of sentence
      nmea_buffer[nmea_idx] = '\0';
      Serial.print("NMEA: ");
      Serial.print(nmea_buffer);
      nmea_idx = 0;
    }
  }
