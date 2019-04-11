#ifndef TRIGGER_H_
#define TRIGGER_H_

// The trigger state machine debounces both the press and release of gun trigger.
// Ultimately, it will activate the transmitter when a debounced press is detected.

// Init trigger data-structures.
// Determines whether the trigger switch of the gun is connected (see discussion in lab web pages).
// Initializes the mio subsystem.
void trigger_init();

// Enable the trigger state machine. The trigger state-machine is inactive until this function is called.
// This allows you to ignore the trigger when helpful (mostly useful for testing).
// I don't have an associated trigger_disable() function because I don't need to disable the trigger.
void trigger_enable();

// Function that returns whether the trigger wishes to shoot (i.e. the debounced trigger has been pressed)
bool trigger_wantsToShoot();

// Function that returns whether the debounced trigger is currently pressed
bool trigger_debouncePressed();

// Function to clear the wantsToShoot flag. Called after servicing wantsToShoot.
void trigger_clearWantsToShoot();

// Standard tick function.
void trigger_tick();

// Trigger test function.
void trigger_runTest();

#endif /* TRIGGER_H_ */
