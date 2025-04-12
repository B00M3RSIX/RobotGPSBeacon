#ifndef ANIMATION_TYPEENUM_H
#define ANIMATION_TYPEENUM_H

#include <Arduino.h>

// Enum for animation commands
enum AnimationCommand {
  // Immediate animations
  CMD_ALL_OFF = 0,
  CMD_SET_COLOR = 2,
  
  // One-time animations
  CMD_ALL_FADE_IN = 12,
  CMD_CYCLONE = 13,

  // Continuous animations
  CMD_BLINK = 100,
  CMD_FADE_BLINK = 101,
  CMD_RGB_NOISE = 150,
  CMD_CONTINUOUS_CYCLONE = 151,
  CMD_PACIFICA= 153,
  CMD_RGB_RAINBOW = 154
};

// Enum for parameter bits
enum ParameterBits {
  // General purpose
  PAR_DIRECTION_REVERSE = 0x1,
  // Animation Start with LED's off
  PAR_START_FROM_BLACK = 0x2,
  // Color specific
  PAR_USE_HSV_COLOR = 0x4
};

// Enum for animation parameter indices
enum AnimationParams {
  PARAM_RED = 0,
  PARAM_GREEN = 1,
  PARAM_BLUE = 2,
  PARAM_BRIGHTNESS = 3,
  PARAM_CMD = 4,
  PARAM_SPEED = 5,
  PARAM_MODIFIER = 6,  
  PARAM_MULTIUSE1 = 7  
};

// Enum for animation types
enum AnimationType {
  ANIMATION_NONE = 0,
  ANIMATION_IMMEDIATE = 1,
  ANIMATION_ONETIME = 2,
  ANIMATION_CONTINUOUS = 3
};

// Enum for animation status (for feedback)
enum AnimationStatus {
  STATUS_IDLE = 0,
  STATUS_STARTED = 1,             // Animation has just started
  STATUS_RUNNING = 2,             // Animation is in progress - will stop automatically
  STATUS_RUNNING_CONTINUOUS = 3,  // Animation is in progress - will continue forever
  STATUS_COMPLETED = 4,           // Animation has completed
  STATUS_ERROR = 5,               // An error occurred
  STATUS_CANCELED = 6,             // An running Animation was Canceled
  STATUS_INIT = 255
};

// Enum for animation status (for feedback)
enum CmdFeedback {
  STATUS_REVOKED = 0,
  STATUS_ACCEPTED = 1,             
};

/**
 * Animation parameter configuration structure
 * Controls behavior modifiers for various animations
 */
struct CMD_para {
  /**
   * Controls animation direction
   * When true, animation runs in reverse direction
   * When false, animation runs in forward direction (default)
   */
  bool reversDirection = false;
  
  /**
   * Controls initial animation state
   * When true, animation begins with all LEDs off (default)
   * When false, animation preserves current LED state
   */
  bool startFromBlack = true;
  
  /**
   * Controls color mode for certain animations
   * When true, uses HSV color space instead of RGB
   * When false, uses RGB color space (default)
   */
  bool useHue = false;
};

/**
 * Curren Active Command
 */
struct CMD_struct {
  /** Current animation command */
  AnimationCommand Animation = CMD_ALL_OFF;
  
  /** RGB color value for the animation */
  CRGB Color = CRGB::Black;
  
  /** Array of RGB values for each LED in the animation sequence */
  CRGB Frame[NUM_LEDS] = {CRGB::Black};
  
  /** Animation parameters (direction, starting state, color mode) */
  CMD_para para;
  
  /** 
   * Animation speed parameter
   * Behavior varies depending on the specific animation type
   */
  uint8_t speed = 20;
  
  /**
   * Multi-purpose tag that has different functions for different animations
   * Check specific animation documentation for usage details
   */
  uint8_t MultiUseTag1;
};

/**
 * Main context structure for LED animation system
 * Holds current state, saved state, and LED array information
 */
struct AnimationContext {
  /**
   * Currently active animation command and parameters
   * Contains all settings for the animation currently being displayed
   */
  CMD_struct cmd;
  
  /**
   * Backup of animation state saved at initialization
   * Used to restore settings or as reference for transitions
   */
  CMD_struct saved;
  
  /**
   * Total number of LEDs in the array
   * Used for bounds checking and iteration
   */
  uint8_t numLeds = NUM_LEDS;
};

#endif // ANIMATION_TYPEENUM_H