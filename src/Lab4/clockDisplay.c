#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "clockDisplay.h"
#include "supportFiles/display.h"
#include "supportFiles/utils.h"

#define CLOCK_TEXT_SIZE 4  // Edit this to size the clock
#define CLOCK_ARRAY_TOTAL_SIZE 9 // Value needed for the time string
#define CLOCK_ARRAY_OFFSET_SIZE 8 // total number of elements within time string
#define CLOCK_START_X (DISPLAY_WIDTH - DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE*CLOCK_ARRAY_OFFSET_SIZE)/2 // starting x-coordinate of clock, final value is calculated using ARRAY_OFFSET
#define CLOCK_START_Y (DISPLAY_HEIGHT - DISPLAY_CHAR_HEIGHT*CLOCK_TEXT_SIZE)/2  // starting y-coordinate of clock
#define CLOCK_END_Y CLOCK_START_Y + DISPLAY_CHAR_HEIGHT*CLOCK_TEXT_SIZE // Final value of y coordinate which changes depending on CLOCK_TEXT_SIZE

#define TRIANGLE_BASE_WIDTH DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE*2 // Width of triangles
#define TRIANGLE_TIP_MARGIN TRIANGLE_BASE_WIDTH/2 // How far across the tip of triangle is from x-coordinate of base
#define TRIANGLE_HEIGHT_UP CLOCK_START_Y/2   // Height of triangle tip when triangle is pointing up
#define TRIANGLE_HEIGHT_DOWN DISPLAY_HEIGHT - (CLOCK_START_Y/2) // Height of triangle tip when triangle is pointing up
#define TRIANGLE_PADDING DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE/2 // Padding between the triangles and the clock

//The following values are used as offsets to access specific elements on the display.
#define CLOCK_ELEMENT_0 0 // HRS LARGE
#define CLOCK_ELEMENT_1 1 // HRS SMALL
#define CLOCK_ELEMENT_2 2 // COLON 1
#define CLOCK_ELEMENT_3 3 // MIN LARGE    // delete some of these as they are not used
#define CLOCK_ELEMENT_4 4 // MIN SMALL
#define CLOCK_ELEMENT_5 5 // COLON 2
#define CLOCK_ELEMENT_6 6 // SEC LARGE
#define CLOCK_ELEMENT_7 7 // SEC SMALL

#define TRIANGLE_MIN_START_X CLOCK_START_X + DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE*CLOCK_ELEMENT_3
#define TRIANGLE_SEC_START_X CLOCK_START_X + DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE*CLOCK_ELEMENT_6

#define PAUSE_WHILE_INC_DEC 300 //Used to slow down the display as it increments inside the runTest() function
#define PAUSE_BETWEEN_TESTS 2000 // Used to pause tests between hours, minutes, and seconds and between their respective increments and decrements
#define ADVANCE_TIME_1_SECOND_MAX 60 // Highest value to show that the function rolls over and updates minutes

// Constants to handle rolling over in clock display
#define MAX_SECONDS 59
#define MIN_SECONDS 0
#define MAX_MINUTES 59
#define MIN_MINUTES 0
#define MAX_HOURS 12
#define MIN_HOURS 1

// Variables for keeping track of the value of digits in clock
int16_t seconds; 
int16_t minutes; 
int16_t hours;   

char current_time[CLOCK_ARRAY_TOTAL_SIZE]; // Used to store the current time string
char previous_time[CLOCK_ARRAY_TOTAL_SIZE]; // Used to store the last time string that was written

