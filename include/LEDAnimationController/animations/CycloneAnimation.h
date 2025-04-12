#ifndef CYCLONE_ANIMATION_H
#define CYCLONE_ANIMATION_H

#include "../AnimationBase.h"

/**
 * @brief Cyclone-Animation
 * 
 * Diese Animation bewegt einen Lichtpunkt über die LEDs und lässt einen Schweif zurück.
 * Sie ist vom Typ ONETIME und wird nach einem vollständigen Durchlauf abgeschlossen.
 * params[PARAM_MULTIUSE1] controlliert die länge des Schweifs höher = kürzerer Schweif
 * im HSV Modus (PAR_USE_HSV_COLOR = FALSE (DEFAULT)) wird RGB als Start für hue verwendent
 * 
 * Some nice Side Effekt with not starting from Black (works best with solid color before)
 * 1. Mulituse1 = 0 (no fading) is lika a circular Fade to solid color
 * 2. Mulituse1 = 255 (full fading) is lika a single Pixel eating up all others
 * 
 */
class CycloneAnimation : public AnimationBase {
private:
    uint8_t currentStep;     // Aktueller Schritt der Animation
    uint8_t totalSteps;      // Gesamtzahl der Schritte
    bool isContinous;        // Animaiton im Continues mode 
    uint8_t firstCylePos;    // Position im ersten Zyklus (für <> PAR_START_FROM_BLACK)  
    
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    CycloneAnimation(CRGB* leds,  AnimationContext* context, uint8_t startHue = 0) 
    : AnimationBase(leds, context) 
        , currentStep(0)
        , totalSteps(context->numLeds)
        , isContinous(false)
        , firstCylePos(0)
        , hue(startHue) 
        {
            
    }

    uint8_t hue;             // Farbton für den HSV-Farbmodus

    /**
     * @brief Gibt den Typ der Animation zurück
     * 
     * @return ANIMATION_ONETIME
     */
    AnimationType getType() const override {
        return (context->cmd.Animation == CMD_CONTINUOUS_CYCLONE) ? ANIMATION_CONTINUOUS : ANIMATION_ONETIME;   ;
    }
    
    /**
     * @brief Prüft, ob die Animation einen bestimmten Befehl unterstützt
     * 
     * @param cmd Der zu prüfende Befehl
     * @return true wenn cmd == CMD_CYCLONE
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return (cmd == CMD_CYCLONE) || (cmd == CMD_CONTINUOUS_CYCLONE);
    }
    
    /**
     * @brief Startet die Animation
     */
    void onStart() override {
        currentStep = 0;
        totalSteps = context->numLeds;
        isContinous = (context->cmd.Animation == CMD_CONTINUOUS_CYCLONE) ;
        CRGB color = context->cmd.Color;
        firstCylePos = 0;
        Serial.println("OnSTART");
        
        hue = rgb2hsv_approximate(color).hue;  
        
/*         Serial.print("RGB: r: ");
        Serial.print(context->cmd.Color.r);
        Serial.print(", g: ");
        Serial.print(context->cmd.Color.g);
        Serial.print(", b: ");
        Serial.print(context->cmd.Color.b);       
        Serial.print (" HUE: ");
        Serial.println (hue); */
       
        if(context->cmd.para.startFromBlack == true){
            // Lösche alle LEDs zu Beginn
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
            if (isContinous){
                completed = false;
                currentStep =0;
            } else {
                completed = true; 
            }
            
            progress = 1.0f;
            return true;
        }
        
        // Berechne den Index basierend auf der Richtung
        int idx = currentStep;
        if (context->cmd.para.reversDirection) {
            idx = context->numLeds - 1 - currentStep;
        }
        
        // Setze die Farbe basierend auf dem Farbmodus
        if (context->cmd.para.useHue == false) {
            // RGB-Farbmodus
            leds[idx] = context->cmd.Color;
        } else {
            // HSV-Farbmodus mit wechselndem Farbton
            //rgb2hsv_approximate()
            leds[idx] = CHSV(hue, 255, 255);
            hue+=5;
        }
        
        // Zeige die Änderung an
        FastLED.show();
        
        // Dimme LEDs für den Schweifeffekt
        if ((firstCylePos < context->numLeds) &&(firstCylePos < context->numLeds -1)) {
            firstCylePos = currentStep;

            if (context->cmd.para.reversDirection) {

                for(int i = idx  ;  i < context->numLeds ; i++) { 

                    leds[i].nscale8((255-context->cmd.MultiUseTag1));
                }
            } else {
             
                for(int i = 0; i < currentStep; i++) { 

                    leds[i].nscale8((255-context->cmd.MultiUseTag1));  
                }
            }


        } else {

            for(int i = 0; i < context->numLeds; i++) { 
                
                    leds[i].nscale8((255-context->cmd.MultiUseTag1));
                
            }
        }



        
        // Aktualisiere den Fortschritt
        currentStep++;
        progress = (float)currentStep / totalSteps;
        
        // Aktualisiere die Zeit für den nächsten Schritt
        lastUpdateTime = currentTime;
        
        return true;
    }
};

#endif // CYCLONE_ANIMATION_H
