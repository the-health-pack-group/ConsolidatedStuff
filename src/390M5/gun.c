#include "gun.h"
#include "../390M3T2/trigger.h"
#include "../390M3T2/transmitter.h"
#include "soundutil.h"
#include <stdio.h>

#define MAX_SHOTCOUNT 10			//The number of shots in the clip before reloading is necessary
#define NO_SHOTS_LEFT 0				//The number of shots when there are no shots left
#define RESET 0						//Used to reset most integer variables
#define SHOT_SUCCESSFUL true		//the value to return when shots are fired
#define SHOT_UNSUCCESSFUL false		//the value to return if no shot was actually fired
#define FORCE_RELOAD_TIME 300000	//the length of time before reloading if forced ( = 3 seconds)
#define AUTO_RELOAD_TIME 200000		//the length of the auto-reload time in ticks ( = 2 seconds)
#define DEBUG 0

//States for the gun state machine
enum gun_st_t {
    init_st,	//initialize everything necessary for the state machine
    wait_st,	//wait here until the trigger is pulled
    shooting_st,//Wait until done shooting before moving on
    shot_st,	//Once the shots have been fired, see whether we need to auto reload or not
    force_reload_st,	//In this state check and see if the player holds down the trigger long enough to force a reload
    auto_reload_st		//Auto reload if the player has run out of ammo
} gun_currentState;

static uint8_t shotCount = MAX_SHOTCOUNT;	//Initialize the gun with a full clip
static bool gun_enabled = true;				//The gun should be initially enabled

//Initialize and resets the gun and sets the player frequency?
void gun_init()
{
    trigger_init();				//Also ensure that the trigger state machine is initialized
    gun_currentState = init_st;	//Start the state machine in the init state
    shotCount = MAX_SHOTCOUNT;	//Make sure the clip is full initially
    trigger_enable();			//Enable the trigger state machine
}

//Reloads the gun and plays the reload sound
void gun_reload()
{
    soundutil_forcePlay(sound_gunReload_e);	//If the gun needs to be reloaded, play the reload sound
    shotCount = MAX_SHOTCOUNT;				//refill the gun's clip
}

bool gun_attemptShot()
{
    if (shotCount > NO_SHOTS_LEFT)	//If we still have shots in our clip
    {
        soundutil_forcePlay(sound_gunFire_e);//Play the shooting sound
        transmitter_run();			//Shoot at our specified frequency
        shotCount--;				//decrement shot count
        return SHOT_SUCCESSFUL;		//Return that we've successfully shot
    }
    soundutil_forcePlay(sound_gunClick_e);//play clicking sound if no shot was made because of an empty clip
    return SHOT_UNSUCCESSFUL;		//Otherwise, if our clip was empty, return that we did not successfully shoot
}

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint() {
    static enum gun_st_t previousState;
    static bool firstPass = true;
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != lockoutTimerState - this prevents reprinting the same state name over and over.
    if (previousState != gun_currentState || firstPass) {
        firstPass = false;                // previousState will be defined, firstPass is false.
        previousState = gun_currentState;     // keep track of the last state that you were in.
        switch (gun_currentState)
        {
        case init_st:
            printf("init_st\n\r");
            break;

        case wait_st:
            printf("wait_st\n\r");
            break;

        case shooting_st:
            printf("shooting_st\n\r");
            break;

        case shot_st:
            printf("shot_st\n\r");
            break;

        case force_reload_st:
            printf("force_reload_st\n\r");
            break;

        case auto_reload_st:
            printf("auto_reload_st\n\r");
            break;
        }
    }
}

