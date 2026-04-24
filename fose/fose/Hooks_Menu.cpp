#include "GameAPI.h"
#include "GameMenus.h"
#include "Hooks_Menu.h"
#include "fose_common\SafeWrite.h"
#include "fose_common\fose_version.h"
#include "GameInterface.h"
#include "PluginAPI.h"
#include "PluginManager.h"
#include "EventManager.h"

// ============================================================================
// UI Menu Action Dispatch Hook
// ============================================================================

// Forward declarations
void MenuActionDispatchHandler(UInt32 actionID, UInt32 returnAddr);
void TileClickHandler(UInt32 tileObj, UInt32 param1, UInt32 param2, UInt32 returnAddr);

// Hook address for the UI menu action dispatch function
// This function handles UIMenuOK, UIMenuCancel, UIMenuMode, etc.
// Found at 0061E7D0 in Fallout 3 1.7
// Prologue: 83EC 6C 8B4424 70 (7 bytes)

#if FALLOUT_VERSION == FALLOUT_VERSION_1_7
	static const UInt32 kMenuActionDispatchHookAddr = 0x0061E7D0;
	static const UInt32 kMenuActionDispatchPrologueEnd = 0x0061E7D7;  // After 7-byte prologue

	// Individual tile-direct menu handler addresses
	// These are menu-specific click handlers that call HandleClick (vtable slot 3)
	static const UInt32 kTileClickHandler1_Addr = 0x006842E4;  // Calls HandleClick, then UIMenuOK
	static const UInt32 kTileClickHandler2_Addr = 0x00686C60;  // Calls HandleClick, then UIMenuOK
	static const UInt32 kTileClickHandler3_Addr = 0x0066D030;  // Passes HandleClick to dispatcher
	static const UInt32 kTileClickHandler4_Addr = 0x0062652A;  // Calls different pattern, UIMenuCancel
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7_0_3
	static const UInt32 kMenuActionDispatchHookAddr = 0x00000000;	// TODO: Find actual address
	static const UInt32 kMenuActionDispatchPrologueEnd = 0x00000000;	// TODO: Find actual address
	static const UInt32 kTileClickHandler1_Addr = 0x00000000;
	static const UInt32 kTileClickHandler2_Addr = 0x00000000;
	static const UInt32 kTileClickHandler3_Addr = 0x00000000;
	static const UInt32 kTileClickHandler4_Addr = 0x00000000;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7_0_4
	// TODO: Find actual addresses for v1.7.0.4 (Steam version)
	// Use x64dbg to search for UIMenuOK string references to find these
	static const UInt32 kMenuActionDispatchHookAddr = 0x00000000;
	static const UInt32 kMenuActionDispatchPrologueEnd = 0x00000000;
	static const UInt32 kTileClickHandler1_Addr = 0x00000000;
	static const UInt32 kTileClickHandler2_Addr = 0x00000000;
	static const UInt32 kTileClickHandler3_Addr = 0x00000000;
	static const UInt32 kTileClickHandler4_Addr = 0x00000000;
#elif FALLOUT_VERSION == FALLOUT_VERSION_1_7ng
	static const UInt32 kMenuActionDispatchHookAddr = 0x00000000;	// TODO: Find actual address
	static const UInt32 kMenuActionDispatchPrologueEnd = 0x00000000;	// TODO: Find actual address
	static const UInt32 kTileClickHandler1_Addr = 0x00000000;
	static const UInt32 kTileClickHandler2_Addr = 0x00000000;
	static const UInt32 kTileClickHandler3_Addr = 0x00000000;
	static const UInt32 kTileClickHandler4_Addr = 0x00000000;
#else
	static const UInt32 kMenuActionDispatchHookAddr = 0x00000000;	// TODO: Find actual address
	static const UInt32 kMenuActionDispatchPrologueEnd = 0x00000000;	// TODO: Find actual address
	static const UInt32 kTileClickHandler1_Addr = 0x00000000;
	static const UInt32 kTileClickHandler2_Addr = 0x00000000;
	static const UInt32 kTileClickHandler3_Addr = 0x00000000;
	static const UInt32 kTileClickHandler4_Addr = 0x00000000;
#endif

