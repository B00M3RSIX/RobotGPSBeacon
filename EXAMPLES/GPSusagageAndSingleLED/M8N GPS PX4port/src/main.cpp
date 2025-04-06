#include <Arduino.h>
#include <TinyGPSPlus.h>
#include "NavSatFixData.h"
#include <Wire.h>
#include "driver/NCP5623.h"

// Debug mode
#define DEBUG_GPS_SENTENCES 0  // Set to 1 to see raw NMEA sentences

// Define Serialport where GPS is connected to and used Baudrate
#define gpsPort Serial3
#define GPS_BAUD 38400  // Standard-Baudrate for M8N Sensor

// Global variables
TinyGPSPlus gps;
NavSatFixData navsat_data;

unsigned long last_gps_display = 0;
unsigned long last_serial_check = 0;
uint32_t serial_bytes_received = 0;
uint32_t last_serial_bytes_count = 0;

// Buffer for debug NMEA sentence
char nmea_buffer[100];
uint8_t nmea_idx = 0;

// Function prototypes
void displayGPSData();
void displayROS2Format();
void checkSerialData();
void handleNMEASentence(char c);
void updateNavSatFixData();

//single RGB-LED
NCP5623 singleRGBLED;


void setup() {
  delay(100);
  Wire.begin();
  singleRGBLED.begin();
  delay(100);
  singleRGBLED.setColor(0,100,0); // Green
  // Initialize Serial for USB communication
  Serial.begin(115200);

  gpsPort.begin(GPS_BAUD);

  unsigned long start_time = millis();
  uint8_t bytes_received = 0;
  
  // Check for data at this baud rate
  while (millis() - start_time < 1000) {
    if (gpsPort.available()) {
      gpsPort.read(); // Just discard the data
      bytes_received++;
    }
    if (bytes_received > 10) {
      break;
    }
  }
  
}

void loop() {
  // Read data from GPS
  while (gpsPort.available()) {
    char c = gpsPort.read();
    
    // Store for debug
    if (DEBUG_GPS_SENTENCES) {
      handleNMEASentence(c);
    }
    
    // Process the character with TinyGPSPlus
    if (gps.encode(c)) {
      // Neue Daten wurden verarbeitet
      updateNavSatFixData();
    }
    
    serial_bytes_received++;
  }
    
  // Check if we're receiving serial data every 2 seconds
  if (millis() - last_serial_check > 2000) {
    checkSerialData();
    last_serial_check = millis();
  }
  
  if (navsat_data.status == NavSatFixData::STATUS_FIX) {
    singleRGBLED.setColor(0,0,100); // blue
  } else {
    singleRGBLED.setColor(100,0,0); // red
  }

  // Display GPS data every second
/* */
  if (millis() - last_gps_display > 1000) {
    displayGPSData();
    displayROS2Format();
    last_gps_display = millis();
  }
/* */
}

void updateNavSatFixData() {
  // Übertrage die Daten aus dem TinyGPSPlus-Objekt in die NavSatFix-Struktur
  
  // Status setzen
  if (gps.location.isValid()) {
    navsat_data.status = NavSatFixData::STATUS_FIX;
  } else {
    navsat_data.status = NavSatFixData::STATUS_NO_FIX;
  }
  
  // Positionsdaten
  if (gps.location.isValid()) {
    navsat_data.latitude = gps.location.lat();
    navsat_data.longitude = gps.location.lng();
  }
  
  // Höhe
  if (gps.altitude.isValid()) {
    navsat_data.altitude = gps.altitude.meters();
  }
  
  // Geschwindigkeit
  if (gps.speed.isValid()) {
    navsat_data.speed_kmph = gps.speed.kmph();
  }
  
  // Kurs
  if (gps.course.isValid()) {
    navsat_data.course_deg = gps.course.deg();
  }
  
  // Satelliten
  if (gps.satellites.isValid()) {
    navsat_data.satellites = gps.satellites.value();
  }
  
  // HDOP-Wert für Genauigkeitsberechnung
  if (gps.hdop.isValid()) {
    navsat_data.hdop = gps.hdop.value() / 100.0; // TinyGPSPlus gibt HDOP in Hundertstel zurück
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
    
    navsat_data.hdop = hdop_value;
  }
  
  // Kovarianzmatrix aktualisieren
  navsat_data.updateCovariance();
  
  // Zeitstempel
  if (gps.date.isValid() && gps.time.isValid()) {
    navsat_data.year = gps.date.year();
    navsat_data.month = gps.date.month();
    navsat_data.day = gps.date.day();
    navsat_data.hour = gps.time.hour();
    navsat_data.minute = gps.time.minute();
    navsat_data.second = gps.time.second();
  }
}

