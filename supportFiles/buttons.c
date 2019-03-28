#include "buttons.h"
#include "supportFiles/display.h"
#include "xil_io.h"
#include "xparameters.h"
#include  <stdio.h>

// GPIO Input needs to be a 1 to configure the buttons as input instead of output
#define GPIO_CONFIG_INPUT 1

#define BUTTONS_GPIO_DEVICE_BASE_ADDRESS XPAR_PUSH_BUTTONS_BASEADDR // base address given in "xparamters.h"

#define TRI_REGISTER_OFFSET 0x04 // offset for tri-register from base address
#define DATA_REGISTER_OFFSET 0x0 // offset for data-register from base address

#define TERMINATE_BUTTONS_SEQUENCE 0xF // user pushes all four buttons down to terminate button portion of program

#define BUTTON0_DEPRESSED 0x1 // hex value for button 0 when depressed (first bit is 1)
#define BUTTON1_DEPRESSED 0x2 // hex value for button 1 when depressed (second bit is 1)
#define BUTTON2_DEPRESSED 0x4 // hex value for button 2 when depressed (third bit is 1)
#define BUTTON3_DEPRESSED 0x8 // hex value for button 3 when depressed (fourth bit is 1)

#define BUTTON0_X 240 // button0 x coordinate- width of screen minus 80
#define BUTTON1_X 160 // button1 x coordinate- minus 160
#define BUTTON2_X 80  // button2 x coordinate- 240
#define BUTTON3_X 0   // button3 x coordinate- screen

#define ALL_BUTTON_Y_START 0    // all buttons y coordinate start at y=0
#define ALL_BUTTON_Y_END 120    // all buttons y coordinate end at y = 120
#define ALL_BUTTON_TEXT_MARGIN 10 // all buttons have a text x margin of 10 from where their x coordinate starts
#define ALL_BUTTON_TEXT_Y 40      // all buttons have a text y margin at y = 40
#define ALL_BUTTON_TEXT_SIZE 2
#define BUTTON_WIDTH 80          // all button width = 80 or screen width/4

#define BUTTON_ON 1               // button state
#define BUTTON_OFF 0



char button0 = 0;   // These chars are used to keep track of current state of each button
char button1 = 0;
char button2 = 0;
char button3 = 0;

/* This function initializes the necessary registers using the built-in Xil_out and Xil_in functions
 as defined in the xil_io.h file.
 @return returns one of two statuses depending on if the correct write went through or not.
*/
int32_t buttons_init()
{

    // write GpioRegister:
    Xil_Out32(BUTTONS_GPIO_DEVICE_BASE_ADDRESS +TRI_REGISTER_OFFSET,GPIO_CONFIG_INPUT);

    // READ GpioRegister:

    // first assign test value to see if correct value was written
     int32_t test_register =0;
     test_register = Xil_In32(BUTTONS_GPIO_DEVICE_BASE_ADDRESS + TRI_REGISTER_OFFSET);


     // read test value and return status based on value read.

    if (test_register ==GPIO_CONFIG_INPUT)
     {

         return BUTTONS_INIT_STATUS_OK;

     }


     return BUTTONS_INIT_STATUS_FAIL;


}


/* A simple function to read the current 32 bit value of the buttons pressed on the zybo board.
 * @param return- a 32 bit value of buttons pressed.
 */
int32_t buttons_read()
{

	int32_t myvar = Xil_In32(BUTTONS_GPIO_DEVICE_BASE_ADDRESS + DATA_REGISTER_OFFSET);
    return myvar;
}



/* This function looks at all the buttons one by one and decides whether to display a rectangle, do nothing, or clear a rectangle.
 * The sequence as described is implemented using an if statement, followed by and else if, and then finally an else. Repeated 4 times. Once for each button.
 * @param curr_button = current value of buttons depressed, found in lowest four bits with 1 meaning button is depressed, and off is released.
 */

