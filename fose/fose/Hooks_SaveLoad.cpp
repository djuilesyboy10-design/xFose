#include "fose_common/SafeWrite.h"
#include "Hooks_SaveLoad.h"
#include "Serialization.h"
#include "EventManager.h"
#include <Windows.h>
#include <cstring>

bool g_gameLoaded = false;
static const char* LoadGameMessage = "---Finished loading game: %s";
static const char* SaveGameMessage = "---Finished saving game: %s";

#if FALLOUT_VERSION == FALLOUT_VERSION_1_0_15
	static const UInt32 kLoadGamePatchAddr = 0x006CDB83;
	static const UInt32 kLoadGameRetnAddr = 0x006CDB88;

	static const UInt32 kSaveGamePatchAddr =	0x006CB6F3;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CB6F8;

	static const UInt32 kNewGamePatchAddr =		0x0067E277;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D16CC;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DB9C1;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_1_35
	static const UInt32 kLoadGamePatchAddr =	0x006D1A43;
	static const UInt32 kLoadGameRetnAddr =		0x006D1A48;

	static const UInt32 kSaveGamePatchAddr =	0x006CF5B3;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CF5B8;

	static const UInt32 kNewGamePatchAddr =		0x006816D7;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D54FC;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DF821;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_4_6
	static const UInt32 kLoadGamePatchAddr =	0x006D1773;
	static const UInt32 kLoadGameRetnAddr =		0x006D1778;

	static const UInt32 kSaveGamePatchAddr =	0x006CF2E3;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CF2E8;

	static const UInt32 kNewGamePatchAddr =		0x006815F7;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D522C;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DF721;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_4_6b
	static const UInt32 kLoadGamePatchAddr =	0x006D20F3;
	static const UInt32 kLoadGameRetnAddr =		0x006D20F8;

	static const UInt32 kSaveGamePatchAddr =	0x006CFC63;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CFC68;

	static const UInt32 kNewGamePatchAddr =		0x00681E47;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D5B8C;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DFE91;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_5_22
	static const UInt32 kLoadGamePatchAddr =	0x006D197A;
	static const UInt32 kLoadGameRetnAddr =		0x006D197F;

	static const UInt32 kSaveGamePatchAddr =	0x006CF4B3;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CF4B8;

	static const UInt32 kNewGamePatchAddr =		0x00681497;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D54CC;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DF7D1;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_6
	static const UInt32 kLoadGamePatchAddr =	0x006D110A;
	static const UInt32 kLoadGameRetnAddr =		0x006D110F;

	static const UInt32 kSaveGamePatchAddr =	0x006CEC43;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CEC48;

	static const UInt32 kNewGamePatchAddr =		0x00680E57;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D4BCC;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DEEA1;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7
	static const UInt32 kLoadGamePatchAddr =	0x006D103A;
	static const UInt32 kLoadGameRetnAddr =		0x006D103F;

	static const UInt32 kSaveGamePatchAddr =	0x006CEB73;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CEB78;

	static const UInt32 kNewGamePatchAddr =		0x00680DE7;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D4AFC;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DEDD1;		// call to rename()

	static const UInt32 kRuntimeScriptErrorPatchAddr = 0x005E3C10;	// RuntimeScriptError function (GOG v1.7.0.3, alternate offset)
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7ng
	static const UInt32 kLoadGamePatchAddr =	0x006D113A;
	static const UInt32 kLoadGameRetnAddr =		0x006D113F;

	static const UInt32 kSaveGamePatchAddr =	0x006CEC73;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CEC78;

	static const UInt32 kNewGamePatchAddr =		0x00680E17;		// overwrite nullsub(void) call
	static const UInt32 kDeleteGamePatchAddr =	0x006D4BFC;		// DeleteFile() call	
	static const UInt32 kRenameGamePatchAddr =	0x006DEF91;		// call to rename()
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7_0_3
	static const UInt32 kLoadGamePatchAddr =	0x006D103A;	// TODO: verify
	static const UInt32 kLoadGameRetnAddr =		0x006D103F;	// TODO: verify

	static const UInt32 kSaveGamePatchAddr =	0x006CEB73;		// push SaveGameMessage
	static const UInt32 kSaveGameRetnAddr =		0x006CEB78;

	static const UInt32 kNewGamePatchAddr =		0x006C71D0;		// NewGame function
	static const UInt32 kDeleteGamePatchAddr =	0x007D4C30;		// DeleteGame function
	static const UInt32 kRenameGamePatchAddr =	0x007D4C80;		// RenameGame function

	static const UInt32 kRuntimeScriptErrorPatchAddr = 0x005E3C00;	// RuntimeScriptError function
