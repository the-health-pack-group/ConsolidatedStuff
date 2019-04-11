#include <stdio.h>
#include "detector.h"
#include "src/390_libs/filter.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/switches.h"
#include "isr.h"
#include "lockoutTimer.h"
#include "hitLedTimer.h"
#include "src/390M5/game.h"

#define DETECTOR_ADC_HALFWAY_POINT 2048.0       // The half-way point for the ADC values
#define DETECTOR_COMPUTE_FROM_SCRATCH false     // Since we are running continously, never compute from scratch
#define DETECTOR_DEBUG false                    // Debug flag

#define DETECTOR_MEDIAN_INDEX 4                 // Index of the median value for 10 values
#define DETECTOR_MAX_INDEX 9                    // Index of the max value for 10 values
#define DETECTOR_PLAYER_COUNT 10                // Number of players
#define DETECTOR_DECIMATION_INIT 0              // Initial decimation counter value
#define DETECTOR_DECIMATION_COUNT 10            // Decimation counter value to run decimation for
#define DEFAULT_RETURN 0                        // Default return value for some functions

#define DETECTOR_FUDGE_FACTOR 150               // The fudge factor: experimentation got us this value


// Values used for the run test
static const double fakeValues1[] = {30, 20, 31, 35, 38, 22, 28, 18, 99, 9500}; // Should detect hit
static const double fakeValues2[] = {30, 20, 31, 35, 38, 22, 28, 18, 99, 50};   // Should NOT detect hit

typedef uint16_t detector_hitCount_t;           // The number of hits detected

static volatile bool hitDetected = false;                       // A flag when a hit is detected
static detector_hitCount_t hitCounts[DETECTOR_PLAYER_COUNT];    // Array of number of hits from each channel
static uint8_t decimationCounter = DETECTOR_DECIMATION_INIT;    // The counter to tell when it is time to run decimation
static bool useFakeData = false;                                // Set to true to use fake data (for run test)
static double fakePowerValues[DETECTOR_PLAYER_COUNT];           // An array used for supplying fake data to the dection algorithm
static uint8_t playerNumber;                                    // The player number  (used for ignoring self)
static uint8_t hitByPlayerNumber;

// Struct used for sorting (remembers the player number)
typedef struct {
    uint8_t playerNumber;   // The player number
    double value;           // The power value
} detector_elem_t;

// Always have to init things.
void detector_init() {
    // Initialize all hitCounts to 0
    for (int i = 0; i < DETECTOR_PLAYER_COUNT; i++) {
        hitCounts[i] = 0;
    }

    // Initialize the filters
    filter_init();
}

// Declare the functions we need internally
uint8_t detector_runDetectionAlgo(bool ignoreSelf, uint8_t playerNum);
void printElems(detector_elem_t values[]);

uint8_t detector_getPlayerNumber() {
    return hitByPlayerNumber;
}

// A function for setting the player number
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

    // Get the number of elements in the ADC input buffer
    uint32_t elementCount = isr_adcBufferElementCount();

    // If we have items to process
    for (uint32_t i = 0; i < elementCount; i++) {
        // Disable interrrupts if needed
        if (interruptsEnabled) {
            interrupts_disableArmInts();
        }

        // Read the raw value from the ADC buffer
        uint32_t rawAdcValue = isr_removeDataFromAdcBuffer();

        // Enable interrrupts if needed
        if (interruptsEnabled) {
            interrupts_enableArmInts();
        }

        // adc value is between 0 and 4095
        // we map it to -1 to 1
        double mappedValue = (rawAdcValue - DETECTOR_ADC_HALFWAY_POINT) / DETECTOR_ADC_HALFWAY_POINT;

        // Add the mapped value to the filter input queue
        filter_addNewInput(mappedValue);

        // Increment the counter for so we can track when it is time to decimate
        decimationCounter++;

        // If we have 10 sames, its time to run decimation
        if (decimationCounter >= DETECTOR_DECIMATION_COUNT) {
            // Run the Low Pass Anti Aliasing Filter
            filter_firFilter();

            // Iterate through each player
            for (uint8_t player = 0; player < DETECTOR_PLAYER_COUNT; player++) {
                // Run the Band Pass Player Filters, Compute Power
                filter_iirFilter(player);
                
                // Compute the power
                filter_computePower(player, DETECTOR_COMPUTE_FROM_SCRATCH, DETECTOR_DEBUG);
            }

            // If we are not getting a hit during the lockout time period
            if (! lockoutTimer_running()) {
                // Run the hit detection algorithm
                uint8_t hitPlayer = detector_runDetectionAlgo(ignoreSelf, playerNumber);

                // If we have determine that the player has been hit
                if (detector_hitDetected()) {

                    // Start the lockout timer
                    lockoutTimer_start();

                    // Increment the number of hits from the channel
                    hitCounts[hitPlayer]++;
                }
            }

            // Reset the decimation counter
            decimationCounter = DETECTOR_DECIMATION_INIT;
        }
    }
}

