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

    // Aktualisiere Hardware-Manager (nicht-blockierend)
    hatchManager.update();
    gpsManager.update();


    // Überprüfe MicroROS-Verbindung
    rosInterface.update();
    
    if (rosInterface.getConnectionState()==BeaconMicroROSInterface::WAITING_AGENT) {
        statusLED.setStatus(LED_STATUS_ERROR);
    } else {
        if (gpsManager.hasValidFix()) {
            statusLED.setStatus(LED_STATUS_CONNECTED_FIX);
        } else {
            statusLED.setStatus(LED_STATUS_CONNECTED_NO_FIX);
        }
    } 
    
    statusLED.update();
}
