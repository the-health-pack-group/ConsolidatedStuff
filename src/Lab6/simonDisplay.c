#include <stdio.h>
#include "simonDisplay.h"
#include "supportFiles/display.h"
#include "supportFiles/utils.h"

#define DISPLAY_HALF_POINT_X DISPLAY_WIDTH/2
#define DISPLAY_HALF_POINT_Y DISPLAY_HEIGHT/2

//Button Starting coordinates
#define BUTTON0_START_X 50
#define BUTTON0_START_Y 30

#define BUTTON1_START_X DISPLAY_HALF_POINT_X + 50
#define BUTTON1_START_Y 30

#define BUTTON2_START_X 50
#define BUTTON2_START_Y DISPLAY_HALF_POINT_Y + 30

#define BUTTON3_START_X DISPLAY_HALF_POINT_X + 50
#define BUTTON3_START_Y DISPLAY_HALF_POINT_Y + 30

//Square Starting coordinates

#define SQUARE_TO_BUTTON_PADDING 20
#define SQUARE_UPPER_START_Y 0
#define SQUARE_LOWER_START_Y DISPLAY_HALF_POINT_Y

#define SQUARE0_START_X BUTTON0_START_X - SQUARE_TO_BUTTON_PADDING
#define SQUARE0_START_Y SQUARE_UPPER_START_Y

#define SQUARE1_START_X BUTTON1_START_X - SQUARE_TO_BUTTON_PADDING
#define SQUARE1_START_Y SQUARE_UPPER_START_Y

#define SQUARE2_START_X BUTTON2_START_X - SQUARE_TO_BUTTON_PADDING
#define SQUARE2_START_Y SQUARE_LOWER_START_Y

#define SQUARE3_START_X BUTTON3_START_X - SQUARE_TO_BUTTON_PADDING
#define SQUARE3_START_Y SQUARE_LOWER_START_Y

#define NUM_BUTTONS 4

//#defines for runTest() function:
#define TOUCH_PANEL_ANALOG_PROCESSING_DELAY_IN_MS 60 // in ms
#define MAX_STR 255
#define TEXT_SIZE 2
#define TEXT_VERTICAL_POSITION 0
#define TEXT_HORIZONTAL_POSITION (DISPLAY_HEIGHT/2)
#define INSTRUCTION_LINE_1 "Touch and release to start the Simon demo."
#define INSTRUCTION_LINE_2 "Demo will terminate after %d touches."
#define DEMO_OVER_MESSAGE_LINE_1 "Simon demo terminated"
#define DEMO_OVER_MESSAGE_LINE_2 "after %d touches."
#define TEXT_VERTICAL_POSITION 0 // Start at the far left.
#define ERASE_THE_SQUARE true  // drawSquare() erases if this is passed in.
#define DRAW_THE_SQUARE false  // drawSquare() draws the square if this is passed in.
#define ERASE_THE_BUTTON true  // drawSquare() erases if this is passed in.
#define DRAW_THE_BUTTON false  // drawSquare() draws the square if this is passed in.


//Computes region number based on x and y coordinates
int8_t simonDisplay_computeRegionNumber(int16_t x, int16_t y)
{
	//Upper left region
   if (x < DISPLAY_HALF_POINT_X && y < DISPLAY_HALF_POINT_Y)
   {
       return SIMON_DISPLAY_REGION_0;
   }

   //Upper right region
   else if (x > DISPLAY_HALF_POINT_X && y < DISPLAY_HALF_POINT_Y)
   {
       return SIMON_DISPLAY_REGION_1;
   }

   //lower left region
   else if (x < DISPLAY_HALF_POINT_X && y > DISPLAY_HALF_POINT_Y)
   {
       return SIMON_DISPLAY_REGION_2;
   }

   //lower right region
   return SIMON_DISPLAY_REGION_3;
}

// Draws a colored "button" that the user can touch.
// The colored button is centered in the region but does not fill the region.
// If erase argument is true, draws the button as black background to erase it.


void simonDisplay_drawButton(uint8_t regionNumber, bool erase)
{
    switch(regionNumber)
    {
    case SIMON_DISPLAY_REGION_0:
    {
		//If erase is true, draw button 0 black, otherwise draw it red
        erase? display_fillRect(BUTTON0_START_X, BUTTON0_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK):
               display_fillRect(BUTTON0_START_X, BUTTON0_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_RED);
    }
        break;

    case SIMON_DISPLAY_REGION_1:
    {
		//If erase is true, draw button 1 black, otherwise draw it yellow
        erase? display_fillRect(BUTTON1_START_X, BUTTON1_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK):
               display_fillRect(BUTTON1_START_X, BUTTON1_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_YELLOW);
    }
        break;
    case SIMON_DISPLAY_REGION_2:
    {
		//If erase is true, draw button 2 black, otherwise draw it blue
        erase? display_fillRect(BUTTON2_START_X, BUTTON2_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK):
               display_fillRect(BUTTON2_START_X, BUTTON2_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLUE);
    }
        break;
    case SIMON_DISPLAY_REGION_3:
    {
		//If erase is true, draw button 3 black, otherwise draw it green
        erase? display_fillRect(BUTTON3_START_X, BUTTON3_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_BLACK):
               display_fillRect(BUTTON3_START_X, BUTTON3_START_Y, SIMON_DISPLAY_BUTTON_WIDTH, SIMON_DISPLAY_BUTTON_HEIGHT, DISPLAY_GREEN);
    }
        break;
    default:
        printf("simonDisplay_drawButton hit default\n\r");
        break;
    }
}

