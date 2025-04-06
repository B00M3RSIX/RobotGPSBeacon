#ifndef NMEAGPS_CFG_H
#define NMEAGPS_CFG_H

//======================================================================
// NMEAGPS_cfg.h - Konfigurationsdatei für NeoGPS
//======================================================================

// Aktiviere die benötigten NMEA-Nachrichtentypen
#define NMEAGPS_PARSE_GGA
#define NMEAGPS_PARSE_GSA
#define NMEAGPS_PARSE_GSV
#define NMEAGPS_PARSE_RMC
#define NMEAGPS_PARSE_VTG
#define NMEAGPS_PARSE_GLL

// Aktiviere die benötigten Datenfelder
#define GPS_FIX_DATE
#define GPS_FIX_TIME
#define GPS_FIX_LOCATION
#define GPS_FIX_ALTITUDE
#define GPS_FIX_SPEED
#define GPS_FIX_HEADING
#define GPS_FIX_SATELLITES

// Aktiviere DOP-Werte (Dilution of Precision)
#define NMEAGPS_PARSE_HDOP
#define NMEAGPS_PARSE_VDOP
#define NMEAGPS_PARSE_PDOP

// Aktiviere Trace-Funktionalität für Debug-Ausgaben
#define NMEAGPS_PARSE_SATELLITES
#define NMEAGPS_STATS

// Konfiguration für die serielle Schnittstelle
#define NMEAGPS_INTERRUPT_PROCESSING
#define NMEAGPS_RECOGNIZE_ALL

// Puffergröße für NMEA-Sätze
#define NMEAGPS_BUFFER_SIZE 96

// Maximale Anzahl von Satelliten, die verfolgt werden können
#define NMEAGPS_MAX_SATELLITES 16

// Aktiviere Fehlerbehandlung
#define NMEAGPS_ERRORS_ENABLED

// Aktiviere Debugging-Funktionen
#define NMEAGPS_STATISTICS

#endif // NMEAGPS_CFG_H
