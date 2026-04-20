#ifndef __Commands_ActorState_H__
#define __Commands_ActorState_H__

#include "CommandTable.h"

static ParamInfo kParams_Actor_OptionalRef[] =
{
	{	"reference",	kParamType_Actor,	1 },
};

DEFINE_CMD_ALT(IsUnconscious, , Returns 1 if the actor is unconscious, 0, 1, kParams_Actor_OptionalRef);
DEFINE_CMD_ALT(IsAlive, , Returns 1 if the actor is alive, 0, 1, kParams_Actor_OptionalRef);
DEFINE_CMD_ALT(GetGameDaysPassed, , Returns the number of game days passed, 0, 0, NULL);

#endif