// draw all buttons 
void simonDisplay_drawAllButtons()
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        simonDisplay_drawButton(i, false); // false means to not erase
    }
}

// erase all buttons
void simonDisplay_eraseAllButtons()
{
    for (uint8_t i = 0; i < NUM_BUTTONS; i++)
    {
        simonDisplay_drawButton(i, true); // true means to erase
    }
}

//Draws a colored square, to make it easier for the user to see what was pressed
void simonDisplay_drawSquare(uint8_t regionNo, bool erase)
{
    switch(regionNo)
        {
        case SIMON_DISPLAY_REGION_0:
        {
			//If erase is true, draw square 0 black, otherwise draw it red
            erase? display_fillRect(SQUARE0_START_X, SQUARE0_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK):
                   display_fillRect(SQUARE0_START_X, SQUARE0_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_RED);
        }
            break;

        case SIMON_DISPLAY_REGION_1:
        {
			//If erase is true, draw square 1 black, otherwise draw it yellow
            erase? display_fillRect(SQUARE1_START_X, SQUARE1_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK):
                   display_fillRect(SQUARE1_START_X, SQUARE1_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_YELLOW);
        }
            break;
        case SIMON_DISPLAY_REGION_2:
        {
			//If erase is true, draw square 2 black, otherwise draw it blue
            erase? display_fillRect(SQUARE2_START_X, SQUARE2_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK):
                   display_fillRect(SQUARE2_START_X, SQUARE2_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLUE);
        }
            break;
        case SIMON_DISPLAY_REGION_3:
        {
			//If erase is true, draw square 3 black, otherwise draw it green
            erase? display_fillRect(SQUARE3_START_X, SQUARE3_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_BLACK):
                   display_fillRect(SQUARE3_START_X, SQUARE3_START_Y, SIMON_DISPLAY_SQUARE_WIDTH, SIMON_DISPLAY_SQUARE_HEIGHT, DISPLAY_GREEN);
        }
            break;
        default:
            printf("simonDisplay_drawSquare hit default\n\r");
            break;
        }
}


void simonDisplay_runTest(uint16_t touchCount) {
  display_init();         // Always initialize the display.
  char str[MAX_STR];      // Enough for some simple printing.
  uint8_t regionNumber = 0;   // Convenience variable.
  uint16_t touches = 0;   // Terminate when you receive so many touches.
  // Write an informational message and wait for the user to touch the LCD.
  display_fillScreen(DISPLAY_BLACK);              // clear the screen.
  display_setCursor(TEXT_VERTICAL_POSITION, TEXT_HORIZONTAL_POSITION); // move to the middle of the screen.
  display_setTextSize(TEXT_SIZE);                 // Set the text size for the instructions.
  display_setTextColor(DISPLAY_RED, DISPLAY_BLACK);   // Reasonable text color.
  sprintf(str, INSTRUCTION_LINE_1);                   // Copy the line to a buffer.
  display_println(str);                               // Print to the LCD.
  display_println();                                  // new-line.
  sprintf(str, INSTRUCTION_LINE_2, touchCount);       // Copy the line to a buffer.
  display_println(str);                               // Print to the LCD.
  while (!display_isTouched());       // Wait here until the screen is touched.
  while (display_isTouched());        // Now wait until the touch is released.
  display_fillScreen(DISPLAY_BLACK);  // Clear the screen.
  simonDisplay_drawAllButtons();      // Draw all of the buttons.
  bool touched = false;         // Keep track of when the pad is touched.
  int16_t x, y;                     // Use these to keep track of coordinates.
  uint8_t z;                        // This is the relative touch pressure.
  while (touches < touchCount) {  // Run the loop according to the number of touches passed in.
    if (!display_isTouched() && touched) {          // user has stopped touching the pad.
      simonDisplay_drawSquare(regionNumber, ERASE_THE_SQUARE);  // Erase the square.
      simonDisplay_drawButton(regionNumber, DRAW_THE_BUTTON);  //  Draw the button.
      touched = false;                  // Released the touch, set touched to false.
    } else if (display_isTouched() && !touched) {   // User started touching the pad.
      touched = true;                             // Just touched the pad, set touched = true.
      touches++;                                  // Keep track of the number of touches.
      display_clearOldTouchData();                // Get rid of data from previous touches.
      // Must wait this many milliseconds for the chip to do analog processing.
      utils_msDelay(TOUCH_PANEL_ANALOG_PROCESSING_DELAY_IN_MS);
      display_getTouchedPoint(&x, &y, &z);        // After the wait, get the touched point.
      regionNumber = simonDisplay_computeRegionNumber(x, y);// Compute the region number, see above.
      simonDisplay_drawSquare(regionNumber, DRAW_THE_SQUARE);  // Draw the square (erase = false).
    }
  }
  // Done with the demo, write an informational message to the user.
  display_fillScreen(DISPLAY_BLACK);        // clear the screen.
  // Place the cursor in the middle of the screen.
  display_setCursor(TEXT_VERTICAL_POSITION, TEXT_HORIZONTAL_POSITION);
  display_setTextSize(TEXT_SIZE); // Make it readable.
  display_setTextColor(DISPLAY_RED, DISPLAY_BLACK);  // red is foreground color, black is background color.
  sprintf(str, DEMO_OVER_MESSAGE_LINE_1);    // Format a string using sprintf.
  display_println(str);                     // Print it to the LCD.
  sprintf(str, DEMO_OVER_MESSAGE_LINE_2, touchCount);  // Format the rest of the string.
  display_println(str);  // Print it to the LCD.
}