#else
#error unsupported fallout version
#endif

static void __stdcall DoLoadGameHook(const char* saveFilePath)
{
	g_gameLoaded = true;

	_MESSAGE("DoLoadGameHook: %s", saveFilePath);
	
	// Dispatch OnPreLoadGame event
	EventManager::DispatchEventByID(EventManager::kEventID_PreLoadGame, nullptr);
	
	Serialization::HandleLoadGame(saveFilePath);
	
	// Dispatch OnLoadGame and OnPostLoadGame events
	EventManager::DispatchEventByID(EventManager::kEventID_LoadGame, nullptr);
	EventManager::DispatchEventByID(EventManager::kEventID_PostLoadGame, nullptr);
}

static __declspec(naked) void LoadGameHook(void)
{
	__asm
	{
		pushad
		push		eax				// filepath for savegame
		call		DoLoadGameHook
		popad

		// overwritten code
		push		offset	LoadGameMessage
		jmp			[kLoadGameRetnAddr]
	}
}

static void __stdcall DoSaveGameHook(const char* saveFilePath)
{
	_MESSAGE("DoSaveGameHook: %s", saveFilePath);
	Serialization::HandleSaveGame(saveFilePath);
	
	// Dispatch OnSaveGame event
	EventManager::DispatchEventByID(EventManager::kEventID_SaveGame, nullptr);
}

static __declspec(naked) void SaveGameHook(void)
{
	__asm
	{
		pushad
		push		ecx				// filepath for savegame
		call		DoSaveGameHook
		popad

		// overwritten code
		push		offset	SaveGameMessage
		jmp			[kSaveGameRetnAddr]
	}
}

static void NewGameHook(void)
{
	_MESSAGE("NewGameHook");
	
	// Dispatch OnNewGame event
	EventManager::DispatchEventByID(EventManager::kEventID_NewGame, nullptr);

	Serialization::HandleNewGame();
}

static void __stdcall DeleteGameHook(const char * path)
{
	_MESSAGE("DeleteGameHook: %s", path);

	// Dispatch OnDeleteGame event
	EventManager::DispatchEventByID(EventManager::kEventID_DeleteGame, nullptr);

	Serialization::HandleDeleteGame(path);

	DeleteFile(path);
}

static void RenameGameHook(const char * oldPath, const char * newPath)
{
	_MESSAGE("RenameGameHook: %s -> %s", oldPath, newPath);

	// Dispatch OnRenameGame event
	EventManager::DispatchEventByID(EventManager::kEventID_RenameGame, nullptr);

	Serialization::HandleRenameGame(oldPath, newPath);

	rename(oldPath, newPath);
}

// Trampoline holds the overwritten bytes + jump back to original function body
static UInt8* s_scriptErrorTrampoline = NULL;

// Discovered via ErrorScanner: 0x00519EC0 is called by 8+ error string pushes
// ("Item '", "Missing ", "Syntax Error", "Invalid ") - likely the script error reporter.
// Prologue dump shows MSVC SEH setup (14 bytes): 6A FF 68 8B 69 C3 00 64 A1 00 00 00 00 50
static const UInt32 kScriptErrorHookAddr = 0x00519EC0;
static const UInt32 kScriptErrorPrologueSize = 14;
static const UInt32 kScriptErrorPrologueEnd = kScriptErrorHookAddr + kScriptErrorPrologueSize;

// C handler for the naked hook - signature: void ScriptError(void* ctx, const char* fmt, ...)
// We capture: retAddr, ctx (arg0), fmt (arg1), and the first vararg (arg2).
static void __stdcall ScriptErrorHandler(UInt32 retAddr, UInt32 ctx, UInt32 fmtAddr, UInt32 vararg0, UInt32 vararg1)
{
	const char* fmt = (fmtAddr > 0x400000 && fmtAddr < 0x2000000) ? (const char*)fmtAddr : "<not-str>";
	const char* v0 = (vararg0 > 0x400000 && vararg0 < 0x2000000) ? (const char*)vararg0 : NULL;
	const char* v1 = (vararg1 > 0x400000 && vararg1 < 0x2000000) ? (const char*)vararg1 : NULL;

	// Try to format the full error message with the first 2 varargs
	char formatted[512];
	_snprintf_s(formatted, sizeof(formatted), _TRUNCATE, fmt,
		v0 ? v0 : (const char*)vararg0,
		v1 ? v1 : (const char*)vararg1);

	_MESSAGE("ScriptError: retAddr=%08X ctx=%08X msg: %s",
		retAddr, ctx, formatted);

	EventManager::DispatchEventByID(EventManager::kEventID_RuntimeScriptError, nullptr);
}