// Called only once - performs any necessary inits.
// This is a good place to draw the triangles and any other
// parts of the clock display that will never change.
void clockDisplay_init()
{
	// Set clock to 12:59:59
     seconds = MAX_SECONDS;
     minutes = MAX_MINUTES;
     hours = MAX_HOURS;

    // Init display
    display_init();
    // Blank the screen
    display_fillScreen(DISPLAY_BLACK);
    //Set text size
    display_setTextSize(CLOCK_TEXT_SIZE);
   // Display clock on touch display
  clockDisplay_updateTimeDisplay(false);

  // Draw triangles

  //Triangle - Hr up
  display_fillTriangle(CLOCK_START_X,CLOCK_START_Y - TRIANGLE_PADDING,CLOCK_START_X + TRIANGLE_BASE_WIDTH,CLOCK_START_Y - TRIANGLE_PADDING,
  CLOCK_START_X + TRIANGLE_TIP_MARGIN, TRIANGLE_HEIGHT_UP,DISPLAY_GREEN);
  //Triangle - Hr down
  display_fillTriangle(CLOCK_START_X,CLOCK_END_Y + TRIANGLE_PADDING,CLOCK_START_X + TRIANGLE_BASE_WIDTH,CLOCK_END_Y + TRIANGLE_PADDING,
  CLOCK_START_X + TRIANGLE_TIP_MARGIN, TRIANGLE_HEIGHT_DOWN,DISPLAY_GREEN);
  //Triangle - Min up
  display_fillTriangle(TRIANGLE_MIN_START_X,CLOCK_START_Y - TRIANGLE_PADDING,TRIANGLE_MIN_START_X+ TRIANGLE_BASE_WIDTH,CLOCK_START_Y - TRIANGLE_PADDING,
  TRIANGLE_MIN_START_X + TRIANGLE_TIP_MARGIN, TRIANGLE_HEIGHT_UP,DISPLAY_GREEN);
  //Triangle - Min down
  display_fillTriangle(TRIANGLE_MIN_START_X,CLOCK_END_Y + TRIANGLE_PADDING,TRIANGLE_MIN_START_X + TRIANGLE_BASE_WIDTH,CLOCK_END_Y + TRIANGLE_PADDING,
  TRIANGLE_MIN_START_X + TRIANGLE_TIP_MARGIN, TRIANGLE_HEIGHT_DOWN,DISPLAY_GREEN);
  //Triangle - Sec up
  display_fillTriangle(TRIANGLE_SEC_START_X,CLOCK_START_Y - TRIANGLE_PADDING,TRIANGLE_SEC_START_X+ TRIANGLE_BASE_WIDTH,CLOCK_START_Y - TRIANGLE_PADDING,
  TRIANGLE_SEC_START_X + TRIANGLE_TIP_MARGIN, TRIANGLE_HEIGHT_UP,DISPLAY_GREEN);
  //Triangle - Sec down
  display_fillTriangle(TRIANGLE_SEC_START_X,CLOCK_END_Y + TRIANGLE_PADDING,TRIANGLE_SEC_START_X + TRIANGLE_BASE_WIDTH,CLOCK_END_Y + TRIANGLE_PADDING,
  TRIANGLE_SEC_START_X + TRIANGLE_TIP_MARGIN, TRIANGLE_HEIGHT_DOWN,DISPLAY_GREEN);


}

// Updates the time display with latest time, making sure to update only those digits that
// have changed since the last update.
// if forceUpdateAll is true, update all digits.
void clockDisplay_updateTimeDisplay(bool forceUpdateAll)
{
    // store new value into current_time
             sprintf(current_time,"%2d:%02d:%02d",hours,minutes,seconds);

    // Check to see if all parts of clock need to update at once
    if(forceUpdateAll)
    {
		// erase current screen
		display_setTextColor(DISPLAY_BLACK);
		display_setCursor(CLOCK_START_X, CLOCK_START_Y);
		display_println(previous_time);

		//update all digits at once
		display_setTextColor(DISPLAY_GREEN);
		display_setCursor(CLOCK_START_X, CLOCK_START_Y);
		display_println(current_time);

		//store new written value into previous_time
        sprintf(previous_time,"%2d:%02d:%02d",hours,minutes,seconds);
        return;
    }


    // Update digits on display only on values that have changed from previous time string to current time string
    for(int i = 0; i<CLOCK_ARRAY_OFFSET_SIZE; i++)
    {
        if(previous_time[i] != current_time[i])
        {
            //Need to erase and rewrite correct value
            //First align cursor with correct clock digit using array offset (i)
            display_setCursor(CLOCK_START_X + i*DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE,CLOCK_START_Y);
           //erase previous character by drawing a black one
            display_setTextColor(DISPLAY_BLACK);
            display_println(previous_time[i]);
            //redraw correct number with correct color
            display_setTextColor(DISPLAY_GREEN);
            display_setCursor(CLOCK_START_X + i*DISPLAY_CHAR_WIDTH*CLOCK_TEXT_SIZE,CLOCK_START_Y);
            display_println(current_time[i]);

        }
    }

    // store new written value into previous_time
    sprintf(previous_time,"%2d:%02d:%02d",hours,minutes,seconds);


}



// This function checks to see if the digits have reached their respective max or min values, so it can reset them.
// @param next- if true, then clockDisplay_AdvanceTimeOnesecond() is being called and it will increment the next type if necessary. (if sec = 59, then on next inc, minutes will be min+1) 
void check_rollover(bool next) 
{

    // Check Seconds
    if (seconds > MAX_SECONDS)
     {
         if (next)
         minutes++;
		 seconds = MIN_SECONDS;
     }

    if (seconds < MIN_SECONDS)
     {
         seconds = 59;
     }


    // Check Minutes
    if (minutes > MAX_MINUTES)
    {
        if(next)
        hours++;
        minutes = MIN_MINUTES;

    }

    if (minutes < MIN_MINUTES)
    {
        minutes = MAX_MINUTES;
    }



    // Check hours
    if(hours > MAX_HOURS)
     {
        hours = MIN_HOURS;
     }
            
	 if (hours <MIN_HOURS)
      {
         hours = MAX_HOURS;
      }

}


