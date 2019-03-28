#include "clockControl.h"
#include "clockDisplay.h"
#include "src/Lab3/intervalTimer.h"
#include <stdio.h>
#include "supportFiles/display.h"
#include "supportFiles/utils.h"


#define ELAPSED_TICK 1
#define ELAPSED_SECOND 1000/50 // 1 second: 1000/ Timer Period in ms
#define ADC_COUNTER_MAX_VALUE 50/50 // 50 ms for the adc: divided by Timer Period in ms
#define AUTO_COUNTER_MAX_VALUE 500/50 // half a second in ms, used to detect when to start auto-incrementing : divide by Timer Period in ms
#define RATE_COUNTER_MAX_VALUE 100/50 // 100 ms; every 1 tenth of a second increment the clock during auto-increment : divide by Timer Period in ms

// States for the controller state machine.
enum clockControl_st_t {
    init_st,                 // Start here, transition out of this state on the first tick.
    never_touched_st,        // Wait here until the first touch - clock is disabled until set.
    waiting_for_touch_st,    // waiting for touch, clock is enabled and running.
    adc_Counter_running_st,     // waiting for the touch-controller ADC to settle.
    auto_Counter_running_st,   // waiting for the auto-update delay to expire
                             // (user is holding down button for auto-inc/dec)
    rate_Counter_running_st,   // waiting for the rate-timer to expire to know when to perform the auto inc/dec.
    rate_Counter_expired_st,  // when the rate-timer expires, perform the inc/dec function.
    inc_Second_st
} currentState;

// Initialize clockControl currentState
void clockControl_init() {
    currentState = init_st;

}

void debugStatePrint() {
  static clockControl_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf("init_st\n\r");
        break;
      case never_touched_st:
        printf("never_touched_st\n\r");
        break;
      case waiting_for_touch_st:
        printf("waiting_for_touch_st\n\r");
        break;
      case adc_Counter_running_st:
        printf("adc_Counter_running_st\n\r");
        break;
      case auto_Counter_running_st:
        printf("auto_Counter_running_st\n\r");
        break;
      case rate_Counter_running_st:
        printf("rate_Counter_running_st\n\r");
        break;
      case rate_Counter_expired_st:
        printf("rate_Counter_expired_st\n\r");
      case inc_Second_st:
        printf("inc_Second_st\n\r");
        break;
     }
  }
}

void clockControl_tick() {

    // used to debug state machine
    debugStatePrint();

    //insert start timers here to time tick function


    //declare counters
    static uint adcCounter =0;
    static uint autoCounter =0;
    static uint rateCounter =0;
    // Counter to count 1 second of elapsed time
    static uint secCounter =0;

    // Perform state update first.
    switch (currentState) {
    case init_st:
        currentState = never_touched_st;
        break;

    case never_touched_st:
        //when display is touched, begin clock by going to waiting_for_touch_st
        if (display_isTouched())
            currentState = waiting_for_touch_st;
        else
            currentState = never_touched_st;
        break;


    case waiting_for_touch_st:
        // if display is touched, transition to adc_Counter_running_st
        if (display_isTouched())
        {
            display_clearOldTouchData();
            currentState = adc_Counter_running_st;
        }

         // if 1 second has elapsed, go to inc_Second_st
        else if  (secCounter >= ELAPSED_SECOND - ELAPSED_TICK)
        {
            currentState = inc_Second_st;
        }
        else
            // return to current state
            currentState = waiting_for_touch_st;
        break;

    case adc_Counter_running_st:
        // if finger is released while adc counter is counting, and adc_counter is max, go to wait state and increment touched button
        if (!display_isTouched() && adcCounter == ADC_COUNTER_MAX_VALUE)
        {
            currentState = waiting_for_touch_st;
            clockDisplay_performIncDec();
            //reset second Counter
                     secCounter = 0;
        }
         // else if finger is still held down after ADC_MAX go to auto counter state
        else if (display_isTouched() && adcCounter == ADC_COUNTER_MAX_VALUE)
        {
            currentState = auto_Counter_running_st;
        }

        else
            // return to current state
            currentState = adc_Counter_running_st;
        break;

    case inc_Second_st:
        //after actions have completed, go directly back to waiting for touch state
        currentState = waiting_for_touch_st;
        break;

    case auto_Counter_running_st:
        // if finger is released while auto counter is running (before half a second), go to waiting for touch and increment display
        if (!display_isTouched())
        {
            currentState = waiting_for_touch_st;
            clockDisplay_performIncDec();
            //reset second Counter
             secCounter = 0;
        }
        // if auto_conter running reaches it's max, go to rate counter running
        else if (display_isTouched() && autoCounter == AUTO_COUNTER_MAX_VALUE)
        {
            currentState = rate_Counter_running_st;
            clockDisplay_performIncDec();
        }

        else
            // return to current state
            currentState = auto_Counter_running_st;
        break;


    case rate_Counter_running_st:
        if (!display_isTouched())
        {
            // if finger is released, go back to wait state
              currentState = waiting_for_touch_st;
            //reset second Counter
               secCounter = 0;
        }

        // if after rateCounterMAX, finger is still held down, go to rate counter expired state and update display there
        else if (display_isTouched() && rateCounter >= RATE_COUNTER_MAX_VALUE - ELAPSED_TICK)
            currentState = rate_Counter_expired_st;
        else
            // return to current state
            currentState = rate_Counter_running_st;
        break;


    case rate_Counter_expired_st:
        if (!display_isTouched())
        {
            // if finger is released, go to waiting for touch state instead of back to rate_Counter_running
            currentState = waiting_for_touch_st;
            //reset second Counter
            secCounter = 0;
        }

        else
        {
            // return back to rate_Counter_running state and update display
            currentState = rate_Counter_running_st;
            clockDisplay_performIncDec();
        }
        break;


    default:
        printf("clockControl_tick state update: hit default\n\r");
        break;
    }

    // Perform state action next.
    switch (currentState) {
    case init_st:
        //initialize variables
        adcCounter = 0;
        autoCounter = 0;
        rateCounter = 0;
        secCounter =0;
        break;
    case never_touched_st:
        //do nothing; wait until first touch
        break;
    case waiting_for_touch_st:
        //set counters equal to zero
        adcCounter  = 0;
        autoCounter = 0;
        rateCounter = 0;
        secCounter++;
        break;
    case adc_Counter_running_st:
        //increment adc_Counter
        adcCounter++;
        break;
    case auto_Counter_running_st:
        //increment autoCounter
        autoCounter++;
        break;
    case rate_Counter_running_st:
        // increment rateCounter
        rateCounter++;
        break;
    case rate_Counter_expired_st:
        //reset rateCounter
        rateCounter = 0;
        break;
    case inc_Second_st:
        //advance time 1 second
         clockDisplay_advanceTimeOneSecond();
         //reset second Counter
         secCounter = 0;
         break;
    default:
        printf("clockControl_tick state action: hit default\n\r");
        break;
    }

    //intervalTimer_stop(0);
    //printf("duration:%f\n\r",intervalTimer_getTotalDurationInSeconds(0));
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.

