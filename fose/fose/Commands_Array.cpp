#include "Commands_Array.h"
#include "ArrayVar.h"
#include "CommandTable.h"
#include "GameAPI.h"
#include "common/IDebugLog.h"

bool Cmd_ar_create_Execute(COMMAND_ARGS)
{
	*result = 0;
	
	UInt32 arrayID = ArrayVarManager::GetSingleton().CreateArray(nullptr);
	*result = arrayID;

	_MESSAGE("ar_create: Created array ID %d", arrayID);

	return true;
}

bool Cmd_ar_set_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	UInt32 index = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID, &index))
	{
		return true;
	}

	// For now, we'll just set the element to the value passed
	// In a more complete implementation, we'd support different types
	ArrayElement element(index); // Using index as value for now
	
	if (ArrayVarManager::GetSingleton().SetElement(arrayID, index, element))
	{
		*result = 1; // Success
		_MESSAGE("ar_set: Set array ID %d index %d to %d", arrayID, index, index);
	}
	else
	{
		*result = 0; // Failure
	}

	return true;
}

bool Cmd_ar_get_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	UInt32 index = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID, &index))
	{
		return true;
	}

	ArrayElement element;
	if (ArrayVarManager::GetSingleton().GetElement(arrayID, index, element))
	{
		if (element.type == kArrayElement_Integer)
		{
			*result = element.intValue;
			_MESSAGE("ar_get: Got array ID %d index %d = %d", arrayID, index, element.intValue);
		}
		else
		{
			*result = 0;
		}
	}
	else
	{
		*result = 0;
	}

	return true;
}

bool Cmd_ar_size_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID))
	{
		return true;
	}

	*result = ArrayVarManager::GetSingleton().GetSize(arrayID);

	_MESSAGE("ar_size: Array ID %d size = %d", arrayID, *result);

	return true;
}

bool Cmd_ar_push_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	UInt32 value = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID, &value))
	{
		return true;
	}

	ArrayElement element(value);
	if (ArrayVarManager::GetSingleton().Push(arrayID, element))
	{
		*result = 1; // Success
		_MESSAGE("ar_push: Pushed %d to array ID %d", value, arrayID);
	}
	else
	{
		*result = 0; // Failure
	}

	return true;
}

bool Cmd_ar_pop_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID))
	{
		return true;
	}

	ArrayElement element;
	if (ArrayVarManager::GetSingleton().Pop(arrayID, element))
	{
		if (element.type == kArrayElement_Integer)
		{
			*result = element.intValue;
			_MESSAGE("ar_pop: Popped %d from array ID %d", element.intValue, arrayID);
		}
		else
		{
			*result = 0;
		}
	}
	else
	{
		*result = 0;
	}

	return true;
}

bool Cmd_ar_remove_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	UInt32 index = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID, &index))
	{
		return true;
	}

	if (ArrayVarManager::GetSingleton().Remove(arrayID, index))
	{
		*result = 1; // Success
		_MESSAGE("ar_remove: Removed index %d from array ID %d", index, arrayID);
	}
	else
	{
		*result = 0; // Failure
	}

	return true;
}

bool Cmd_ar_clear_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID))
	{
		return true;
	}

	if (ArrayVarManager::GetSingleton().Clear(arrayID))
	{
		*result = 1; // Success
		_MESSAGE("ar_clear: Cleared array ID %d", arrayID);
	}
	else
	{
		*result = 0; // Failure
	}

	return true;
}

bool Cmd_ar_is_valid_Execute(COMMAND_ARGS)
{
	UInt32 arrayID = 0;
	if (!ExtractArgsEx(EXTRACT_ARGS_EX, &arrayID))
	{
		return true;
	}

	*result = ArrayVarManager::GetSingleton().IsValid(arrayID) ? 1 : 0;

	_MESSAGE("ar_is_valid: Array ID %d is %s", arrayID, *result ? "valid" : "invalid");

	return true;
}
