#pragma once

struct PlayerActionState
{
	bool move = false;
	bool blink = false;
	bool jump = false;
	bool attack = false;

	// RXR‚Ì•ûŒÀ’è
	bool wallbreak = false; // •Ç‚ğ‰ó‚µ‚½‚©
	bool hotSpring = false; // ‰·ò‚É“ü‚Á‚½‚©
};