#include "CommandTable.h"
#include "StringVar.h"
#include "GameAPI.h"
#include "common/IDebugLog.h"

// String variable commands for Fallout 3 scripts

// sv_create "string" - creates a new string variable and returns its ID
bool Cmd_sv_create_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	char str[kMaxMessageLength] = { 0 };
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &str))
	{
		return true;
	}
	
	UInt32 stringID = CreateString(str, nullptr);
	*result = stringID;
	
	_MESSAGE("sv_create: Created string ID %d with value '%s'", stringID, str);
	
	return true;
}

// sv_set id "string" - sets the value of an existing string variable
bool Cmd_sv_set_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 stringID = 0;
	char str[kMaxMessageLength] = { 0 };
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &stringID, &str))
	{
		return true;
	}
	
	SetString(stringID, str);
	*result = 1; // Success
	
	_MESSAGE("sv_set: Set string ID %d to '%s'", stringID, str);
	
	return true;
}

// sv_get id - returns the value of a string variable
// Note: This is a placeholder - actual string return needs special handling
bool Cmd_sv_get_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 stringID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &stringID))
	{
		return true;
	}
	
	const char* str = GetString(stringID);
	if (str)
	{
		// For now, return the length as a simple test
		// TODO: Implement proper string return to script
		*result = strlen(str);
		_MESSAGE("sv_get: Got string ID %d, length = %d", stringID, (int)strlen(str));
	}
	
	return true;
}

// sv_length id - returns the length of a string variable
bool Cmd_sv_length_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 stringID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &stringID))
	{
		return true;
	}
	
	const char* str = GetString(stringID);
	if (str)
	{
		*result = strlen(str);
	}
	
	return true;
}

// sv_concat id1 id2 - concatenates two strings and stores in id1
bool Cmd_sv_concat_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 stringID1 = 0;
	UInt32 stringID2 = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &stringID1, &stringID2))
	{
		return true;
	}
	
	const char* str1 = GetString(stringID1);
	const char* str2 = GetString(stringID2);
	
	if (str1 && str2)
	{
		std::string concatResult = str1;
		concatResult += str2;
		SetString(stringID1, concatResult.c_str());
		*result = 1; // Success
	}
	
	return true;
}

// sv_compare id1 id2 - compares two strings, returns 1 if equal, 0 if not
bool Cmd_sv_compare_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 stringID1 = 0;
	UInt32 stringID2 = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &stringID1, &stringID2))
	{
		return true;
	}
	
	const char* str1 = GetString(stringID1);
	const char* str2 = GetString(stringID2);
	
	if (str1 && str2)
	{
		*result = (strcmp(str1, str2) == 0) ? 1 : 0;
	}
	
	return true;
}

// sv_is_valid id - checks if a string ID is valid
bool Cmd_sv_is_valid_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 stringID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &stringID))
	{
		return true;
	}
	
	*result = StringVarManager::GetSingleton().IsValid(stringID) ? 1 : 0;
	
	return true;
}
