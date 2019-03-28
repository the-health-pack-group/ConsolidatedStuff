#include <stdio.h>
#include "buttonHandler.h"
#include "simonDisplay.h"
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#define ADC_COUNTER_MAX_VALUE 100/100 // 100 ms for the adc: divided by Timer Period in ms


//Run_Test() #define statements:
#define RUN_TEST_TERMINATION_MESSAGE1 "buttonHandler_runTest()"  // Info message.
#define RUN_TEST_TERMINATION_MESSAGE2 "terminated."              // Info message.
#define RUN_TEST_TEXT_SIZE 2            // Make text easy to see.
#define RUN_TEST_TICK_PERIOD_IN_MS 100  // Assume a 100 ms tick period.
#define TEXT_MESSAGE_ORIGIN_X 0                  // Text is written starting at the right, and
#define TEXT_MESSAGE_ORIGIN_Y (DISPLAY_HEIGHT/2) // middle.


//A boolean that only this state machine can see; used to enable and disable state machine
static bool enable_flag = false; // initialize to false for disable
static bool finger_removed = false; // initialize to false (no touch has happened)

// States for the buttonHandler state machine.
enum buttonHandler_st_t {
    init_st, //Initial state of the state machine
    never_touched_st, // State that button handler waits in until the display is touched
    adc_Counter_running_st, // State for the adc to settle
    waiting_release_st, // State that waits until user releases finger to register a touch
    final_st //Final state before returning to init_st

} currentState;

uint8_t buttonHandler_getRegionNumber()
{
    //declare 3 local variables to read touch coordinates.
        int16_t x;
        int16_t y;
        uint8_t z;

    //get display touch coordinates:
        display_getTouchedPoint(&x, &y, &z);

        return simonDisplay_computeRegionNumber(x,y);
}


void buttonHandler_enable()
{
    //enable State Machine flag
    enable_flag = true;
}


void buttonHandler_disable()
{
    //disable State Machine flag
        enable_flag = false;
}

// The only thing this function does is return a boolean flag set by the buttonHandler state machine. To wit:
// Once enabled, the buttonHandler state-machine first waits for a touch. Once a touch is detected, the
// buttonHandler state-machine computes the region-number for the touched area. Next, the buttonHandler
// state-machine waits until the player removes their finger. At this point, the state-machine should
// set a bool flag that indicates the the player has removed their finger. Once the buttonHandler()
// state-machine is disabled, it should clear this flag.
// All buttonHandler_releasedDetected() does is return the value of this flag.
// As such, the body of this function should only contain a single line of code.
// If this function does more than return a boolean set by the buttonHandler state machine, you are going about
// this incorrectly.
bool buttonHandler_releaseDetected()
{
    return finger_removed; 
}

// Initialize the state machine
void buttonHandler_init()
{
    currentState = init_st;
}



// Standard tick function.
void buttonHandler_tick()
{
    static uint8_t adcCounter; // counter for adc Touchpad
    static uint8_t region_No; // variable used to compute region number

    switch(currentState)
    {
		// Initial state of the state machine
    case init_st:
    {
		
        if(enable_flag) // Check enable flag before proceeding with state machine
        {
            currentState = never_touched_st;
            //initialize variables
            adcCounter = 0;
            region_No = 0;
            finger_removed = false;
        }

        else
            currentState = init_st;
    }
    break;

	// State that button handler waits in until the display is touched
    case never_touched_st:
    {
		//Check to make sure enable flag is still valid, to not hang in a state
		if (!enable_flag)
			currentState = init_st; // Return to init state if flag is disabled
		//If display is touched, proceed to adc_Counter_running_st
        else if (display_isTouched())
        {

            display_clearOldTouchData();
            currentState = adc_Counter_running_st;
        }

        else
			//Remain in current state
            currentState = never_touched_st;
    }
    break;

	// State for the adc to settle
    case adc_Counter_running_st:
    {      
		   //Check to make sure enable flag is still valid, to not hang in a state
		   if (!enable_flag)
			  currentState = init_st; // Return to init state if flag is disabled
		   //Check to see if adcCounter expired
		   else if (adcCounter == ADC_COUNTER_MAX_VALUE)
           {
			   
               region_No = buttonHandler_getRegionNumber(); // compute region number
               simonDisplay_drawSquare( region_No, false); // false means do not erase
               currentState = waiting_release_st; //Transition to next states
           }
           else
            currentState = adc_Counter_running_st;

    }
    break;

	// State that waits until user releases finger to register a touch
    case waiting_release_st:
    {
		
        //Check to make sure enable flag is still valid, to not hang in a state
        if (!enable_flag)
            currentState = init_st;

		else if (!display_isTouched())
        {
            //finger is removed
            finger_removed = true;
            currentState = final_st;
            //erase big square
            simonDisplay_drawSquare(region_No, true); // true means erase
            //redraw button
            simonDisplay_drawButton(region_No, false); // false means do not erase
        }

        else
            currentState = waiting_release_st;
    }
    break;

	//Final state before returning to init_st
    case final_st:
    {
        if (enable_flag)
            currentState = final_st;
        else

            currentState = init_st;

    }
    break;

    }

    //State actions:
    switch (currentState)
    {
    case init_st:
       ; // do nothing
    break;
    case never_touched_st:
        ;// do nothing
        break;
    case adc_Counter_running_st:
        adcCounter++;
        break;
    case waiting_release_st:
         ;// do nothing
        break;
    case final_st:
         // do nothing
        break;
    }
}

// This tests the functionality of the buttonHandler state machine.
// buttonHandler_runTest(int16_t touchCount) runs the test until
// the user has touched the screen touchCount times. It indicates
// that a button was pushed by drawing a large square while
// the button is pressed and then erasing the large square and
// redrawing the button when the user releases their touch.
void buttonHandler_runTest(int16_t touchCountArg) {
    int16_t touchCount = 0;                 // Keep track of the number of touches.
    display_init();                         // Always have to init the display.
    display_fillScreen(DISPLAY_BLACK);      // Clear the display.
    // Draw all the buttons for the first time so the buttonHandler doesn't need to do this in an init state.
    // Ultimately, simonControl will do this when the game first starts up.
    simonDisplay_drawAllButtons();
    buttonHandler_init();                   // Initialize the button handler state machine
    buttonHandler_enable();
    while (touchCount < touchCountArg) {    // Loop here while touchCount is less than the touchCountArg
        buttonHandler_tick();               // Advance the state machine.
        utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
        if (buttonHandler_releaseDetected()) {  // If a release is detected, then the screen was touched.
            touchCount++;                       // Keep track of the number of touches.
            // Get the region number that was touched.
            printf("button released: %d\n\r", buttonHandler_getRegionNumber());
            // Interlocked behavior: handshake with the button handler (now disabled).
            buttonHandler_disable();
            utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
            buttonHandler_tick();               // Advance the state machine.
            buttonHandler_enable();             // Interlocked behavior: enable the buttonHandler.
            utils_msDelay(RUN_TEST_TICK_PERIOD_IN_MS);
            buttonHandler_tick();               // Advance the state machine.
        }
    }
    display_fillScreen(DISPLAY_BLACK);        // clear the screen.
    display_setTextSize(RUN_TEST_TEXT_SIZE);  // Set the text size.
    display_setCursor(TEXT_MESSAGE_ORIGIN_X, TEXT_MESSAGE_ORIGIN_Y); // Move the cursor to a rough center point.
    display_println(RUN_TEST_TERMINATION_MESSAGE1); // Print the termination message on two lines.
    display_println(RUN_TEST_TERMINATION_MESSAGE2);
}
