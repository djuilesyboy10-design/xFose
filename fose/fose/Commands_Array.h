#pragma once

#include "CommandTable.h"
#include "ParamInfos.h"

// Array command execution functions
bool Cmd_ar_create_Execute(COMMAND_ARGS);
bool Cmd_ar_set_Execute(COMMAND_ARGS);
bool Cmd_ar_get_Execute(COMMAND_ARGS);
bool Cmd_ar_size_Execute(COMMAND_ARGS);
bool Cmd_ar_push_Execute(COMMAND_ARGS);
bool Cmd_ar_pop_Execute(COMMAND_ARGS);
bool Cmd_ar_remove_Execute(COMMAND_ARGS);
bool Cmd_ar_clear_Execute(COMMAND_ARGS);
bool Cmd_ar_is_valid_Execute(COMMAND_ARGS);

// Define array commands
DEFINE_COMMAND(ar_create, "Creates a new array", 0, 0, NULL);
DEFINE_COMMAND(ar_set, "Sets an element in an array", 0, 2, kParams_TwoInts);
DEFINE_COMMAND(ar_get, "Gets an element from an array", 0, 2, kParams_TwoInts);
DEFINE_COMMAND(ar_size, "Returns the size of an array", 0, 1, kParams_OneInt);
DEFINE_COMMAND(ar_push, "Pushes an element to the end of an array", 0, 2, kParams_TwoInts);
DEFINE_COMMAND(ar_pop, "Pops an element from the end of an array", 0, 1, kParams_OneInt);
DEFINE_COMMAND(ar_remove, "Removes an element from an array", 0, 2, kParams_TwoInts);
DEFINE_COMMAND(ar_clear, "Clears all elements from an array", 0, 1, kParams_OneInt);
DEFINE_COMMAND(ar_is_valid, "Checks if an array ID is valid", 0, 1, kParams_OneInt);
