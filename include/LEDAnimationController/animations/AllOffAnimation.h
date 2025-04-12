#ifndef ALL_OFF_ANIMATION_H
#define ALL_OFF_ANIMATION_H

#include "../AnimationBase.h"

/**
 * @brief Animation zum Ausschalten aller LEDs
 * 
 * Diese Animation schaltet alle LEDs aus (setzt sie auf schwarz).
 * Sie ist vom Typ IMMEDIATE und wird sofort abgeschlossen.
 */
class AllOffAnimation : public AnimationBase {
public:
    /**
     * @brief Konstruktor
     * 
     * @param leds Zeiger auf das LED-Array
     * @param context current Animation context
     */
    AllOffAnimation(CRGB* leds, AnimationContext* context) 
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
     * @return true wenn cmd == CMD_ALL_OFF
     */
    bool supportsCommand(AnimationCommand cmd) const override {
        return cmd == CMD_ALL_OFF;
    }
    
    /**
     * @brief Führt die Animation aus
     * 
     * Schaltet alle LEDs aus (setzt sie auf schwarz).
     * 
     * @param currentTime Aktuelle Zeit in Millisekunden (wird nicht verwendet)
     * @return true wenn die Animation erfolgreich ausgeführt wurde
     */
    bool run(uint32_t currentTime) override {
        // Setze alle LEDs auf schwarz
        fill_solid(leds, context->numLeds, CRGB::Black);
        
        // Zeige die Änderung an
        FastLED.show();
        
        // Markiere die Animation als abgeschlossen
        completed = true;
        progress = 1.0f;
        
        return true;
    }
};

#endif // ALL_OFF_ANIMATION_H