// Reads the touched coordinates and performs the increment or decrement,
// depending upon the touched region.
void clockDisplay_performIncDec()
{
	//declare 3 local variables to read touch coordinates.
    int16_t x;
    int16_t y;
    uint8_t z;


	//get display touch coordinates:
    display_getTouchedPoint(&x, &y, &z);



    //find the coordinate based on 6 possible touch locations

	// touch coordinates will be in either 1 of 6 places
    if(x< DISPLAY_WIDTH/3 && y < DISPLAY_HEIGHT/2)
        hours++;
    else if(x< DISPLAY_WIDTH/3 && y > DISPLAY_HEIGHT/2)
        hours--;
    else if(x>DISPLAY_WIDTH/3 && x < (2*DISPLAY_WIDTH/3) && y < DISPLAY_HEIGHT/2)
        minutes++;
    else if (x>DISPLAY_WIDTH/3 && x < (2*DISPLAY_WIDTH/3) && y > DISPLAY_HEIGHT/2)
        minutes--;
    else if (y > DISPLAY_HEIGHT/2)
        seconds--;
	// only touch area that remains is upper right corner (increment seconds)
    else
        seconds++;

    // check for numbers rolling over, but do not update minutes and hours unlesss their arrows are touched- hence false bool
    check_rollover(false);



    //update display with the received INC/DEC
    clockDisplay_updateTimeDisplay(false);
}



// Advances the time forward by 1 second and update the display.
void clockDisplay_advanceTimeOneSecond()
{

	// advance Seconds
    seconds++;
	// check for rollover but set bool to true to increment minutes and hours if needed
    check_rollover(true);
    clockDisplay_updateTimeDisplay(false);
}


// Run a test of clock-display functions.
void clockDisplay_runTest()
{
	
    // increment hours
    for (int i = 0; i<5; i++)
    {
        hours++;
		//pause so digits are readable:
		utils_msDelay(PAUSE_WHILE_INC_DEC);

        clockDisplay_updateTimeDisplay(false);

    }
    // 2- second delay...
    utils_msDelay(2000);


    // decrement hours
    for (int i = 0; i<5; i++)
    {
        hours--;
		//pause so digits are readable:
		utils_msDelay(PAUSE_WHILE_INC_DEC);
        clockDisplay_updateTimeDisplay(false);

    }
    // 2- second delay...
    utils_msDelay(2000);

    //increment minutes
    for (int i = 0; i<5; i++)
    {
        minutes++;
		//pause so digits are readable:
		utils_msDelay(PAUSE_WHILE_INC_DEC);
        clockDisplay_updateTimeDisplay(false);

    }
    // 2- second delay...
    utils_msDelay(2000);

    //decrement minutes
    for (int i = 0; i<5; i++)
    {
        minutes--;
		//pause so digits are readable:
		utils_msDelay(PAUSE_WHILE_INC_DEC);
        clockDisplay_updateTimeDisplay(false);

    }
    // 2- second delay...
    utils_msDelay(2000);

    //increment seconds
    for (int i = 0; i<5; i++)
    {
        seconds++;
		//pause so digits are readable:
		utils_msDelay(PAUSE_WHILE_INC_DEC);
        clockDisplay_updateTimeDisplay(false);

    }
    // 2- second delay...
    utils_msDelay(2000);

    //decrement seconds
    for (int i = 0; i<5; i++)
    {
        seconds--;
		//pause so digits are readable:
		utils_msDelay(PAUSE_WHILE_INC_DEC);
        clockDisplay_updateTimeDisplay(false);

    }

	//Auto counter enable for a short period of time until rollover is seen in minutes:
	for (int i = 0; i < ADVANCE_TIME_1_SECOND_MAX; i++)
	{
		clockDisplay_advanceTimeOneSecond();
		utils_msDelay(PAUSE_WHILE_INC_DEC);

	}

	// Allow functionality for performIncDec to run
    while(1)
    {

		if (display_isTouched())
		{
			// clear old data
			display_clearOldTouchData();
			//wait 50 ms
			utils_msDelay(100);
			clockDisplay_performIncDec();

		}
    }
}
