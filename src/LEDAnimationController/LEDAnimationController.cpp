#include "LEDAnimationController/LEDAnimationController.h"
#define USE_WS2812SERIAL
// Include the Animation header here to resolve the forward declaration

LEDAnimationController::LEDAnimationController() {
    
    // Initialize state
    status = STATUS_INIT;
    
    // Initialize callbacks
    eventCallback = nullptr;
    feedbackCallback = nullptr;
    resultCallback = nullptr;
    
    // Initialize animation pointers
    currentAnimation = nullptr;

}

LEDAnimationController::~LEDAnimationController() {
    
    // Clean up all animations in the registry
    for (Animation* anim : animationRegistry) {
        delete anim;
    }
    animationRegistry.clear();
    
    // Current animation is just a pointer to one in the registry, don't delete it separately
    currentAnimation = nullptr;
}

void LEDAnimationController::begin() {
    // Initialize LED strip
    FastLED.addLeds<WS2812SERIAL, DATA_PIN, RGB>(leds, context.numLeds);
    fill_solid(leds, context.numLeds, CRGB::Black);
    FastLED.show();
    // Register all animations
    registerAnimations();
    UpdateStatus(status,STATUS_IDLE);
}

void LEDAnimationController::registerAnimations() {
    // Create and register all animation types
    ::registerAnimations(animationRegistry, leds,  &context);

    // Initial Setup of all animations... e.g. Animation pre-calculation or filling patternArray etc.
    for (Animation* anim : animationRegistry) {
        anim->setup();
    }
    
}

void LEDAnimationController::startAnimation(uint32_t newCommandID, uint8_t* newParams) {

    uint8_t paraCpy[NUM_LEDS] = {};
    memcpy(paraCpy, newParams, PARAM_COUNT * sizeof(paraCpy[0]));
       
    // Set global brightness
    FastLED.setBrightness(paraCpy[PARAM_BRIGHTNESS]);

    //Fill the new context
    //Save old command
    memcpy(&context.saved, &context.cmd, sizeof(CMD_struct));

    //New command
    context.cmd.Animation = AnimationCommand(paraCpy[PARAM_CMD]);
    context.cmd.Color = CRGB(paraCpy[PARAM_RED],paraCpy[PARAM_GREEN],paraCpy[PARAM_BLUE]);
    fill_solid(context.cmd.Frame, context.numLeds, context.cmd.Color);
    context.cmd.speed = paraCpy[PARAM_SPEED] > 0 ? paraCpy[PARAM_SPEED] : 20;
    context.cmd.MultiUseTag1 = paraCpy[PARAM_MULTIUSE1];
    //New command parameter
    context.cmd.para.reversDirection = (paraCpy[PARAM_MODIFIER] & PAR_DIRECTION_REVERSE) != 0;
    context.cmd.para.startFromBlack = (paraCpy[PARAM_MODIFIER] & PAR_START_FROM_BLACK) != 0;
    context.cmd.para.useHue = (paraCpy[PARAM_MODIFIER] & PAR_USE_HSV_COLOR) != 0;

     
    // Find the appropriate animation based on command
    currentAnimation = nullptr;    
    for (Animation* anim : animationRegistry) {
        if (anim->supportsCommand(context.cmd.Animation)) {
            currentAnimation = anim;         
            break;
        }
    }
    // No supported Animation has been found --> Report Error
    if (currentAnimation == nullptr) {       
        // Trigger callbacks if registered
        ReportStatus(STATUS_REVOKED);
        return;
    }
    ReportStatus(STATUS_ACCEPTED);
    

    // Cancel current animation if running
    if (currentAnimation != nullptr && isAnimationRunning()) {
        UpdateStatus(status, STATUS_CANCELED);
        currentAnimation->cancel();
    }

    UpdateStatus(status,STATUS_STARTED);
    
    // Trigger feedback callback if registered
    if (feedbackCallback != nullptr) {
        feedbackCallback(0.0f, status);
    }
}