static UInt8* s_menuActionDispatchTrampoline = NULL;
static UInt8* s_tileClickHandler1_Trampoline = NULL;
static UInt8* s_tileClickHandler2_Trampoline = NULL;
static UInt8* s_tileClickHandler3_Trampoline = NULL;
static UInt8* s_tileClickHandler4_Trampoline = NULL;

// Hook function for UI menu action dispatch
// Stdcall convention: parameters on stack
// Stack: [esp+70] = action ID (0-36 based on jump table)
static __declspec(naked) void MenuActionDispatchHook(void)
{
	__asm {
		// Save registers
		pushad
		push ebp
		mov ebp, esp

		// Extract action ID + return address from stack
		// Before original prologue runs, stack layout is:
		// [esp] = return address, [esp+4] = first param (action ID)
		// After pushad (32) + push ebp (4) + mov ebp,esp:
		// [ebp] = saved ebp, [ebp+4..+0x23] = pushad (32), [ebp+0x24] = return addr, [ebp+0x28] = param
		mov eax, [ebp+0x28]	// action ID
		mov edx, [ebp+0x24]	// return address (identifies caller / menu)

		push edx			// push return addr (2nd arg)
		push eax			// push action ID (1st arg)
		call MenuActionDispatchHandler

		// Restore stack
		add esp, 8

		// Restore registers
		mov esp, ebp
		pop ebp
		popad

		// Jump to trampoline (runs original prologue + jumps to post-prologue code)
		jmp dword ptr [s_menuActionDispatchTrampoline]
	}
}

// ============================================================================
// Tile-Direct Menu Click Hooks
// ============================================================================

// Hook for tile click handler 1 (0x006842E4)
// Pattern: mov eax, [esi]; mov edx, [eax+0xC]; push 0; push 9; call edx
// We hook at the call edx to capture the HandleClick call
static __declspec(naked) void TileClickHandler1_Hook(void)
{
	__asm {
		// Save registers
		pushad
		push ebp
		mov ebp, esp

		// Extract parameters from before the call
		// Stack before call: [esp] = return addr, [esp+4] = param2 (9), [esp+8] = param1 (0)
		// esi = tile object (in register)
		mov eax, esi			// tile object
		mov edx, [ebp+0x28]		// param2 (button ID)
		mov ecx, [ebp+0x2C]		// param1 (usually 0)
		mov ebx, [ebp+0x24]		// return address

		push ebx				// return addr
		push ecx				// param1
		push edx				// param2
		push eax				// tile object
		call TileClickHandler

		// Restore stack
		add esp, 16

		// Restore registers
		mov esp, ebp
		pop ebp
		popad

		// Execute original call edx (HandleClick)
		call edx

		// Return to original code
		ret
	}
}

// Hook for tile click handler 2 (0x00686C60)
// Similar pattern to handler 1
static __declspec(naked) void TileClickHandler2_Hook(void)
{
	__asm {
		// Save registers
		pushad
		push ebp
		mov ebp, esp

		// Extract parameters
		mov eax, esi			// tile object
		mov edx, [ebp+0x28]		// param2
		mov ecx, [ebp+0x2C]		// param1
		mov ebx, [ebp+0x24]		// return address

		push ebx
		push ecx
		push edx
		push eax
		call TileClickHandler

		add esp, 16

		mov esp, ebp
		pop ebp
		popad

		// Execute original call
		call eax

		ret
	}
}

// Hook for tile click handler 3 (0x0066D030)
// This one passes HandleClick pointer to a dispatcher
static __declspec(naked) void TileClickHandler3_Hook(void)
{
	__asm {
		// Save registers
		pushad
		push ebp
		mov ebp, esp

		// Extract parameters
		mov eax, esi			// tile object
		mov edx, [ebp+0x28]		// param2
		mov ecx, [ebp+0x2C]		// param1
		mov ebx, [ebp+0x24]		// return address

		push ebx
		push ecx
		push edx
		push eax
		call TileClickHandler

		add esp, 16

		mov esp, ebp
		pop ebp
		popad

		// Execute original code (push HandleClick pointer, call dispatcher)
		jmp dword ptr [s_tileClickHandler3_Trampoline]
	}
}

