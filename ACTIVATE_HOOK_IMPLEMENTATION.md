# Activate Hook Implementation - Fallout 3 FOSE

## Problem
The Activate function hook was causing crashes when activating objects (terminals, beds, doors, etc.) in Fallout 3.

## Root Cause Analysis

### Why Previous Approaches Failed
1. **Call-site hooks inside Activate** — All 6 internal call sites were object-specific (conditional), causing crashes on objects that didn't follow that path
2. **External call-site hooks** — 40+ external references, all object-specific, not universal
3. **Direct 5-byte JMP at function entry** — The prologue is 6 bytes (`sub esp, 114`), not 5. A 5-byte JMP would leave a dangling byte and break stack alignment

### The Real Issue: FPO-Optimized Prologue
```
Address: 0x004EE000
Bytes:   81 EC 14 01 00 00
Instr:   sub esp, 114
Length:  6 bytes
```

This is a Frame Pointer Omission (FPO) optimization that adjusts the stack by 276 bytes (0x114). Overwriting this with a 5-byte `JMP rel32` would:
- Leave 1 orphaned byte from the original instruction
- Break stack alignment for the entire function
- Cause unpredictable crashes

## Solution: Detour Hook with Trampoline

### Architecture
```
Original Function Entry (0x004EE000):
  [6-byte prologue] -> [rest of function]

After Hook Installation:
  [5-byte JMP to ActivateHook] [NOP] -> [rest of function]

Trampoline (allocated memory):
  [6-byte prologue copy] [5-byte JMP back to 0x004EE006]
```

### Implementation Steps

