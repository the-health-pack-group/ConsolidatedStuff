#include "game.h"

static bool runDetection = true;
static bool wasShot = false;

static uint8_t hits = 0;
static uint8_t livesRemaining = 3;
static uint32_t delay = 0;

#define GAME_HITS_PER_LIFE 5
#define GAME_RESPAWN_DELAY 500e3 // 5 seconds
#define GAME_RETURN_TO_BASE_DELAY 100e3 // 1 second


enum game_st_t {
    init_st,
    alive_st,
    respawn_st,
    game_over_st 
} triggerState;


void game_init () {
    state = init_st;
    runDetection = true;
    hits = 0;
    livesRemaining = 3;
    delay = 0;
}

void game_tick () {
    // Transitions
    switch (state) {
        case init_st: state = alive_st; break;
        case alive_st: {
            if (game_wasShot()) {
                // Still has hits
                if (hits < GAME_HITS_PER_LIFE) {
                    hits++;
                    // TODO: hit sound
                }
                // No hits left
                else {
                    lives--;
                    hits = 0;
                    // TODO: death sound
                    game_setRunDetection(false);
                    gun_disable();
                }
                
                // Has lives
                if (lives > 0) {
                    lives--;
                    state = respawn_st;
                }
                // No lives left
                else {
                    lives--;
                    state = game_over_st;
                }
                
                game_clearShot();
            }
        }
        break;
        case respawn_st: {
            if (delay >= GAME_RESPAWN_DELAY) {
                delay = 0;
                gun_enable();
                game_setRunDetection(true);
                state = alive_st;
            }
        }
        break;
        case game_over_st: break;
        default: printf("Something is wrong"); break;
    }
    
    // Actions
    switch (state) {
        case init_st: break;
        case alive_st: break;
        case respawn_st: {
            delay++;
        }
        break;
        case game_over_st: 
            // TODO: this should be a second of SILENCE
            if(delay >= GAME_RETURN_TO_BASE_DELAY) {
                // TODO: play return to base sound
                delay = 0;
            }
            
            delay++;
        }
        break;
        default: printf("Something is wrong"); break;
    }
}

void game_setShot() {
    wasShot = true;
}

void game_clearShot() {
    wasShot = false;
}

bool game_wasShot() {
    return wasShot;
}

void game_setRunDetection(bool newRunDetection) {
    runDetection = newRunDetection;
}

bool game_runDetection() {
    return runDetection;
}
