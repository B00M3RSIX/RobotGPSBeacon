#ifndef NAVSATFIXDATA_H
#define NAVSATFIXDATA_H

#include <Arduino.h>

// NavSatFix-ähnliche Struktur als Vorbereitung für ROS2-Integration
struct NavSatFixData {
  // Konstanten analog zu ROS2 NavSatFix
  static const uint8_t COVARIANCE_TYPE_UNKNOWN = 0;
  static const uint8_t COVARIANCE_TYPE_APPROXIMATED = 1;
  static const uint8_t COVARIANCE_TYPE_DIAGONAL_KNOWN = 2;
  static const uint8_t COVARIANCE_TYPE_KNOWN = 3;
  
  // Status-Konstanten
  static const uint8_t STATUS_NO_FIX = 0;
  static const uint8_t STATUS_FIX = 1;
  static const uint8_t STATUS_SBAS_FIX = 2;
  static const uint8_t STATUS_GBAS_FIX = 3;
  
  // Datenfelder
  uint8_t status;
  double latitude;
  double longitude;
  double altitude;
  double position_covariance[9];
  uint8_t position_covariance_type;
  
  // Zusätzliche Felder für Debug und Anzeige
  uint8_t satellites;
  float hdop;
  float speed_kmph;
  float course_deg;
  
  // Zeitstempel
  uint16_t year;
  uint8_t month;
  uint8_t day;
  uint8_t hour;
  uint8_t minute;
  uint8_t second;
  
  // Initialisierung
  NavSatFixData() {
    status = STATUS_NO_FIX;
    latitude = 0.0;
    longitude = 0.0;
    altitude = 0.0;
    
    // Initialisiere Kovarianzmatrix
    for (int i = 0; i < 9; i++) {
      position_covariance[i] = 0.0;
    }
    
    position_covariance_type = COVARIANCE_TYPE_UNKNOWN;
    satellites = 0;
    hdop = 0.0;
    speed_kmph = 0.0;
    course_deg = 0.0;
    
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    second = 0;
  }
  
  // Berechnet die Genauigkeit in Metern basierend auf HDOP
  float getHorizontalAccuracyMeters() const {
    // Typische Umrechnung: HDOP * 2.5m (für GPS)
    return hdop * 2.5f;
  }
  
  // Berechnet die vertikale Genauigkeit (typischerweise 1.5x horizontale Genauigkeit)
  float getVerticalAccuracyMeters() const {
    return getHorizontalAccuracyMeters() * 1.5f;
  }
  
  // Aktualisiert die Kovarianzmatrix basierend auf HDOP
  void updateCovariance() {
    float horizontal_accuracy = getHorizontalAccuracyMeters();
    float vertical_accuracy = getVerticalAccuracyMeters();
    
    // Diagonale Kovarianzmatrix (x, y, z)
    position_covariance[0] = horizontal_accuracy * horizontal_accuracy; // x-Varianz
    position_covariance[4] = horizontal_accuracy * horizontal_accuracy; // y-Varianz
    position_covariance[8] = vertical_accuracy * vertical_accuracy;     // z-Varianz
    
    // Andere Elemente auf 0 setzen (keine Korrelation)
    position_covariance[1] = position_covariance[2] = position_covariance[3] = 0;
    position_covariance[5] = position_covariance[6] = position_covariance[7] = 0;
    
    // Setze Kovarianztyp
    if (satellites > 0 && hdop > 0) {
      position_covariance_type = COVARIANCE_TYPE_APPROXIMATED;
    } else {
      position_covariance_type = COVARIANCE_TYPE_UNKNOWN;
    }
  }
  
  // Dummy-Funktion für spätere microROS-Integration
  void toROS2Message() {
    // Diese Funktion wird später implementiert, wenn microROS hinzugefügt wird
    // Hier nur als Platzhalter
  }
};

#endif // NAVSATFIXDATA_H
