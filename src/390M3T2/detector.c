#include <stdio.h>
#include "detector.h"
#include "src/390_libs/filter.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/switches.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include "src/390M5/game.h"

#define DETECTOR_ADC_HALFWAY_POINT 2048.0
#define DETECTOR_COMPUTE_FROM_SCRATCH false
#define DETECTOR_DEBUG false

#define DETECTOR_MEDIAN_INDEX 4
#define DETECTOR_MAX_INDEX 9
#define DETECTOR_FUDGE_FACTOR 150
#define DETECTOR_PLAYER_COUNT 10
#define DEFAULT_RETURN 0
typedef uint16_t detector_hitCount_t;


static volatile bool hitDetected = false;
static detector_hitCount_t hitCounts[DETECTOR_PLAYER_COUNT];
static uint8_t decimationCounter = 0;
static bool useFakeData = false;
static double fakePowerValues[DETECTOR_PLAYER_COUNT];
static uint8_t playerNumber;

typedef struct {
    uint8_t playerNumber;
    double value;
} detector_elem_t;

// Always have to init things.
void detector_init() {
    // Initialize all hitCounts to 0
    for (int i = 0; i < DETECTOR_PLAYER_COUNT; i++) {
        hitCounts[i] = 0;
    }

    filter_init();
}


uint8_t detector_runDetectionAlgo(bool ignoreSelf, uint8_t playerNum);
void printElems(detector_elem_t values[]);

void detector_setSelfFrequency(uint8_t nPlayerNumber) {
    playerNumber = nPlayerNumber;
}

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
void detector(bool interruptsEnabled, bool ignoreSelf) {

    uint32_t elementCount = isr_adcBufferElementCount();

    // If we have items to process
    for (uint32_t i = 0; i < elementCount; i++) {
        // Disable interrrupts if needed
        if (interruptsEnabled) {
            interrupts_disableArmInts();
        }

        uint32_t rawAdcValue = isr_removeDataFromAdcBuffer();

        // Enable interrrupts if needed
        if (interruptsEnabled) {
            interrupts_enableArmInts();
        }

        // adc value is between 0 and 4095
        // we map it to -1 to 1
        double mappedValue = (rawAdcValue - DETECTOR_ADC_HALFWAY_POINT) / DETECTOR_ADC_HALFWAY_POINT;

        filter_addNewInput(mappedValue);

        decimationCounter++;

        if (decimationCounter >= 10) {
            // Low Pass Anti Aliasing Filter
            filter_firFilter();

            // Band Pass Player Filters, Compute Power
            for (uint8_t player = 0; player < DETECTOR_PLAYER_COUNT; player++) {
                filter_iirFilter(player);
                filter_computePower(player, DETECTOR_COMPUTE_FROM_SCRATCH, DETECTOR_DEBUG);
            }

            if (! lockoutTimer_running()) {
                // Hit Detection
                uint8_t hitPlayer = detector_runDetectionAlgo(ignoreSelf, playerNumber);

                if (detector_hitDetected()) {

                    lockoutTimer_start();
                    hitLedTimer_start();

                    hitCounts[hitPlayer]++;
                }
            }

            decimationCounter = 0;
        }
    }
}

void swap(detector_elem_t *xp, detector_elem_t *yp)
{
    detector_elem_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}

void sort(detector_elem_t values[])
{
   int i, j;
   for (i =0; i<9; i++)
   {
       for(j =0; j< 10-i-1; j++)
          if (values[j].value > values[j+1].value)
              swap(&values[j], &values[j+1]);
   }

}


double detector_getCurrentPowerValueForPlayer(uint8_t player) {
    if (useFakeData) {
        return fakePowerValues[player];
    }
    else {
        return filter_getCurrentPowerValue(player);
    }

}

void detector_setFakePowerValues(double fake[]) {
    for (uint8_t i = 0; i < DETECTOR_PLAYER_COUNT; i++) fakePowerValues[i] = fake[i];
}


uint8_t detector_runDetectionAlgo(bool ignoreSelf, uint8_t playerNum) {
    detector_elem_t values[DETECTOR_PLAYER_COUNT];

    // Copy values into sorting structure
    for (uint8_t player = 0; player < DETECTOR_PLAYER_COUNT; player++) {
        values[player].playerNumber = player;
        values[player].value = detector_getCurrentPowerValueForPlayer(player);
    }



    //Sort in ascending order
    sort(values);



    // Find the median
    double median = values[DETECTOR_MEDIAN_INDEX].value;


    //Find the max
    detector_elem_t max = values[DETECTOR_MAX_INDEX];
//    detector_elem_t max2 = values[DETECTOR_MAX_INDEX - 1];



    // If max > median * fudge factor
    double threshold = median * DETECTOR_FUDGE_FACTOR;

    bool ignoreMax = (ignoreSelf && playerNum == max.playerNumber);

    if (max.value > threshold && ! ignoreMax)
    {
        //Hit detected
        if (game_runDetection()) {
            hitDetected = true;
        }
        //Return which player was detected
        return max.playerNumber;
    }
//    else if (max2.value > threshold && ignoreMax)
//    {
//        //Hit detected
//        hitDetected = true;
//        //Return which player was detected
//        return max2.playerNumber;
//    }

    return DEFAULT_RETURN;
}


// Returns true if a hit was detected.
bool detector_hitDetected() {
    return hitDetected;
}

// Clear the detected hit once you have accounted for it.
void detector_clearHit() {
    hitDetected = false;
}

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
    // Copy counts
    for (int i = 0; i < DETECTOR_PLAYER_COUNT; i++) {
        hitArray[i] = hitCounts[i];
    }
}

void printElems (detector_elem_t values[]) {
    for (uint8_t i = 0; i < DETECTOR_PLAYER_COUNT; i++) {
        printf("%d: %d, %f\n", i, values[i].playerNumber, values[i].value);
    }
}

// Test function
void detector_runTest() {
    printf("Starting runTest..\n\r");
    //Two sets of fake data

    //Set 1 (To detect a hit with fudge factor)
    //The median here is 30 which means the threshold is 30*300 = 9000
    useFakeData = true;
    double fake1[] = {30, 20, 31, 35, 38, 22, 28, 18, 99, 9500};
    detector_setFakePowerValues(fake1);
    detector_runDetectionAlgo(false, 0);
    if (!detector_hitDetected())
        printf("Failed to detect a correct hit\n\r");



    //Set 2 (To not detect a hit with fudge factor)
    //Clear the previous hit
    detector_clearHit();
    double fake2[] = {30, 20, 31, 35, 38, 22, 28, 18, 99, 50};
      detector_setFakePowerValues(fake2);
      detector_runDetectionAlgo(false, 0);
      if (detector_hitDetected())
          printf("Detected a false hit\n\r");

   printf("runTest ended.\n\r");
}












