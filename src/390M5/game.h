#include <stdbool.h>
#include <stdint.h>

void game_init();
void game_tick();

void game_setShot();
bool game_wasShot();
void game_clearShot();

void game_setRunDetection(bool runDetection);
bool game_runDetection();
