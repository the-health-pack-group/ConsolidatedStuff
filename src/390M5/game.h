#include <stdbool.h>
#include <stdint.h>

void game_init();   // Initialize game SM
void game_tick();   // Tick for Game SM

void game_setShot();    // Raise was shot flag
bool game_wasShot();    // Determine if was shot flag is set
void game_clearShot();  // Clear was shot flag
bool game_isRunning();  // Return true if the game is running

void game_setRunDetection(bool runDetection);   // Set true if hit detection should run, false if not
bool game_runDetection();                       // Returns true if hit detection should run
