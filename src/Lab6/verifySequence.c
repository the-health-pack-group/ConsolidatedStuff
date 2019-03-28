#include"verifySequence.h"
#include"simonDisplay.h"
#include"buttonHandler.h"
#include"globals.h"
#include "src/Lab2/buttons.h"
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#include<stdbool.h>
#include<stdint.h>

#define TIME_OUT_COUNTER_MAX_VALUE 2000/100 // Duration for the timer that decides when a player's time has run out.
#define ARRAY_OFFSET 1 // Offset value for taking into account that array indexes begin at 0, not 1- used in verify_sequence_st
//RunTest() define statements: 
#define MESSAGE_X 0
//#define MESSAGE_Y (display_width()/4)
#define MESSAGE_Y (display_height()/2)
#define MESSAGE_TEXT_SIZE 2
//#define MESSAGE_STARTING_OVER
#define BUTTON_0 0  // Index for button 0
#define BUTTON_1 1  // Index for button 1
#define BUTTON_2 2  // Index for button 2
#define BUTTON_3 3  // Index for button 3


//A boolean that only this state machine can see; used to enable and disable state machine
static bool enable_flag = false; // initialize to false for disable

static bool timeOut = false; // A boolean for the timeOut error
static bool inputError = false; // A boolean for the input error
static bool verify_sequence_complete = false; // A boolean indicating that state machine finished

// States for the verifySequence state machine.
enum verifySequence_st_t {
    init_st, //Initial state of the state machine
    waiting_release_st, // State where waiting release of finger 
    verify_sequence_st, // State that handles whether the correct region was touched
    final_st // final state before returning to initial state

} currentState_verify;



// State machine will run when enabled.
void verifySequence_enable()
{
    //enable State Machine flag
      enable_flag = true;
}

// This is part of the interlock. You disable the state-machine and then enable it again.
void verifySequence_disable()
{
    //enable State Machine flag
      enable_flag = false;
}

// Used to detect if there has been a time-out error.
bool verifySequence_isTimeOutError()
{
    return timeOut;
}

// Used to detect if the user tapped the incorrect sequence.
bool verifySequence_isUserInputError()
{
    return inputError;
}

// Used to detect if the verifySequence state machine has finished verifying.
bool verifySequence_isComplete()
{
    return verify_sequence_complete;
}

// Initialize the state machine
void verifySequence_init()
{
    currentState_verify = init_st;
}

// Standard tick function.
void verifySequence_tick()
{
      //A counter to determine if the player timed out. Gets incremented on subsequent ticks
       static uint8_t timeOut_counter = 0;

       //A variable to index the sequence. Gets incremented on subsequent ticks
       static uint16_t sequence_indexer = 0;

       switch (currentState_verify)
       {
       case init_st:
       {

           if (enable_flag) // Check enable flag before proceeding with state machine
           {


               //initialize variables
               verify_sequence_complete = false;
               timeOut = false;
               inputError = false;
               timeOut_counter = 0;
               sequence_indexer = 0;

               //commence interlock with buttonHandler state machine
               buttonHandler_enable();
               currentState_verify = waiting_release_st;
           }

           else
               currentState_verify = init_st;
       }

       break;

       case waiting_release_st:
       {
           //Check to see if time-out timer expired
            if (timeOut_counter == TIME_OUT_COUNTER_MAX_VALUE)
                {
                   timeOut = true; // user timed out set boolean
                   currentState_verify = final_st; //buttonHandler disabled in final_st action
                }
            else if (buttonHandler_releaseDetected())
            {
                //Disable Button Handler
                buttonHandler_disable();
                currentState_verify = verify_sequence_st; // advance to verify sequence state
            }

            else
                currentState_verify = waiting_release_st; // remain in current state while finger is still held down
       }

       break;

       case verify_sequence_st:
       {
           //check if correct region was touched, and set the boolean inputError accordingly
            buttonHandler_getRegionNumber() == globals_getSequenceValue(sequence_indexer) ? inputError = false : inputError = true;

            //Check if current sequence has finished or input is an error
             if ((sequence_indexer == globals_getSequenceIterationLength() - ARRAY_OFFSET) || inputError)
                    {
                       //Transition to final_st
                       currentState_verify = final_st;
                    }

             else
                    {
                       //Increment sequence_indexer to get next element of current sequence
                       sequence_indexer++;
                       // Transition back to waiting_release st and enable button Handler
                       currentState_verify = waiting_release_st;
                       buttonHandler_enable();

                       // reset timeOut counter
                       timeOut_counter = 0;
                    }
       }

       break;
       case final_st:
       {
           if (enable_flag) //Check enable flag, if set remain in state until it is disabled
               currentState_verify = final_st;
           else
           {
               currentState_verify = init_st; // Flag is disabled. Transition back to init state
               verify_sequence_complete = false; // Reset sequence_completed boolean for next reentry of state machine
           }

       }

       break;

       default:
       printf("currentState_verify state update hit state default\n\r");
       break;

       }

       //State actions:
       switch (currentState_verify)
       {
       case init_st:
           ; // do nothing
           break;
       case waiting_release_st:
           //increment timeOut_counter
           timeOut_counter++;
       case verify_sequence_st:
           ; // do nothing
           break;
       case final_st:
       {
           //Terminate the interlock with buttonHandler
            buttonHandler_disable();
            //Tell other state machines that sequence is completed
            verify_sequence_complete = true;
       }
           break;
       default:
           printf("currentState_verify state action hit state default\n\r");
           break;
       }
}


