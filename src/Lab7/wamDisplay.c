#include "supportFiles/display.h"
#include "src/Lab2/switches.h"
#include "wamDisplay.h"
#include "wamControl.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CIRCLE_RADIUS DISPLAY_WIDTH/12
#define COLUMN_WIDTH DISPLAY_WIDTH / 3 // Width of each column
#define CENTER_POSITION_X DISPLAY_WIDTH/6 // center of square x-coordinate
#define CENTER_POSITION_Y DISPLAY_HEIGHT/6 // center of square y-coordinate
#define MAX_SCORE_ARRAY_SIZE 500

#define COL_0 0
#define COL_1 1
#define COL_2 2

#define SCORE_OFFSET_Y 15

#define SWITCH_VALUE_9 9  // Binary 9 on the switches indicates 9 moles.
#define SWITCH_VALUE_6 6  // Binary 6 on the switches indicates 6 moles.
#define SWITCH_VALUE_4 4  // Binary 9 on the switches indicates 4 moles.
#define SWITCH_MASK 0xf   // Ignore potentially extraneous bits.
/********************** typedefs **********************/
// This keeps track of all mole information.
typedef struct {
        wamDisplay_point_t origin;  // This is the origin of the hole for this mole.
        // A mole is active if either of the tick counts are non-zero. The mole is dormant otherwise.
        // During operation, non-zero tick counts are decremented at a regular rate by the control state machine.
        // The mole remains in his hole until ticksUntilAwake decrements to zero and then he pops out.
        // The mole remains popped out of his hole until ticksUntilDormant decrements to zero.
        // Once ticksUntilDomant goes to zero, the mole hides in his hole and remains dormant until activated again.
        wamDisplay_moleTickCount_t ticksUntilAwake;  // Mole will wake up (pop out of hole) when this goes from 1 -> 0.
        wamDisplay_moleTickCount_t ticksUntilDormant; // Mole will go dormant (back in hole) this goes 1 -> 0.
} wamDisplay_moleInfo_t;

// This will contain pointers to all of the mole info records.
// This will ultimately be treated as an array of pointers.
static wamDisplay_moleInfo_t** wamDisplay_moleInfo;

static wamDisplay_moleCount_e mole_count;


static uint16_t hit_count;     //Hit counts during the game
static uint16_t miss_count;    //Miss counts during the game
static uint16_t current_level; // Current level during the game


