#include "ScriptRunner.h"
#include "common/IDebugLog.h"

// ScriptContext implementations
ScriptContext::ScriptContext()
{
	script = NULL;
	callingObj = NULL;
	container = NULL;
	eventList = NULL;
}

ScriptContext::~ScriptContext()
{
	// Don't delete owned objects, they're managed by the game
}

void ScriptContext::Initialize(Script* script, TESObjectREFR* callingObj, TESObjectREFR* container)
{
	this->script = script;
	this->callingObj = callingObj;
	this->container = container;
	this->eventList = NULL;
}

// ScriptRunner implementations
bool ScriptRunner::ExecuteScriptLine(Script* script, const char* line, ScriptContext* context)
{
	_MESSAGE("ScriptRunner::ExecuteScriptLine called (stub)");
	return false;
}

bool ScriptRunner::ExecuteScript(Script* script, ScriptContext* context)
{
	_MESSAGE("ScriptRunner::ExecuteScript called (stub)");
	return false;
}

// ParameterExtractor implementations
bool ParameterExtractor::ExtractUInt8(UInt8* out, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData)
		return false;
	
	UInt8* data = (UInt8*)*scriptData + *offset;
	*out = *data;
	*offset += sizeof(UInt8);
	return true;
}

bool ParameterExtractor::ExtractUInt16(UInt16* out, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData)
		return false;
	
	UInt16* data = (UInt16*)((UInt8*)*scriptData + *offset);
	*out = *data;
	*offset += sizeof(UInt16);
	return true;
}

bool ParameterExtractor::ExtractUInt32(UInt32* out, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData)
		return false;
	
	UInt32* data = (UInt32*)((UInt8*)*scriptData + *offset);
	*out = *data;
	*offset += sizeof(UInt32);
	return true;
}

bool ParameterExtractor::ExtractFloat(float* out, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData)
		return false;
	
	float* data = (float*)((UInt8*)*scriptData + *offset);
	*out = *data;
	*offset += sizeof(float);
	return true;
}

bool ParameterExtractor::ExtractDouble(double* out, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData)
		return false;
	
	double* data = (double*)((UInt8*)*scriptData + *offset);
	*out = *data;
	*offset += sizeof(double);
	return true;
}

bool ParameterExtractor::ExtractString(char* out, UInt32 maxLen, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData || maxLen == 0)
		return false;
	
	char* data = (char*)((UInt8*)*scriptData + *offset);
	UInt32 len = 0;
	
	// Find null terminator
	while (len < maxLen - 1 && data[len] != '\0')
	{
		out[len] = data[len];
		len++;
	}
	
	out[len] = '\0';
	*offset += len + 1;
	return true;
}

bool ParameterExtractor::ExtractForm(TESForm** out, void** scriptData, UInt32* offset)
{
	if (!out || !scriptData || !*scriptData)
		return false;
	
	UInt32 formID = 0;
	if (!ExtractUInt32(&formID, scriptData, offset))
		return false;
	
	*out = LookupFormByID(formID);
	return true;
}

// ReturnValueHandler implementations
void ReturnValueHandler::SetReturnValue(double value, void** scriptData, UInt32* offset)
{
	if (!scriptData || !*scriptData || !offset)
		return;
	
	double* data = (double*)((UInt8*)*scriptData + *offset);
	*data = value;
	*offset += sizeof(double);
}

void ReturnValueHandler::SetReturnValueFloat(float value, void** scriptData, UInt32* offset)
{
	if (!scriptData || !*scriptData || !offset)
		return;
	
	float* data = (float*)((UInt8*)*scriptData + *offset);
	*data = value;
	*offset += sizeof(float);
}

void ReturnValueHandler::SetReturnValueInt(SInt32 value, void** scriptData, UInt32* offset)
{
	if (!scriptData || !*scriptData || !offset)
		return;
	
	SInt32* data = (SInt32*)((UInt8*)*scriptData + *offset);
	*data = value;
	*offset += sizeof(SInt32);
}

// UDFCaller implementations
bool UDFCaller::CallUDF(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, 
	double* result, UInt8 numArgs, va_list args)
{
	_MESSAGE("UDFCaller::CallUDF called (stub)");
	if (result) *result = 0;
	return false;
}

bool UDFCaller::CallUDF(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container, 
	double* result, UInt8 numArgs, ...)
{
	va_list args;
	va_start(args, numArgs);
	bool success = CallUDF(funcScript, callingObj, container, result, numArgs, args);
	va_end(args);
	return success;
}