// Hook for tile click handler 4 (0x0062652A)
// This one plays UIMenuCancel, different pattern
static __declspec(naked) void TileClickHandler4_Hook(void)
{
	__asm {
		// Save registers
		pushad
		push ebp
		mov ebp, esp

		// Extract parameters
		mov eax, edi			// tile object (in edi for this one)
		mov edx, [ebp+0x28]		// param2
		mov ecx, [ebp+0x2C]		// param1
		mov ebx, [ebp+0x24]		// return address

		push ebx
		push ecx
		push edx
		push eax
		call TileClickHandler

		add esp, 16

		mov esp, ebp
		pop ebp
		popad

		// Execute original code
		jmp dword ptr [s_tileClickHandler4_Trampoline]
	}
}

// Walk up the parent chain from a tile to find the TileMenu that owns it.
// Returns the TileMenu (as Tile*, caller can cast) or NULL.
// NOTE: Tile::GetTypeID() actually returns a const char* pointer to the class
// name string ("IMGE", "RECT", "TEXT", "MENU", etc.), despite its UInt32 return
// type in the header. TileMenu's type string is "MENU".
static Tile* FindOwningTileMenu(Tile* tile)
{
	for (int depth = 0; tile && depth < 32; ++depth)
	{
		const char* typeStr = (const char*)tile->GetTypeID();
		if (typeStr && strcmp(typeStr, "MENU") == 0)
			return tile;
		tile = tile->parent;
	}
	return NULL;
}

// Handler function called from the hook
void MenuActionDispatchHandler(UInt32 actionID, UInt32 returnAddr)
{
	// Gather menu context from InterfaceManager
	UInt32 menuType = 0;
	const char* tileName = "";

	InterfaceManager* im = InterfaceManager::GetSingleton();
	if (im)
	{
		Tile* active = im->activeTile;
		if (active)
		{
			// Get tile name (owned by game, valid during dispatch)
			if (active->name.m_data)
				tileName = active->name.m_data;

			// Walk up to find owning TileMenu and extract menu type
			Tile* ownerTile = FindOwningTileMenu(active);
			if (ownerTile)
			{
				TileMenu* tileMenu = (TileMenu*)ownerTile;
				if (tileMenu->menu)
					menuType = tileMenu->menu->typeID;
			}
		}
	}

	// Log the menu action with full context (retAddr identifies the caller / menu)
	_MESSAGE("MenuActionDispatch - actionID=%d menuType=0x%03X tile='%s' retAddr=0x%08X",
		actionID, menuType, tileName, returnAddr);

	// Dispatch OnMenuClick event with 3 params: actionID, menuType, tileName
	void* params[3] = { (void*)actionID, (void*)menuType, (void*)tileName };
	EventManager::DispatchEvent("OnMenuClick", params);
}

// Handler for tile-direct menu clicks (called from individual handler hooks)
void TileClickHandler(UInt32 tileObj, UInt32 param1, UInt32 param2, UInt32 returnAddr)
{
	// Convert tile object to Tile pointer
	Tile* tile = (Tile*)tileObj;
	if (!tile)
		return;

	// Gather menu context
	UInt32 menuType = 0;
	const char* tileName = "";

	if (tile->name.m_data)
		tileName = tile->name.m_data;

	// Walk up to find owning TileMenu and extract menu type
	Tile* ownerTile = FindOwningTileMenu(tile);
	if (ownerTile)
	{
		TileMenu* tileMenu = (TileMenu*)ownerTile;
		if (tileMenu->menu)
			menuType = tileMenu->menu->typeID;
	}

	// Log the tile click with full context
	_MESSAGE("TileClick - tileObj=0x%08X param1=%d param2=%d menuType=0x%03X tile='%s' retAddr=0x%08X",
		tileObj, param1, param2, menuType, tileName, returnAddr);

	// Dispatch OnMenuClick event with 3 params: actionID (param2), menuType, tileName
	void* params[3] = { (void*)param2, (void*)menuType, (void*)tileName };
	EventManager::DispatchEvent("OnMenuClick", params);
}

