#include <Arduino.h>
#include "LEDAnimationController/LEDAnimationController.h"
#include "config.h"
#include "HatchManager.h"
#include "GPSManager.h"
#include "StatusLEDManager.h"
#include "BeaconMicroROSInterface.h"

// Manager-Instanzen
HatchManager hatchManager(HATCH_LEFT_PIN, HATCH_RIGHT_PIN);
GPSManager gpsManager;
StatusLEDManager statusLED;
LEDAnimationController ledAnimationController;

// MicroROS-Interface (enthält den LED-Strip Controller)
BeaconMicroROSInterface rosInterface(&hatchManager, &gpsManager);

// Status-Tracking
bool rosConnected = false;
bool previousRosConnected = false;

void setup() {
    Serial.begin(115200);
    // Initialisiere Hardware-Manager
    statusLED.begin();
    
    // Initialisiere LED-Strip Controller
    ledAnimationController.begin();

    hatchManager.begin();
    gpsManager.begin(GPS_SERIAL, GPS_BAUD);
    delay(5000);
    
    // Setze anfänglichen LED-Status auf "Verbindung wird hergestellt"
    statusLED.setStatus(LED_STATUS_CONNECTING);
    
    // Initialisiere MicroROS
    rosInterface.initialize();
    
    // Warte kurz für die Initialisierung
    delay(1000);
}

void loop() {

    // Aktualisiere LED-Strip Animation (nicht-blockierend)
    ledAnimationController.update();

    // Überprüfe MicroROS-Verbindung
    rosInterface.checkConnection();
    
    // Aktualisiere Hardware-Manager (nicht-blockierend)
    hatchManager.update();
    gpsManager.update();
    statusLED.update();
    
    // Aktualisiere Verbindungsstatus
    rosConnected = rosInterface.isConnected();
    
    // Aktualisiere RGB-LED basierend auf Verbindungs- und GPS-Status
    if (!rosConnected) {
        // Verbindung wird hergestellt oder verloren
        statusLED.setStatus(LED_STATUS_CONNECTING);
    } else if (rosConnected && !previousRosConnected) {
        // Verbindung wurde gerade hergestellt
        if (gpsManager.hasValidFix()) {
            statusLED.setStatus(LED_STATUS_CONNECTED_FIX);
        } else {
            statusLED.setStatus(LED_STATUS_CONNECTED_NO_FIX);
        }
    } else if (rosConnected) {
        // Verbindung besteht bereits
        if (gpsManager.hasValidFix()) {
            statusLED.setStatus(LED_STATUS_CONNECTED_FIX);
        } else {
            statusLED.setStatus(LED_STATUS_CONNECTED_NO_FIX);
        }
    }
    
    // Speichere aktuellen Verbindungsstatus für den nächsten Durchlauf
    previousRosConnected = rosConnected;
    
    // Verarbeite MicroROS-Nachrichten
    if (rosConnected) {
        rosInterface.processMessages();
        
        // Veröffentliche Daten (die Methoden prüfen intern die Publish-Rate)
        rosInterface.publishHatchStatus();
        rosInterface.publishGPSData();
    }
}
