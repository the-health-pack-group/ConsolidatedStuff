#include "wamDisplay.h"
#include "wamControl.h"
#include "supportFiles/utils.h"
#include "supportFiles/display.h"
#include "src/Lab3/intervalTimer.h"  // Modify as necessary to point to your intervalTimer.h
#include "supportFiles/leds.h"
#include "supportFiles/interrupts.h"
#include "src/Lab2/switches.h"  // Modify as necessary to point to your switches.h
#include "src/Lab2/buttons.h"   // Modify as necessary to point to your buttons.h
#include <stdio.h>
#include <xparameters.h>

int main()
{
    display_init();
    wamDisplay_runMilestone1_test();
}

void isr_function() {
    // Empty for now.
}
