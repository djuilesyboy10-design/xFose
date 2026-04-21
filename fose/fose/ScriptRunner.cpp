#include "ScriptRunner.h"
#include "common/IDebugLog.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "Hooks_Script.h"

// Game's ScriptRunner::ExecuteLine function signature
// __thiscall member function: this in ECX, callee cleans 9 stack params (ret 0x24)
// Using __fastcall to simulate __thiscall (ECX=this, EDX=unused)
// Parameter layout confirmed by capturing game's natural calls and testing:
typedef bool (__fastcall * _ExecuteLine)(
	void* thisPtr,                // ECX - ScriptRunner instance (zeroed OK)
	void* edx,                    // EDX - unused (__fastcall padding)
	Script* script,               // p1: Script to execute
	UInt32 blockBodyOffset,       // p2: bytecode offset where block body starts (e.g. 0x10)
	void* executionContext,       // p3: execution context (ScriptEventList or owning object)
	UInt32* opcodeOffsetPtr,      // p4: &opcodeOffset (tracked by g_ScriptDataBytes hook)
	UInt32 chunkDataLen,          // p5: Begin block chunk data length (e.g. 8)
	UInt32* secondaryOffsetPtr,   // p6: &secondary offset (adjacent to p4 in game)
	UInt32 execModeFlag,          // p7: execution mode (game passes 2)
	UInt32 unused1,               // p8: always 0
	UInt32 unused2                // p9: always 0
);

#if FALLOUT_VERSION == FALLOUT_VERSION_1_0
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x0053F9D0;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_0_15
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x0053F9D0;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_1_35
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540820;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_4_6
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540270;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_4_6b
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540270;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_5_22
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540270;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_6
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540270;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540270;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7_0_3
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540270;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7ng
const _ExecuteLine ExecuteLine = (_ExecuteLine)0x00540920;
#else
#error unsupported fallout version
#endif

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
	if (!funcScript)
	{
		_MESSAGE("UDFCaller::CallUDF: funcScript is NULL");
		if (result) *result = 0;
		return false;
	}

	if (!funcScript->data || funcScript->info.dataLength < 16)
	{
		_MESSAGE("UDFCaller::CallUDF: Script %08X has no data or data too short", funcScript->refID);
		if (result) *result = 0;
		return false;
	}

	_MESSAGE("UDFCaller::CallUDF: Script refID=%08X type=%d dataLen=%d numArgs=%d", 
		funcScript->refID, funcScript->info.type, funcScript->info.dataLength, numArgs);

	// Parse bytecode to find the first Begin block and extract offsets
	// Bytecode layout: [4-byte header] [Begin opcode 2] [chunkLen 2] [chunkData...] [body...]
	UInt8* bdata = (UInt8*)funcScript->data;
	UInt32 headerSize = 4;
	UInt16 beginOpcode = *(UInt16*)(bdata + headerSize);
	UInt16 chunkLen = *(UInt16*)(bdata + headerSize + 2);
	UInt32 blockBodyOffset = headerSize + 2 + 2 + chunkLen; // header + opcode + chunkLen + chunkData

	if (beginOpcode != 0x0010)
	{
		_MESSAGE("UDFCaller::CallUDF: Expected Begin opcode (0x0010) at offset %d, got 0x%04X", headerSize, beginOpcode);
		if (result) *result = 0;
		return false;
	}

	_MESSAGE("UDFCaller::CallUDF: Begin block: chunkLen=%d, blockBodyOffset=0x%X", chunkLen, blockBodyOffset);

	// Create ScriptEventList for function execution
	ScriptEventList* eventList = (ScriptEventList*)FormHeap_Allocate(sizeof(ScriptEventList));
	if (!eventList)
	{
		_MESSAGE("UDFCaller::CallUDF: Failed to allocate ScriptEventList");
		if (result) *result = 0;
		return false;
	}

	// Initialize ScriptEventList
	memset(eventList, 0, sizeof(ScriptEventList));
	eventList->m_script = funcScript;

	// Initialize variables from script definition
	UInt32 varCount = funcScript->vars.Count();
	UInt32 varsInitialized = 0;
	for (UInt32 i = 0; i < varCount; i++)
	{
		Script::VariableInfo* varInfo = funcScript->vars.GetNthItem(i);
		if (!varInfo) continue;

		ScriptEventList::Var* var = (ScriptEventList::Var*)FormHeap_Allocate(sizeof(ScriptEventList::Var));
		if (var)
		{
			var->id = varInfo->idx;
			var->data = varInfo->data;
			var->nextEntry = NULL;
			
			ScriptEventList::VarEntry* entry = (ScriptEventList::VarEntry*)FormHeap_Allocate(sizeof(ScriptEventList::VarEntry));
			if (entry)
			{
				entry->var = var;
				entry->next = eventList->m_vars;
				eventList->m_vars = entry;
				varsInitialized++;
			}
		}
	}

	// Set parameter values on the first N variables
	if (numArgs > 0)
	{
		ScriptEventList::VarEntry* varEntry = eventList->m_vars;
		ScriptEventList::Var* varArray[32];
		UInt32 totalVars = 0;
		while (varEntry && totalVars < 32)
		{
			if (varEntry->var)
				varArray[totalVars++] = varEntry->var;
			varEntry = varEntry->next;
		}
		// Reverse to get original declaration order (prepend reverses)
		for (UInt32 i = 0; i < totalVars / 2; i++)
		{
			ScriptEventList::Var* tmp = varArray[i];
			varArray[i] = varArray[totalVars - 1 - i];
			varArray[totalVars - 1 - i] = tmp;
		}
		for (UInt32 i = 0; i < numArgs && i < totalVars; i++)
		{
			double paramVal = va_arg(args, double);
			varArray[i]->data = paramVal;
		}
	}

	// ScriptRunner instance (zeroed is sufficient)
	UInt8 scriptRunner[256];
	memset(scriptRunner, 0, sizeof(scriptRunner));

	// Execution state: two adjacent UInt32 vars (matching game's stack layout)
	// [0] = opcodeOffset (tracked by g_ScriptDataBytes hook)
	// [1] = secondary offset
	UInt32 execState[2] = {0, 0};

	double localResult = 0.0;

	// Call ExecuteLine with correct parameter layout:
	// ECX=ScriptRunner*, p1=Script*, p2=blockBodyOffset, p3=eventList,
	// p4=&execState[0], p5=chunkLen, p6=&execState[1], p7=2, p8=0, p9=0
	bool success = ExecuteLine(
		scriptRunner,      // ECX - ScriptRunner instance
		NULL,              // EDX unused (__fastcall padding)
		funcScript,        // p1: Script*
		blockBodyOffset,   // p2: offset where block body starts
		eventList,         // p3: execution context
		&execState[0],     // p4: &opcodeOffset (g_ScriptDataBytes target)
		chunkLen,          // p5: Begin block chunk data length
		&execState[1],     // p6: &secondary offset
		0x02,              // p7: execution mode flag
		0,                 // p8: unused
		0                  // p9: unused
	);

	_MESSAGE("UDFCaller::CallUDF: ExecuteLine returned %d, opcodeOffset=%d", success, execState[0]);

	if (success && result)
	{
		*result = localResult;
	}

	// Clean up variables
	ScriptEventList::VarEntry* entry = eventList->m_vars;
	while (entry)
	{
		ScriptEventList::VarEntry* next = entry->next;
		if (entry->var)
			FormHeap_Free(entry->var);
		FormHeap_Free(entry);
		entry = next;
	}
	
	FormHeap_Free(eventList);
	return success;
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
