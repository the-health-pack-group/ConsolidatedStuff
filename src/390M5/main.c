#include "src/390M3T2/runningModes.h"
#include "src/390_libs/filter.h"
#include "src/390M3T2/detector.h"
#include "src/390M3T2/isr.h"
#include "src/390M3T2/hitLedTimer.h"
#include "src/390M3T1/histogram.h"
#include "src/390M3T2/isr.h"
#include "src/390M3T2/transmitter.h"
#include "src/390M3T2/trigger.h"
#include "supportFiles/interrupts.h"
#include "supportFiles/switches.h"
#include "src/390M3T2/lockoutTimer.h"
#include "src/390M3T2/sound.h"
#include "soundutil.h"
#include "game.h"
#include "gun.h"

#define DETECTOR_HIT_ARRAY_SIZE FILTER_FREQUENCY_COUNT  // The array contains one location per user frequency.
#define HISTOGRAM_BAR_COUNT FILTER_FREQUENCY_COUNT      // As many histogram bars as user filter frequencies.
#define SYSTEM_TICKS_PER_HISTOGRAM_UPDATE 30000         // Update the histogram about 3 times per second.

#define GAME_MASK_SWITCH 0x1    // Mask for reading the position of the first switch
#define GAME_FREQ_TEAM_A 6      // Channel for Team A
#define GAME_FREQ_TEAM_B 9      // Channel for Team B

// Declare functions we need later
void initializeFrequency();
void init();

// General initialization for the game
void init() {
    // Display
    display_init();
    
    // Histogram
    histogram_init(HISTOGRAM_BAR_COUNT);
    
    // Detector
    detector_init();
    
    // Filters
    filter_init();
    
    // Interrupt routine
    isr_init();
    
    // Hit LED SM
    hitLedTimer_init();
    
    // Lockout timer SM
    lockoutTimer_init();
    
    // Transmitter SM
    transmitter_init();
    
    // Trigger debouncing SM
    trigger_init();
    
    // Gun SM
    gun_init();
    
    // Game SM
    game_init();
    
    // Intialize soung
    sound_init();
    
    // Set max volume
    sound_setVolume(sound_maximumVolume_e);

    // Interrupts
    interrupts_initAll(true);
    
    // Enablle global interrupts and timer
    interrupts_enableTimerGlobalInts();
    interrupts_startArmPrivateTimer();
    interrupts_enableArmInts();
}

// Main function
int main() {
    // Initialize all the things
    init();
    
    // Read and set the frequency for the player
    initializeFrequency();
    
    // Play the game start sound
    soundutil_forcePlay(sound_gameStart_e);
    
    // Wait until the sound is done playing
    while(sound_isBusy());

    // Turn on the lockout timer to avoid false detects
    lockoutTimer_start();

    // Run the main game loop
    while (game_isRunning()) {
        // Run hit detection, ignoring hits from self (in this case, team)
        detector(true, true);
        
        // If a hit was detected
        if (detector_hitDetected()) {
            // Clear it
            detector_clearHit();
            
            // If the game says to care about hits
            if (game_runDetection()) {
                // set the wasShot flag
                game_setShot();
                
                // Prep and display the hit counter
                detector_hitCount_t hitCounts[DETECTOR_HIT_ARRAY_SIZE]; // Store the hit-counts here.
                detector_getHitCounts(hitCounts);  // Get the current hit counts.
                histogram_plotUserHits(hitCounts); // Plot the hit counts on the TFT.
            }
        }
    }
    
    // Pac-Man Death
    sound_setSound(sound_gameOver_e);   // Set it
    sound_startSound();                 // Play it
    while (sound_isBusy());             // Wait for it to finish

    // Play the sound over and over again, ensuring the player shuts off the system :D
    while (true) {
        // Play game over, return to base
        sound_setSound(sound_returnToBase_e);   // Set it
        sound_startSound();                     // Play it
        while (sound_isBusy());                 // Wait for it to finish

        // Wait for one second
        sound_setSound(sound_oneSecondSilence_e);   // Set it
        sound_startSound();                         // Play it
        while (sound_isBusy());                     // Wait for it to finish
    }

    // Disable interrupts
    interrupts_disableArmInts();
    
    // Make sure the hit LED is off
    hitLedTimer_turnLedOff();
}

// Read the switches and set the player frequency
void initializeFrequency() {
    // Read the position of the first switch
    uint16_t playerChannel = switches_read() & GAME_MASK_SWITCH;
    
    // Determine what the real frequency must be
    uint16_t realFreq = (!playerChannel) ? GAME_FREQ_TEAM_A : GAME_FREQ_TEAM_B;
    
    // Set the frequency for the transmitter
    transmitter_setFrequencyNumber(realFreq);
    
    // Set the frequency on the detector so that ignore self will work
    detector_setSelfFrequency(realFreq);
}