void draw_button( int32_t curr_button)
{
     // **********Checking button 0 first**********

    // mask curr_button with button0 mask to get button0's value, if depressed and it's current state is off (released), draw rectangle
	if( (curr_button & BUTTONS_BTN0_MASK) == BUTTON0_DEPRESSED && (button0 == BUTTON_OFF))
	{
        // Display rectangle with button0 coordinates
		display_fillRect(BUTTON0_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_WHITE);
		display_setCursor(BUTTON0_X + ALL_BUTTON_TEXT_MARGIN, ALL_BUTTON_TEXT_Y);
		display_setTextColor(DISPLAY_BLACK);
		display_setTextSize(ALL_BUTTON_TEXT_SIZE);
		display_println("BTN 0");
		button0 = BUTTON_ON;
	}

	// same as above, but if the button's current state is on (depressed), do nothing
	else if((curr_button & BUTTONS_BTN0_MASK) == BUTTON0_DEPRESSED && (button0 == BUTTON_ON))
	    ;

    // button0 is not depressed, so clear the rectangle
	else
	{
	    display_fillRect(BUTTON0_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_BLACK);
	    button0 = BUTTON_OFF;
	}

	                   // **********Checking button 1 **********

	         // mask curr_button with button1 mask to get button1's value, if depressed and it's current state is off (released), draw rectangle

	if ( (curr_button & BUTTONS_BTN1_MASK) == BUTTON1_DEPRESSED && (button1 == BUTTON_OFF))
	{
	            // Display rectangle with button1 coordinates
	            display_fillRect(BUTTON1_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_CYAN);
	            display_setCursor(BUTTON1_X + ALL_BUTTON_TEXT_MARGIN, ALL_BUTTON_TEXT_Y);
	            display_setTextColor(DISPLAY_BLACK);
	            display_setTextSize(ALL_BUTTON_TEXT_SIZE);
	            display_println("BTN 1");
	            button1 = BUTTON_ON;
	}
	        // same as above, but if the button's current state is on (depressed), do nothing
	else if ((curr_button & BUTTONS_BTN1_MASK) == BUTTON1_DEPRESSED && (button1 == BUTTON_ON))
	    ;

	        // button1 is not depressed, so clear the rectangle
	else
	{
	    display_fillRect(BUTTON1_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_BLACK);
	    button1 = BUTTON_OFF;
	}

	                   // **********Checking button 2 **********

	              // mask curr_button with button2 mask to get button2's value, if depressed and it's current state is off (released), draw rectangle
	if ( (curr_button & BUTTONS_BTN2_MASK) == BUTTON2_DEPRESSED && (button2 == BUTTON_OFF))
	{
	            // Display rectangle with button2 coordinates
	            display_fillRect(BUTTON2_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_GREEN);
	            display_setCursor(BUTTON2_X + ALL_BUTTON_TEXT_MARGIN, ALL_BUTTON_TEXT_Y);
	            display_setTextColor(DISPLAY_BLACK);
	            display_setTextSize(ALL_BUTTON_TEXT_SIZE);
	            display_println("BTN 2");
	            button2 = BUTTON_ON;
	}

	// same as above, but if the button's current state is on (depressed), do nothing
	else if ((curr_button & BUTTONS_BTN2_MASK) == BUTTON2_DEPRESSED && (button2 == BUTTON_ON))
	    ;

	// button2 is not depressed, so clear the rectangle
	else
	{
	    display_fillRect(BUTTON2_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_BLACK);
	    button2 = BUTTON_OFF;
	}
	                       // **********Checking button 3 **********

	    // mask curr_button with button0 mask to get button0's value, if depressed and it's current state is off (released), draw rectangle
	if ((curr_button & BUTTONS_BTN3_MASK) == BUTTON3_DEPRESSED && (button3 == BUTTON_OFF))
	{
	            // Display rectangle with button3 coordinates
	            display_fillRect(BUTTON3_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_BLUE);
	            display_setCursor(BUTTON3_X + ALL_BUTTON_TEXT_MARGIN, ALL_BUTTON_TEXT_Y);
	            display_setTextColor(DISPLAY_BLACK);
	            display_setTextSize(ALL_BUTTON_TEXT_SIZE);
	            display_println("BTN 3");
	            button3 = BUTTON_ON;
	}
	     // same as above, but if the button's current state is on (depressed), do nothing
	else if ((curr_button & BUTTONS_BTN3_MASK) == BUTTON3_DEPRESSED && (button3 == BUTTON_ON))
	    ;
	           // button3 is not depressed, so clear the rectangle
	else
	{
	    display_fillRect(BUTTON3_X,ALL_BUTTON_Y_START,BUTTON_WIDTH,ALL_BUTTON_Y_END,DISPLAY_BLACK);
	    button3 = BUTTON_OFF;
	}

}


/* This function calls buttons_init and checks to see that everything is okay before continuing.
 * The function then calls the draw_button function until the current_button sequence equals the terminate sequence.
 */

void buttons_runTest()
{

    // call button_init function and return status
    if(buttons_init() != BUTTONS_INIT_STATUS_OK)
    {
        printf("Buttons_init function failed \n\r");
        return;
    }



    int32_t current_buttons =0;

    // continue to read current_buttons until terminate sequence is reached
    while(current_buttons != TERMINATE_BUTTONS_SEQUENCE)
    {
        current_buttons = (buttons_read() & 0xF);
        draw_button(current_buttons);

    }

    //Clear the screen when done
    display_fillScreen(DISPLAY_BLACK);

}
