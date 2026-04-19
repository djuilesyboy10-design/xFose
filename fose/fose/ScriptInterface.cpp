#include "PluginAPI.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "common/IDebugLog.h"

// Stub implementations for FOSEScriptInterface functions
// These will be fully implemented later

bool ScriptInterface_CallFunction(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container,
	double* result, UInt8 numArgs, ...)
{
	_MESSAGE("ScriptInterface_CallFunction called (stub)");
	if (result) *result = 0;
	return false;
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
