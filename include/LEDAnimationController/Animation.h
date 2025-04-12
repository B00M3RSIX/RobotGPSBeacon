#ifndef ANIMATION_H
#define ANIMATION_H

#include <Arduino.h>
#include <FastLED.h>


/**
 * @brief Basisinterface für alle Animationen
 * 
 * Diese abstrakte Klasse definiert die Schnittstelle, die alle Animationen
 * implementieren müssen. Sie ermöglicht polymorphes Verhalten und eine
 * einheitliche Behandlung verschiedener Animationstypen.
 */
class Animation {
public:
    virtual ~Animation() = default;
   
    /**
     * @brief Initialisiert die Animation mit Parametern
     * 
     * @return true wenn die Initialisierung erfolgreich war
     */
    virtual bool setup() = 0;
    
    /**
     * @brief Startet die Animation
     * 
     * @param params Array von Parametern für die Animation
     * @return true wenn der Start erfolgreich war
     */
    virtual bool start() = 0;
    
    /**
     * @brief Führt einen Animationsschritt aus
     * 
     * Diese Methode muss non-blocking sein und sofort zurückkehren.
     * 
     * @param currentTime Aktuelle Zeit in Millisekunden
     * @return true wenn der Schritt erfolgreich war
     */
    virtual bool run(uint32_t currentTime) = 0;
    
    /**
     * @brief Bricht die Animation ab
     * 
     * @return true wenn der Abbruch erfolgreich war
     */
    virtual bool cancel() = 0;
    
    /**
     * @brief Prüft, ob die Animation abgeschlossen ist
     * 
     * @return true wenn die Animation abgeschlossen ist
     */
    virtual bool isCompleted() const = 0;
    
    /**
     * @brief Gibt den Fortschritt der Animation zurück
     * 
     * @return Fortschritt als Wert zwischen 0.0 und 1.0
     */
    virtual float getProgress() const = 0;
    
    /**
     * @brief Gibt den Typ der Animation zurück
     * 
     * @return int (ANIMATION_IMMEDIATE, ANIMATION_ONETIME, ANIMATION_CONTINUOUS)
     */
    virtual AnimationType getType() const = 0;
    
    /**
     * @brief Prüft, ob die Animation einen bestimmten Befehl unterstützt
     * 
     * @param cmd Der zu prüfende Befehl
     * @return true wenn die Animation den Befehl unterstützt
     */
    virtual bool supportsCommand(AnimationCommand cmd) const = 0;
    
};

#endif // ANIMATION_H