// Naked hook: captures args, logs them, then jumps to trampoline.
static __declspec(naked) void ScriptErrorHook(void)
{
	__asm {
		pushad
		pushfd

		// After pushad(32)+pushfd(4) = 36 bytes pushed, original stack starts at [esp+36]
		// [esp+36]=retAddr, [esp+40]=ctx, [esp+44]=fmt, [esp+48]=vararg0, [esp+52]=vararg1
		// Push right-to-left for __stdcall: vararg1, vararg0, fmt, ctx, retAddr
		mov		eax, [esp + 52]		// vararg1
		push	eax
		mov		eax, [esp + 52]		// vararg0 (+4 shift)
		push	eax
		mov		eax, [esp + 52]		// fmt (+8 shift)
		push	eax
		mov		eax, [esp + 52]		// ctx (+12 shift)
		push	eax
		mov		eax, [esp + 52]		// retAddr (+16 shift)
		push	eax
		call	ScriptErrorHandler	// __stdcall cleans its own 20 bytes

		popfd
		popad

		// Jump to trampoline which runs original prologue bytes then jumps back
		jmp		s_scriptErrorTrampoline
	}
}

// Memory scanner: finds script error format strings in .rdata and their xrefs in .text.
// This helps identify the actual RuntimeScriptError function without guessing addresses.
static void ScanForScriptErrorHandlers(void)
{
	HMODULE hExe = GetModuleHandleA(NULL);
	if (!hExe) { _MESSAGE("ErrorScanner: GetModuleHandle failed"); return; }

	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hExe;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((UInt8*)hExe + dos->e_lfanew);

	UInt8* textBase = NULL; UInt32 textSize = 0;
	UInt8* rdataBase = NULL; UInt32 rdataSize = 0;

	PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
	for (UInt32 i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++)
	{
		char name[9] = {0};
		memcpy(name, sec->Name, 8);
		if (strcmp(name, ".text") == 0) {
			textBase = (UInt8*)hExe + sec->VirtualAddress;
			textSize = sec->Misc.VirtualSize;
		}
		else if (strcmp(name, ".rdata") == 0) {
			rdataBase = (UInt8*)hExe + sec->VirtualAddress;
			rdataSize = sec->Misc.VirtualSize;
		}
	}

	_MESSAGE("ErrorScanner: .text=%08X size=%08X  .rdata=%08X size=%08X",
		(UInt32)textBase, textSize, (UInt32)rdataBase, rdataSize);

	if (!textBase || !rdataBase) { _MESSAGE("ErrorScanner: sections not found"); return; }

	// Candidate strings likely referenced by the script error handler
	static const char* candidates[] = {
		"Error in script",
		"Script '",
		"script '",
		"not found for parameter",
		"Item '",
		"Unable to find",
		"Missing end",
		"is not an actor",
		", line ",
		"at line",
		"line %d",
		"line: %d",
		"Operator",
		"Token",
		"Missing ",
		"syntax error",
		"Syntax Error",
		"Invalid ",
		"Cannot ",
		"cannot ",
		"bad formID",
		"bad form",
		"(%08X)",
		"%s (%08",
		"Script %s",
		"in file",
		"Script:",
		"script:",
		"ERROR:",
		"Error:",
	};

	for (size_t ci = 0; ci < sizeof(candidates)/sizeof(candidates[0]); ci++)
	{
		const char* target = candidates[ci];
		size_t slen = strlen(target);

		// Find all occurrences of the string in .rdata
		int foundCount = 0;
		for (UInt8* p = rdataBase; p + slen < rdataBase + rdataSize; p++)
		{
			if (p[0] != target[0]) continue;
			if (memcmp(p, target, slen) != 0) continue;
			// Ensure null-termination context (preceding byte is 0 = string start)
			if (p > rdataBase && p[-1] != 0) continue;

			UInt32 strAddr = (UInt32)p;
			_MESSAGE("ErrorScanner: string '%s' at %08X", target, strAddr);
			foundCount++;

			// Scan .text for xrefs (push imm32 = 0x68 XX XX XX XX)
			// For each xref, look ahead for a CALL and log its target
			int xrefCount = 0;
			for (UInt8* q = textBase; q + 5 < textBase + textSize && xrefCount < 5; q++)
			{
				if (*q == 0x68 && *(UInt32*)(q+1) == strAddr)
				{
					// Find next CALL within 64 bytes
					UInt32 callTarget = 0;
					UInt32 callSite = 0;
					for (UInt8* r = q + 5; r + 5 < q + 64 && r + 5 < textBase + textSize; r++)
					{
						if (*r == 0xE8) {
							UInt32 rel = *(UInt32*)(r+1);
							callTarget = (UInt32)r + 5 + rel;
							callSite = (UInt32)r;
							break;
						}
					}
					_MESSAGE("ErrorScanner:   xref push at %08X -> next CALL at %08X -> target %08X",
						(UInt32)q, callSite, callTarget);
					xrefCount++;
				}
			}
			if (foundCount >= 2) break;  // limit per-string to avoid spam
		}
	}

	// Scan for CALL xrefs to candidate functions (Console_Print, ShowRuntimeError, RuntimeScriptError)
	// CALL rel32 encoding: 0xE8 XX XX XX XX where target = q+5+rel32
	static const UInt32 callTargets[] = {
		0x005B5420,  // Console_Print (per user info)
		0x005E3B50,  // ShowRuntimeError (per user info)
		0x005E3C00,  // RuntimeScriptError Main (per user info)
		0x005E3C10,  // RuntimeScriptError alt (per user info)
	};

	for (size_t ci = 0; ci < sizeof(callTargets)/sizeof(callTargets[0]); ci++)
	{
		UInt32 target = callTargets[ci];
		int callerCount = 0;
		_MESSAGE("ErrorScanner: scanning CALL xrefs to %08X", target);
		for (UInt8* q = textBase; q + 5 < textBase + textSize && callerCount < 20; q++)
		{
			if (*q != 0xE8) continue;
			UInt32 rel = *(UInt32*)(q+1);
			UInt32 callTarget = (UInt32)q + 5 + rel;
			if (callTarget == target)
			{
				_MESSAGE("ErrorScanner:   CALL at %08X -> %08X", (UInt32)q, target);
				callerCount++;
			}
		}
		_MESSAGE("ErrorScanner: total callers of %08X = %d (capped at 20)", target, callerCount);
	}

	_MESSAGE("ErrorScanner: scan complete");
}

