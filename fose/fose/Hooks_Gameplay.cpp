#include "CommandTable.h"
#include "GameAPI.h"
#include "Hooks_Gameplay.h"
#include "fose_common\SafeWrite.h"
#include "fose_common\fose_version.h"
#include "GameInterface.h"
#include "PluginAPI.h"
#include "PluginManager.h"
#include "EventManager.h"

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
		msgToSend = FOSEMessagingInterface::kMessage_ExitGame_Console;
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
		// Fast-path: check if any events are in use
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

static const UInt32 kActivateHookAddr = 0x004EE000;	// Activate function entry (CRC 0x6E3D50D1)
static const UInt32 kActivatePrologueEnd = 0x004EE006;	// After 6-byte prologue (sub esp,114 = 81 EC 14 01 00 00)
static UInt8* s_activateTrampoline = NULL;				// Allocated trampoline memory

static const UInt32 kEquipItemHookAddr = 0x0053CF40;	// EquipItem function entry (CRC 0x6E3D50D1)
static const UInt32 kEquipItemPrologueEnd = 0x0053CF4E;	// After 14-byte prologue (6A FF 68 2872C300 64:A1 00000000)
static UInt8* s_equipItemTrampoline = NULL;				// Allocated trampoline memory

static void ActivateHandler()
{
	// Dispatch OnActivate event to registered handlers
	// TODO: Extract activator and target from stack/registers
	void* params[2] = { nullptr, nullptr };  // activator, target
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
		// Save all registers
		pushad
		pushfd

		// Call our handler
		call	ActivateHandler

		// Restore registers
		popfd
		popad

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
