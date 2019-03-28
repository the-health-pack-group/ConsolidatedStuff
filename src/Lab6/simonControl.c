#include "simonControl.h"
#include "verifySequence.h"
#include "flashSequence.h"
#include "simonDisplay.h"
#include "globals.h"
#include "supportFiles/display.h"
#include<stdbool.h>
#include <stdlib.h>

#define MESSAGE_X 0
#define YAY_MESSAGE_X (display_width()/2)
#define WELCOME_MESSAGE_X YAY_MESSAGE_X
#define MESSAGE_Y (display_height()/2)
#define MESSAGE_TEXT_SIZE 2 // Standard text size
#define WELCOME_TEXT_SIZE 5 // Text size for welcome message
#define STARTING_ITERATION_LENGTH 1 // Iteration of any sequence is always 1
#define STARTING_SEQUENCE_LENGTH 4 // Starting overall sequence is 4
#define MAX_SEQUENCE_LENGTH 100 // Max sequence ever will be 100 entries
#define MESSAGE_COUNTER_MAX_VALUE 4000/100 // Duration for the timer to display longest sequence, or level player achieved
#define LEVEL_UP_COUNTER_MAX_VALUE 3000/100 // Duration for the timer to allow the player to touch the screen to continue playing

#define ADC_COUNTER_MAX_VALUE 100/100 // 100 ms for the adc: divided by Timer Period in ms
#define ARRAY_OFFSET 1 // Offset value for taking into account that array indexes begin at 0, not 1
#define LONGEST_SEQUENCE_ARRAY_SIZE 20 // Enough characters to print message to display
#define WELCOME_VERTICAL_OFFSET_TOUCH_TO_START 80 // A little space to separate text from main welcome text
#define TOTAL_REGION_CNT 4 // Number of regions in game
#define INCREMENT_SEQUENCE_VALUE 1 // incrementSequence variable always gets incremented by 1 

static bool player_quit = false; // A boolean indicating that the player chose not to continue with the game
static bool game_intro = false; // A boolean to tell the state machine if the game is in the welcome to simon process. True = game hasn't started, False = game has started
static uint8_t randomSequence[MAX_SEQUENCE_LENGTH] = {0}; // A variable that will keep track of the current random sequence

// States for the simonControl state machine.
enum simonControl_st_t {
    init_st, //Initial state of the state machine
    welcome_screen_st, // State that displays the welcome screen
    welcome_waiting_release_st, // State that waits for them to release finger at the start of game
    flash_enable_st, // State to enable the flashSequence state machine
    verify_enable_st, // State to enable the verifySequence state machine
    error_check_st, // State to see if an error was detected from the verifySequence state machine
    delay_message_st, // State to delay messages on the display
    waiting_next_level_st, // State where the user can choose to advance a level or not
    adc_Counter_running_st, // State to allow the adc to settle
    waiting_release_st, // State waiting release for player to continue play

} currentState_control;

// Init the state machine
void simonControl_init()
{
    currentState_control = init_st;
}

// Random Sequence generator function.
void genSequence(uint8_t length)
{
	//Loop until the sequence length is reached, I am using an array offset to account for array index beginning at 0.
    for(uint8_t i = 0; i < length - ARRAY_OFFSET; i++)
    {
        randomSequence[i] = rand()%4; // Mod 4 generates a random number between 0 and 3 (the regions needed)
    }
}

//Print function to tell user their longest sequence achieved
void printLongestSequence(bool erase)
{
    display_setTextSize(MESSAGE_TEXT_SIZE);    //Set text size
    display_setCursor(MESSAGE_X, MESSAGE_Y);   //Set cursor to middle of screen

    if(erase)
    display_setTextColor(DISPLAY_BLACK);// Erase text
    else
    display_setTextColor(DISPLAY_WHITE);

   //Declare a char array to print the correct sequence achieved to display
   char mychar[LONGEST_SEQUENCE_ARRAY_SIZE];
   sprintf(mychar,"Longest Sequence %d",globals_getSequenceLength());
   display_println(mychar);


}

//Print function to display that user completed the current level
void printYay(bool erase)
{
    display_setTextSize(MESSAGE_TEXT_SIZE);    //Set text size
    display_setCursor(YAY_MESSAGE_X, MESSAGE_Y); //Set yay message cursor to center of screen

    if(erase)
    display_setTextColor(DISPLAY_BLACK); //Erase text
    else
    display_setTextColor(DISPLAY_WHITE);

	//Message when user completes level
    display_println("Yay!");
}

