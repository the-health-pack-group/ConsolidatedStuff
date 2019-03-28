//#include "fake.h"

#include "lockoutTimer.h"
#include <stdint.h>
#include <stdio.h>
#include "supportFiles/utils.h"
#include "supportFiles/intervalTimer.h"

#define DEBUG 0 //Enables our debugStatePrint
#define COUNTER_INIT_VALUE 0 //Counter init value

enum lockoutTimer_st_t {
    init_st, // Init state
    wait_st, //Waiting for counter expire
    running_st //Counter running
} lockoutTimerState;

static volatile bool running = false; // State machine running (set true when activated)

// Standard init function.
void lockoutTimer_init() {
    //Initialize state machine and go to init_st
    running = false;

    lockoutTimerState = init_st;
}

// Calling this starts the timer.
void lockoutTimer_start() {
    running = true;
}

// Returns true if the timer is running.
bool lockoutTimer_running() {
    return running;
}

static void debugStatePrint();

// Standard tick function.
void lockoutTimer_tick() {
    //If debug, print state changes
    if (DEBUG) {
        debugStatePrint();
    }

    static uint32_t counter = 0;

    // Transitions
    switch(lockoutTimerState) {
        case init_st: {
            //Transition to waiting for activate
            lockoutTimerState = wait_st;
        }
        break;
        case wait_st: {
            //Once activated, transition to running state
            if (running) {
                counter = COUNTER_INIT_VALUE;
                lockoutTimerState = running_st;
            }
        }
        break;
        case running_st: {
            //Remain in running state until timer expires
            if (counter >= LOCKOUT_TIMER_EXPIRE_VALUE) {
                running = false;
                //Return to waiting state
                lockoutTimerState = wait_st;

            }
        }
        break;
    }

    // Actions
    switch(lockoutTimerState) {
        case init_st: break;
        case wait_st: break;
        case running_st: {
            //Increment counter
            counter++;
        }
        break;
    }
}

// Test function.
void lockoutTimer_runTest() {
    intervalTimer_init(INTERVAL_TIMER_TIMER_1);
    intervalTimer_reset(INTERVAL_TIMER_TIMER_1);
    intervalTimer_start(INTERVAL_TIMER_TIMER_1);
    lockoutTimer_start();
    while (lockoutTimer_running()); // Run until done!
    intervalTimer_stop(INTERVAL_TIMER_TIMER_1);
    printf("Time Duration: %f\r\n", intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_1));
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
static void debugStatePrint() {
  static enum lockoutTimer_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != lockoutTimerState - this prevents reprinting the same state name over and over.
  if (previousState != lockoutTimerState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = lockoutTimerState;     // keep track of the last state that you were in.
    switch(lockoutTimerState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf("init_st\n\r");
        break;
      case wait_st:
        printf("wait_st\n\r");
        break;
      case running_st:
        printf("running_st\n\r");
        break;
     }
  }
}