// Swap function used for sorting
void swap(detector_elem_t *xp, detector_elem_t *yp) {
    detector_elem_t temp = *xp;
    *xp = *yp;
    *yp = temp;
}

// Sort using bubble sort
void sort(detector_elem_t values[]) {
   int i, j;
   for (i =0; i<DETECTOR_PLAYER_COUNT-1; i++) {
       for(j =0; j< DETECTOR_PLAYER_COUNT-i-1; j++)
          if (values[j].value > values[j+1].value)
              swap(&values[j], &values[j+1]);
   }

}

// Get the current power value for the given player number
double detector_getCurrentPowerValueForPlayer(uint8_t player) {
    // Return fake data when needed (used for testing)
    if (useFakeData) {
        return fakePowerValues[player];
    }
    // Return real data for normal use
    else {
        // Just call our other function to get the power value :)
        return filter_getCurrentPowerValue(player);
    }

}

// Set the fake power values to use for testing the detection algorithm
void detector_setFakePowerValues(double fake[]) {
    for (uint8_t i = 0; i < DETECTOR_PLAYER_COUNT; i++) fakePowerValues[i] = fake[i];
}


// This code actually runs our detection algorithm
uint8_t detector_runDetectionAlgo(bool ignoreSelf, uint8_t playerNum) {
    // Temporary variable for storing each player and their power
    detector_elem_t values[DETECTOR_PLAYER_COUNT];

    // Copy values into sorting structure
    for (uint8_t player = 0; player < DETECTOR_PLAYER_COUNT; player++) {
        values[player].playerNumber = player;
        values[player].value = detector_getCurrentPowerValueForPlayer(player);
    }

    //Sort in ascending order (low to high)
    sort(values);

    // Find the median
    double median = values[DETECTOR_MEDIAN_INDEX].value;

    //Find the max
    detector_elem_t max = values[DETECTOR_MAX_INDEX];

    // If max > median * fudge factor
    double threshold = median * DETECTOR_FUDGE_FACTOR;

    // Determine if we should ignore the max value
    bool ignoreMax = (ignoreSelf && playerNum == max.playerNumber);

    // If the max channel power is greater than our threshold
    if (max.value > threshold && ! ignoreMax) {
        //Hit detected
        if (game_runDetection()) {
            hitDetected = true;
        }
        
        hitByPlayerNumber = max.playerNumber;
        
        //Return which player was detected
        return max.playerNumber;
    }

    // Assume a return value of 0, since we have to return a value, but there was no hit
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

// Print the elements in the given array of (used for debugging)
void printElems (detector_elem_t values[]) {
    // Iterate through each element
    for (uint8_t i = 0; i < DETECTOR_PLAYER_COUNT; i++) {
        // Print out the index, player number, and power value
        printf("%d: %d, %f\n", i, values[i].playerNumber, values[i].value);
    }
}

// Test function
void detector_runTest() {
    printf("Starting runTest..\n\r");
    
    //Two sets of fake data

    //Set 1 (To detect a hit with fudge factor)
    //The median here is 30 which means the threshold is 30*300 = 9000
    useFakeData = true;                         // We need to use the fake data
    detector_setFakePowerValues(fakeValues1);   // Set the fake values
    detector_runDetectionAlgo(false, 0);        // Run the detection algo
    
    if (!detector_hitDetected())                        // If no hit was detected, it's wrong
        printf("Failed to detect a correct hit\n\r");   // Print problem

    //Set 2 (To not detect a hit with fudge factor)
    detector_clearHit();                        // Clear the previous hit
    detector_setFakePowerValues(fakeValues2);   // Set the fake values
    detector_runDetectionAlgo(false, 0);        // Run the detection algo
    
    if (detector_hitDetected())                 // If a hit was detected, it's wrong
        printf("Detected a false hit\n\r");     // Print problem

   printf("runTest ended.\n\r");
}












