#include "CommandTable.h"
#include "GameAPI.h"
#include "Hooks_Gameplay.h"
#include "fose_common\SafeWrite.h"
#include "fose_common\fose_version.h"
#include "GameInterface.h"
#include "PluginAPI.h"
#include "PluginManager.h"
#include "EventManager.h"
#include "Scanner.h"

void ToggleUIMessages(bool bEnable)
{
	// Disable: write an immediate return at function entry
	// Enable: restore the push instruction at function entry
	SafeWrite8((UInt32)QueueUIMessage, bEnable ? 0x6A : 0xC3);
}

bool RunCommand_NS(COMMAND_ARGS, Cmd_Execute cmd)
{
	ToggleUIMessages(false);
	bool cmdResult = cmd(PASS_COMMAND_ARGS);
	ToggleUIMessages(true);

	return cmdResult;
}

#if FALLOUT_VERSION == FALLOUT_VERSION_1_7
	static const UInt32 kMainMenuFromIngameMenuPatchAddr = 0x006799C4;	// 4th reference to g_osGlobals after RaceSexMenu_func0008
	static const UInt32 kMainMenuFromIngameMenuRetnAddr	 = 0x00433A30;	// original call

	static const UInt32 kExitGameViaQQQPatchAddr		 = 0x0051ED15;	// Inside Cmd_QuitGame_Execute, call ConsolePrintF
	static const UInt32 kExitGameViaQQQRetnAddr			 = 0x006195D0;	// original call

	static const UInt32 kExitGameFromMenuPatchAddr       = 0x0067B06B;	// Sets the proper bit in OsGlobals, to be confirmed
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7_0_3
	static const UInt32 kMainMenuFromIngameMenuPatchAddr = 0x006799C4;	// 4th reference to g_osGlobals after RaceSexMenu_func0008
	static const UInt32 kMainMenuFromIngameMenuRetnAddr	 = 0x00433A30;	// original call

	static const UInt32 kExitGameViaQQQPatchAddr		 = 0x0051ED15;	// Inside Cmd_QuitGame_Execute, call ConsolePrintF
	static const UInt32 kExitGameViaQQQRetnAddr			 = 0x006195D0;	// original call

	static const UInt32 kExitGameFromMenuPatchAddr       = 0x0067B06B;	// Sets the proper bit in OsGlobals, to be confirmed
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7ng
	static const UInt32 kMainMenuFromIngameMenuPatchAddr = 0x00679AF4;	// 4th reference to g_osGlobals after RaceSexMenu_func0008
	static const UInt32 kMainMenuFromIngameMenuRetnAddr	 = 0x00433A20;	// original call

	static const UInt32 kExitGameViaQQQPatchAddr		 = 0x0051F345;	// Inside Cmd_QuitGame_Execute, call ConsolePrintF
	static const UInt32 kExitGameViaQQQRetnAddr			 = 0x006196B0;	// original call

	static const UInt32 kExitGameFromMenuPatchAddr       = 0x0067B19B;
#else
	// #error - Version not supported, using default addresses
	static const UInt32 kMainMenuFromIngameMenuPatchAddr = 0x006799C4;	// Default to 1.7 addresses
	static const UInt32 kMainMenuFromIngameMenuRetnAddr	 = 0x00433A30;	// original call

	static const UInt32 kExitGameViaQQQPatchAddr		 = 0x0051ED15;	// Inside Cmd_QuitGame_Execute, call ConsolePrintF
	static const UInt32 kExitGameViaQQQRetnAddr			 = 0x006195D0;	// original call

	static const UInt32 kExitGameFromMenuPatchAddr       = 0x0067B06B;	// Sets the proper bit in OsGlobals, to be confirmed
#endif

static enum QuitGameMessage
{
	kQuit_ToMainMenu,
	kQuit_ToWindows,
	kQuit_QQQ,
};