// Prints the instructions that the user should follow when
// testing the verifySequence state machine.
// Takes an argument that specifies the length of the sequence so that
// the instructions are tailored for the length of the sequence.
// This assumes a simple incrementing pattern so that it is simple to
// instruct the user.
void verifySequence_printInstructions(uint8_t length, bool startingOver) {
	display_fillScreen(DISPLAY_BLACK);              // Clear the screen.
	display_setTextSize(MESSAGE_TEXT_SIZE);     // Make it readable.
	display_setCursor(MESSAGE_X, MESSAGE_Y);    // Rough center.
	if (startingOver) {                                             // Print a message if you start over.
		display_fillScreen(DISPLAY_BLACK);          // Clear the screen if starting over.
		display_setTextColor(DISPLAY_WHITE);        // Print whit text.
		display_println("Starting Over. ");         // Starting over message.
	}
	// Print messages are self-explanatory, no comments needed.
	// These messages request that the user touch the buttons in a specific sequence.
	display_println("Tap: ");
	display_println();
	switch (length) {
	case 1:
		display_println("red");
		break;
	case 2:
		display_println("red, yellow ");
		break;
	case 3:
		display_println("red, yellow, blue ");
		break;
	case 4:
		display_println("red, yellow, blue, green ");
		break;
	default:
		break;
	}
	display_println("in that order.");
	display_println();
	display_println("hold BTN0 to quit.");
}

// Just clears the screen and draws the four buttons used in Simon.
void verifySequence_drawButtons() {
	display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
	simonDisplay_drawAllButtons();      // Draw all the buttons.
}

// This will set the sequence to a simple sequential pattern.
#define MAX_TEST_SEQUENCE_LENGTH 4  // the maximum length of the pattern
uint8_t verifySequence_testSequence[MAX_TEST_SEQUENCE_LENGTH] = { 0, 1, 2, 3 };  // A simple pattern.
#define MESSAGE_WAIT_MS 4000  // Display messages for this long.

																				 // Increment the sequence length making sure to skip over 0.
																				 // Used to change the sequence length during the test.
int16_t incrementSequenceLength(int16_t sequenceLength) {
	int16_t value = (sequenceLength + 1) % (MAX_TEST_SEQUENCE_LENGTH + 1);
	if (value == 0) value++;
	return value;
}

// Used to select from a variety of informational messages.
enum verifySequence_infoMessage_t {
	user_time_out_e,            // means that the user waited too long to tap a color.
	user_wrong_sequence_e,      // means that the user tapped the wrong color.
	user_correct_sequence_e,    // means that the user tapped the correct sequence.
	user_quit_e                 // means that the user wants to quite.
};