// Install the UI menu action dispatch hook
void InstallMenuActionDispatchHook(void)
{
	if (kMenuActionDispatchHookAddr == 0)
	{
		_MESSAGE("MenuActionDispatch hook: Address not set, skipping hook installation");
		return;
	}

	_MESSAGE("MenuActionDispatch hook: Installing at %08X", kMenuActionDispatchHookAddr);

	// Allocate executable trampoline memory
	s_menuActionDispatchTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!s_menuActionDispatchTrampoline)
	{
		_MESSAGE("MenuActionDispatch hook: Failed to allocate trampoline memory");
		return;
	}

	_MESSAGE("MenuActionDispatch hook: Trampoline allocated at %08X", s_menuActionDispatchTrampoline);

	// Copy original prologue to trampoline (7 bytes)
	UInt32 prologueSize = 7;
	memcpy(s_menuActionDispatchTrampoline, (void*)kMenuActionDispatchHookAddr, prologueSize);

	// Add JMP back to original function after prologue
	s_menuActionDispatchTrampoline[prologueSize] = 0xE9;  // JMP rel32 opcode
	UInt32 jumpBackOffset = kMenuActionDispatchPrologueEnd - ((UInt32)s_menuActionDispatchTrampoline + prologueSize + 5);
	*(UInt32*)(s_menuActionDispatchTrampoline + prologueSize + 1) = jumpBackOffset;
	_MESSAGE("MenuActionDispatch hook: Trampoline JMP offset = %08X (target = %08X)", jumpBackOffset, kMenuActionDispatchPrologueEnd);

	// Write 5-byte jump from function entry to our hook
	WriteRelJump(kMenuActionDispatchHookAddr, (UInt32)&MenuActionDispatchHook);

	// NOP remaining 2 bytes from prologue (7 total - 5 for JMP = 2 bytes to NOP)
	SafeWrite8(kMenuActionDispatchHookAddr + 5, 0x90);
	SafeWrite8(kMenuActionDispatchHookAddr + 6, 0x90);

	_MESSAGE("MenuActionDispatch hook: Installed successfully at %08X -> %08X", kMenuActionDispatchHookAddr, (UInt32)&MenuActionDispatchHook);
}

// Install tile click handler 1 (0x006842E4)
void InstallTileClickHandler1(void)
{
	if (kTileClickHandler1_Addr == 0)
	{
		_MESSAGE("TileClickHandler1 hook: Address not set, skipping hook installation");
		return;
	}

	_MESSAGE("TileClickHandler1 hook: Installing at %08X", kTileClickHandler1_Addr);

	// Allocate trampoline
	s_tileClickHandler1_Trampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!s_tileClickHandler1_Trampoline)
	{
		_MESSAGE("TileClickHandler1 hook: Failed to allocate trampoline memory");
		return;
	}

	// Copy original code to trampoline (we need to copy enough to cover the call instruction)
	// The call is at offset from function start, so we copy from the call site
	UInt32 hookOffset = 0;  // TODO: Calculate actual offset to the call instruction
	UInt32 codeSize = 5;     // Size of call instruction
	memcpy(s_tileClickHandler1_Trampoline, (void*)(kTileClickHandler1_Addr + hookOffset), codeSize);

	// Add JMP back to original code after the call
	s_tileClickHandler1_Trampoline[codeSize] = 0xE9;
	UInt32 jumpBackOffset = (kTileClickHandler1_Addr + hookOffset + codeSize) - ((UInt32)s_tileClickHandler1_Trampoline + codeSize + 5);
	*(UInt32*)(s_tileClickHandler1_Trampoline + codeSize + 1) = jumpBackOffset;

	// Write 5-byte jump to our hook at the call site
	WriteRelJump(kTileClickHandler1_Addr + hookOffset, (UInt32)&TileClickHandler1_Hook);

	_MESSAGE("TileClickHandler1 hook: Installed successfully at %08X", kTileClickHandler1_Addr + hookOffset);
}