void __stdcall SendQuitGameMessage(QuitGameMessage msg)
{
	UInt32 msgToSend = FOSEMessagingInterface::kMessage_ExitGame;
	if (msg == kQuit_ToMainMenu)
	{
		msgToSend = FOSEMessagingInterface::kMessage_ExitToMainMenu;
		// Dispatch OnExitToMainMenu event
		EventManager::DispatchEventByID(EventManager::kEventID_ExitToMainMenu, nullptr);
	}
	else if (msg == kQuit_QQQ)
	{
		msgToSend = FOSEMessagingInterface::kMessage_ExitGame_Console;
		// Dispatch OnQQQ event (console-triggered exit via 'qqq' command)
		EventManager::DispatchEventByID(EventManager::kEventID_QQQ, nullptr);
	}
	else
	{
		// Dispatch OnExitGame event for kQuit_ToWindows
		EventManager::DispatchEventByID(EventManager::kEventID_ExitGame, nullptr);
	}

	PluginManager::Dispatch_Message(0, msgToSend, NULL, 0, NULL);
}

static __declspec(naked) void ExitGameFromMenuHook(void)
{
	__asm {
		pushad

		push	kQuit_ToWindows
		call	SendQuitGameMessage

		popad
		mov byte ptr [ eax + 1], 1
		retn
	}
}

static __declspec(naked) void ExitGameViaQQQHook(void)
{
	__asm {
		pushad

		push	kQuit_QQQ
		call	SendQuitGameMessage

		popad
		jmp		[kExitGameViaQQQRetnAddr]
	}
}

// MarkEvent hook - intercepts ScriptEventList::MarkEvent to dispatch events
// Function at 0x005183C0 is thiscall: ecx = ScriptEventList*, args = (TESForm* target, UInt32 eventMask)
// Prologue: 57 8B7908 32C0 (push edi / mov edi,[ecx+8] / xor al,al) = 6 bytes
static const UInt32 kMarkEventHookAddr = 0x005183C0;
static const UInt32 kMarkEventPrologueEnd = 0x005183C6;  // first instruction after overwritten bytes
static UInt8* s_markEventTrampoline = NULL;

// We need to reference s_eventsInUse from EventManager for the fast-path check
extern UInt32 EventManager::s_eventsInUse;

static __declspec(naked) void MarkEventHook(void)
{
	__asm
	{
		// At entry: ecx = ScriptEventList*, [esp+4] = target TESForm*, [esp+8] = eventMask
		mov		eax, [esp+8]			// eax = eventMask
		test	EventManager::s_eventsInUse, eax
		jz		skipHandle

		// Save ecx (this pointer needed by original function)
		push	ecx

		// Call HandleGameEvent(eventMask, target) via __stdcall
		push	dword ptr [esp+8]		// push target (esp shifted +4 by push ecx)
		push	eax						// push eventMask
		call	EventManager::HandleGameEvent

		// Restore ecx
		pop		ecx

	skipHandle:
		// Jump to trampoline (executes original prologue then continues)
		jmp		s_markEventTrampoline
	}
}

// MarkEvent variant #2 at 0x00518430 - discovered via MaskScanner.
// Handles: OnSell, OnStartCombat, OnOpen, OnClose, OnGrab (and likely more).
// Thiscall: ecx = ScriptEventList*, eventMask at [ecx+8].
// Prologue: 8B 41 08 85 C0 = 5 bytes (mov eax,[ecx+8]; test eax,eax).
static const UInt32 kMarkEvent2HookAddr = 0x00518430;
static const UInt32 kMarkEvent2PrologueEnd = 0x00518435;
static UInt8* s_markEvent2Trampoline = NULL;

static __declspec(naked) void MarkEvent2Hook(void)
{
	__asm
	{
		// At entry: ecx = ScriptEventList*, eventMask at [ecx+8]
		mov		eax, [ecx+8]			// eax = eventMask
		test	EventManager::s_eventsInUse, eax
		jz		skipHandle

		// Save ecx (this pointer needed by original function)
		push	ecx

		// Call HandleGameEvent(eventMask, target=0)
		push	0						// target = nullptr
		push	eax						// eventMask
		call	EventManager::HandleGameEvent

		pop		ecx

	skipHandle:
		jmp		s_markEvent2Trampoline
	}
}

// MarkEvent variant #3 at 0x00518DF0 - discovered via MaskScanner.
// Handles: OnStartCombat, OnOpen, OnClose, OnGrab (overlapping with variant #2).
// stdcall with multiple params: eventMask at [esp+8] at entry.
// Prologue: 53 56 8B 74 24 10 = 6 bytes (push ebx; push esi; mov esi,[esp+10h]).
// After 2 pushes (8 bytes), [esp+0x10] = original [esp+8] at entry.
static const UInt32 kMarkEvent3HookAddr = 0x00518DF0;
static const UInt32 kMarkEvent3PrologueEnd = 0x00518DF6;
static UInt8* s_markEvent3Trampoline = NULL;

