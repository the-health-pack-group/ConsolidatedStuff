#include "gun.h"
#include "trigger.h"
#include "transmitter.h"

#define MAX_SHOTCOUNT 10

enum gun_st_t {
	init_st,
	wait_st,
	shoot_st,
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
}

void gun_tick()
{
	//State transitions
	switch (gun_currentState)
	{
	case init_st:
		gun_currentState = wait_st;
		break;
		
	case wait_st:
		if (gun_enabled && trigger_wantsToShoot())
		{
			transmitter_run();//TODO Change so we only shoot if we have ammo
			trigger_clearWantsToShoot();
			gun_currentState = shoot_st;
			//TODO decrement shot count
		}
		break;
		
	case shoot_st:
		//Check if we need to auto-reload (out of ammo), or if we should go to force-reload state
		break;
		
	case force_reload_st:
		break;
		
	case auto_reload_st:
		break;
	}
	
	//State actions
	switch (gun_currentState)
	{
		case init_st:
		break;
		
		case wait_st:
		break;
		
		case shoot_st:
		break;
		
		case force_reload_st:
		break;
		
		case auto_reload_st:
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
