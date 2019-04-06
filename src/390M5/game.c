#include "game.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "gun.h"
#include "soundutil.h"

static volatile bool runDetection = true;
static volatile bool wasShot = false;

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
} gameState;

void game_debugStatePrint();

void game_init () {
    gameState = init_st;
    runDetection = true;
    hits = 0;
    livesRemaining = 3;
    delay = 0;
}

void game_tick () {
    // Transitions
    switch (gameState) {
        case init_st: gameState = alive_st; break;
        case alive_st: {
            if (game_wasShot()) {
                // Still has hits
                if (hits <= GAME_HITS_PER_LIFE) {
                    hits++;
                    soundutil_forcePlay(sound_hit_e);
                }
                // No hits left
                else {
                    hits = 0;
                    soundutil_forcePlay(sound_loseLife_e);
                    game_setRunDetection(false);
                    gun_disable();

                    livesRemaining--;

                    // Has lives
                    if (livesRemaining > 0) {
                        gameState = respawn_st;
                    }
                    // No lives left
                    else {
                        gameState = game_over_st;
                    }
                }
                
                game_clearShot();
            }
        }
        break;
        case respawn_st: {
            if (delay >= GAME_RESPAWN_DELAY) {
                delay = 0;
                gun_init();
                gun_enable();
                game_setRunDetection(true);
                gameState = alive_st;
            }
        }
        break;
        case game_over_st: break;
        default: printf("Something is wrong"); break;
    }
    
    // Actions
    switch (gameState) {
        case init_st: break;
        case alive_st: break;
        case respawn_st: {
            delay++;
        }
        break;
        case game_over_st: break;
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

bool game_isRunning() {
    return (gameState != game_over_st);
}

bool game_runDetection() {
    return runDetection;
}

void game_debugStatePrint () {
    static enum game_st_t previousState;
    static bool firstPass = true;
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (previousState != gameState || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        previousState = gameState;     // keep track of the last state that you were in.

        switch(gameState) {            // This prints messages based upon the state that you were in.
            case init_st:
                printf("init_st\n\r");
                break;
            case alive_st:
                printf("alive_st\n\r");
                break;
            case respawn_st:
                printf("respawn_st\n\r");
                break;
            case game_over_st:
                printf("game_over_st\n\r");
                break;
        }

    }
}


