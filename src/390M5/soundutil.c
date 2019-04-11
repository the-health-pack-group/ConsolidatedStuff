#include "soundutil.h"
#include "src/390M3T2/sound.h"

// Utility function that will stop whatever sound is playing
// and play the new one
void soundutil_forcePlay(sound_sounds_t sound) {
    // If a sound is currently playing
    if (sound_isBusy()) {
        // Stop it
        sound_stopSound();
    }

    // Set the new sound
    sound_setSound(sound);
    
    // Play the new sound
    sound_startSound();
}