//Print function to inform user to touch to increment level
void printTouchNextLevel(bool erase)
{
    display_setTextSize(MESSAGE_TEXT_SIZE);    //Set text size
    display_setCursor(MESSAGE_X, MESSAGE_Y);   //Set cursor to middle of screen

    if(erase)
    display_setTextColor(DISPLAY_BLACK);// Erase text
    else
    display_setTextColor(DISPLAY_WHITE);

   //Message to user to advance level
   display_println("Touch for new level");
}

//Print function to welcome user to game and give instructions
void printWelcome(bool erase)
{
    display_setTextSize(WELCOME_TEXT_SIZE);    //Set text size
    display_setCursor(MESSAGE_X, MESSAGE_Y);   //Set cursor to middle of screen

    if(erase)
    display_setTextColor(DISPLAY_BLACK);// Erase text
    else
    display_setTextColor(DISPLAY_WHITE);

   //Welcome message:
   display_println("   SIMON");
   display_setTextSize(MESSAGE_TEXT_SIZE);
   display_setCursor(MESSAGE_X, MESSAGE_Y + WELCOME_VERTICAL_OFFSET_TOUCH_TO_START);   //Set cursor a bit below simon
   display_println("     Touch to start");
}

//A function that erases the 4 squares. This is used when a user times out and may still have their finger held down
void eraseRegions()
{
    for (uint8_t i; i<TOTAL_REGION_CNT; i++)
    {
        simonDisplay_drawSquare(i,true); // true means erase
    }
}