void Hook_SaveLoad_Init(void)
{
	WriteRelJump(kLoadGamePatchAddr, (UInt32)&LoadGameHook);
	WriteRelJump(kSaveGamePatchAddr, (UInt32)&SaveGameHook);
	WriteRelCall(kNewGamePatchAddr, (UInt32)&NewGameHook);
	WriteRelCall(kDeleteGamePatchAddr, (UInt32)&DeleteGameHook);
	SafeWrite8(kDeleteGamePatchAddr + 5, 0x90);		// nop out leftover byte from original instruction
	WriteRelCall(kRenameGamePatchAddr, (UInt32)&RenameGameHook);

	// Run memory scanner to locate the real RuntimeScriptError function
	ScanForScriptErrorHandlers();

	// Install ScriptError hook using trampoline pattern (14-byte SEH prologue)
	s_scriptErrorTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("ScriptError hook: trampoline allocated at %08X", (UInt32)s_scriptErrorTrampoline);
	if (s_scriptErrorTrampoline)
	{
		// Copy original 14-byte prologue to trampoline
		memcpy(s_scriptErrorTrampoline, (void*)kScriptErrorHookAddr, kScriptErrorPrologueSize);

		// Append JMP back to original function after prologue
		s_scriptErrorTrampoline[kScriptErrorPrologueSize] = 0xE9;
		UInt32 jumpBackOffset = kScriptErrorPrologueEnd - ((UInt32)s_scriptErrorTrampoline + kScriptErrorPrologueSize + 5);
		*(UInt32*)(s_scriptErrorTrampoline + kScriptErrorPrologueSize + 1) = jumpBackOffset;
		_MESSAGE("ScriptError hook: trampoline JMP offset=%08X (target=%08X)", jumpBackOffset, kScriptErrorPrologueEnd);

		// Write 5-byte JMP from function entry to our hook, NOP the remaining 9 bytes
		WriteRelJump(kScriptErrorHookAddr, (UInt32)&ScriptErrorHook);
		for (UInt32 i = 5; i < kScriptErrorPrologueSize; i++)
			SafeWrite8(kScriptErrorHookAddr + i, 0x90);
		_MESSAGE("ScriptError hook: installed at %08X -> %08X", kScriptErrorHookAddr, (UInt32)&ScriptErrorHook);
	}
}