// Prints out informational messages based upon a message type (see above).
void verifySequence_printInfoMessage(verifySequence_infoMessage_t messageType) {
	// Setup text color, position and clear the screen.
	display_setTextColor(DISPLAY_WHITE);
	display_setCursor(MESSAGE_X, MESSAGE_Y);
	display_fillScreen(DISPLAY_BLACK);
	switch (messageType) {
	case user_time_out_e:  // Tell the user that they typed too slowly.
		display_println("Error:");
		display_println();
		display_println("  User tapped sequence");
		display_println("  too slowly.");
		break;
	case user_wrong_sequence_e:  // Tell the user that they tapped the wrong color.
		display_println("Error: ");
		display_println();
		display_println("  User tapped the");
		display_println("  wrong sequence.");
		break;
	case user_correct_sequence_e:  // Tell the user that they were correct.
		display_println("User tapped");
		display_println("the correct sequence.");
		break;
	case user_quit_e:             // Acknowledge that you are quitting the test.
		display_println("quitting runTest().");
		break;
	default:
		break;
	}
}

#define TICK_PERIOD_IN_MS 100
// Tests the verifySequence state machine.
// It prints instructions to the touch-screen. The user responds by tapping the
// correct colors to match the sequence.
// Users can test the error conditions by waiting too long to tap a color or
// by tapping an incorrect color.
void verifySequence_runTest() {
	display_init();  // Always must do this.
	buttons_init();  // Need to use the push-button package so user can quit.
	int16_t sequenceLength = 1;  // Start out with a sequence length of 1.
	verifySequence_printInstructions(sequenceLength, false);  // Tell the user what to do.
	utils_msDelay(MESSAGE_WAIT_MS);  // Give them a few seconds to read the instructions.
	verifySequence_drawButtons();    // Now, draw the buttons.
									 // Set the test sequence and it's length.
	globals_setSequence(verifySequence_testSequence, MAX_TEST_SEQUENCE_LENGTH);
	globals_setSequenceIterationLength(sequenceLength);
	verifySequence_init();    // Initialize the verifySequence state machine
							  // Enable the verifySequence state machine.
	verifySequence_enable();  // Everything is interlocked, so first enable the machine.
							  // Need to hold button until it quits as you might be stuck in a delay.
	while (!(buttons_read() & BUTTONS_BTN0_MASK)) {
		// verifySequence uses the buttonHandler state machine so you need to "tick" both of them.
		verifySequence_tick();  // Advance the verifySequence state machine.
		buttonHandler_tick();   // Advance the buttonHandler state machine.
		utils_msDelay(TICK_PERIOD_IN_MS);       // Wait for a tick period.
												// If the verifySequence state machine has finished, check the result, 
												// otherwise just keep ticking both machines.
		if (verifySequence_isComplete()) {
			if (verifySequence_isTimeOutError()) {                // Was the user too slow?
				verifySequence_printInfoMessage(user_time_out_e); // Yes, tell the user that they were too slow.
			}
			else if (verifySequence_isUserInputError()) {       // Did the user tap the wrong color?
				verifySequence_printInfoMessage(user_wrong_sequence_e); // Yes, tell them so.
			}
			else {
				verifySequence_printInfoMessage(user_correct_sequence_e); // User was correct if you get here.
			}
			utils_msDelay(MESSAGE_WAIT_MS);                            // Allow the user to read the message.
			sequenceLength = incrementSequenceLength(sequenceLength);  // Increment the sequence.
			globals_setSequenceIterationLength(sequenceLength);        // Set the length for the verifySequence state machine.
			verifySequence_printInstructions(sequenceLength, true);    // Print the instructions.
			utils_msDelay(MESSAGE_WAIT_MS);                            // Let the user read the instructions.
			verifySequence_drawButtons();                              // Draw the buttons.
			verifySequence_disable();                                  // Interlock: first step of handshake.
			verifySequence_tick();
			buttonHandler_tick();                                       // Advance the verifySequence machine.
			utils_msDelay(TICK_PERIOD_IN_MS);                          // Wait for tick period.
			verifySequence_enable();                                   // Interlock: second step of handshake.
			utils_msDelay(TICK_PERIOD_IN_MS);                          // Wait for tick period.
		}
	}
	verifySequence_printInfoMessage(user_quit_e);  // Quitting, print out an informational message.
}
