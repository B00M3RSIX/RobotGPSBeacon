#ifndef ANIMATIONSREGISTRY_H
#define ANIMATIONSREGISTRY_H

#include "LEDAnimationController.h"

// Basis-Klassen
#include "Animation.h"
#include "AnimationBase.h"

// Einfache Animationen
#include "animations/SetColorAnimation.h"
#include "animations/AllOffAnimation.h"
#include "animations/CycloneAnimation.h"
#include "animations/RGBNoiseAnimation.h"
#include "animations/AllFadeInAnimation.h"
#include "animations/RGBRainbowAnimation.h"

// Composite-Animationen
#include "animations/WaitAnimation.h"
#include "animations/CompositeAnimation.h"
#include "animations/BlinkAnimation.h" 
#include "animations/FadeBlinkAnimation.h" 


/**
 * @brief Registriert alle verfügbaren Animationen beim Controller
 * 
 * Diese Funktion erzeugt alle Animation-Objekte und fügt sie
 * zur Registry des Controllers hinzu. Neue Animationen sollten
 * hier hinzugefügt werden.
 * 
 * @param controller Referenz auf den LED-AnimationController
 * @param leds Zeiger auf das LED-Array
 * @param context current Animation context
 */
inline void registerAnimations(std::vector<Animation*>& animationRegistry, CRGB* leds, AnimationContext* context) {
    animationRegistry.push_back(new SetColorAnimation(leds,context));
    animationRegistry.push_back(new AllOffAnimation(leds, context));
    animationRegistry.push_back(new CycloneAnimation(leds, context));
    animationRegistry.push_back(new RGBNoiseAnimation(leds, context));
    animationRegistry.push_back(new BlinkAnimation(leds, context)); 
    animationRegistry.push_back(new AllFadeInAnimation(leds, context));
    animationRegistry.push_back(new RGBRainbowAnimation(leds, context));
    animationRegistry.push_back(new FadeBlinkAnimation(leds, context)); 
       

    /*
    Knightrider AllFadeInAnimation with pixerl Start end offset?
    
    */
}

#endif // ANIMATIONSREGISTRY_H
