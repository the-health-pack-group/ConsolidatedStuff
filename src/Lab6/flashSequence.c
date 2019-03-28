#include"flashSequence.h"
#include"simonDisplay.h"
#include"globals.h"
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#include<stdbool.h>
#include<stdint.h>

#define FLASH_DURATION_MAX_VALUE 500/100 // Duration for each part of sequence that is displayed on the screen. .5sec in miliseconds/ Timer Period.
#define ARRAY_OFFSET 1 // Offset value for taking into account that array indexes begin at 0, not 1- used in time_on_screen_st

//RunTest() define statements:
// This will set the sequence to a simple sequential pattern.
// It starts by flashing the first color, and then increments the index and flashes the first
// two colors and so forth. Along the way it prints info messages to the LCD screen.
#define TEST_SEQUENCE_LENGTH 8  // Just use a short test sequence.
uint8_t flashSequence_testSequence[TEST_SEQUENCE_LENGTH] = {
    SIMON_DISPLAY_REGION_0,
    SIMON_DISPLAY_REGION_1,
    SIMON_DISPLAY_REGION_2,
    SIMON_DISPLAY_REGION_3,
    SIMON_DISPLAY_REGION_3,
    SIMON_DISPLAY_REGION_2,
    SIMON_DISPLAY_REGION_1,
    SIMON_DISPLAY_REGION_0 };   // Simple sequence.

#define INCREMENTING_SEQUENCE_MESSAGE1 "Incrementing Sequence"  // Info message.
#define RUN_TEST_COMPLETE_MESSAGE "Runtest() Complete"          // Info message.
#define MESSAGE_TEXT_SIZE 2     // Make the text easy to see.
#define TWO_SECONDS_IN_MS 2000  // Two second delay.
#define TICK_PERIOD 75          // 200 millisecond delay.
#define TEXT_ORIGIN_X 0                  // Text starts from far left and
#define TEXT_ORIGIN_Y (DISPLAY_HEIGHT/2) // middle of screen.




//A boolean that only this state machine can see; used to enable and disable state machine
static bool enable_flag = false; // initialize to false for disable
//A boolean that indicates when the sequence is complete.
static bool sequence_completed = false; // initialize to false (no initial sequence displayed)


// States for the flashSequence state machine.
enum flashSequence_st_t {
    init_st, //Initial state of the state machine
    flash_region_st, // State that flashes a square on the screen
    time_on_screen_st, // State to allow a suitable delay of the flashed square
    final_st // Final state before returning to init st

} currentState_flash;

// Turns on the state machine. Part of the interlock.
void flashSequence_enable()
{
    //enable State Machine flag
    enable_flag = true;
}

// Turns off the state machine. Part of the interlock.
void flashSequence_disable()
{
    //disable State Machine flag
    enable_flag = false;
}

// Other state machines can call this to determine if this state machine is finished.
bool flashSequence_isComplete()
{
    return sequence_completed;
}

// Initialize the state machine
void flashSequence_init()
{
    currentState_flash = init_st;
}

