
#include "hitLedTimer.h"

#include "supportFiles/leds.h"
#include "supportFiles/mio.h"
#include "supportFiles/utils.h"
#include <stdio.h>

#define DEBUG 0 //Enables our debugStatePrint
#define COUNTER_INIT_VALUE 0

#define LD0_OFF 0x00 //Byte to turn LED OFF
#define LD0_ON 0x01 //Byte to turn LED ON

#define LED_OFF 0  // Value to write to pin
#define LED_ON 1  //Value to write to pin

enum hitLedTimer_st_t {
    init_st, //Initial state
    wait_st, //Waiting for start
    on_st   //Led on for 500ms
} hitLedTimerState;

static bool running = false; // State machine running (set true when hit)

static void debugStatePrint(); // Debug function

// Standard init function. Implement it even if it is not necessary. You may need it later.
void hitLedTimer_init() {
    //Init mio, and set LED pin as output
    mio_init(DEBUG);
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
    //Init leds
    leds_init(DEBUG);

    //Make sure led is off
    hitLedTimer_turnLedOff();

    // Start and initialize state machine as init_st
    running = false;
    hitLedTimerState = init_st;
}

// Calling this starts the timer.
void hitLedTimer_start() {
    running = true;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return running;
}

// Standard tick function.
void hitLedTimer_tick() {

    //Print state changes
    if (DEBUG) {
        debugStatePrint();
    }

    //Counter to keep track of time
    static uint16_t counter = 0;

    // Transitions
    switch(hitLedTimerState) {
        case init_st: {
            //Move to wait_st
            hitLedTimerState = wait_st;
        }
        break;
        case wait_st: {
            //If state machine is running (hit detected) then turn on led
            if (running) {
                counter = COUNTER_INIT_VALUE;

                hitLedTimer_turnLedOn();
                //transition to on_st
                hitLedTimerState = on_st;
            }
        }
        break;
        case on_st: {
            //Remain in on_st for 500ms
            if (counter >= HIT_LED_TIMER_EXPIRE_VALUE) {
                running = false;
                //Turn off led and return back to wait_st
                hitLedTimer_turnLedOff();

                hitLedTimerState = wait_st;
            }
        }
        break;
    }

    // Actions
    switch(hitLedTimerState) {
        case init_st: break;
        case wait_st: break;
        case on_st: {
            //Increment counter
            counter++;
        }
        break;
    }

}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    //Turn both LED and Hit indicator on
    leds_write(LD0_ON);
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    //Turn both LED and Hit indicator off
    leds_write(LD0_OFF);
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, LED_OFF);
}

// Test function
#define HIT_LED_REPEAT_DELAY_MS 500
void hitLedTimer_runTest() {

    while (true) {
        //Pretend we are hit
        hitLedTimer_start();
        while (hitLedTimer_running()); // Run until done!
        utils_msDelay(HIT_LED_REPEAT_DELAY_MS); //Delay a little
    }
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint() {
  static enum hitLedTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != lockoutTimerState - this prevents reprinting the same state name over and over.
  if (previousState != hitLedTimerState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = hitLedTimerState;     // keep track of the last state that you were in.
    switch(hitLedTimerState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf("init_st\n\r");
        break;
      case wait_st:
        printf("wait_st\n\r");
        break;
      case on_st:
        printf("on_st\n\r");
        break;
     }
  }
}
