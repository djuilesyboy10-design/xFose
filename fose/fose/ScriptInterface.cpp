#include "PluginAPI.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "GameData.h"
#include "ScriptRunner.h"
#include "common/IDebugLog.h"

// Implementations for FOSEScriptInterface functions

bool ScriptInterface_CallFunction(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container,
	double* result, UInt8 numArgs, ...)
{
	_MESSAGE("ScriptInterface_CallFunction: ENTRY POINT - function called");
	
	if (!funcScript)
	{
		_MESSAGE("ScriptInterface_CallFunction: funcScript is NULL");
		if (result) *result = 0;
		return false;
	}

	// Use UDFCaller to call the user-defined function
	va_list args;
	va_start(args, numArgs);
	
	bool success = UDFCaller::CallUDF(funcScript, callingObj, container, result, numArgs, args);
	
	va_end(args);
	
	return success;
}

UInt32 ScriptInterface_GetFunctionParams(Script* funcScript, UInt8* paramTypesOut)
{
	_MESSAGE("ScriptInterface_GetFunctionParams called (stub)");
	return -1;
}

bool ScriptInterface_ExtractArgsEx(ParamInfo * paramInfo, void * scriptDataIn, UInt32 * scriptDataOffset, Script * scriptObj,
	ScriptEventList * eventList, ...)
{
	_MESSAGE("ScriptInterface_ExtractArgsEx called (stub)");
	return false;
}

bool ScriptInterface_ExtractFormatStringArgs(UInt32 fmtStringPos, char* buffer, ParamInfo * paramInfo, void * scriptDataIn, 
	UInt32 * scriptDataOffset, Script * scriptObj, ScriptEventList * eventList, UInt32 maxParams, ...)
{
	_MESSAGE("ScriptInterface_ExtractFormatStringArgs called (stub)");
	return false;
}

FOSEScriptInterface g_scriptInterface = {
	1, // kVersion
	ScriptInterface_CallFunction,
	ScriptInterface_GetFunctionParams,
	ScriptInterface_ExtractArgsEx,
	ScriptInterface_ExtractFormatStringArgs
};

// Exported debug function to get actual address of ScriptInterface_CallFunction
extern "C" __declspec(dllexport) void* GetScriptInterfaceCallFunctionAddress()
{
	return (void*)ScriptInterface_CallFunction;
}

// Exported debug function to find the first valid script in the game's loaded data
extern "C" __declspec(dllexport) void* FindFirstScript()
{
	_MESSAGE("FindFirstScript: searching for a valid script...");
	if (!g_dataHandler || !*g_dataHandler)
	{
		_MESSAGE("FindFirstScript: DataHandler not available");
		return nullptr;
	}
	
	DataHandler* dh = *g_dataHandler;
	int count = 0;
	Script* questScript = nullptr;
	Script* objectScript = nullptr;
	
	for (tList<Script>::Iterator iter = dh->scriptList.Begin(); !iter.End(); ++iter)
	{
		Script* script = iter.Get();
		if (script)
		{
			count++;
			
			if (script->data && script->info.dataLength > 10)
			{
				// Prefer quest scripts (type=1) - simpler execution
				if (!questScript && script->info.type == Script::eType_Quest)
				{
					questScript = script;
					_MESSAGE("FindFirstScript: Found quest script #%d: refID=%08X dataLen=%d",
						count, script->refID, script->info.dataLength);
				}
				// Fallback to object script
				if (!objectScript && script->info.type == Script::eType_Object)
				{
					objectScript = script;
				}
			}
		}
		
		// Stop once we have a quest script
		if (questScript)
			break;
	}
	
	Script* bestScript = questScript ? questScript : objectScript;
	
	if (bestScript)
	{
		_MESSAGE("FindFirstScript: selected refID=%08X type=%d dataLen=%d varCount=%d",
			bestScript->refID, bestScript->info.type, bestScript->info.dataLength, bestScript->info.varCount);
	}
	else
	{
		_MESSAGE("FindFirstScript: no suitable script found after %d scripts", count);
	}
	return (void*)bestScript;
}

// Log initialization to verify function pointers
static struct ScriptInterfaceInitLogger
{
	ScriptInterfaceInitLogger()
	{
		_MESSAGE("ScriptInterface: g_scriptInterface initialized at %08X", &g_scriptInterface);
		_MESSAGE("ScriptInterface: CallFunction pointer = %08X", g_scriptInterface.CallFunction);
		_MESSAGE("ScriptInterface: GetFunctionParams pointer = %08X", g_scriptInterface.GetFunctionParams);
		_MESSAGE("ScriptInterface: Actual ScriptInterface_CallFunction address = %08X", ScriptInterface_CallFunction);
	}
} g_scriptInterfaceLogger;