static __declspec(naked) void MarkEvent3Hook(void)
{
	__asm
	{
		// At entry: [esp+8] = second param (eventMask based on scanner pattern)
		mov		eax, [esp+8]			// eax = eventMask
		test	EventManager::s_eventsInUse, eax
		jz		skipHandle

		push	0						// target = nullptr
		push	eax						// eventMask
		call	EventManager::HandleGameEvent
		add		esp, 8					// clean up arguments (stdcall caller-cleanup)

	skipHandle:
		jmp		s_markEvent3Trampoline
	}
}

// Additional event dispatch targets for missing events

// Target 0x00826590: handles OnDrop
// Prologue: 56 8B 74 24 08 85 F6 = push esi; mov esi,[esp+8]; test esi,esi
// stdcall: after 1 push (4 bytes), [esp+8] = original [esp+4] at entry
// Based on push-imm pattern, eventMask should be at [esp+4]
static const UInt32 kOnDropHookAddr = 0x00826590;
static const UInt32 kOnDropPrologueEnd = 0x00826596;  // after 6 bytes
static UInt8* s_onDropTrampoline = NULL;

static __declspec(naked) void OnDropHook(void)
{
	__asm
	{
		// At entry: [esp+4] = eventMask (based on push-imm pattern)
		mov		eax, [esp+4]			// eax = eventMask

		push	0						// target = nullptr
		push	eax						// eventMask
		call	EventManager::HandleGameEvent

		jmp		s_onDropTrampoline
	}
}

// Target 0x008FE580: handles OnDrop (alternate)
// Prologue: 55 8B EC 83 EC 08 = push ebp; mov ebp,esp; sub esp,8
// stdcall: after prologue, [ebp+8] = first param (eventMask)
static const UInt32 kOnDropAltHookAddr = 0x008FE580;
static const UInt32 kOnDropAltPrologueEnd = 0x008FE585;  // after 5 bytes
static UInt8* s_onDropAltTrampoline = NULL;

static __declspec(naked) void OnDropAltHook(void)
{
	__asm
	{
		// At entry: [ebp+8] = eventMask (after standard prologue)
		// But we hook before prologue, so read from [esp+4]
		mov		eax, [esp+4]			// eax = eventMask

		push	0						// target = nullptr
		push	eax						// eventMask
		call	EventManager::HandleGameEvent

		jmp		s_onDropAltTrampoline
	}
}

// Target 0x0040A4C0: handles OnHit
// Prologue: 8B 44 24 04 53 33 DB = mov eax,[esp+4]; push ebx; xor ebx,ebx
// stdcall: eventMask at [esp+4] at entry
static const UInt32 kOnHitHookAddr = 0x0040A4C0;
static const UInt32 kOnHitPrologueEnd = 0x0040A4C6;  // after 6 bytes
static UInt8* s_onHitTrampoline = NULL;

static __declspec(naked) void OnHitHook(void)
{
	__asm
	{
		// At entry: [esp+4] = eventMask
		mov		eax, [esp+4]			// eax = eventMask

		push	0						// target = nullptr
		push	eax						// eventMask
		call	EventManager::HandleGameEvent

		jmp		s_onHitTrampoline
	}
}

// Target 0x0086B930: handles OnHit, OnSell, OnGrab (top target in Scanner)
// Prologue: 83 EC 08 8B C1 80 38 00 89 04 24 75 13 8B 44 24 = sub esp,8; mov eax,ecx; cmp byte ptr [eax],0; mov [esp],eax; jne; mov eax,[esp]
// 5-byte prologue (sub esp,8)
static const UInt32 kMultiEventHookAddr = 0x0086B930;
static const UInt32 kMultiEventPrologueEnd = 0x0086B935;  // after 5 bytes
static UInt8* s_multiEventTrampoline = NULL;

static __declspec(naked) void MultiEventHook(void)
{
	__asm
	{
		// At entry: ecx = this pointer (need to check if valid)
		// This is a thiscall function, eventMask might be in ecx or derived from it
		test	ecx, ecx				// check if ecx is valid
		jz		skipEvent				// if ecx is null, skip event dispatch

		// For now, we can't extract eventMask from this function safely
		// The prologue shows: mov eax, ecx; cmp byte ptr [eax], 0
		// This suggests ecx is a pointer, not the eventMask
		// We'll need to investigate further to extract the correct parameters

skipEvent:
		jmp		s_multiEventTrampoline
	}
}