void gun_tick()
{
    if (DEBUG) debugStatePrint();		//If we are in DEBUG mode, then print out the current state
    static uint32_t reloadTimer = RESET;//Initialize our timer that will keep track of how long we wait before reloading
    //State transitions
    switch (gun_currentState)
    {
    case init_st:
        gun_currentState = wait_st;	//Go to our first state where we will wait for the trigger to be pulled
        reloadTimer = RESET;		//Reset the reload timer initially
        break;

    case wait_st:
        if (gun_enabled && trigger_wantsToShoot())	//If we are enabled and the trigger has been pulled
        {
            gun_attemptShot();			//Attempt to shoot
            trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
            gun_currentState = shooting_st;	//Go to the state where we have already shot and will decide if we need to reload
        }
        else
        {
            gun_currentState = wait_st;	//If the trigger was not pulled or we are not enabled, keep waiting
            trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
            reloadTimer = RESET;        //Reset the reload timer initially
        }
        break;

    case shooting_st:
        if (!gun_enabled)
        {
            gun_currentState = wait_st;
            trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
            reloadTimer = RESET;        //Reset the reload timer initially
        }
        else if(!transmitter_running()){ //If the transmitter is not running anymore
            gun_currentState = shot_st; //Go to the state where we have already shot and will decide if we need to reload
        }
        else
        {
            gun_currentState = shooting_st; //Otherwise, stay in the same state
        }
        break;

    case shot_st:
        //Check if we need to auto-reload (out of ammo), or if we should go to force-reload state
        reloadTimer = RESET;	//Reset the reload timer before going into either reload state
        if (!gun_enabled)
        {
            gun_currentState = wait_st;
            trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
            reloadTimer = RESET;        //Reset the reload timer initially
        }
        else if (shotCount == NO_SHOTS_LEFT)	//If we have no shots left
        {
            gun_currentState = auto_reload_st;	//Then we need to auto-reload no matter what
        }
        else
        {
            gun_currentState = force_reload_st;	//Otherwise, check to see if the player wants to force a reload
        }
        break;

    case force_reload_st:	//If trigger continues being pulled, reload after 3 seconds. If trigger is released, return to wait_st
        if (!gun_enabled)
        {
            gun_currentState = wait_st;
            trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
            reloadTimer = RESET;        //Reset the reload timer initially
        }
        else if (!trigger_debouncePressed())	//If the debounced trigger is no longer being pulled
        {
            gun_currentState = wait_st;	//Go back to the state where we will wait for the next shot to be fired
        }
        else if (reloadTimer > FORCE_RELOAD_TIME)	//if the trigger is still being pressed and 3 seconds have passed
        {
            gun_reload();				//Reload the gun
            gun_currentState = wait_st;	//Go back to the wait state
        }
        else	//Otherwise, if the trigger is still being pulled but 3 seconds have not passed yet
        {
            gun_currentState = force_reload_st;	//Stay in the same state
        }
        break;

    case auto_reload_st:	//Wait for 2 seconds before auto-reloading
        if (!gun_enabled)
        {
            gun_currentState = wait_st;
            trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
            reloadTimer = RESET;        //Reset the reload timer initially
        }
        else if (reloadTimer > AUTO_RELOAD_TIME)	//If we reached the 2 second auto-reloading time
        {

            gun_reload();				//Reload the gun
            gun_currentState = wait_st;	//Go back to the wait state
        }
        else	//Otherwise, if it hasn't been 2 seconds yet
        {
            if (trigger_wantsToShoot())	//If the user is trying to shoot
            {
                gun_attemptShot();			//Attempt to shoot (but since we are out of ammo, it will just play the clicking noise)
                trigger_clearWantsToShoot();//Clear the shoot flag to indicate we have handled the shooting attempt
            }
            gun_currentState = auto_reload_st;	//stay in this state
        }
        break;
    }

    //State actions
    switch (gun_currentState)
    {
    case init_st:
        break;

    case wait_st:
        break;

    case shooting_st:
        break;

    case shot_st:
        break;

    case force_reload_st:
        reloadTimer++;		//Increment the reload timer while we are in this state
        break;

    case auto_reload_st:
        reloadTimer++;		//Increment the reload timer while we are in this state
        break;
    }
}

//When the gun is enabled, the state machine will progress as normal
void gun_enable()
{
    gun_enabled = true;	//Enable the gun state machine
}

//When the gun is disabled, the state machine will stay in the wait state
void gun_disable()
{
    gun_enabled = false;//Disable the gun state machine
}

//Returns whether the gun is enabled or not
bool gun_isEnabled()
{
    return gun_enabled;//Return whether the gun is actually enabled
}
