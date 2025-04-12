#ifndef FASTLED_ANIMATIONCONTROLLER_H
#define FASTLED_ANIMATIONCONTROLLER_H

#include <Arduino.h>
#include <WS2812Serial.h>
#define USE_WS2812SERIAL
#include <FastLED.h>
#include <vector>

// LED-Konfiguration
#define NUM_LEDS 36 
#define DATA_PIN 1

#define PARAM_COUNT  8  // Gesamtzahl der Parameter

// Einbinden der gemeinsamen Typen und Definitionen
#include "LEDAnimationController/AnimationTypeEnums.h"
#include "LEDAnimationController/AnimationRegistry.h"

// Forward declare Animation class to avoid circular dependency
class Animation;

/**
 * @brief Controller für LED-Animationen
 * 
 * Diese Klasse verwaltet LED-Animationen über ein objektorientiertes Framework.
 * Sie verwendet eine Registry von Animationsobjekten und delegiert die
 * Animationslogik an spezialisierte Klassen.
 */
class LEDAnimationController {
public:
  // Event callback support
  typedef void (*AnimationEventCallback)(AnimationStatus);
  
  // Action Server callback support
  typedef void (*AnimationFeedbackCallback)(float progress, AnimationStatus status);
  typedef void (*AnimationResultCallback)(bool success, AnimationStatus finalStatus);

  LEDAnimationController();
  ~LEDAnimationController(); // Add destructor to clean up
  
  // Main methods
  void begin();               // Setup of the Controller, sets all LED's to Black
  void startAnimation(uint32_t commandID, uint8_t* newParams); // Stat a New Animation
  void update();                    // Call this in every loop iteration
  bool cancelCurrentAnimation();    // Aborts the current Animation
  bool stopCurrentAnimation();      // Aborts the current Animation but sets all LED's to Black
  
  
  // Status getters for action server integration
  AnimationStatus getStatus();
  bool isAnimationRunning();
  bool isAnimationComplete();
  float getCurrentProgress() const;
  
   
  // Set callback for animation events
  void setEventCallback(AnimationEventCallback callback);
  
  // Set callbacks for Action Server integration
  void setFeedbackCallback(AnimationFeedbackCallback callback);
  void setResultCallback(AnimationResultCallback callback);

  // Parameter bit manipulation functions
  void setPara(uint8_t &parabyte, ParameterBits parameter);
  void resetPara(uint8_t &parabyte, ParameterBits parameter);
  void setParaValue(uint8_t &parabyte, ParameterBits parameter, bool value);
  
  // Additional helper function to check parameter state
  bool getParaValue(uint8_t parabyte, ParameterBits parameter);

private:

  CRGB leds[NUM_LEDS];       // Define the array of leds

  // Event callbacks
  AnimationEventCallback eventCallback;
  AnimationFeedbackCallback feedbackCallback;
  AnimationResultCallback resultCallback;
    
  // The animation context as a class member
  AnimationContext context;
  AnimationStatus status;          // Current animation status
  
  // Animation registry and current animation
  std::vector<Animation*> animationRegistry;
  Animation* currentAnimation;
  
  // Register all animations
  void registerAnimations();

  // Then create a helper method to call the callback and handle the status

 void UpdateStatus(AnimationStatus &status, AnimationStatus newStatus) {
      status = newStatus;     
      if (eventCallback != nullptr) { eventCallback(status); }
  }
 
 void UpdateStatus(AnimationStatus &status) {   
      if (eventCallback != nullptr) { eventCallback(status); }
  }


 void ReportStatus(CmdFeedback value) {   
      if (resultCallback != nullptr) { 
        resultCallback(value == STATUS_ACCEPTED, status);
      }
  }
 
};




#endif // FASTLED_ANIMATIONCONTROLLER_H