void handleNMEASentence(char c) {
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

void checkSerialData() {
  uint32_t bytes_since_last_check = serial_bytes_received - last_serial_bytes_count;
  last_serial_bytes_count = serial_bytes_received;
  
  Serial.print("Bytes received in last 2 seconds: ");
  Serial.println(bytes_since_last_check);
  
  if (bytes_since_last_check == 0) {
    Serial.println("WARNING: No data received on Serial3! Check GPS connection and baud rate.");
  } else {
    Serial.println("Serial3 connection is working.");
    
    // Check if we're getting valid NMEA data
    if (gps.charsProcessed() > 0 && !gps.location.isValid()) {
      Serial.println("NMEA data is being received but no valid fix obtained yet.");
      Serial.print("Processed chars: ");
      Serial.println(gps.charsProcessed());
      Serial.print("Sentences with fix: ");
      Serial.println(gps.sentencesWithFix());
      Serial.print("Failed checksums: ");
      Serial.println(gps.failedChecksum());
    }
  }
}

void displayROS2Format() {
  Serial.println("\n---- ROS-MSG-----");
  Serial.println("header:");
  Serial.println("  stamp:");
  
  // Wenn Zeitstempel verfügbar, diesen verwenden
  if (gps.date.isValid() && gps.time.isValid()) {
    Serial.println("    sec: " + String(navsat_data.second));
    Serial.println("    nanosec: 0");
  } else {
    Serial.println("    sec: 0");
    Serial.println("    nanosec: 0");
  }
  
  Serial.println("  frame_id: \"gps\"");
  
  // Status
  Serial.println("status:");
  Serial.println("  status: " + String(navsat_data.status));
  Serial.println("  service: 1");  // GPS = 1 in ROS2 NavSatFix
  
  // Position
  Serial.print("latitude: ");
  if (gps.location.isValid()) {
    Serial.println(navsat_data.latitude, 6);
  } else {
    Serial.println("0.0");
  }
  
  Serial.print("longitude: ");
  if (gps.location.isValid()) {
    Serial.println(navsat_data.longitude, 6);
  } else {
    Serial.println("0.0");
  }
  
  Serial.print("altitude: ");
  if (gps.altitude.isValid()) {
    Serial.println(navsat_data.altitude, 2);
  } else {
    Serial.println("0.0");
  }
  
  // Kovarianzmatrix
  Serial.print("position_covariance: [");
  for (int i = 0; i < 9; i++) {
    Serial.print(navsat_data.position_covariance[i], 1);
    if (i < 8) Serial.print(", ");
  }
  Serial.println("]");
  
  Serial.println("position_covariance_type: " + String(navsat_data.position_covariance_type));
  
  // Zusätzliche Informationen (nicht Teil des Standard-NavSatFix)
  Serial.println("# Zusätzliche Informationen:");
  Serial.println("satellites: " + String(navsat_data.satellites));
  Serial.println("hdop: " + String(navsat_data.hdop, 2));
  Serial.println("speed_kmph: " + String(navsat_data.speed_kmph, 2));
  Serial.println("course_deg: " + String(navsat_data.course_deg, 2));
  
  // Zeitstempel im ISO-Format
  if (gps.date.isValid() && gps.time.isValid()) {
    char dateTime[32];
    sprintf(dateTime, "%04d-%02d-%02dT%02d:%02d:%02d", 
            navsat_data.year, navsat_data.month, navsat_data.day,
            navsat_data.hour, navsat_data.minute, navsat_data.second);
    Serial.println("timestamp: \"" + String(dateTime) + "\"");
  }
}

void displayGPSData() {
  Serial.println("\n----- GPS Data -----");
  Serial.print("Valid fix: ");
  Serial.println(gps.location.isValid() ? "YES" : "NO");
  
  // Print processing stats
  Serial.print("GPS stats - Processed chars: ");
  Serial.print(gps.charsProcessed());
  Serial.print(", Valid sentences: ");
  Serial.print(gps.passedChecksum());
  Serial.print(", Failed checksum: ");
  Serial.println(gps.failedChecksum());
  
  // Print raw NMEA sentence if data is coming in but no fix
  if (!gps.location.isValid() && serial_bytes_received > last_serial_bytes_count) {
    Serial.println("No valid fix, but GPS data is being received.");
    Serial.println("Possible causes: Poor satellite visibility, antenna issues, or time needed for first fix.");
    Serial.println("Try moving to a location with better sky visibility.");
  }
  
  if (gps.location.isValid()) {
    Serial.print("Location: ");
    Serial.print(gps.location.lat(), 6);
    Serial.print(", ");
    Serial.println(gps.location.lng(), 6);
    
    if (gps.altitude.isValid()) {
      Serial.print("Altitude: ");
      Serial.print(gps.altitude.meters());
      Serial.println(" m");
    }
    
    if (gps.speed.isValid()) {
      Serial.print("Speed: ");
      Serial.print(gps.speed.kmph());
      Serial.println(" km/h");
    }
    
    if (gps.course.isValid()) {
      Serial.print("Course: ");
      Serial.print(gps.course.deg());
      Serial.println(" degrees");
    }
    
    Serial.print("Satellites: ");
    Serial.println(gps.satellites.isValid() ? gps.satellites.value() : 0);
    
    // Accuracy information
    Serial.print("HDOP: ");
    Serial.println(navsat_data.hdop); // Verwende den berechneten HDOP-Wert
    
    // Genauigkeitsberechnung basierend auf HDOP
    float horizontalAccuracy = navsat_data.getHorizontalAccuracyMeters();
    Serial.print("Estimated Horizontal Accuracy: ");
    Serial.print(horizontalAccuracy, 1);
    Serial.println(" meters");
    
    float verticalAccuracy = navsat_data.getVerticalAccuracyMeters();
    Serial.print("Estimated Vertical Accuracy: ");
    Serial.print(verticalAccuracy, 1);
    Serial.println(" meters");
    
    // Kovarianzinformationen
    Serial.print("Position Covariance Type: ");
    switch (navsat_data.position_covariance_type) {
      case NavSatFixData::COVARIANCE_TYPE_UNKNOWN:
        Serial.println("Unknown");
        break;
      case NavSatFixData::COVARIANCE_TYPE_APPROXIMATED:
        Serial.println("Approximated");
        break;
      case NavSatFixData::COVARIANCE_TYPE_DIAGONAL_KNOWN:
        Serial.println("Diagonal Known");
        break;
      case NavSatFixData::COVARIANCE_TYPE_KNOWN:
        Serial.println("Known");
        break;
    }
    
    // Interpret the accuracy
    if (horizontalAccuracy < 2.0) {
      Serial.println("Position Quality: Excellent (RTK-like, <2m)");
    } else if (horizontalAccuracy < 5.0) {
      Serial.println("Position Quality: Good (2-5m)");
    } else if (horizontalAccuracy < 10.0) {
      Serial.println("Position Quality: Moderate (5-10m)");
    } else {
      Serial.println("Position Quality: Poor (>10m)");
    }
    
    // Date/Time
    if (gps.date.isValid() && gps.time.isValid()) {
      char dateTime[32];
      sprintf(dateTime, "%04d-%02d-%02d %02d:%02d:%02d", 
              gps.date.year(), gps.date.month(), gps.date.day(),
              gps.time.hour(), gps.time.minute(), gps.time.second());
      Serial.print("Date/Time: ");
      Serial.println(dateTime);
    } else {
      Serial.println("Date/Time: INVALID");
    }
  }
}
