// hooks for interfacing with/patching script compiler/interpreter

#pragma once

void Hook_Script_Init();

extern UInt32* g_ScriptDataBytes;

// Captured params from last natural game call to ExecuteLine
extern UInt32 g_ExecLine_ECX;
extern UInt32 g_ExecLine_Params[9];
extern UInt32 g_ExecLine_CaptureCount;
