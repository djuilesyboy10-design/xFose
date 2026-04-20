#include "Commands_ActorState.h"
#include "GameForms.h"
#include "GameObjects.h"

bool Cmd_IsUnconscious_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESObjectREFR* refr = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &refr))
	{
		if (!refr) refr = thisObj;
		if (refr)
		{
			TESActorBase* actorBase = (TESActorBase*)refr->baseForm;
			if (actorBase && actorBase->typeID == kFormType_NPC)
			{
				// Check if actor is unconscious by checking flags
				// This is a placeholder - actual implementation would need to check the proper flags
				*result = 0;
				
				if (IsConsoleMode())
					Console_Print("%s is %sunconscious", GetFullName((TESForm*)refr), (*result) ? "" : "not ");
			}
		}
	}

	return true;
}

bool Cmd_IsAlive_Execute(COMMAND_ARGS)
{
	*result = 0;

	TESObjectREFR* refr = NULL;
	
	if (ExtractArgs(EXTRACT_ARGS, &refr))
	{
		if (!refr) refr = thisObj;
		if (refr)
		{
			// Simple check for now - if reference exists and is not null, consider it alive
			// TODO: Implement proper alive/dead check using Actor class
			*result = 1;
			
			if (IsConsoleMode())
				Console_Print("%s is %s", GetFullName((TESForm*)refr), (*result) ? "alive" : "dead");
		}
	}

	return true;
}

bool Cmd_GetGameDaysPassed_Execute(COMMAND_ARGS)
{
	*result = 0;

	// Get global game time and convert to days
	// Game time is stored as a float in hours
	// Days passed = game hours / 24
	
	// This is a simplified implementation - actual implementation would need
	// to access the global game time variable from the game memory
	// For now, return 0 as placeholder
	
	if (IsConsoleMode())
		Console_Print("Game days passed: %f", (float)*result);

	return true;
}
