#define USEBLINKK
#ifdef USEBLINKK
#ifndef BLINK_ANIMATION_H
#define BLINK_ANIMATION_H

#include "../AnimationBase.h"
#include "CompositeAnimation.h"
#include "SetColorAnimation.h"
#include "AllOffAnimation.h"

/**
 * @brief Blink-Animation
 * 
 * Diese Animation lässt die LEDs in einer bestimmten Farbe blinken.
 * Sie verwendet die CompositeAnimation, um SetColorAnimation und AllOffAnimation
 * zu kombinieren und zu wiederholen.
 * Sie ist vom Typ ONETIME und wird nach einer bestimmten Anzahl von Wiederholungen abgeschlossen.
 */
class BlinkAnimation : public AnimationBase {
private:
    CompositeAnimation* sequence;  // Composite-Animation für einen Blink-Zyklus
    int repeatCount;               // Anzahl der Wiederholungen
    int currentRepeat;             // Aktuelle Wiederholung
    
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    BlinkAnimation(CRGB* leds, AnimationContext* context) 
        : AnimationBase(leds,  context)
        , repeatCount(1)
        , currentRepeat(0) {
        
    }
    
    /**
     * @brief Destruktor
     * 
     * Gibt die Composite-Animation frei.
     */
    ~BlinkAnimation() {
        delete sequence;
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
     * @param cmd Der zu prüfende Befehl
     * @return true wenn cmd == CMD_BLINK
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return cmd == CMD_BLINK;
    }
       
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        currentRepeat = 0;
        if (context->cmd.MultiUseTag1 > 0) {
            repeatCount = context->cmd.MultiUseTag1 ;
        } else {
            repeatCount = 1;
        }      

        // Erstelle die Composite-Sequenz
        sequence = new CompositeAnimation(leds, context);
        
        // Baue das Blink-Muster auf
        sequence->addAnimation(new SetColorAnimation(leds, context));  // Einschalten
        sequence->addWait(context->cmd.speed*5);                      // Warten
        sequence->addAnimation(new AllOffAnimation(leds,  context));    // Ausschalten
        sequence->addWait(context->cmd.speed*5);                      // Warten
        sequence->start();

    }
    
    /**
     * @brief Bricht die Animation ab
     */
    void onCancel() override {
        sequence->cancel();
    }
    
    /**
     * @brief Führt einen Animationsschritt aus
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
        
        // Führe die Sequenz aus
        sequence->run(currentTime);
        
        // Wenn die Sequenz abgeschlossen ist, aber noch Wiederholungen übrig sind
        if (sequence->isCompleted()) {
            currentRepeat++;
            
            // Aktualisiere den Fortschritt
            progress = (float)currentRepeat / repeatCount;
            
            if (currentRepeat < repeatCount) {
                // Starte die Sequenz neu
                sequence->start();
            } else {
                // Alle Wiederholungen abgeschlossen
                completed = true;
                progress = 1.0f;
            }
        }
        
        return true;
    }
};

#endif // BLINK_ANIMATION_H

#endif 
