#ifndef RGBRAINBOW_ANIMATION_H
#define RGBRAINBOW_ANIMATION_H

#include "../AnimationBase.h"

/**
 * @brief Rainbow Cycle
 * 
 * Diese Animation erzeugt einen Regenbogenverlauf
 * 
 * @note Wichtig: Der OLDAnimationContext muss zwei 2D-Arrays enthalten:
 * old_frame und new_frame, die mit [led_index][color_component] zugreifbar sind
 */
class RGBRainbowAnimation : public AnimationBase {
private:
    uint8_t currentStep;     // Aktueller Schritt der Animation
    uint8_t totalSteps;      // Gesamtzahl der Schritte

    CRGB Wheel(byte WheelPos) {
        WheelPos = 255 - WheelPos;
        if(WheelPos < 85) {
          return CRGB(255 - WheelPos * 3, 0, WheelPos * 3);
        }
        if(WheelPos < 170) {
          WheelPos -= 85;
          return CRGB(0, WheelPos * 3, 255 - WheelPos * 3);
        }
        WheelPos -= 170;
        return CRGB(WheelPos * 3, 255 - WheelPos * 3, 0);
      }  
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    RGBRainbowAnimation(CRGB* leds, AnimationContext* context) 
    : AnimationBase(leds,  context)
        , currentStep(0)
        , totalSteps(255)
   {
    }
    
    /**
     * @brief Gibt den Typ der Animation zurück
     * 
     * @return ANIMATION_CONTINUOUS
     */
    AnimationType getType() const override {
        return ANIMATION_CONTINUOUS;
    }
    
    /**
     * @brief Prüft, ob die Animation einen bestimmten Befehl unterstützt
     * 
     * @param cmd Der zu prüfende Befehl
     * @return true wenn cmd == CMD_ALL_FADE_IN
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return (cmd == CMD_RGB_RAINBOW);
    }
    
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        currentStep = 0;
        totalSteps = context->cmd.MultiUseTag1;
        if (context->cmd.para.startFromBlack) {
            // Lösche alle LEDs zu Beginn
            fill_solid(context->saved.Frame, context->numLeds, CRGB::Black);
            fill_solid(leds, context->numLeds, CRGB::Black);
            FastLED.show();
        }
    }

    uint8_t hue;             // Farbton für den HSV-Farbmodus
    
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
        


        // Update base hue only on certain frames for slower movement
        if ((currentStep % 3 ) == 0) {
            hue = (hue + context->cmd.MultiUseTag1) % 255;
        }

        
        // Use bit manipulation function for direction control
        fill_rainbow_circular(leds, context->numLeds, hue, context->cmd.para.reversDirection);
        FastLED.show();  
        
        // Aktualisiere den Fortschritt
        currentStep++;
        progress = (float)currentStep / totalSteps;
        
        // Aktualisiere die Zeit für den nächsten Schritt
        lastUpdateTime = currentTime;
        
        return true;
    }

};

#endif // RGBRAINBOW_ANIMATION_H