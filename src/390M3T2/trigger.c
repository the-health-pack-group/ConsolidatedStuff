
#include "trigger.h"

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define GUN_TRIGGER_PRESSED 1
#define TRIGGER_TIMER_MAX 5000
#define RESET 0

#include <stdio.h>
#include <stdint.h>
#include "transmitter.h"
#include "supportFiles/buttons.h"
#include "supportFiles/utils.h"
#include "supportFiles/mio.h"

#define DEBUG 0 //Enables our debugStatePrint

enum trigger_st_t {
    init_st, //Init state
    inactive_st, //Waiting for trigger to be pulled
    off_st, //Output off
    on_st  //Output on
} triggerState;

static bool ignoreGunInput = false; //If true, ignore trigger
static bool enabled = false;        //To activate state machine
volatile static bool wantsToShoot = false;	//Set to true as soon as the trigger press is debounced. Outside state machine must service flag and set to false
static bool debouncePressed = false;	//Set to true when the trigger press is debounced. Set to false when the trigger release is debounced.

//True when trigger is pressed
bool triggerPressed();

static void debugStatePrint(); //Standard debug function

// Function that returns whether the trigger wishes to shoot (i.e. the trigger has been debounced and pulled)
bool trigger_wantsToShoot()
{
	return wantsToShoot;
}

// Function that returns whether the debounced trigger is currently pressed
bool trigger_debouncePressed()
{
	return debouncePressed;
}

// Function to clear the wantsToShoot flag. Called after servicing wantsToShoot. Prevents double shots from being possible.
void trigger_clearWantsToShoot()
{
	wantsToShoot = false;
}


// Initializes the mio subsystem.
void trigger_init() {
  mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
  // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
  if (triggerPressed()) {
    ignoreGunInput = true;
  }

  //Transition to init state
  triggerState = init_st;
}

// Enable the trigger state machine. The trigger state-machine is inactive until this function is called.
// This allows you to ignore the trigger when helpful (mostly useful for testing).
// I don't have an associated trigger_disable() function because I don't need to disable the trigger.
void trigger_enable() {
    enabled = true;
}

// Standard tick function.
void trigger_tick() {
    //If debug mode, print state changes
    if (DEBUG)
        debugStatePrint();

    static uint16_t timer = RESET;

    // Transitions
    switch (triggerState) {
        case init_st: {
            timer = RESET;	//We want to initially reset the debounce timer
            triggerState = inactive_st;	//We want to initially go to the state where trigger is not enabled
        }
        break;
        case inactive_st: {
            if (enabled) {	//As soon as the trigger state machine becomes enabled
                triggerState = off_st;	//Go to the state where the trigger is initially considered "off"
            }
        }
        break;
        case off_st: {
            if (triggerPressed() && timer >= TRIGGER_TIMER_MAX) {//If the trigger remained pressed after the debouncing time
                timer = RESET;	//Reset the debounce timer
                triggerState = on_st;	//Go to the state where the trigger is considered to be "on"
                wantsToShoot = true;	//Raise the flag to indicate that this is the time where a shot may be fired
				debouncePressed = true;	//Raise the flag which indicates that the trigger has been debounced and is currently pressed
            }
            else if (!triggerPressed()) {	//If the trigger stops being pressed before the debouncing time limit
                timer = RESET;	//Reset the debounce timer
            }
        }
        break;
        case on_st: {
            if (!triggerPressed() && timer >= TRIGGER_TIMER_MAX) {	//If the trigger stops being pressed after the debouncing time
                timer = RESET;	//Reset the debounce timer
                triggerState = off_st;	//Go back to the state where the trigger is considered "off"
				debouncePressed = false;//Indicate that the trigger has been debounced and is no longer pressed
            }
            else if (triggerPressed()) {//If the trigger has not been released for longer than the debouncing time
                timer = RESET;	//Reset the debounce timer
            }
        }
        break;
        default: {

        }
        break;
    }

    // Actions
    switch (triggerState) {
        case init_st: {

        }
        break;
        case inactive_st: {

        }
        break;
        case off_st: {
            timer++;	//Increase the debounce timer while in this state

        }
        break;
        case on_st: {
            timer++;	//Increase the debounce timer while in this state

        }
        break;
        default: break;
    }
}

// Trigger test function.
void trigger_runTest() {
    trigger_enable();
    while(true)
    {
      //To keep state machine running
    }

}

// Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// Gun input is ignored if the gun-input is high when the init() function is invoked.
bool triggerPressed() {
    return (
        (!ignoreGunInput && mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED) ||
        (buttons_read() & BUTTONS_BTN0_MASK)
    );
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum trigger_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != triggerState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = triggerState;     // keep track of the last state that you were in.
    switch(triggerState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf("init_st\n\r");
        break;
      case inactive_st:
        printf("inactive_st\n\r");
        break;
      case off_st:
        printf("U\n\r");
        break;
      case on_st:
        printf("D\n\r");
        break;
     }
  }
}

