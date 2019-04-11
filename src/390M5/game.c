#include "game.h"
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "gun.h"
#include "soundutil.h"
#include "src/390M3T2/hitLedTimer.h"

#define GAME_RESPAWN_DELAY 500e3            // 5 seconds; time to hide after losing a life.
#define GAME_NUM_LIFES 3                    // 3 lives for the game
#define GAME_INITIAL_HITS 0                 // 0 initial hits
#define GAME_HITS_PER_LIFE 4                // 5 hits per life
#define GAME_HITS_LOW_HEALTH 3				// Hit # 3 is a low health warning
#define GAME_INITIAL_DELAY_VALUE 0          // start delay counter at zero

#define HEALING_CHANNEL 3

static volatile bool runDetection = true;   // Used in the detector to determine whether hits should be detected
static volatile bool wasShot = false;       // A flag set by the detection loop when a hit occurred

static uint8_t hits = GAME_INITIAL_HITS;            // The number of times the player has been shot in this life
static uint8_t livesRemaining = GAME_NUM_LIFES;     // The number of lives remaining for the player
static uint32_t delay = GAME_INITIAL_DELAY_VALUE;   // A counter used to time how long to stay in the respawn state

// Game States
enum game_st_t {
    init_st,        // Initial state
    alive_st,       // Alive state (has health, can shoot and be hit)
    respawn_st,     // Respawn state (after losing a life, time to hide) 
    game_over_st    // Game Over state (after 3 lives lost)
} gameState;

// Needed to debug, but not public
void game_debugStatePrint();

// Initialize the game state machine
void game_init () {
    // Start in the initial state
    gameState = init_st;
    
    // Assume we want hit detection running
    runDetection = true;
    
    // Reset hits to zero
    hits = GAME_INITIAL_HITS;
    
    // Reset remaining lives to 3
    livesRemaining = GAME_NUM_LIFES;
    
    // Reset delay counter
    delay = GAME_INITIAL_DELAY_VALUE;
}

// Game tick function
void game_tick () {
    // Transitions
    switch (gameState) {
        // Initial state to Alive state
        case init_st: gameState = alive_st; break;
        case alive_st: { // When alive
            // If the player was shot
            if (game_wasShot()) { 
                // IF the player still has health (not 5 hits yet)
                if (detector_getPlayerNumber() == HEALING_CHANNEL && hits > GAME_INITIAL_HITS) {
                    // Increment the number of hits
                    hits--;
                    
                    // Play the heal sound
                    if (hits == 0) {
	                    soundutil_forcePlay(sound_healed_e);
					}
					else {
	                    soundutil_forcePlay(sound_healing_e);
					}
                }
                // IF the player still has health (not 5 hits yet)
                else if (detector_getPlayerNumber() != HEALING_CHANNEL && hits < GAME_HITS_PER_LIFE) {
                    // Increment the number of hits
                    hits++;
                    
                    // Star the hit LED
                    hitLedTimer_start();
                    
					if (hits >= GAME_HITS_LOW_HEALTH) {
		                // Play the low health sound
		                soundutil_forcePlay(sound_low_health_e);
					}
					else {
		                // Play the hit sound
		                soundutil_forcePlay(sound_hit_e);
					}
                }
                // Otherwise, 5 hits causes the player to lose a life
                else if (detector_getPlayerNumber() != HEALING_CHANNEL) {
                    // Reset the number of hits in this life to zero
                    hits = GAME_INITIAL_HITS;

                    // Play the "lose life" sound
                    soundutil_forcePlay(sound_loseLife_e);
                    
                    // Shut off hit detection (give time for player to hide)
                    game_setRunDetection(false);
                    
                    // Disable shooting the gun
                    gun_disable();
                    
                    // Take away a life
                    livesRemaining--;

                    // If player has lives left
                    if (livesRemaining > 0) {
                        // Transition to respawn state
                        gameState = respawn_st;
                    }
                    // Otherwise, player has no lives left
                    else {
                        // Transition to game over state
                        gameState = game_over_st;
                    }
                }
                
                // Clear the wasShot flag
                game_clearShot();
            }
        }
        break;
        case respawn_st: {
            // If we hit our delay time
            if (delay >= GAME_RESPAWN_DELAY) {
                // Reset delay counter
                delay = GAME_INITIAL_DELAY_VALUE;
                
                // Reset the gun
                gun_init();
                
                // Enable shooting again
                gun_enable();
                
                // Turn hit detection back on
                game_setRunDetection(true);
                
                // Transition to the alive state
                gameState = alive_st;
            }
        }
        break;
        case game_over_st: break;   // No transitions out of game over state
        default: printf("Something is wrong"); break;
    }
    
    // Actions
    switch (gameState) {
        case init_st: break;        // No action
        case alive_st: break;       // No action
        case respawn_st: {          // When in respawn state
            delay++;                // Increment the delay counter 
        }
        break;
        case game_over_st: break;   // No action
        default: printf("Something is wrong"); break
    }
}

// Set the wasShot flag to signal 
// the game state machine that the player was hit
void game_setShot() {
    wasShot = true;
}

// Clear the wasShot flag when state machine has handled it
void game_clearShot() {
    wasShot = false;
}

// Returns true if the shot flag was raised
bool game_wasShot() {
    return wasShot;
}

// Set whether hit detection should be running right now
void game_setRunDetection(bool newRunDetection) {
    runDetection = newRunDetection;
}

// Determine if the game is running (meaning the game is not over)
bool game_isRunning() {
    return (gameState != game_over_st); // If we are not in the game over state, we are in the game
}

// Return true when hit detection should be 
bool game_runDetection() {
    return runDetection;
}

// Debug function to print when state transitions :)
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


