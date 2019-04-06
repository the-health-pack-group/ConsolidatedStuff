#include "gun.h"
#include "trigger.h"
#include "transmitter.h"
#include "switches.h"

#define MAX_SHOTCOUNT 10
#define NO_SHOTS_LEFT 0
#define RESET 0
#define SHOT_SUCCESSFUL true
#define SHOT_UNSUCCESSFUL false
#define FILTER_FREQUENCY_COUNT 10
#define FORCE_RELOAD_TIME 1	//TODO calculate the actual value this needs to be (to = 3 seconds)
#define AUTO_RELOAD_TIME 1	//TODO calculate the actual value this needs to be (to = 2 seconds)

enum gun_st_t {
	init_st,
	wait_st,
	shot_st,
	force_reload_st,
	auto_reload_st
} gun_currentState;

static uint8_t shotCount = MAX_SHOTCOUNT;
static bool gun_enabled = true;

//Initialize and resets the gun and sets the player frequency?
void gun_init()
{
	gun_currentState = init_st;
	shotCount = MAX_SHOTCOUNT;
}

//Reloads the gun and plays the reload sound
void gun_reload()
{
	shotCount = MAX_SHOTCOUNT;
	//TODO play reload sound
}

bool gun_attemptShot()
{
	if (shotCount > NO_SHOTS_LEFT)	//If we still have shots in our clip
	{
		transmitter_run();			//Shoot at our specified frequency
		shotCount--;				//decrement shot count
		//TODO play shooting sound
		return SHOT_SUCCESSFUL;		//Return that we've successfully shot
	}
	//TODO play empty clip sound
	return SHOT_UNSUCCESSFUL;		//Otherwise, if our clip was empty, return that we did not successfully shoot
}

void gun_tick()
{
	static uint32_t reloadTimer = RESET;
	//State transitions
	switch (gun_currentState)
	{
	case init_st:
		gun_currentState = wait_st;	//Go to our first state where we will wait for the trigger to be pulled
		reloadTimer = RESET;
		break;
		
	case wait_st:
		if (gun_enabled && trigger_wantsToShoot())	//If we are enabled and the trigger has been pulled
		{
			gun_attemptShot();			//Attempt to shoot
			trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to indicate we have handled it)
			gun_currentState = shot_st;	//Go to the state where we have already shot and will decide if we need to reload
		}
		break;
		
	case shot_st:
		//Check if we need to auto-reload (out of ammo), or if we should go to force-reload state
		reloadTimer = RESET;	//Reset the reload timer before going into either reload state
		if (shotCount == NO_SHOTS_LEFT)
		{
			gun_currentState = auto_reload_st;
		}
		else
		{
			gun_currentState = force_reload_st;
		}
		break;
		
	case force_reload_st:	//If trigger continues being pulled, reload after 3 seconds. If trigger is released, return to wait_st
		if (!trigger_debouncePressed())
		{
			gun_currentState = wait_st;
		}
		else if (reloadTimer > FORCE_RELOAD_TIME)	//if the trigger is still being pressed and 3 seconds have passed
		{
			//TODO is there anything else we need to do here besides the state transition and calling reload?
			gun_reload();				//Reload the gun
			gun_currentState = wait_st;	//Go back to the wait state
		}
		else	//Otherwise, if the trigger is still being pulled but 3 seconds have not passed yet
		{
			gun_currentState = force_reload_st;	//Stay in the same state
		}
		break;
		
	case auto_reload_st:	//Wait for 2 seconds before auto-reloading
		if (reloadTimer > AUTO_RELOAD_TIME)	//If we reached the 2 second auto-reloading time
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
	gun_enabled = true;
}

//When the gun is disabled, the state machine will stay in the wait state
void gun_disable()
{
	gun_enabled = false;
}

//Returns whether the gun is enabled or not
bool gun_isEnabled()
{
	return gun_enabled;
}
