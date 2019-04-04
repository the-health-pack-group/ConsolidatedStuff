#include "soundutil.h"
#include "src/390M3T2/sound.h"


void soundutil_forcePlay(sound_sounds_t sound)
{
    if (sound_isBusy())
    {
        sound_stopSound();
    }

    sound_setSound(sound);
    sound_startSound();

}
