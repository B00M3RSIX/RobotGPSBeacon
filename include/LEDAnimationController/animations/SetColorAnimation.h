#ifndef SET_COLOR_ANIMATION_H
#define SET_COLOR_ANIMATION_H

#include "../AnimationBase.h"

/**
 * @brief Animation zum Setzen einer Farbe für alle LEDs
 * 
 * Diese Animation setzt alle LEDs auf eine bestimmte Farbe.
 * Sie ist vom Typ IMMEDIATE und wird sofort abgeschlossen.
 */
class SetColorAnimation : public AnimationBase {
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    SetColorAnimation(CRGB* leds, AnimationContext* context) 
        : AnimationBase(leds, context) {
    }
    
    /**
     * @brief Gibt den Typ der Animation zurück
     * 
     * @return ANIMATION_IMMEDIATE
     */
    AnimationType getType() const override {
        return ANIMATION_IMMEDIATE;
    }
    
    /**
     * @brief Prüft, ob die Animation einen bestimmten Befehl unterstützt
     * 
     * @param cmd Der zu prüfende Befehl
     * @return true wenn cmd == CMD_SET_COLOR
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return cmd == CMD_SET_COLOR;
    }
    
    /**
     * @brief Führt die Animation aus
     * 
     * Setzt alle LEDs auf die in den Parametern angegebene Farbe.
     * 
     * @param currentTime Aktuelle Zeit in Millisekunden (wird nicht verwendet)
     * @return true wenn die Animation erfolgreich ausgeführt wurde
     */
    bool run(uint32_t currentTime) override {
        // Get the color from the command structure

        CRGB color = context->cmd.Color;
        
        // Set all LEDs to this color
        fill_solid(leds, context->numLeds, color);
   

        // Show the change
        FastLED.show();

        // Mark the animation as completed
        completed = true;
        progress = 1.0f;
        
        return true;
    }
};

#endif // SET_COLOR_ANIMATION_H
