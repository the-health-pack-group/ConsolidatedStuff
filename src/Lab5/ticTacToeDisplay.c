#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "supportFiles/display.h"
#include "supportFiles/utils.h"
#include "ticTacToeDisplay.h"
#include "src/Lab2/buttons.h"
#include "src/Lab2/switches.h"


#define X_START 0 // Starting X value on display
#define Y_START 0 // Starting Y value on display
#define X_PADDING_HORIZONTAL 22 // Horizontal padding for the X character 
#define X_PADDING_VERTICAL 6 // Vertical padding for the X character 
#define CIRCLE_RADIUS DISPLAY_WIDTH/12 // Radius of O character
#define TERMINATE_TEST_SEQUENCE 0x2 // Sequence to determine when button 1 was pressed during the run_test() routine.
#define CLEAR_SCREEN_SEQUENCE 0x1   // Sequence to determine when button 0 was pressed during the run_test() routine.
#define SWITCH_0_HIGH 1 // Sequence to determine when switch 0 is high during the run_test() routine.
#define VERTICAL_LINE_1_X DISPLAY_WIDTH/3 //First vertical line of the game board
#define VERTICAL_LINE_2_X 2*(DISPLAY_WIDTH/3) // Second vertical line of the game board
#define HORIZONTAL_LINE_1_Y DISPLAY_HEIGHT/3  // First horizontal line of the game board
#define HORIZONTAL_LINE_2_Y 2*(DISPLAY_HEIGHT/3) // Second horizontal line of the game board
#define WELCOME_SCRREN_START_X 20 // Starting x-coordinate for the welcome screen message
#define WELCOME_SCRREN_START_Y 100 // Starting x-coordinate for the welcome screen message
#define WELCOME_SCREEN_START_SIZE 2 // Text size of Welcome Screen
#define EXCESS_PADDING 2 // Used when drawing and erasing characters to compensate for excess horizontal and vertical padding. Since there
                         // is padding used on both sides of a character, and the starting coordinate is determined by 0 1 2 (columns and rows), there can be excess padding.

#define COLUMN_WIDTH DISPLAY_WIDTH / 3 // Width of each column
#define ROW_HEIGHT DISPLAY_HEIGHT / 3  // height of each row
#define CENTER_SQUARE_X DISPLAY_WIDTH/6 // center of square x-coordinate
#define CENTER_SQUARE_Y DISPLAY_HEIGHT/6 // center of square y-coordinate
#define ROW_2_HEIGHT 2 * (ROW_HEIGHT) // Starting height of final row
#define COLUMN_2_WIDTH 2 * (COLUMN_WIDTH) // Starting width of final column 
#define ROW_0 0  // row 0
#define ROW_1 1  // row 1
#define ROW_2 2  // row 2
#define COLUMN_0 0 // column 0
#define COLUMN_1 1 // column 1
#define COLUMN_2 2 // column 2
#define ADC_DELAY 50


void ticTacToeDisplay_init()
{
    display_init();
    // Blank the screen
    display_fillScreen(DISPLAY_BLACK);

    //display welcome message
    display_setCursor(WELCOME_SCRREN_START_X, WELCOME_SCRREN_START_Y); // Start text around the middle of the screen
    display_setTextColor(DISPLAY_WHITE);  // Make the text white.
    display_setTextSize(WELCOME_SCREEN_START_SIZE);
    display_println("Touch Board to play as X\n        --or--\n wait for the computer\n      and play as O"); // Welcome message with proper spacing

}


void ticTacToeDisplay_drawX(uint8_t row, uint8_t column, bool erase)
{
    //Compute first diagonal line starting coordinates:
    int16_t diagonal_1_Xstart = X_START + column*(COLUMN_WIDTH) + X_PADDING_HORIZONTAL;
    int16_t diagonal_1_Ystart = Y_START + row*(ROW_HEIGHT) + X_PADDING_VERTICAL;

    //Compute second diagonal line starting coordinates
    int16_t diagonal_2_Xstart = diagonal_1_Xstart;
    int16_t diagonal_2_Ystart = diagonal_1_Ystart + (ROW_HEIGHT) - EXCESS_PADDING*(X_PADDING_VERTICAL);

    if (erase)
    {
        //Erase X

        //erase first diagonal line from top left
        display_drawLine(diagonal_1_Xstart, diagonal_1_Ystart, diagonal_1_Xstart + (COLUMN_WIDTH) - EXCESS_PADDING*(X_PADDING_HORIZONTAL),diagonal_1_Ystart + (ROW_HEIGHT) - EXCESS_PADDING*(X_PADDING_VERTICAL),DISPLAY_BLACK);

        //erase second diagonal line from bottom left
        display_drawLine(diagonal_2_Xstart,diagonal_2_Ystart, diagonal_2_Xstart + (COLUMN_WIDTH) - EXCESS_PADDING*(X_PADDING_HORIZONTAL), diagonal_1_Ystart,DISPLAY_BLACK);
        return;
    }

    //draw first diagonal line from top left
    display_drawLine(diagonal_1_Xstart, diagonal_1_Ystart, diagonal_1_Xstart + (COLUMN_WIDTH) - EXCESS_PADDING*(X_PADDING_HORIZONTAL),diagonal_1_Ystart + ROW_HEIGHT) - EXCESS_PADDING*(X_PADDING_VERTICAL),DISPLAY_YELLOW);

    //draw second diagonal line from bottom left
    display_drawLine(diagonal_2_Xstart,diagonal_2_Ystart, diagonal_2_Xstart + (COLUMN_WIDTH) - EXCESS_PADDING*(X_PADDING_HORIZONTAL), diagonal_1_Ystart,DISPLAY_YELLOW);
}

