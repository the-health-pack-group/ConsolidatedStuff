#include "game.h"

static bool runDetection = true;
static bool wasShot = false;

static uint8_t hits = 0;
static uint8_t livesRemaining = 3;
static uint32_t delay = 0;

void game_init () {
    state = st_alive;
    runDetection = true;
    hits = 0;
    livesRemaining = 3;
    delay = 0;
}

void game_tick () {
    // Transitions
    switch (state) {
        case st_alive: {
            if (game_wasShot()) {
                if (hits < 10) {
                    hits++;
                    // TODO: hit sound
                }
                elseif (lives == 1) {
                    lives--;
                    gun_disable();
                    // TODO: death sound
                    state = st_respawn;
                }
                elseif (lives > 1) {
                    lives--;
                    gun_disable();
                    // TODO: play game over sound
                    state = st_game_over;
                }
                
                game_clearShot();
            }
        }
        break;
        case st_respawn: {
            if (delay >= RESPAWN_DELAY) {
                delay = 0;
                hits = 0;
                gun_enable();
                state = st_alive;
            }
        }
        break;
        case st_game_over: {} break;
        default: printf("Something is wrong"); break;
    }
    
    // Actions
    switch (state) {
        case st_alive: {
            
        }
        break;
        case st_respawn: {
            delay++;
        }
        break;
        case st_game_over: {
            if(delay >= RETURN_TO_BASE_DELAY) {
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