void LEDAnimationController::update() {
    // Skip if no animation is running
    if (currentAnimation == nullptr || 
        (status != STATUS_RUNNING && status != STATUS_STARTED && status != STATUS_RUNNING_CONTINUOUS)) {
        return;
    }
    
    // Transition from STARTED to RUNNING
    if (status == STATUS_STARTED) {

        //Set the running event
        UpdateStatus(status, (currentAnimation->getType() == ANIMATION_CONTINUOUS) ?  STATUS_RUNNING_CONTINUOUS : STATUS_RUNNING);       
        currentAnimation->start();

        // Trigger feedback callback if registered
        if (feedbackCallback != nullptr) {
            feedbackCallback(currentAnimation->getProgress(), status);
        }
    }
    
    // Run the animation
    uint32_t currentTime = millis();
    currentAnimation->run(currentTime);
    
    // Provide feedback if registered
    if (feedbackCallback != nullptr && (status == STATUS_RUNNING || status == STATUS_RUNNING_CONTINUOUS)) {
        feedbackCallback(currentAnimation->getProgress(), status);
    }
    
    // Check for animation completion (only for non-continuous animations)
    if (currentAnimation->isCompleted() && (currentAnimation->getType() != ANIMATION_CONTINUOUS)) {
        UpdateStatus(status, STATUS_COMPLETED);
    }
}

bool LEDAnimationController::stopCurrentAnimation() {
    if (currentAnimation == nullptr || !isAnimationRunning()) {
        // Trigger result callback if registered
        // Error as there is no running Animation
        ReportStatus(STATUS_REVOKED);
        return false;
    }

    // Force animation to stop
    currentAnimation->cancel();

    // Turn off all LEDs
    fill_solid(leds, context.numLeds, CRGB::Black);
    FastLED.show();
    UpdateStatus(status, STATUS_CANCELED);
    ReportStatus(STATUS_ACCEPTED);

    return true;

}

bool LEDAnimationController::cancelCurrentAnimation() {
    if (currentAnimation == nullptr || !isAnimationRunning()) {
        // Trigger result callback if registered
        // Error as there is no running Animation
        ReportStatus(STATUS_REVOKED);
        return false;
    }
    // Force animation to stop
    currentAnimation->cancel();   
    UpdateStatus(status, STATUS_CANCELED);
    
    // Trigger result callback if registered
    ReportStatus(STATUS_ACCEPTED);
    
    return true;
}

void LEDAnimationController::setEventCallback(AnimationEventCallback callback) {
    eventCallback = callback;
}

void LEDAnimationController::setFeedbackCallback(AnimationFeedbackCallback callback) {
    feedbackCallback = callback;
}

void LEDAnimationController::setResultCallback(AnimationResultCallback callback) {
    resultCallback = callback;
}

// Status getters
AnimationStatus LEDAnimationController::getStatus() {
    return status;
}

bool LEDAnimationController::isAnimationRunning() {
    return status == STATUS_RUNNING || status == STATUS_STARTED || status == STATUS_RUNNING_CONTINUOUS;
}

bool LEDAnimationController::isAnimationComplete() {
    return status == STATUS_COMPLETED;
}

float LEDAnimationController::getCurrentProgress() const {
    if (currentAnimation != nullptr) {
        return currentAnimation->getProgress();
    }
    return 0.0f;
}
// Parameter bit manipulation functions
void LEDAnimationController::setPara(uint8_t &parabyte, ParameterBits parameter) {
    parabyte |= parameter;  // Set the bit using OR operation
  }
  
  void LEDAnimationController::resetPara(uint8_t &parabyte, ParameterBits parameter) {
    parabyte &= ~parameter;  // Clear the bit using AND with inverted bits
  }
  
  void LEDAnimationController::setParaValue(uint8_t &parabyte, ParameterBits parameter, bool value) {
    if (value) {
      setPara(parabyte, parameter);  // Set the bit if value is true
    } else {
      resetPara(parabyte, parameter);  // Reset the bit if value is false
    }
  }
  
  // Get the current state of a parameter bit
  bool LEDAnimationController::getParaValue(uint8_t parabyte, ParameterBits parameter) {
    return (parabyte & parameter) != 0;  // Return true if the bit is set
  }