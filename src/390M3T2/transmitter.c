
#include <stdio.h>
#include "transmitter.h"
#include "supportFiles/mio.h"
#include "supportFiles/switches.h"
#include "supportFiles/buttons.h"
#include "supportFiles/utils.h"
#include "transmitter.h"
#include "src/390_libs/filter.h"
#include "supportFiles/interrupts.h"

#define TRANSMITTER_OUTPUT_PIN 13
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0

static bool running = false;
static bool continuousMode = false;
static bool testMode = false;
static uint8_t frequency = 0;

enum transmitter_st_t {
    init_st,
    wait_for_startFlag_st,
    output_low_st,
    output_high_st
} currentState;

//Declare these functions that we will define later
void transmitter_debugStatePrint();
void transmitter_set_jf1_to_one();
void transmitter_set_jf1_to_zero();


void transmitter_init() {
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.

    currentState = init_st;
}

void transmitter_tick () {

    // We want to run for 200ms at a time
    static uint16_t runtimeCounter;

    // We use this to know how long to stay in the high or low state
    static uint16_t halfPeriodCounter;

    // This tracks what the initial half-period counter should be
    static uint16_t initHalfPeriodCounter;

    // State Transitions
    switch (currentState) {
        case init_st: {
                currentState = wait_for_startFlag_st;
            }
            break;
        case wait_for_startFlag_st:
            if (running || continuousMode) {
                // Read the number of ticks for a full period for the given frequency
                initHalfPeriodCounter = filter_frequencyTickTable[frequency] / 2;

                halfPeriodCounter = initHalfPeriodCounter;

                runtimeCounter = TRANSMITTER_WAVEFORM_WIDTH;


                // write high to the IO pin
                transmitter_set_jf1_to_one();

                currentState = output_high_st;
            }
            else {
                // Keep the current state
                currentState = wait_for_startFlag_st; //?
            }
            break;
        case output_low_st: {
                if (runtimeCounter == 0) {
                    // write low to the IO pin
                    transmitter_set_jf1_to_zero();
                    running = false;
                    currentState = wait_for_startFlag_st;
                }
                else if (halfPeriodCounter == 0) {
                    halfPeriodCounter = initHalfPeriodCounter;

                    // write high to the IO pin
                    transmitter_set_jf1_to_one();

                    currentState = output_high_st;
                }
            }
            break;
        case output_high_st: {
                if (runtimeCounter == 0) {
                    // write low to the IO pin
                    transmitter_set_jf1_to_zero();

                    running = false;
                    currentState = wait_for_startFlag_st;

                }
                else if (halfPeriodCounter == 0) {
                    halfPeriodCounter = initHalfPeriodCounter;

                    // write low to the IO pin
                    transmitter_set_jf1_to_zero();

                    currentState = output_low_st;
                }
            }
            break;
        default: printf("transmitter state update hit default! This shouldn't happen.");
    }

    if (testMode)
        transmitter_debugStatePrint();

    // State Actions
    switch (currentState) {
        case init_st: break;
        case wait_for_startFlag_st: break;
        case output_low_st: {
                if(testMode)
                    printf("0");
                runtimeCounter--;
                halfPeriodCounter--;
            }
            break;
        case output_high_st:  {
                if(testMode)
                    printf("1");
                runtimeCounter--;
                halfPeriodCounter--;
            }
            break;
        default: printf("transmitter state action hit default! This shouldn't happen.");
    }
}

void transmitter_run () {
    running = true;
}

bool transmitter_running() {
    return running;
}

void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    frequency = frequencyNumber;
}

// Prints out the clock waveform to stdio. Terminates when BTN1 is pressed.
// Prints out one line of 1s and 0s that represent one period of the clock signal, in terms of ticks.
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
void transmitter_runTest() {
    printf("starting transmitter_runTest()\n\r");
    buttons_init();                                                         // Using buttons
    switches_init();                                                        // and switches.
    transmitter_init();                                                     // init the transmitter.
    transmitter_enableTestMode();                                           // Prints diagnostics to stdio.
    while (!(buttons_read() & BUTTONS_BTN1_MASK )) {                                 // Run continuously until btn1 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;    // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);                        // set the frequency number based upon switch value.
        transmitter_run();                                                  // Start the transmitter.
        while (transmitter_running()) {                                     // Keep ticking until it is done.
            transmitter_tick();                                             // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);              // short delay between ticks.
        }
        printf("completed one test period.\n\r");
    }
    transmitter_disableTestMode();
    printf("exiting transmitter_runTest()\n\r");
}

void transmitter_debugStatePrint () {
    static enum transmitter_st_t previousState;
    static bool firstPass = true;
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (previousState != currentState || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        previousState = currentState;     // keep track of the last state that you were in.

        switch(currentState) {            // This prints messages based upon the state that you were in.
            case init_st:
                printf("\r\ninit_st\n\r");
                break;
            case wait_for_startFlag_st:
                printf("\r\nwait_for_startFlag_st\n\r");
                break;
            case output_low_st:
                printf("\r\noutput_low_st\n\r");
                break;
            case output_high_st:
                printf("\r\noutput_high_st\n\r");
                break;
        }

    }
}

void transmitter_setContinuousMode(bool continuousModeFlag) {
    continuousMode = continuousModeFlag;
}

void transmitter_enableTestMode() {
    testMode = true;
}

void transmitter_disableTestMode() {
    testMode = false;
}

void transmitter_set_jf1_to_one() {
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

void transmitter_set_jf1_to_zero() {
  mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '0' to JF-1.
}
