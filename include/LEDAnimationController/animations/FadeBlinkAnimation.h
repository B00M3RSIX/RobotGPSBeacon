#define USEBLINKK
#ifdef USEBLINKK
#ifndef FADEBLINK_ANIMATION_H
#define FADEBLINK_ANIMATION_H

#include "../AnimationBase.h"
#include "CompositeAnimation.h"
#include "AllFadeInAnimation.h"

/**
 * @brief Blink-Animation
 * 
 * Diese Animation lässt die LEDs in einer bestimmten Farbe blinken indem es Faded.
 * Sie verwendet die CompositeAnimation, um SetColorAnimation und AllOffAnimation
 * zu kombinieren und zu wiederholen.
 * Sie ist vom Typ ONETIME und wird nach einer bestimmten Anzahl von Wiederholungen abgeschlossen.
 */
class FadeBlinkAnimation : public AnimationBase {
private:
    CompositeAnimation* sequence;  // Composite-Animation für einen Blink-Zyklus
    int repeatCount;               // Anzahl der Wiederholungen
    int currentRepeat;             // Aktuelle Wiederholung
    AnimationContext ctx1;
    CMD_struct tmp;

public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    FadeBlinkAnimation(CRGB* leds, AnimationContext* context) 
        : AnimationBase(leds,  context)
        , repeatCount(1)
        , currentRepeat(0) {
        
    }
    
    /**
     * @brief Destruktor
     * 
     * Gibt die Composite-Animation frei.
     */
    ~FadeBlinkAnimation() {
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
        return cmd == CMD_FADE_BLINK;
    }
       
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        if (context->cmd.para.startFromBlack) {
            // Lösche alle LEDs zu Beginn
            fill_solid(context->saved.Frame, context->numLeds, CRGB::Black);
            context->cmd.Color = CRGB::Black;
            fill_solid(leds, context->numLeds, CRGB::Black);
            FastLED.show();
        }

        currentRepeat = 0;
        if (context->cmd.MultiUseTag1 > 0) {
            repeatCount = context->cmd.MultiUseTag1 ;
        } else {
            repeatCount = 1;
        }      

        // Erstelle die Composite-Sequenz
        context->saved.para.startFromBlack = false;
        context->cmd.para.startFromBlack = false;
        context->cmd.MultiUseTag1 = 150;
        memcpy(&ctx1, context, sizeof(AnimationContext));

        sequence = new CompositeAnimation(leds, context);
    
        // Baue das Blink-Muster auf
        sequence->addAnimation(new AllFadeInAnimation(leds, context));  // Einschalten
        //sequence->addWait(1000);                      // Warten

        memcpy(&tmp, &ctx1.saved, sizeof(CMD_struct));    
        memcpy(&ctx1.saved, &ctx1.cmd, sizeof(CMD_struct));
        memcpy(&ctx1.cmd, &tmp, sizeof(CMD_struct));
        ctx1.cmd.para.startFromBlack = false;
        ctx1.saved.para.startFromBlack = false;
        ctx1.cmd.MultiUseTag1 = 150;
        ctx1.saved.Color= context->cmd.Color;
        sequence->addAnimation(new AllFadeInAnimation(leds,  &ctx1));    // Ausschalten
        //sequence->addWait(1000);                      // Warten 
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

#endif // FADEBLINK_ANIMATION_H

#endif 