//Convenience function to draw two circles in columns 0 and 2, on the edges
void draw4Holes()
{
    //Draw column 0 circles:
    display_fillCircle(CENTER_POSITION_X, CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper left
    display_fillCircle(CENTER_POSITION_X, DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower left

    //Draw column 2 circles
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2 , CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper right
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2 , DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK);

}

//Convenience function to draw two circles in columns 0-2, on the edges
void draw6Holes()
{
    //Draw column 0 circles:
    display_fillCircle(CENTER_POSITION_X, CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper left
    display_fillCircle(CENTER_POSITION_X, DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower left


    //Draw column 1 circles:
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_1 , CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper middle
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_1 , DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower middle


    //Draw column 2 circles
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2 , CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper right
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2 , DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower right

}

//Convenience function to draw all 9 circles
void draw9Holes()
{
    //Draw column 0 circles:
    display_fillCircle(CENTER_POSITION_X, CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper left
    display_fillCircle(CENTER_POSITION_X, DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower left
    display_fillCircle(CENTER_POSITION_X, DISPLAY_HEIGHT/2 -(10), CIRCLE_RADIUS, DISPLAY_BLACK); // middle in column 0

    //Draw column 1 circles:
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_1 , CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper middle
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_1 , DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower middle
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_1, DISPLAY_HEIGHT/2 -(10), CIRCLE_RADIUS, DISPLAY_BLACK); // middle in column 1

    //Draw column 2 circles
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2 , CENTER_POSITION_Y, CIRCLE_RADIUS, DISPLAY_BLACK); // upper right
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2 , DISPLAY_HEIGHT-(CENTER_POSITION_Y +20), CIRCLE_RADIUS, DISPLAY_BLACK); // lower right
    display_fillCircle(CENTER_POSITION_X + COLUMN_WIDTH*COL_2, DISPLAY_HEIGHT/2 -(10), CIRCLE_RADIUS, DISPLAY_BLACK); // middle in column 2
}

// Allocates the memory for wamDisplay_moleInfo_t records.
// Computes the origin for each mole assuming a simple row-column layout:
// 9 moles: 3 rows, 3 columns, 6 moles: 2 rows, 3 columns, 4 moles: 2 rows, 2 columns
// Also inits the tick counts for awake and dormant.
void wamDisplay_computeMoleInfo() {
    // Setup all of the moles, creates and inits mole info records.
    // Create the container array. It contains pointers to each of the mole-hole info records.
    //    wamDisplay_moleInfo =  // Need to uncomment this line and initialize this variable via malloc().
}

// Provide support to set games with varying numbers of moles. This function
// would be called prior to calling wamDisplay_init();
void wamDisplay_selectMoleCount(wamDisplay_moleCount_e moleCount)
{
   mole_count = moleCount;
}

// Call this before using any wamDisplay_ functions.
void wamDisplay_init()
{

}

// Draw the game display with a background and mole holes.
void wamDisplay_drawMoleBoard()
{
    display_fillRect(0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT - 20, DISPLAY_GREEN);

    switch(mole_count)
    {
    case wamDisplay_moleCount_4:
        draw4Holes();
        break;
    case wamDisplay_moleCount_6:
        draw6Holes();
        break;
    case wamDisplay_moleCount_9:
        draw9Holes();
        break;
    default:
        printf("wamDisplay_drawMoleBoard hit default\r\n");
        break;
    }

    wamDisplay_drawScoreScreen();

}

// Draw the initial splash (instruction) screen.
void wamDisplay_drawSplashScreen()
{
    //Set cursor to middle of screen
    display_setCursor(DISPLAY_WIDTH/6, DISPLAY_HEIGHT/2);
    display_setTextSize(3);
    display_setTextColor(DISPLAY_WHITE);
    display_println("Whack a Mole!\n");
    display_setTextSize(2);
    display_println("     Touch to start");

}

// Draw the game-over screen.
void wamDisplay_drawGameOverScreen()
{
    //Declare 3 char arrays to display, hits, misses, and level
    char hitchar[MAX_SCORE_ARRAY_SIZE];
    char misschar[MAX_SCORE_ARRAY_SIZE];
    char levelchar[MAX_SCORE_ARRAY_SIZE];

    //Set cursor to a bit above middle of screen
    display_setCursor(DISPLAY_WIDTH/5, DISPLAY_HEIGHT/2 - 50);
    display_setTextSize(3);
    display_setTextColor(DISPLAY_WHITE);
    display_println("Game Over\n");

    //Reduce Text size
    display_setTextSize(2);

    //Display hits
    sprintf(hitchar,"        Hits:%d",wamDisplay_getHitScore());
    display_println(hitchar);

    //Display misses
    sprintf(misschar,"       Misses:%d",wamDisplay_getMissScore());
    display_println(misschar);

    //Display level
    sprintf(levelchar,"      Final Level:%d\n",wamDisplay_getLevel());
    display_println(levelchar);

    display_println("  (Touch to Try Again)");

}

// Selects a random mole and activates it.
// Activating a mole means that the ticksUntilAwake and ticksUntilDormant counts are initialized.
// See the comments for wamDisplay_moleInfo_t for details.
// Returns true if a mole was successfully activated. False otherwise. You can
// use the return value for error checking as this function should always be successful
// unless you have a bug somewhere.
bool wamDisplay_activateRandomMole()
{

}

// This takes the provided coordinates and attempts to whack a mole. If a
// mole is successfully whacked, all internal data structures are updated and
// the display and score is updated. You can only whack a mole if the mole is awake (visible).
// The return value can be used during testing (you could just print which mole is
// whacked without having to implement the entire game).
wamDisplay_moleIndex_t wamDisplay_whackMole(wamDisplay_point_t* whackOrigin)
{

}

// This updates the ticksUntilAwake/ticksUntilDormant clocks for all of the moles.
void wamDisplay_updateAllMoleTickCounts()
{

}

// Returns the count of currently active moles.
// A mole is active if it is not dormant, if:
// ticksUntilAwake or ticksUntilDormant are non-zero (in the moleInfo_t struct).
uint16_t wamDisplay_getActiveMoleCount()
{

}

// Sets the hit value in the score window.
void wamDisplay_setHitScore(uint16_t hits)
{
   hit_count = hits;
}

// Gets the current hit value.
uint16_t wamDisplay_getHitScore()
{
  return hit_count;
}

// Sets the miss value in the score window.
void wamDisplay_setMissScore(uint16_t misses)
{
  miss_count = misses;
}

// Gets the miss value.
// Can be used for testing and other functions.
uint16_t wamDisplay_getMissScore()
{
   return miss_count;
}

// Sets the level value on the score board.
void wamDisplay_incrementLevel()
{
   current_level++;
}

// Retrieves the current level value.
// Can be used for testing and other functions.
uint16_t wamDisplay_getLevel()
{
   return current_level;
}

// Completely draws the score screen.
// This function renders all fields, including the text fields for "Hits" and "Misses".
// Usually only called once when you are initializing the game.
void wamDisplay_drawScoreScreen()
{

    //Set cursor to bottom of board
    display_setCursor(0, DISPLAY_HEIGHT - SCORE_OFFSET_Y);
    display_setTextSize(2);
    display_setTextColor(DISPLAY_WHITE);


    //Declare a char array to print the score screen
    char mychar[MAX_SCORE_ARRAY_SIZE];
    sprintf(mychar,"Hit:%d    Miss:%d  Level:%d",wamDisplay_getHitScore(),wamDisplay_getMissScore(),wamDisplay_getLevel());
    display_println(mychar);

}

// Make this function available for testing purposes.
void wamDisplay_incrementMissScore()
{
  miss_count++;
}

// Reset the scores and level to restart the game.
void wamDisplay_resetAllScoresAndLevel()
{
   hit_count = 0;
   miss_count = 0;
   current_level = 0;
}

void selectMoleCountFromSwitches(uint16_t switchValue) {
    switch(switchValue & SWITCH_MASK) {
    case SWITCH_VALUE_9:    // this switch pattern = 9 moles.
        wamDisplay_selectMoleCount(wamDisplay_moleCount_9);
        break;
    case SWITCH_VALUE_6:    // this switch pattern = 6 moles.
        wamDisplay_selectMoleCount(wamDisplay_moleCount_6);
        break;
    case SWITCH_VALUE_4:   // this switch pattern = 4 moles.
        wamDisplay_selectMoleCount(wamDisplay_moleCount_4);
        break;
    default:    // Any other pattern of switches = 9 moles.
        wamDisplay_selectMoleCount(wamDisplay_moleCount_9);
        break;
    }
}

// Test function that can be called from main() to demonstrate milestone 1.
// Invoking this function should provide the same behavior as shown in the Milestone 1 video.
void wamDisplay_runMilestone1_test()
{
    while(1)
    {
        display_fillScreen(DISPLAY_BLACK);

        wamDisplay_drawSplashScreen();
        selectMoleCountFromSwitches(switches_read());

        while (!display_isTouched()); // wait for user to touch screen
        while (display_isTouched());  // now wait for user to remove finger
        display_fillScreen(DISPLAY_BLACK);

        wamDisplay_drawMoleBoard();

        while (!display_isTouched()); // wait for user to touch screen
        while (display_isTouched());  // now wait for user to remove finger

        display_fillScreen(DISPLAY_BLACK);
        wamDisplay_drawGameOverScreen();

        while (!display_isTouched()); // wait for user to touch screen
        while (display_isTouched());  // now wait for user to remove finger

    }


}







