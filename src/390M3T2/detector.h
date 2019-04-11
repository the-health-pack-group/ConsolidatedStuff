#ifndef DETECTOR_H_
#define DETECTOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "src/390_libs/queue.h"


typedef uint16_t detector_hitCount_t;

// Always have to init things.
void detector_init();

// Runs the entire detector: decimating fir-filter, iir-filters, power-computation, hit-detection.
// if interruptsEnabled = false, interrupts are not running. If interruptsEnabled = false
// you can pop values from the ADC queue without disabling interrupts.
// If interruptsEnabled = true, do the following:
// 1. disable interrupts.
// 2. pop the value from the ADC queue.
// 3. re-enable interrupts.
// Use this to determine whether you should disable and re-enable interrrupts when accessing the adc queue.
// if ignoreSelf == true, ignore hits that are detected on your frequency.
// Your frequency is simply the frequency indicated by the slide switches.
void detector(bool interruptsEnabled, bool ignoreSelf);

// Returns true if a hit was detected.
bool detector_hitDetected();

// Get the channel/player number that caused the hit
uint8_t detector_getPlayerNumber();

// Clear the detected hit once you have accounted for it.
void detector_clearHit();

void detector_setSelfFrequency(uint8_t playerNumber);

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]);

// Test function
void detector_runTest();

#endif /* DETECTOR_H_ */
