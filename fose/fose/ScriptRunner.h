#pragma once

#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"

// ScriptContext - Maintains script execution state
class ScriptContext
{
public:
	ScriptContext();
	~ScriptContext();

	// Initialize context for script execution
	void Initialize(Script* script, TESObjectREFR* callingObj, TESObjectREFR* container);

	// Get/set execution context objects
	TESObjectREFR* GetCallingObject() const { return callingObj; }
	TESObjectREFR* GetContainer() const { return container; }
	Script* GetScript() const { return script; }
	ScriptEventList* GetEventList() const { return eventList; }

	// Set event list for variable access
	void SetEventList(ScriptEventList* evtList) { eventList = evtList; }

private:
	Script*				script;			// Script being executed
	TESObjectREFR*		callingObj;		// Object calling the script
	TESObjectREFR*		container;		// Container object
	ScriptEventList*		eventList;		// Runtime script state (from GameAPI.h)
};

// ScriptRunner - Execute script lines with context
class ScriptRunner
{
public:
	static bool ExecuteScriptLine(Script* script, const char* line, ScriptContext* context);
	static bool ExecuteScript(Script* script, ScriptContext* context);
};

// ParameterExtractor - Convert script data to C++ values
class ParameterExtractor
{
public:
	static bool ExtractUInt8(UInt8* out, void** scriptData, UInt32* offset);
	static bool ExtractUInt16(UInt16* out, void** scriptData, UInt32* offset);
	static bool ExtractUInt32(UInt32* out, void** scriptData, UInt32* offset);
	static bool ExtractFloat(float* out, void** scriptData, UInt32* offset);
	static bool ExtractDouble(double* out, void** scriptData, UInt32* offset);
	static bool ExtractString(char* out, UInt32 maxLen, void** scriptData, UInt32* offset);
	static bool ExtractForm(TESForm** out, void** scriptData, UInt32* offset);
};

// ReturnValueHandler - Convert C++ values to script format
class ReturnValueHandler
{
public:
	static void SetReturnValue(double value, void** scriptData, UInt32* offset);
	static void SetReturnValueFloat(float value, void** scriptData, UInt32* offset);
	static void SetReturnValueInt(SInt32 value, void** scriptData, UInt32* offset);
};

// UDFCaller - Call User-Defined Functions from C++
class UDFCaller
{
public:
	static bool CallUDF(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, 
		double* result, UInt8 numArgs, va_list args);
	static bool CallUDF(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, 
		double* result, UInt8 numArgs, ...);
};
