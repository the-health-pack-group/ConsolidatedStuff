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

#define DETECTOR_HIT_ARRAY_SIZE FILTER_FREQUENCY_COUNT // The array contains one location per user frequency.
#define HISTOGRAM_BAR_COUNT FILTER_FREQUENCY_COUNT     // As many histogram bars as user filter frequencies.
#define SYSTEM_TICKS_PER_HISTOGRAM_UPDATE 30000 // Update the histogram about 3 times per second.

#define GAME_FREQ_TEAM_A 6
#define GAME_FREQ_TEAM_B 9

void initializeFrequency();
void init();

void init() {
    display_init();
    histogram_init(HISTOGRAM_BAR_COUNT);
    
    detector_init();
    filter_init();
    isr_init();
    hitLedTimer_init();
    lockoutTimer_init();
    
    transmitter_init();
    trigger_init();
    
    sound_init();
    sound_setVolume(sound_maximumVolume_e);

    interrupts_initAll(true);
    

    interrupts_enableTimerGlobalInts();
    interrupts_startArmPrivateTimer();
    interrupts_enableArmInts();
}

int main() {
    init();

    
    lockoutTimer_start();
    
    initializeFrequency();
    
    soundutil_forcePlay(sound_gameStart_e);
    
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