static const UInt32 kActivateHookAddr = 0x004EE000;	// Activate function entry (CRC 0x6E3D50D1)
static const UInt32 kActivatePrologueEnd = 0x004EE006;	// After 6-byte prologue (sub esp,114 = 81 EC 14 01 00 00)
static UInt8* s_activateTrampoline = NULL;				// Allocated trampoline memory

static const UInt32 kEquipItemHookAddr = 0x0053CF40;	// EquipItem function entry (CRC 0x6E3D50D1)
static const UInt32 kEquipItemPrologueEnd = 0x0053CF4E;	// After 14-byte prologue (6A FF 68 2872C300 64:A1 00000000)
static UInt8* s_equipItemTrampoline = NULL;				// Allocated trampoline memory

static void __cdecl ActivateHandler(void* target, void* activator)
{
	// Dispatch OnActivate event to registered handlers
	// Captured from naked hook: ecx was target, [esp+4] was activator
	void* params[2] = { activator, target };  // source=activator, target=activated
	EventManager::DispatchEvent("OnActivate", params);
}

static void EquipItemHandler()
{
	// Dispatch OnEquip event to registered handlers
	// TODO: Extract actor and item from stack/registers
	void* params[2] = { nullptr, nullptr };  // actor, item
	EventManager::DispatchEvent("OnEquip", params);
}

static __declspec(naked) void ActivateHook(void)
{
	__asm {
		// Capture args BEFORE pushad corrupts esp-relative offsets
		// At hook entry: ecx = target (this), [esp+4] = activator (first stack arg)
		// Stash them in eax/edx before pushad so we can use them after
		push	ecx					// save target on stack for later push
		push	dword ptr [esp + 8]	// save activator (was [esp+4] before push ecx)

		// Save all other registers
		pushad
		pushfd

		// Call cdecl handler: ActivateHandler(target, activator)
		// Stack layout now: [esp] flags, [esp+4..35] pushad, [esp+36]=activator, [esp+40]=target
		push	dword ptr [esp + 36]	// push activator (2nd arg)
		push	dword ptr [esp + 44]	// push target (1st arg) - offset shifted by 4 from prev push
		call	ActivateHandler
		add		esp, 8					// cdecl caller cleanup

		// Restore registers
		popfd
		popad

		// Clean up the 2 values we stashed before pushad
		add		esp, 8

		// Jump to trampoline to execute original prologue
		jmp		s_activateTrampoline
	}
}

static __declspec(naked) void EquipItemHook(void)
{
	__asm {
		// Save all registers
		pushad
		pushfd

		// Call our handler
		call	EquipItemHandler

		// Restore registers
		popfd
		popad

		// Jump to trampoline to execute original prologue
		jmp		s_equipItemTrampoline
	}
}

static __declspec(naked) void MainMenuFromIngameMenuHook(void)
{
	__asm {
		pushad

		push	kQuit_ToMainMenu
		call	SendQuitGameMessage

		popad
		jmp		[kMainMenuFromIngameMenuRetnAddr]
	}
}