// Install tile click handler 2 (0x00686C60)
void InstallTileClickHandler2(void)
{
	if (kTileClickHandler2_Addr == 0)
	{
		_MESSAGE("TileClickHandler2 hook: Address not set, skipping hook installation");
		return;
	}

	_MESSAGE("TileClickHandler2 hook: Installing at %08X", kTileClickHandler2_Addr);

	// Allocate trampoline
	s_tileClickHandler2_Trampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!s_tileClickHandler2_Trampoline)
	{
		_MESSAGE("TileClickHandler2 hook: Failed to allocate trampoline memory");
		return;
	}

	// Copy original code and set up trampoline (similar to handler 1)
	UInt32 hookOffset = 0;
	UInt32 codeSize = 5;
	memcpy(s_tileClickHandler2_Trampoline, (void*)(kTileClickHandler2_Addr + hookOffset), codeSize);
	s_tileClickHandler2_Trampoline[codeSize] = 0xE9;
	UInt32 jumpBackOffset = (kTileClickHandler2_Addr + hookOffset + codeSize) - ((UInt32)s_tileClickHandler2_Trampoline + codeSize + 5);
	*(UInt32*)(s_tileClickHandler2_Trampoline + codeSize + 1) = jumpBackOffset;

	WriteRelJump(kTileClickHandler2_Addr + hookOffset, (UInt32)&TileClickHandler2_Hook);

	_MESSAGE("TileClickHandler2 hook: Installed successfully at %08X", kTileClickHandler2_Addr + hookOffset);
}

// Install tile click handler 3 (0x0066D030)
void InstallTileClickHandler3(void)
{
	if (kTileClickHandler3_Addr == 0)
	{
		_MESSAGE("TileClickHandler3 hook: Address not set, skipping hook installation");
		return;
	}

	_MESSAGE("TileClickHandler3 hook: Installing at %08X", kTileClickHandler3_Addr);

	// Allocate trampoline
	s_tileClickHandler3_Trampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!s_tileClickHandler3_Trampoline)
	{
		_MESSAGE("TileClickHandler3 hook: Failed to allocate trampoline memory");
		return;
	}

	// Copy original code and set up trampoline
	UInt32 codeSize = 10;  // This handler has different pattern, needs more bytes
	memcpy(s_tileClickHandler3_Trampoline, (void*)kTileClickHandler3_Addr, codeSize);
	s_tileClickHandler3_Trampoline[codeSize] = 0xE9;
	UInt32 jumpBackOffset = (kTileClickHandler3_Addr + codeSize) - ((UInt32)s_tileClickHandler3_Trampoline + codeSize + 5);
	*(UInt32*)(s_tileClickHandler3_Trampoline + codeSize + 1) = jumpBackOffset;

	WriteRelJump(kTileClickHandler3_Addr, (UInt32)&TileClickHandler3_Hook);

	_MESSAGE("TileClickHandler3 hook: Installed successfully at %08X", kTileClickHandler3_Addr);
}

// Install tile click handler 4 (0x0062652A)
void InstallTileClickHandler4(void)
{
	if (kTileClickHandler4_Addr == 0)
	{
		_MESSAGE("TileClickHandler4 hook: Address not set, skipping hook installation");
		return;
	}

	_MESSAGE("TileClickHandler4 hook: Installing at %08X", kTileClickHandler4_Addr);

	// Allocate trampoline
	s_tileClickHandler4_Trampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!s_tileClickHandler4_Trampoline)
	{
		_MESSAGE("TileClickHandler4 hook: Failed to allocate trampoline memory");
		return;
	}

	// Copy original code and set up trampoline
	UInt32 codeSize = 10;
	memcpy(s_tileClickHandler4_Trampoline, (void*)kTileClickHandler4_Addr, codeSize);
	s_tileClickHandler4_Trampoline[codeSize] = 0xE9;
	UInt32 jumpBackOffset = (kTileClickHandler4_Addr + codeSize) - ((UInt32)s_tileClickHandler4_Trampoline + codeSize + 5);
	*(UInt32*)(s_tileClickHandler4_Trampoline + codeSize + 1) = jumpBackOffset;

	WriteRelJump(kTileClickHandler4_Addr, (UInt32)&TileClickHandler4_Hook);

	_MESSAGE("TileClickHandler4 hook: Installed successfully at %08X", kTileClickHandler4_Addr);
}

// Initialize menu hooks
void InitMenuHooks(void)
{
	_MESSAGE("Initializing menu hooks...");
	InstallMenuActionDispatchHook();

	// Install tile-direct menu click handlers
	InstallTileClickHandler1();
	InstallTileClickHandler2();
	InstallTileClickHandler3();
	InstallTileClickHandler4();
}
