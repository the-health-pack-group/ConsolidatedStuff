
// Print out "hello world" on both the console and the LCD screen.

#include <stdio.h>
#include "supportFiles/display.h"
/*int main() {
	display_init();  // Must init all of the software and underlying hardware for LCD.
	display_fillScreen(DISPLAY_BLACK);  // Blank the screen.
	display_setCursor(0, 0);            // The upper left of the LCD screen.
	display_setTextColor(DISPLAY_RED);  // Make the text red.
	display_setTextSize(2);             // Make the text a little larger.

	display_drawLine( 0, 0, 320, 240, DISPLAY_GREEN);
	display_drawLine(0, 240, 320,0, DISPLAY_GREEN);

	display_fillTriangle(130,30,190,30,160,90, DISPLAY_YELLOW);
	display_drawTriangle(130,210,190,210,160,150, DISPLAY_YELLOW);
	display_fillCircle(235,120,30,DISPLAY_RED);
	display_drawCircle(85,120,30,DISPLAY_RED);

	//display_println("hello world (on the LCD)!");    // This actually prints the string to the LCD.
	printf("hello world!");  // This prints on the console.
}*/

// This function must be defined but can be left empty for now.
// You will use this function in a later lab.
// It is called in the timer interrupt service routine (see interrupts.c in supportFiles).
void isr_function() {
    // Empty for now.
}