void Hook_Gameplay_Init()
{
	// Install EventManager hooks
	EventManager::InstallGameHooks();

	// patch enchanted item check for saving cloned forms
	// conveniently the code and hence the patch are almost identical to that in Oblivion

	// inconveniently, the patched code is never called - Fallout doesn't bother saving the created
	// base objects. Given the code path is intact we may be able to hook it back up to
	// make it work
#if FALLOUT_VERSION == FALLOUT_VERSION_1_0_15
	SafeWrite8(0x006D9F7D, 0x20);
	WriteRelJump(0x006DA06F, 0x006DA102);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_1_35
	SafeWrite8(0x006DDD8D, 0x20);				// unknown
	WriteRelJump(0x006DDE7F, 0x006DDF12);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_4_6
	SafeWrite8(0x006DDCDD, 0x20);				// unknown
	WriteRelJump(0x006DDDCF, 0x006DDE62);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_4_6b
	SafeWrite8(0x006DE42D, 0x20);				// unknown
	WriteRelJump(0x006DE51F, 0x006DE5B2);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_5_22
	SafeWrite8(0x006DDD2D, 0x20);				// unknown
	WriteRelJump(0x006DDE1F, 0x006DDEB2);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_6
	SafeWrite8(0x006DD45D, 0x20);				// unknown
	WriteRelJump(0x006DD54F, 0x006DD5E2);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7
	SafeWrite8(0x006DD35D, 0x20);				// unknown
	WriteRelJump(0x006DD44F, 0x006DD4E2);
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7ng
	SafeWrite8(0x006DD52D, 0x20);				// unknown
	WriteRelJump(0x006DD61F, 0x006DD6B2);
#else
#error unsupported fallout version
#endif

	// hook exit to main menu or to windows
	WriteRelCall(kMainMenuFromIngameMenuPatchAddr, (UInt32)&MainMenuFromIngameMenuHook);
	WriteRelCall(kExitGameViaQQQPatchAddr, (UInt32)&ExitGameViaQQQHook);
	WriteRelCall(kExitGameFromMenuPatchAddr, (UInt32)&ExitGameFromMenuHook);

	// Run MaskScanner to discover event-marking functions for masks that
	// don't flow through ScriptEventList::MarkEvent at 0x005183C0.
	Scanner::RunMissingEventMaskScanner();

	// hook MarkEvent (ScriptEventList event marking) at entry with detour (6-byte prologue)
	s_markEventTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("MarkEvent hook: trampoline allocated at %08X", s_markEventTrampoline);
	if (s_markEventTrampoline)
	{
		// Copy original 6 bytes: push edi / mov edi,[ecx+8] / xor al,al
		memcpy(s_markEventTrampoline, (void*)kMarkEventHookAddr, 6);

		// Add JMP back to original function after prologue (at 0x005183C6)
		s_markEventTrampoline[6] = 0xE9;  // JMP rel32 opcode
		UInt32 markJumpOffset = kMarkEventPrologueEnd - ((UInt32)s_markEventTrampoline + 6 + 5);
		*(UInt32*)(s_markEventTrampoline + 7) = markJumpOffset;
		_MESSAGE("MarkEvent hook: trampoline JMP offset = %08X (target = %08X)", markJumpOffset, kMarkEventPrologueEnd);

		// Write 5-byte jump from function entry to our hook + NOP orphaned 6th byte
		WriteRelJump(kMarkEventHookAddr, (UInt32)&MarkEventHook);
		SafeWrite8(kMarkEventHookAddr + 5, 0x90);  // NOP the orphaned byte
		_MESSAGE("MarkEvent hook: installed at %08X -> %08X", kMarkEventHookAddr, (UInt32)&MarkEventHook);
	}

	// Re-enabling MarkEvent2Hook - covers OnSell, OnStartCombat, OnOpen, OnClose, OnGrab
	// Fast-path check prevents HandleGameEvent calls during initialization
	// hook MarkEvent variant #2 at 0x00518430 (5-byte prologue: 8B 41 08 85 C0)
	s_markEvent2Trampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("MarkEvent2 hook: trampoline allocated at %08X", s_markEvent2Trampoline);
	if (s_markEvent2Trampoline)
	{
		memcpy(s_markEvent2Trampoline, (void*)kMarkEvent2HookAddr, 5);
		s_markEvent2Trampoline[5] = 0xE9;
		UInt32 jump2Offset = kMarkEvent2PrologueEnd - ((UInt32)s_markEvent2Trampoline + 5 + 5);
		*(UInt32*)(s_markEvent2Trampoline + 6) = jump2Offset;
		_MESSAGE("MarkEvent2 hook: trampoline JMP offset = %08X (target = %08X)", jump2Offset, kMarkEvent2PrologueEnd);
		WriteRelJump(kMarkEvent2HookAddr, (UInt32)&MarkEvent2Hook);
		_MESSAGE("MarkEvent2 hook: installed at %08X -> %08X", kMarkEvent2HookAddr, (UInt32)&MarkEvent2Hook);
	}

	// DISABLED: MarkEvent3Hook - causes runtime error on main menu load
	// Not a true MarkEvent variant - calling convention or parameter extraction incorrect
	// MarkEvent2Hook already covers these events (OnStartCombat, OnOpen, OnClose, OnGrab)
	/*
	// hook MarkEvent variant #3 at 0x00518DF0 (6-byte prologue: 53 56 8B 74 24 10)
	s_markEvent3Trampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("MarkEvent3 hook: trampoline allocated at %08X", s_markEvent3Trampoline);
	if (s_markEvent3Trampoline)
	{
		memcpy(s_markEvent3Trampoline, (void*)kMarkEvent3HookAddr, 6);
		s_markEvent3Trampoline[6] = 0xE9;
		UInt32 jump3Offset = kMarkEvent3PrologueEnd - ((UInt32)s_markEvent3Trampoline + 6 + 5);
		*(UInt32*)(s_markEvent3Trampoline + 7) = jump3Offset;
		_MESSAGE("MarkEvent3 hook: trampoline JMP offset = %08X (target = %08X)", jump3Offset, kMarkEvent3PrologueEnd);
		WriteRelJump(kMarkEvent3HookAddr, (UInt32)&MarkEvent3Hook);
		SafeWrite8(kMarkEvent3HookAddr + 5, 0x90);
		_MESSAGE("MarkEvent3 hook: installed at %08X -> %08X", kMarkEvent3HookAddr, (UInt32)&MarkEvent3Hook);
	}
	*/

	// DISABLED: OnDrop and OnHit hooks causing game crash
	// These targets might be called during initialization or have wrong addresses/prologues
	/*
	// Hook OnDrop at 0x00826590 (6-byte prologue: 56 8B 74 24 08 85 F6)
	s_onDropTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("OnDrop hook: trampoline allocated at %08X", s_onDropTrampoline);
	if (s_onDropTrampoline)
	{
		memcpy(s_onDropTrampoline, (void*)kOnDropHookAddr, 6);

		s_onDropTrampoline[6] = 0xE9;
		UInt32 jumpDropOffset = kOnDropPrologueEnd - ((UInt32)s_onDropTrampoline + 6 + 5);
		*(UInt32*)(s_onDropTrampoline + 7) = jumpDropOffset;
		_MESSAGE("OnDrop hook: trampoline JMP offset = %08X (target = %08X)", jumpDropOffset, kOnDropPrologueEnd);

		WriteRelJump(kOnDropHookAddr, (UInt32)&OnDropHook);
		SafeWrite8(kOnDropHookAddr + 5, 0x90);  // NOP orphaned byte
		_MESSAGE("OnDrop hook: installed at %08X -> %08X", kOnDropHookAddr, (UInt32)&OnDropHook);
	}

	// Hook OnDrop alternate at 0x008FE580 (5-byte prologue: 55 8B EC 83 EC 08)
	s_onDropAltTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("OnDropAlt hook: trampoline allocated at %08X", s_onDropAltTrampoline);
	if (s_onDropAltTrampoline)
	{
		memcpy(s_onDropAltTrampoline, (void*)kOnDropAltHookAddr, 5);

		s_onDropAltTrampoline[5] = 0xE9;
		UInt32 jumpDropAltOffset = kOnDropAltPrologueEnd - ((UInt32)s_onDropAltTrampoline + 5 + 5);
		*(UInt32*)(s_onDropAltTrampoline + 6) = jumpDropAltOffset;
		_MESSAGE("OnDropAlt hook: trampoline JMP offset = %08X (target = %08X)", jumpDropAltOffset, kOnDropAltPrologueEnd);

		WriteRelJump(kOnDropAltHookAddr, (UInt32)&OnDropAltHook);
		_MESSAGE("OnDropAlt hook: installed at %08X -> %08X", kOnDropAltHookAddr, (UInt32)&OnDropAltHook);
	}

	// Hook OnHit at 0x0040A4C0 (6-byte prologue: 8B 44 24 04 53 33 DB)
	s_onHitTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("OnHit hook: trampoline allocated at %08X", s_onHitTrampoline);
	if (s_onHitTrampoline)
	{
		memcpy(s_onHitTrampoline, (void*)kOnHitHookAddr, 6);

		s_onHitTrampoline[6] = 0xE9;
		UInt32 jumpHitOffset = kOnHitPrologueEnd - ((UInt32)s_onHitTrampoline + 6 + 5);
		*(UInt32*)(s_onHitTrampoline + 7) = jumpHitOffset;
		_MESSAGE("OnHit hook: trampoline JMP offset = %08X (target = %08X)", jumpHitOffset, kOnHitPrologueEnd);

		WriteRelJump(kOnHitHookAddr, (UInt32)&OnHitHook);
		SafeWrite8(kOnHitHookAddr + 5, 0x90);  // NOP orphaned byte
		_MESSAGE("OnHit hook: installed at %08X -> %08X", kOnHitHookAddr, (UInt32)&OnHitHook);
	}
	*/

	// DISABLED: MultiEvent hook at 0x0086B930
	// This is a thiscall function with ecx as a pointer, not suitable for eventMask extraction
	// Based on NVSE investigation, OnSell needs custom barter menu hooks instead
	/*
	s_multiEventTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("MultiEvent hook: trampoline allocated at %08X", s_multiEventTrampoline);
	if (s_multiEventTrampoline)
	{
		memcpy(s_multiEventTrampoline, (void*)kMultiEventHookAddr, 5);

		s_multiEventTrampoline[5] = 0xE9;
		UInt32 jumpMultiOffset = kMultiEventPrologueEnd - ((UInt32)s_multiEventTrampoline + 5 + 5);
		*(UInt32*)(s_multiEventTrampoline + 6) = jumpMultiOffset;
		_MESSAGE("MultiEvent hook: trampoline JMP offset = %08X (target = %08X)", jumpMultiOffset, kMultiEventPrologueEnd);

		WriteRelJump(kMultiEventHookAddr, (UInt32)&MultiEventHook);
		_MESSAGE("MultiEvent hook: installed at %08X -> %08X", kMultiEventHookAddr, (UInt32)&MultiEventHook);
	}
	*/

	// hook Activate function at entry with detour (handles 6-byte FPO prologue)
	// Allocate executable trampoline memory (6 bytes prologue + 5 bytes jump back)
	s_activateTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("Activate hook: trampoline allocated at %08X", s_activateTrampoline);
	if (s_activateTrampoline)
	{
		// Copy original 6-byte prologue (sub esp,114) to trampoline
		memcpy(s_activateTrampoline, (void*)kActivateHookAddr, 6);

		// Add JMP back to original function after prologue (at 0x004EE006)
		s_activateTrampoline[6] = 0xE9;  // JMP rel32 opcode
		UInt32 jumpBackOffset = kActivatePrologueEnd - ((UInt32)s_activateTrampoline + 6 + 5);
		*(UInt32*)(s_activateTrampoline + 7) = jumpBackOffset;
		_MESSAGE("Activate hook: trampoline JMP offset = %08X (target = %08X)", jumpBackOffset, kActivatePrologueEnd);

		// Write 5-byte jump from function entry to our hook + NOP orphaned 6th byte
		WriteRelJump(kActivateHookAddr, (UInt32)&ActivateHook);
		SafeWrite8(kActivateHookAddr + 5, 0x90);  // NOP the orphaned byte
		_MESSAGE("Activate hook: installed at %08X -> %08X", kActivateHookAddr, (UInt32)&ActivateHook);
	}

	// hook EquipItem function at entry with detour (handles 14-byte prologue)
	// Allocate executable trampoline memory (14 bytes prologue + 5 bytes jump back)
	s_equipItemTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	_MESSAGE("EquipItem hook: trampoline allocated at %08X", s_equipItemTrampoline);
	if (s_equipItemTrampoline)
	{
		// Copy original 14-byte prologue (6A FF 68 2872C300 64:A1 00000000) to trampoline
		memcpy(s_equipItemTrampoline, (void*)kEquipItemHookAddr, 14);

		// Add JMP back to original function after prologue (at 0x0053CF4E)
		s_equipItemTrampoline[14] = 0xE9;  // JMP rel32 opcode
		UInt32 jumpBackOffset = kEquipItemPrologueEnd - ((UInt32)s_equipItemTrampoline + 14 + 5);
		*(UInt32*)(s_equipItemTrampoline + 15) = jumpBackOffset;
		_MESSAGE("EquipItem hook: trampoline JMP offset = %08X (target = %08X)", jumpBackOffset, kEquipItemPrologueEnd);

		// Write 5-byte jump from function entry to our hook + NOP remaining 9 bytes
		WriteRelJump(kEquipItemHookAddr, (UInt32)&EquipItemHook);
		for (UInt32 i = 5; i < 14; i++)
			SafeWrite8(kEquipItemHookAddr + i, 0x90);  // NOP orphaned bytes
		_MESSAGE("EquipItem hook: installed at %08X -> %08X", kEquipItemHookAddr, (UInt32)&EquipItemHook);
	}

}
