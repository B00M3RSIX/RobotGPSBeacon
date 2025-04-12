#ifndef WAIT_ANIMATION_H
#define WAIT_ANIMATION_H

#include "../AnimationBase.h"

/**
 * @brief Warte-Animation
 * 
 * Diese Animation führt keine sichtbaren Änderungen durch, sondern wartet nur
 * eine bestimmte Zeit. Sie wird für Pausen in Animations-Sequenzen verwendet.
 * Sie ist vom Typ ONETIME und wird nach Ablauf der Wartezeit abgeschlossen.
 */
class WaitAnimation : public AnimationBase {
private:
    uint32_t duration;    // Dauer der Wartezeit in Millisekunden
    uint32_t startTime;   // Startzeit der Animation
    
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     * @param waitDuration Dauer der Wartezeit in Millisekunden
     */
    WaitAnimation(CRGB* leds,  AnimationContext* context, uint32_t waitDuration) 
        : AnimationBase(leds, context)
        , duration(waitDuration)
        , startTime(0) {
    }
    
    /**
     * @brief Gibt den Typ der Animation zurück
     * 
     * @return ANIMATION_ONETIME
     */
    AnimationType getType() const override {
        return ANIMATION_ONETIME;
    }
    
    /**
     * @brief Prüft, ob die Animation einen bestimmten Befehl unterstützt
     * 
     * Die WaitAnimation wird nicht direkt durch Befehle ausgelöst,
     * sondern nur als Teil von Composite-Animationen verwendet.
     * 
     * @param cmd Der zu prüfende Befehl
     * @return false (unterstützt keine direkten Befehle)
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return false;
    }
    
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        startTime = millis();
    }
    
    /**
     * @brief Führt einen Animationsschritt aus
     * 
     * Prüft, ob die Wartezeit abgelaufen ist.
     * 
     * @param currentTime Aktuelle Zeit in Millisekunden
     * @return true wenn der Schritt erfolgreich ausgeführt wurde
     */
    bool run(uint32_t currentTime) override {
        // Prüfe, ob die Animation abgebrochen wurde
        if (cancelRequested) {
            completed = true;
            progress = 1.0f;
            return true;
        }
        
        // Berechne die verstrichene Zeit
        uint32_t elapsedTime = currentTime - startTime;
        
        // Aktualisiere den Fortschritt
        progress = min(1.0f, (float)elapsedTime / duration);
        
        // Prüfe, ob die Wartezeit abgelaufen ist
        if (elapsedTime >= duration) {
            completed = true;
            progress = 1.0f;
        }
        
        return true;
    }
};

#endif // WAIT_ANIMATION_H