void ticTacToeDisplay_drawO(uint8_t row, uint8_t column, bool erase)
{
    //Compute center of circle
    int16_t circle_Xstart = X_START + column*(COLUMN_WIDTH) + (CENTER_SQUARE_X);
    int16_t circle_Ystart = Y_START + row*(ROW_HEIGHT) + (CENTER_SQUARE_Y);

    if (erase)
    {
        //Erase Circle

        display_drawCircle(circle_Xstart, circle_Ystart, CIRCLE_RADIUS, DISPLAY_BLACK);
        return;
    }

    // draw circle
    display_drawCircle(circle_Xstart, circle_Ystart, CIRCLE_RADIUS, DISPLAY_YELLOW);


}

void ticTacToeDisplay_touchScreenComputeBoardRowColumn(uint8_t* row, uint8_t* column)
{
    //declare 3 local variables to read touch coordinates.
    int16_t x;
    int16_t y;
    uint8_t z;


    //get display touch coordinates:
    display_getTouchedPoint(&x, &y, &z);

    //find the row coordinate
    if (y < ROW_HEIGHT)
        *row = ROW_0;
    else if (y > ROW_HEIGHT && y < ROW_2_HEIGHT)
        *row = ROW_1;
    else
        *row = ROW_2;

    //find the column coordinate
    if (x < COLUMN_WIDTH)
        *column = COLUMN_0;
    else if (x > COLUMN_WIDTH && x < COLUMN_2_WIDTH)
        *column = COLUMN_1;
    else
        *column = COLUMN_2;
    return;
}

void ticTacToeDisplay_drawBoardLines()
{
    //draw vertical lines:
    display_drawLine(VERTICAL_LINE_1_X, Y_START, VERTICAL_LINE_1_X, DISPLAY_HEIGHT,DISPLAY_YELLOW);
    display_drawLine(VERTICAL_LINE_2_X, Y_START, VERTICAL_LINE_2_X, DISPLAY_HEIGHT,DISPLAY_YELLOW);

    // draw horizontal lines:
    display_drawLine(X_START, HORIZONTAL_LINE_1_Y, DISPLAY_WIDTH, HORIZONTAL_LINE_1_Y,DISPLAY_YELLOW);
    display_drawLine(X_START, HORIZONTAL_LINE_2_Y, DISPLAY_WIDTH, HORIZONTAL_LINE_2_Y,DISPLAY_YELLOW);
}

void ticTacToeDisplay_runTest()
{
	//Declare temporary variables to store selected squares
    uint8_t row;
    uint8_t column;
	//Declare a boolean that will decide if the person testing is drawing an X or an O
    bool player_is_o;
	//Execute the run_Test() functionality until button 1 is pressed
    while ((buttons_read() & BUTTONS_BTN1_MASK) != TERMINATE_TEST_SEQUENCE)
    {
		// If switch 0 is high, the tester will draw O
        if ((switches_read() & SWITCHES_SW0_MASK) == SWITCH_0_HIGH)
            player_is_o = true;
        else
			// the tester will draw X
            player_is_o = false;

		// If button 0 is pressed, the screen will erase and redraw the game board
        if ((buttons_read() & BUTTONS_BTN0_MASK) == CLEAR_SCREEN_SEQUENCE)
        {
            //clear display
            display_fillScreen(DISPLAY_BLACK);
            ticTacToeDisplay_drawBoardLines();
        }

		// when display is touched, the square selected is determined, and the appropriate character is drawn
        if (display_isTouched())
        {
			//Erase old touch coordinates, delay for the adc to settle, determine what was pressed
            display_clearOldTouchData();
            utils_msDelay(ADC_DELAY);
            ticTacToeDisplay_touchScreenComputeBoardRowColumn(&row, &column);
			 
            if (player_is_o)
				// draw O character
                ticTacToeDisplay_drawO(row, column, false); // false means do not erase
            else
				// draw X character
                ticTacToeDisplay_drawX(row, column, false); // false means do not erase
        }

    }
    //clear display
    display_fillScreen(DISPLAY_BLACK);
    display_setTextColor(DISPLAY_WHITE); // Set text color to display terminate test message
    display_println("Terminating Test");  



}


