#ifndef ALLFADEIN_ANIMATION_H
#define ALLFADEIN_ANIMATION_H

#include "../AnimationBase.h"

/**
 * @brief All fade in Animation
 * 
 * Diese Animation Faded alle LEDS bis zu RGB
 * Wahlweise von schwarz beginnend
 * 
 * @note Wichtig: Der OLDAnimationContext muss zwei 2D-Arrays enthalten:
 * old_frame und new_frame, die mit [led_index][color_component] zugreifbar sind
 */
class AllFadeInAnimation : public AnimationBase {
private:
    uint8_t currentStep;     // Aktueller Schritt der Animation
    uint8_t totalSteps;      // Gesamtzahl der Schritte
    
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    AllFadeInAnimation(CRGB* leds, AnimationContext* context) 
    : AnimationBase(leds,  context)
        , currentStep(0)
        , totalSteps(100)
   {
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
     * @return true wenn cmd == CMD_ALL_FADE_IN
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return cmd == CMD_ALL_FADE_IN;
    }
    
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        currentStep = 0;
        totalSteps = context->cmd.MultiUseTag1;
        if (context->cmd.para.startFromBlack) {
            // Lösche alle LEDs zu Beginn
            context->saved.Color = CRGB::Black;
            fill_solid(context->saved.Frame, context->numLeds, CRGB::Black);
            fill_solid(leds, context->numLeds, CRGB::Black);
            FastLED.show();
        }
    }
    
    /**
     * @brief Führt einen Animationsschritt aus
     * 
     * @param currentTime Aktuelle Zeit in Millisekunden
     * @return true wenn der Schritt erfolgreich ausgeführt wurde
     */
    bool run(uint32_t currentTime) override {
        // Prüfe, ob die Animation bereits abgeschlossen ist
        if (completed || cancelRequested) {
            return true;
        }
        
        // Prüfe, ob es Zeit für den nächsten Schritt ist
        if (currentTime - lastUpdateTime < stepDuration) {
            return true; // Noch nicht Zeit für Update, aber erfolgreich
        }
        
        // Prüfe, ob alle Schritte abgeschlossen sind
        if (currentStep >= totalSteps) {
            completed = true;
            progress = 1.0f;
            return true;
        }
        
          
        if (currentStep < totalSteps ) {
            // Match the original array access style from LEDAnimationController.cpp
            for(int b = 0; b < context->numLeds; b++) { 
                // Überprüfen ob OLDcontext nicht null ist
                    leds[b].r = context->saved.Frame[b].r + (context->cmd.Frame[b].r - context->saved.Frame[b].r) * currentStep / (totalSteps );
                    leds[b].g = context->saved.Frame[b].g + (context->cmd.Frame[b].g - context->saved.Frame[b].g) * currentStep / (totalSteps );
                    leds[b].b = context->saved.Frame[b].b + (context->cmd.Frame[b].b - context->saved.Frame[b].b) * currentStep / (totalSteps );
            }
        } else {
            // Verwende context->numLeds anstatt NUM_LEDS und überprüfe OLDcontext
            fill_solid(leds, context->numLeds, context->cmd.Color);
        }
        FastLED.show();  
        
        // Aktualisiere den Fortschritt
        currentStep++;
        progress = (float)currentStep / totalSteps;
        
        // Aktualisiere die Zeit für den nächsten Schritt
        lastUpdateTime = currentTime;
        
        return true;
    }
};

#endif // ALLFADEIN_ANIMATION_H