// Standard tick function.
void simonControl_tick()
{
    static uint8_t message_counter = 0; // counter for delay messages on screen
    static uint8_t level_up_counter = 0; // counter that times out if the user doesn't decide to level up (by not touching the screen)
    static uint8_t adcCounter = 0;        // counter for adc Touchpad
    static uint8_t seedCounter = 0;       // counter for random number generator. Used at start of game
    switch (currentState_control)
    {
    case init_st:
    {
		//Init starting iteration length
        globals_setSequenceIterationLength(STARTING_ITERATION_LENGTH);

		//Initialize variables
        player_quit = false;
        game_intro = true;

		//Display welcome message
        printWelcome(false); // false means do not erase
        currentState_control = welcome_screen_st; // Transition to welcome screen state
    }
    break;

    case welcome_screen_st:
    {
		// Remain in state until screen is touched
        if(display_isTouched())
        {
            display_clearOldTouchData();
            currentState_control = adc_Counter_running_st; // transition to adc Counter running state
            srand(seedCounter); // generate a seed for random number generator

            //generate random sequence of length 4
            genSequence(STARTING_SEQUENCE_LENGTH);
            globals_setSequence(randomSequence, STARTING_SEQUENCE_LENGTH);

            //reset seedCounter
            seedCounter = 0;
        }

        else
            currentState_control = welcome_screen_st; 

    }
    break;

    case welcome_waiting_release_st:
    {
        //Remain in state until display is released
        if(!display_isTouched())
        {
            //Once released, erase what's on screen and transition to flash_enable_st
            printWelcome(true); // true means erase

            currentState_control = flash_enable_st;

            //Game has begun, so set game_intro to false
            game_intro = false;
        }
        else
            currentState_control = welcome_waiting_release_st;
    }
    break;

    case flash_enable_st:
    {

        //Enable flash state machine
        flashSequence_enable();

        if (flashSequence_isComplete())
        {
            currentState_control = verify_enable_st;


            //Disable flash state machine to advance it
            flashSequence_disable();

            //Draw buttons
            simonDisplay_drawAllButtons();
        }

        else
        {
            //remain in current state
            currentState_control = flash_enable_st;
        }

    }
    break;

    case verify_enable_st:
    {


       //Enable verify state machine
        verifySequence_enable();

        if (verifySequence_isComplete())
        {
            //Check for verify sequence error:
            if(verifySequence_isTimeOutError() || verifySequence_isUserInputError())
            {
                currentState_control = error_check_st;
                //Erase regions if necessary
                eraseRegions();

            }
            //Test to see if level was completed
            else if (globals_getSequenceIterationLength() == globals_getSequenceLength())
            {
               currentState_control = error_check_st;
            }
            else
            {

                //Transition to flash_enable_st to flash new incremented sequence:

                //Increment iteration length
                uint16_t current_it_length = globals_getSequenceIterationLength();
                globals_setSequenceIterationLength(current_it_length+ INCREMENT_SEQUENCE_VALUE);

                currentState_control = flash_enable_st;
            }

            //Disable verify state machine to advance it
            verifySequence_disable();

            //Erase buttons
            simonDisplay_eraseAllButtons();
        }

        else
        {
            //remain in current state
            currentState_control = verify_enable_st;
        }

    }
    break;

    case error_check_st:
    {
        //Check for errors during verify state machine:

        //Timeout error or user input error
        if (verifySequence_isTimeOutError() || verifySequence_isUserInputError())
        {

            printLongestSequence(false); //false means do not erase

        }

        //No error occurred, display "YAY" message and increment the sequence to the next "level"
        else
        {

            printYay(false); //false means do not erase

        }

        currentState_control = delay_message_st;
    }
    break;

    case delay_message_st:
    {
        if (message_counter == MESSAGE_COUNTER_MAX_VALUE)
        {

            // erase current message on screen

            //If there was an error, then the Longest Sequence message is present on the screen
            if(verifySequence_isTimeOutError() || verifySequence_isUserInputError() || player_quit)
            {

                printLongestSequence(true); // true means erase
                currentState_control = init_st; // transfer back to start of game
            }

            //If not, then the printYay message is
            else
            {

                printYay(true); // true means erase
                currentState_control = waiting_next_level_st; // transfer to waiting_next_level_st

                //print "Touch for next level" on screen
                printTouchNextLevel(false); // false means do not erase

            }
            //Reset message counter:
            message_counter = 0;

        }

        else
        {
            currentState_control = delay_message_st;
        }

    }
    break;

    case waiting_next_level_st:
    {
		//Remain in current state until screen is touched or the level_up_counter expires
        if (display_isTouched())
        {
            display_clearOldTouchData();
            currentState_control = adc_Counter_running_st;

            //Reset level_up_counter as it will be used again in waiting_release_st
            level_up_counter = 0;
        }
        else if(level_up_counter == LEVEL_UP_COUNTER_MAX_VALUE)
        {
            //Erase current message on screen
            printTouchNextLevel(true); // true means erase

            //Print longest sequence message:
            printLongestSequence(false); // false means do not erase

            //Set flag indicating that player decided to stop playing
            player_quit = true;
            //Delay message by going to delay_message_st;
            currentState_control = delay_message_st;

            //Reset level_up_counter
            level_up_counter = 0;
        }
        else
            currentState_control = waiting_next_level_st;

    }
    break;

    case adc_Counter_running_st:
    {
        if (adcCounter == ADC_COUNTER_MAX_VALUE)
        {
            //If the game was just started, go to a different waiting release state to handle the release correctly
            if(game_intro)
                currentState_control = welcome_waiting_release_st;
            else
                currentState_control = waiting_release_st;

            //Reset adcCounter
            adcCounter = 0;
        }
        else
            currentState_control = adc_Counter_running_st;

    }
    break;

    case waiting_release_st:
    {
        //Check if a release happened
        if(!display_isTouched())
        {
            //Erase current message on screen
            printTouchNextLevel(true); // true means erase

            //Player wants to continue to next level, generate new sequence with increased sequence value
            genSequence(globals_getSequenceLength()+ INCREMENT_SEQUENCE_VALUE);
            globals_setSequence(randomSequence, globals_getSequenceLength()+ INCREMENT_SEQUENCE_VALUE);

            //Reset sequence iteration length
            globals_setSequenceIterationLength(STARTING_ITERATION_LENGTH);

            currentState_control = flash_enable_st;

            //Reset level_up_counter
            level_up_counter = 0;
        }

        else if (level_up_counter == LEVEL_UP_COUNTER_MAX_VALUE)
        {
            //Erase current message on screen
            printTouchNextLevel(true); // true means erase

            //Print longest sequence message:
            printLongestSequence(false); // false means do not erase

            //Set flag indicating that player decided to stop playing
            player_quit = true;
            //Delay message by going to delay_message_st;
            currentState_control = delay_message_st;

            //Reset level_up_counter
            level_up_counter = 0;

        }

        else
            currentState_control = waiting_release_st;

    }
    break;
    }

    //State actions:
    switch (currentState_control)
    {
    case init_st:
        ; // do nothing
        break;
    case welcome_screen_st:
        //increment seedCounter
        seedCounter++;
        break;
    case welcome_waiting_release_st:
        ;
        break;
    case flash_enable_st:
        ;// do nothing
        break;
    case verify_enable_st:

        break;
    case error_check_st:
        break;
    case delay_message_st:
        //increment message counter
        message_counter++;
        break;
    case waiting_next_level_st:
        //incremenet level_up_counter
        level_up_counter++;
        break;
    case adc_Counter_running_st:
        //increment adcCounter
        adcCounter++;
        break;
    case waiting_release_st:
        //increment level_up_counter
        level_up_counter++;
        break;

    }
}
