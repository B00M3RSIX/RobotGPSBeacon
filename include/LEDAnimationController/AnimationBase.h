#ifndef ANIMATION_BASE_H
#define ANIMATION_BASE_H

#include "Animation.h"

/**
 * @brief Basisklasse für Animationen
 * 
 * Diese Klasse implementiert gemeinsame Funktionalität für alle Animationen
 * und reduziert so den Implementierungsaufwand für konkrete Animationsklassen.
 */
class AnimationBase : public Animation {
protected:
    CRGB* leds;                  // Zeiger auf das LED-Array
    AnimationContext* context;   // Zeiger auf den Animations-Kontext
    bool completed;              // Flag für Animationsabschluss
    bool cancelRequested;        // Flag für Abbruchanforderung
    float progress;              // Fortschritt der Animation (0.0 - 1.0)
    uint32_t lastUpdateTime;     // Zeitpunkt des letzten Updates
    uint32_t stepDuration;       // Dauer eines Animationsschritts

    
public:
    /**
     * @brief Konstruktor
     * 
     * @param ctx Zeiger auf den Animations-Kontext
     */
    AnimationBase(CRGB* ledArray, AnimationContext* ctx) 
        : leds(ledArray)
        , context(ctx)
        , completed(false)
        , cancelRequested(false)
        , progress(0.0f)
        , lastUpdateTime(0)
        , stepDuration(20) 
        

    {
    }
   
    /**
     * @brief Virtueller Destruktor
     */
    virtual ~AnimationBase() = default;

    /**
     * @brief Initialisiert die Animation mit Parametern
     * 
     * @return true wenn die Initialisierung erfolgreich war
     */
    bool setup() override {
        // Rufe die spezifische Setup-Methode der abgeleiteten Klasse auf
        onSetup();
        return true;
    }
    
    /**
     * @brief Startet die Animation
     * 
     * @return true wenn der Start erfolgreich war
     */
    bool start() override {
        completed = false;
        cancelRequested = false;
        progress = 0.0f;
        // Setze die Schrittdauer basierend auf dem Geschwindigkeitsparameter
        stepDuration = context->cmd.speed > 0 ? context->cmd.speed : 20;
        lastUpdateTime = millis();
        
        // Rufe die spezifische Start-Methode der abgeleiteten Klasse auf
        onStart();
        return true;
    }
    
    /**
     * @brief Bricht die Animation ab
     * 
     * @return true wenn der Abbruch erfolgreich war
     */
    bool cancel() override {
        cancelRequested = true;
        onCancel();
        return true;
    }
    
    /**
     * @brief Prüft, ob die Animation abgeschlossen ist
     * 
     * @return true wenn die Animation abgeschlossen ist
     */
    bool isCompleted() const override {
        return completed || cancelRequested;
    }
    
    /**
     * @brief Gibt den Fortschritt der Animation zurück
     * 
     * @return Fortschritt als Wert zwischen 0.0 und 1.0
     */
    float getProgress() const override {
        return progress;
    }
    
protected:
    /**
     * @brief Wird von setup() aufgerufen, kann in abgeleiteten Klassen überschrieben werden
     */
    virtual void onSetup() {}
    
    /**
     * @brief Wird von start() aufgerufen, kann in abgeleiteten Klassen überschrieben werden
     */
    virtual void onStart() {}
    
    /**
     * @brief Wird von cancel() aufgerufen, kann in abgeleiteten Klassen überschrieben werden
     */
    virtual void onCancel() {}
    
    /**
     * @brief Hilfsmethode zum Dimmen aller LEDs
     * 
     * @param scaledown Skalierungsfaktor (0-255)
     */
    void fadeall(uint8_t scaledown) {
        for(int i = 0; i < context->numLeds; i++) { 
            leds[i].nscale8(scaledown);
        }
    }
};

#endif // ANIMATION_BASE_H