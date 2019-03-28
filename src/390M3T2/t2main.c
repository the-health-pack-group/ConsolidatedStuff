#include "transmitter.h"
#include <stdio.h>
#include "supportFiles/switches.h"
#include "supportFiles/buttons.h"
#include "supportFiles/utils.h"
#include "src/390_libs/filter.h"
#include "supportFiles/interrupts.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include "trigger.h"

void runTransmitterNonContinuousTest();
void runTransmitterContinuousTest();

int main()
{
    //trigger_init();
    //hitLedTimer_init();
    lockoutTimer_init();
    interrupts_initAll(true);               // main interrupt init function.
    interrupts_enableTimerGlobalInts();     // enable global interrupts.
    interrupts_startArmPrivateTimer();      // start the main timer.
    interrupts_enableArmInts();             // now the ARM processor can see interrupts.




    //runTransmitterNonContinuousTest();



    //trigger_runTest();


    //hitLedTimer_runTest();


    lockoutTimer_runTest();
}

void isr_function() {
       //transmitter_tick();
       //trigger_tick();
       //hitLedTimer_tick();
       lockoutTimer_tick();
   }

void runTransmitterNonContinuousTest()
{

    transmitter_init();

    printf(" starting runTransmitterNonContinuousTest\n\r");
    buttons_init();                                                         // Using buttons
    switches_init();                                                        // and switches.
    while (!(buttons_read() & BUTTONS_BTN1_MASK )) {                        // Run continuously until btn1 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;    // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);                        // set the frequency number based upon switch value.
        transmitter_run();                                                  // Start the transmitter.
        while (transmitter_running()) {                                     // Keep ticking until it is done.
            utils_msDelay(10);
        }
        utils_msDelay(400);
    }
    printf("exiting runTransmitterNonContinuousTest()\n\r");
}

void runTransmitterContinuousTest()
{

    transmitter_init();

    printf(" starting runTransmitterContinuousTest\n\r");
    buttons_init();                                                         // Using buttons
    switches_init();                                                        // and switches.
    transmitter_run();
    transmitter_setContinuousMode(true);                                    //set continuous mode
    while (!(buttons_read() & BUTTONS_BTN1_MASK )) {                        // Run continuously until btn1 is pressed.
        uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;    // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue);                        // set the frequency number based upon switch value.

    }
    printf("exiting runTransmitterContinuousTest()\n\r");
}
