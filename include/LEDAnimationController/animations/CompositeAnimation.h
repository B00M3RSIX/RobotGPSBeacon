#ifndef COMPOSITE_ANIMATION_H
#define COMPOSITE_ANIMATION_H
#include "../AnimationTypeEnums.h"
#include "../Animation.h"
#include "WaitAnimation.h"
#include <vector>

/**
 * @brief Composite-Animation
 * 
 * Diese Klasse ermöglicht die Kombination mehrerer Animationen zu einer Sequenz.
 * Die Animationen werden nacheinander ausgeführt.
 * Sie ist vom Typ ONETIME und wird abgeschlossen, wenn alle Teilanimationen abgeschlossen sind.
 */
class CompositeAnimation : public Animation {
private:
    std::vector<Animation*> sequence;  // Sequenz von Animationen
    size_t currentIndex;               // Index der aktuellen Animation
    bool ownsAnimations;               // Flag, ob die Animationen beim Zerstören freigegeben werden sollen
    CRGB* leds;                        // Zeiger auf das LED-Array
    AnimationContext* context;         // Zeiger auf den Animations-Kontext
    bool completed;                    // Flag für Animationsabschluss
    bool cancelRequested;              // Flag für Abbruchanforderung
    float progress;                    // Fortschritt der Animation (0.0 - 1.0)
    
public:
    /**
     * @brief Konstruktor
     * 
     * @param ledArray Zeiger auf das LED-Array
     * @param ownsAnimationObjects Flag, ob die Animationen beim Zerstören freigegeben werden sollen
     */
    CompositeAnimation(CRGB* ledArray, AnimationContext* ctx, bool ownsAnimationObjects = true) 
        : currentIndex(0)
        , ownsAnimations(ownsAnimationObjects)
        , leds(ledArray)
        , context(ctx)
        , completed(false)
        , cancelRequested(false)
        , progress(0.0f) {
    }
    
    /**
     * @brief Destruktor
     * 
     * Gibt die Animationen frei, wenn ownsAnimations true ist.
     */
    ~CompositeAnimation() {
        if (ownsAnimations) {
            for (Animation* anim : sequence) {
                delete anim;
            }
        }
    }
   
    /**
     * @brief Fügt eine Animation zur Sequenz hinzu
     * 
     * @param anim Zeiger auf die hinzuzufügende Animation
     */
    void addAnimation(Animation* anim) {
        sequence.push_back(anim);
    }
    
    /**
     * @brief Fügt eine Wartezeit zur Sequenz hinzu
     * 
     * @param waitTime Wartezeit in Millisekunden
     */
    void addWait(uint32_t waitTime) {
        sequence.push_back(new WaitAnimation(leds, context, waitTime));
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
     * Die CompositeAnimation wird normalerweise nicht direkt durch Befehle ausgelöst,
     * sondern durch spezialisierte Wrapper-Klassen.
     * 
     * @param cmd Der zu prüfende Befehl
     * @return false (unterstützt keine direkten Befehle)
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return false;
    }
    
    /**
     * @brief Initialisiert die Animation mit Parametern
     * 
     * @return true wenn die Initialisierung erfolgreich war
     */
    bool setup() override {

        return true;
    }
    
    /**
     * @brief Startet die Animation
     * 
     * @param newParams Array von Parametern für die Animation
     * @return true wenn der Start erfolgreich war
     */
    bool start() override {
        currentIndex = 0;
        completed = false;
        cancelRequested = false;
        progress = 0.0f;
        
        // Starte die erste Animation, wenn vorhanden
        if (!sequence.empty()) {
            sequence[0]->start();
        }
        
        return true;
    }
    
    /**
     * @brief Führt einen Animationsschritt aus
     * 
     * Führt einen Schritt der aktuellen Animation aus und wechselt zur nächsten,
     * wenn die aktuelle abgeschlossen ist.
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
        
        // Prüfe, ob die Sequenz leer ist oder alle Animationen abgeschlossen sind
        if (sequence.empty() || currentIndex >= sequence.size()) {
            completed = true;
            progress = 1.0f;
            return true;
        }
        
        // Führe die aktuelle Animation aus
        Animation* current = sequence[currentIndex];
        Serial.println(currentIndex);
        current->run(currentTime);
        
        // Prüfe, ob die aktuelle Animation abgeschlossen ist
        if (current->isCompleted()) {
            currentIndex++;
            
            // Starte die nächste Animation, wenn vorhanden
            if (currentIndex < sequence.size()) {
                sequence[currentIndex]->start();
            } else {
                // Alle Animationen abgeschlossen
                completed = true;
                progress = 1.0f;
            }
        }
        
        // Berechne den Gesamtfortschritt
        if (!sequence.empty()) {
            float stepProgress = (float)currentIndex / sequence.size();
            float currentProgress = 0.0f;
            
            if (currentIndex < sequence.size()) {
                currentProgress = sequence[currentIndex]->getProgress() / sequence.size();
            }
            
            progress = stepProgress + currentProgress;
        }
        
        return true;
    }
    
    /**
     * @brief Bricht die Animation ab
     * 
     * @return true wenn der Abbruch erfolgreich war
     */
    bool cancel() override {
        cancelRequested = true;
        
        // Brich die aktuelle Animation ab, wenn vorhanden
        if (currentIndex < sequence.size()) {
            sequence[currentIndex]->cancel();
        }
        
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
};

#endif // COMPOSITE_ANIMATION_H
