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

enum gun_st_t {
	init_st,
	wait_st,
	shot_st,
	force_reload_st,
	auto_reload_st
} gun_currentState;

static uint8_t shotCount = MAX_SHOTCOUNT;
static bool gun_enabled = true;

//Initialize and resets the gun
void gun_init()
{
	gun_currentState = init_st;
	shotCount = MAX_SHOTCOUNT;
	//set our gun's frequency according to the switches. TODO (Do we want to do that in the game instead or is this good?)
	uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;    // Compute a safe number from the switches.
	transmitter_setFrequencyNumber(switchValue);                        // set the frequency number based upon switch value
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
			trigger_clearWantsToShoot();//Clear the flag that told us the player is trying to shoot (to say we have handled it)
			gun_currentState = shot_st;	//Go to the state where we have already shot and will decide if we need to reload
		}
		break;
		
	case shot_st:
		//Check if we need to auto-reload (out of ammo), or if we should go to force-reload state
		reloadTimer = RESET;	//Reset the reload timer
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
		if (reloadTimer > FORCE_RELOAD_TIME)
		{
			//TODO implement state transition, calling reload, etc.
		}
		break;
		
	case auto_reload_st:	//Wait for 2 seconds before auto-reloading
		if (reloadTimer > AUTO_RELOAD_TIME)
		{
			//TODO implement state transition, calling reload, etc.
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
		reloadTimer++;
		break;
		
	case auto_reload_st:	
		reloadTimer++;
		break;
	}
}

void gun_enable()
{
	gun_enabled = true;
}

void gun_disable()
{
	gun_enabled = false;
}

bool gun_isEnabled()
{
	return gun_enabled;
}