// Standard tick function.
void flashSequence_tick()
{
    //Small duration to give user to see each piece of the sequence. Gets incremented on subsequent ticks
    static uint8_t flash_duration = 0;

    //A variable to index the sequence. Gets incremented on subsequent ticks
    static uint16_t sequence_indexer = 0;

    switch (currentState_flash)
    {
		//Initial state of the state machine
    case init_st:
    {

        if (enable_flag) // Check enable flag before proceeding with state machine
        {
            currentState_flash = flash_region_st; //Transition to flash_region_st

            //initialize variables
            sequence_completed = false;
            flash_duration = 0;
            sequence_indexer = 0;
        }

        else
            currentState_flash = init_st; // Flag is not enabled, remain in state
    }

    break;

	// State that flashes a square on the screen
    case flash_region_st:
    {
        //Flash appropriate region on screen
        simonDisplay_drawSquare(globals_getSequenceValue(sequence_indexer), false); // false means to not erase
        //Transition to time_on_screen_st for small delay
        currentState_flash = time_on_screen_st;
    }
    break;

	// State to allow a suitable delay of the flashed square
    case time_on_screen_st:
    {
        //If flash duration counter expires, erase and either flash another region or terminate sequence
        if (flash_duration == FLASH_DURATION_MAX_VALUE)
        {
            // clear flashed region
            simonDisplay_drawSquare(globals_getSequenceValue(sequence_indexer), true);; // true means to erase

            //Check if current sequence has finished
            if (sequence_indexer == globals_getSequenceIterationLength()- ARRAY_OFFSET)
                //Transition to final_st
                currentState_flash = final_st;
            else
            {
                //Increment sequence_indexer to get next element of current sequence
                sequence_indexer++;
                // Transition to flash_region_st
                currentState_flash = flash_region_st;
            }

            //reset flash_duration counter
            flash_duration = 0;

        }

        //else remain in state and increment flash_duration counter
        else
            currentState_flash = time_on_screen_st;

    }
    break;

	// Final state before returning to init st
    case final_st:
    {
        if (enable_flag) // Check enable flag, remain in state until it goes low
            currentState_flash = final_st;
        else
        {
            currentState_flash = init_st; // Return to init_st
            sequence_completed = false;  //Reset sequence_completed boolean for next reentry of state machine
        }

    }
    break;
    }

    //State actions:
    switch (currentState_flash)
    {
    case init_st:
        ; // do nothing
        break;
    case flash_region_st:
        ;// do nothing
        break;
    case time_on_screen_st:
        //increment flash_duration_counter
        flash_duration++;
        break;
    case final_st:
        //Tell other state machines that sequence is completed
         sequence_completed = true;
        break;
    }
}

// Print the incrementing sequence message.
void flashSequence_printIncrementingMessage() {
    display_fillScreen(DISPLAY_BLACK);  // Otherwise, tell the user that you are incrementing the sequence.
    display_setCursor(TEXT_ORIGIN_X, TEXT_ORIGIN_Y);// Roughly centered.
    display_println(INCREMENTING_SEQUENCE_MESSAGE1);// Print the message.
    utils_msDelay(TWO_SECONDS_IN_MS);   // Hold on for 2 seconds.
    display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
}

// Run the test: flash the sequence, one square at a time
// with helpful information messages.
void flashSequence_runTest() {
    display_init();                 // We are using the display.
    display_fillScreen(DISPLAY_BLACK);  // Clear the display.
    globals_setSequence(flashSequence_testSequence, TEST_SEQUENCE_LENGTH);  // Set the sequence.
    flashSequence_init();               // Initialize the flashSequence state machine
    flashSequence_enable();             // Enable the flashSequence state machine.
    int16_t sequenceLength = 1;         // Start out with a sequence of length 1.
    globals_setSequenceIterationLength(sequenceLength); // Set the iteration length.
    display_setTextSize(MESSAGE_TEXT_SIZE); // Use a standard text size.
    while (1) {                             // Run forever unless you break.
        flashSequence_tick();             // tick the state machine.
        utils_msDelay(TICK_PERIOD);   // Provide a 1 ms delay.
        if (flashSequence_isComplete()) {   // When you are done flashing the sequence.
            flashSequence_disable();          // Interlock by first disabling the state machine.
            flashSequence_tick();             // tick is necessary to advance the state.
            utils_msDelay(TICK_PERIOD);       // don't really need this here, just for completeness.
            flashSequence_enable();           // Finish the interlock by enabling the state machine.
            utils_msDelay(TICK_PERIOD);       // Wait 1 ms for no good reason.
            sequenceLength++;                 // Increment the length of the sequence.
            if (sequenceLength > TEST_SEQUENCE_LENGTH)  // Stop if you have done the full sequence.
                break;
            // Tell the user that you are going to the next step in the pattern.
            flashSequence_printIncrementingMessage();
            globals_setSequenceIterationLength(sequenceLength);  // Set the length of the pattern.
        }
    }
    // Let the user know that you are finished.
    display_fillScreen(DISPLAY_BLACK);              // Blank the screen.
    display_setCursor(TEXT_ORIGIN_X, TEXT_ORIGIN_Y);// Set the cursor position.
    display_println(RUN_TEST_COMPLETE_MESSAGE);     // Print the message.
}
