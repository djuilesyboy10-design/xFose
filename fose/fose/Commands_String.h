#pragma once

#include "CommandTable.h"
#include "ParamInfos.h"

bool Cmd_sv_create_Execute(COMMAND_ARGS);
bool Cmd_sv_set_Execute(COMMAND_ARGS);
bool Cmd_sv_get_Execute(COMMAND_ARGS);
bool Cmd_sv_length_Execute(COMMAND_ARGS);
bool Cmd_sv_concat_Execute(COMMAND_ARGS);
bool Cmd_sv_compare_Execute(COMMAND_ARGS);
bool Cmd_sv_is_valid_Execute(COMMAND_ARGS);

static ParamInfo kParams_StringIDAndString[2] =
{
	{	"id",	kParamType_Integer,	0	},
	{	"string",	kParamType_String,	0	},
};

DEFINE_COMMAND(sv_create, "Creates a new string variable", 0, 1, kParams_OneString);
DEFINE_COMMAND(sv_set, "Sets the value of a string variable", 0, 2, kParams_StringIDAndString);
DEFINE_COMMAND(sv_get, "Gets the value of a string variable (returns length)", 0, 1, kParams_OneInt);
DEFINE_COMMAND(sv_length, "Returns the length of a string variable", 0, 1, kParams_OneInt);
DEFINE_COMMAND(sv_concat, "Concatenates two strings", 0, 2, kParams_TwoInts);
DEFINE_COMMAND(sv_compare, "Compares two strings", 0, 2, kParams_TwoInts);
DEFINE_COMMAND(sv_is_valid, "Checks if a string ID is valid", 0, 1, kParams_OneInt);