#### 1. Allocate Executable Memory
```cpp
s_activateTrampoline = (UInt8*)VirtualAlloc(NULL, 32, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
```
- Must use `VirtualAlloc` with `PAGE_EXECUTE_READWRITE` (HeapAlloc doesn't give execute permission)
- Allocate 32 bytes (more than enough for 6 + 5 = 11 bytes)

#### 2. Copy Original Prologue
```cpp
memcpy(s_activateTrampoline, (void*)kActivateHookAddr, 6);
```
- Copy the 6-byte `sub esp, 114` instruction to trampoline

#### 3. Write Jump Back to Original Function
```cpp
s_activateTrampoline[6] = 0xE9;  // JMP rel32 opcode
UInt32 jumpBackOffset = kActivatePrologueEnd - ((UInt32)s_activateTrampoline + 6 + 5);
*(UInt32*)(s_activateTrampoline + 7) = jumpBackOffset;
```
- Calculate relative offset: `target - (trampoline + 6 + 5)`
- Write JMP at byte 6, offset at bytes 7-10
- Jump back to `0x004EE006` (immediately after original prologue)

#### 4. Install Hook at Function Entry
```cpp
WriteRelJump(kActivateHookAddr, (UInt32)&ActivateHook);
SafeWrite8(kActivateHookAddr + 5, 0x90);  // NOP orphaned byte
```
- Write 5-byte relative JMP from `0x004EE000` to our hook function
- NOP the 6th byte (orphaned from original 6-byte instruction)

#### 5. Hook Function with Proper Context Preservation
```cpp
static __declspec(naked) void ActivateHook(void)
{
    __asm {
        // Save all registers
        pushad
        pushfd

        // Call our handler
        call    ActivateHandler

        // Restore registers
        popfd
        popad

        // Jump to trampoline to execute original prologue
        jmp     s_activateTrampoline
    }
}
```
- `pushad` saves all 8 general-purpose registers
- `pushfd` saves flags register
- Call handler, then restore everything
- Jump to trampoline which executes the original prologue and jumps back

#### 6. Handler Function
```cpp
static void ActivateHandler()
{
    _MESSAGE("Activate Hook Called!");
    Console_Print("Activate Hook Called!");
}
```
- `_MESSAGE` writes to `fose.log` for debugging
- `Console_Print` shows in in-game console (press `~`)

## Key Lessons

### 1. Always Check Instruction Length
Before hooking at function entry, examine the prologue:
- Use x64dbg to view the first few instructions
- Count the total bytes of the prologue
- If prologue > 5 bytes, you need a detour with trampoline

### 2. Executable Memory is Required
- `HeapAlloc` does NOT provide execute permission
- Must use `VirtualAlloc` with `PAGE_EXECUTE_READWRITE`
- Failure to do this causes access violation crashes

### 3. JMP Opcode Must Come Before Offset
WRONG:
```cpp
*(UInt32*)(s_activateTrampoline + 7) = jumpOffset;
s_activateTrampoline[7] = 0xE9;  // Overwrites first byte of offset!
```
CORRECT:
```cpp
s_activateTrampoline[6] = 0xE9;  // Write opcode first
*(UInt32*)(s_activateTrampoline + 7) = jumpOffset;  // Then offset
```

### 4. NOP Orphaned Bytes
When overwriting a multi-byte instruction with a 5-byte JMP:
- Any bytes beyond the 5 must be NOP'd (0x90)
- This prevents execution of partial instructions

### 5. Relative Jump Offset Calculation
```
offset = destination - (source + instruction_length)
```
For JMP at trampoline byte 6:
```
offset = kActivatePrologueEnd - ((UInt32)s_activateTrampoline + 6 + 5)
```

## Testing Results
- ✅ Terminal activation — No crash, hook fires
- ✅ Bed activation — No crash, hook fires
- ✅ Door activation — No crash, hook fires
- ✅ Multiple activations — Hook fires each time reliably
- ✅ No performance issues or side effects observed

## Code Location
`C:\Users\djuil\CascadeProjects\FOSEBeta\fose\fose\Hooks_Gameplay.cpp`

Lines 106-132: Hook constants and handler
Lines 189-208: Hook installation with trampoline setup

## Future Hooks
Use the same detour approach for:
- AddItem (0x00530D7A) — **Not needed** - Activate covers item pickup from world
- EquipItem (0x0053CF40) — **Implemented** - See below

## EquipItem Hook Implementation

### Address Verification
Initial research suggested `0x0053CF52`, but this was incorrect — it's in the middle of the prologue. The actual function entry is at `0x0053CF40`.

### Prologue Analysis
```
Address: 0x0053CF40
Bytes:   6A FF 68 2872C300 64:A1 00000000
Instr:   push FFFFFFFF
         push fallout3.C37228
         mov eax,dword ptr fs:[0]
Length:  14 bytes
```

### Implementation
Same detour pattern as Activate, but with 14-byte prologue:
1. Allocate 32-byte trampoline with `VirtualAlloc`
2. Copy 14-byte prologue to trampoline
3. Write JMP back to `0x0053CF4E` (after prologue)
4. Write 5-byte JMP from `0x0053CF40` to hook
5. NOP remaining 9 bytes (14 - 5)

### Code Location
`C:\Users\djuil\CascadeProjects\FOSEBeta\fose\fose\Hooks_Gameplay.cpp`

Lines 114-116: Hook constants
Lines 130-134: Handler function
Lines 174-191: Hook function
Lines 288-308: Hook installation

### Testing Results
- ✅ Weapon equip — Hook fires, no crash
- ✅ Armor equip — Hook fires, no crash
- ✅ No performance issues observed

## MarkEvent Hook Implementation

### Purpose
The MarkEvent hook intercepts the game's `ScriptEventList::MarkEvent` function to enable event dispatch for ScriptEventList events (OnHit, OnDeath, OnLoad, OnEquip, etc.) through the FOSE EventManager. This provides a centralized hook point for all ScriptEventList events instead of individual hooks per event type.

### Function Analysis
```
Address: 0x005183C0
Type:    thiscall member function of ScriptEventList
Args:    TESForm* target, UInt32 eventMask
Prologue: 6 bytes (push edi, mov edi, [ecx+8])
```

The function iterates through the event list and ORs the event mask into matching events. It's a thiscall, meaning `ecx` holds the `this` pointer (ScriptEventList*).

### Implementation

#### 1. Event Mask Constants
Added ScriptEventList event mask constants to `GameAPI.h` matching the game's bitmask values:
```cpp
enum EventMask
{
    kEventMask_OnAdd = 0x01,
    kEventMask_OnEquip = 0x02,
    kEventMask_OnDrop = 0x04,
    kEventMask_OnUnequip = 0x08,
    kEventMask_OnDeath = 0x10,
    kEventMask_OnMurder = 0x20,
    kEventMask_OnCombatEnd = 0x40,
    kEventMask_OnHit = 0x80,
    // ... etc
};
```

#### 2. EventID Mapping
Expanded `EventManager.h` EventID enum with ScriptEventList event IDs corresponding to mask bits. Added `EventIDForMask()` and `MaskForEventID()` functions to convert between masks and EventIDs.

#### 3. Event Mask Tracking
Added `s_eventsInUse` bitmask to track which ScriptEventList events have registered handlers. This allows a fast-path check in the hook to skip HandleGameEvent when no handlers are registered for a given event.

#### 4. HandleGameEvent Dispatcher
Implemented `HandleGameEvent()` in `EventManager.cpp`:
- Maps event mask to EventID using `EventIDForMask()`
- Performs deduplication to prevent duplicate dispatch within the same frame
- Uses `GetTickCount()` for per-frame dedup reset (no main loop hook needed)
- Dispatches to registered handlers via `DispatchEventByID()`
- Logs events for debugging

#### 5. Hook Installation
Same detour pattern as Activate/EquipItem:
1. Allocate 32-byte trampoline with `VirtualAlloc` (`PAGE_EXECUTE_READWRITE`)
2. Copy 6-byte prologue to trampoline
3. Write JMP back to `0x005183C6` (after prologue)
4. Write 5-byte JMP from `0x005183C0` to hook
5. NOP the 6th byte (orphaned from original 6-byte instruction)

#### 6. Hook Function with Fast-Path Check
```cpp
static __declspec(naked) void MarkEventHook(void)
{
    __asm {
        push    ebp
        mov     ebp, esp
        sub     esp, 8

        mov     eax, [ebp+0xC]      // eventMask
        test    eax, eax
        jz      skipHandle

        test    s_eventsInUse, eax  // Fast-path: check if any handlers registered
        jz      skipHandle

        push    dword ptr [ebp+8]   // target
        push    ecx                  // ScriptEventList* (this)
        push    eax                  // eventMask
        call    HandleGameEvent

    skipHandle:
        jmp     kMarkEvent_RetnAddr
    }
}
```

The fast-path check on `s_eventsInUse` minimizes overhead when no handlers are registered for a given event type.

#### 7. Handler Registration Update
Modified `RegisterEventHandler()` to OR event mask bits into `s_eventsInUse` when handlers register for ScriptEventList events:
```cpp
if (eventInfo->evID < kEventID_ScriptEventListMAX) {
    s_eventsInUse |= MaskForEventID(eventInfo->evID);
    _MESSAGE("RegisterEventHandler: '%s' -> s_eventsInUse = 0x%08X", eventName, s_eventsInUse);
}
```

### Code Location
`C:\Users\djuil\CascadeProjects\FOSEBeta\fose\fose\Hooks_Gameplay.cpp`
- Lines 107-146: MarkEvent hook function
- Lines 259-298: Hook installation with trampoline setup

`C:\Users\djuil\CascadeProjects\FOSEBeta\fose\fose\EventManager.cpp`
- Lines 8-157: Event mask lookup table and MaskForEventID function
- Lines 105-152: HandleGameEvent dispatcher with deduplication
- Lines 280-293: RegisterEventHandler update to set s_eventsInUse
- Lines 166-171: Test diagnostic callback (for verification)

`C:\Users\djuil\CascadeProjects\FOSEBeta\fose\fose\EventManager.h`
- Lines 19-67: Expanded EventID enum with ScriptEventList events
- Lines 158-172: EventIDForMask, MaskForEventID, HandleGameEvent declarations
- Line 167: s_eventsInUse extern declaration

`C:\Users\djuil\CascadeProjects\FOSEBeta\fose\fose\GameAPI.h`
- Lines 20-66: ScriptEventList event mask constants

### Testing Results
- ✅ Hook installed successfully at 0x005183C0
- ✅ s_eventsInUse correctly set when handlers register (0x00001092 for OnHit, OnDeath, OnLoad, OnEquip)
- ✅ HandleGameEvent fires for OnHit (mask=0x80, evID=7)
- ✅ HandleGameEvent fires for OnDeath (mask=0x10, evID=4)
- ✅ HandleGameEvent fires for OnLoad (mask=0x1000, evID=12)
- ✅ HandleGameEvent fires for OnEquip (mask=0x02, evID=1)
- ✅ Test handlers dispatch successfully
- ✅ Deduplication prevents duplicate dispatch
- ✅ No crashes or performance issues observed

### Key Differences from xNVSE
- **FO3 MarkEvent is a thiscall** (ecx = ScriptEventList*), whereas xNVSE's is stdcall
- **No source TESObjectREFR extraction** in FO3 hook (not feasible with thiscall signature), target is passed directly
- **Inline Tick() via GetTickCount** for dedup reset instead of main loop hook

## Summary
- **Activate Hook**: Fully working at `0x004EE000` (6-byte prologue)
- **EquipItem Hook**: Fully working at `0x0053CF40` (14-byte prologue)
- **MarkEvent Hook**: Fully working at `0x005183C0` (6-byte prologue, thiscall)
- **AddItem Hook**: Not needed — Activate covers world item interactions
