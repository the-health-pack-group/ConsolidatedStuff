#include <stdint.h>

//Initializes and resets the gun state machine
void gun_init();

//Runs a single tick of the gun state machine
void gun_tick();

//When the gun state machine is enabled, it will be able to shoot, reload, etc.
void gun_enable();

//When the gun state machine is disabled, the gun will no longer be able to shoot, reload, etc. but will stay in a waiting state
void gun_disable();

//Returns whether the gun state machine is enabled or not
bool gun_isEnabled();