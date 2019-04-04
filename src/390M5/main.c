#include "runningModes.h"
#include "src/390_libs/filter.h"
#include "detector.h"
#include "isr.h"
#include "hitLedTimer.h"
#include "src/390M3T1/histogram.h"
#include "src/390M3T2/isr.h"

#include "game.h"
#include "gun.h"

#define DETECTOR_HIT_ARRAY_SIZE FILTER_FREQUENCY_COUNT // The array contains one location per user frequency.
#define HISTOGRAM_BAR_COUNT FILTER_FREQUENCY_COUNT     // As many histogram bars as user filter frequencies.
#define SYSTEM_TICKS_PER_HISTOGRAM_UPDATE 30000 // Update the histogram about 3 times per second.

void initializeFrequency();
void init();

void init() {
    display_init();
    histogram_init(HISTOGRAM_BAR_COUNT);
    
    detector_init();
    filter_init();
    isr_init();
    hitLedTimer_init();
    
    transmitter_init();
    trigger_init();
    
    interrupts_initAll(true);
    
    interrupts_enableArmInts();
}

int main() {
    init();
    
    detector_clearHit();
    detector_getHitCounts(hitCounts);
    histogram_plotUserHits(hitCounts);
    
    lockoutTimer_start();
    
    initializeFrequency();
    
    // TODO: start-up sound
    
    while (true) {
        detector(true, true);
        
        if (detector_hitDetected()) {
            detector_clearHit();
            
            if (game_runDetection()) {
                game_setShot();
                
                detector_hitCount_t hitCounts[DETECTOR_HIT_ARRAY_SIZE]; // Store the hit-counts here.
                detector_getHitCounts(hitCounts);  // Get the current hit counts.
                histogram_plotUserHits(hitCounts); // Plot the hit counts on the TFT.
            }
        }
    }
    
    interrupts_disableArmInts();
    hitLedTimer_turnLedOff();
}

void initializeFrequency() {
    uint16_t playerChannel = switches_read() & 0x1;
    
    uint16_t realFreq = playerChannel == 0 ? GAME_FREQ_TEAM_A : GAME_FREQ_TEAM_B;
    
    transmitter_setFrequencyNumber(realFreq);
}
