# ErrorScanner - Runtime Address Discovery Tool

## Overview

ErrorScanner is a universal memory scanning infrastructure built into FOSE that discovers function addresses at runtime by analyzing the game's executable. It eliminates the need for hardcoded addresses and enables version-agnostic hook discovery.

## Why This Matters

Traditional FOSE development requires:
- Pre-computed addresses for each game version (GOG, Steam, Anniversary)
- Manual reverse engineering with IDA/Ghidra
- Testers with specific game versions
- Risk of wrong addresses causing crashes

**ErrorScanner solves all of this** by finding addresses automatically at runtime.

## Scanner Types

### 1. String Xref Scanner

Finds all locations in `.text` that push a string address (from `.rdata`).

```cpp
// Pattern: push imm32 (0x68 XX XX XX XX)
for (each string in .rdata) {
    scan .text for: 0x68 [string_address]
}
```

**Use case:** Find error handlers by searching for error message strings.

### 2. CALL Xref Scanner

Finds all locations in `.text` that call a specific function.

```cpp
// Pattern: call rel32 (0xE8 XX XX XX XX)
// target = address + 5 + rel32
target = 0x005B5420;  // Console_Print
scan .text for calls to target
```

**Use case:** Verify if an address is actually a function being called.

### 3. Next-CALL Scanner (The Key Innovation)

For each string push, finds the next CALL instruction within N bytes.

```cpp
for (each string push at address Q) {
    for (next 64 bytes after Q) {
        if (find 0xE8 call) {
            target = Q + 5 + rel32
            log: "string at Q -> CALL at R -> target T"
        }
    }
}
```

**Use case:** Find the actual error handler function that consumes the error string.

### 4. Prologue Dumper

Reads function entry bytes to determine hook safety.

```cpp
UInt8* fn = (UInt8*)0x00519EC0;
log bytes: fn[0..15]
// Decode: 6A FF = push -1, 68 XX = push offset, etc.
```

**Use case:** Determine trampoline size needed for hook installation.

## Real-World Example: Finding RuntimeScriptError

### Step 1: Define Candidate Strings

```cpp
static const char* candidates[] = {
    "Syntax Error",
    "Missing ",
    "Invalid ",
    "Item '",
    "Script command",
    // ... more error strings
};
```

### Step 2: Run String + Next-CALL Scan

```
ErrorScanner: string 'Syntax Error' at 00DD440C
ErrorScanner:   xref push at 0051AA5D -> next CALL at 0051AA63 -> target 00519EC0
ErrorScanner:   xref push at 0051AAA5 -> next CALL at 0051AAAB -> target 00519EC0

ErrorScanner: string 'Missing ' at 00DD4910
ErrorScanner:   xref push at 0051C409 -> next CALL at 0051C40F -> target 00519EC0
```

### Step 3: Analyze Results

- `0x00519EC0` appears **8 times** as the CALL target
- Other targets (0x00887DA0, 0x0051A3A0, 0x00519A20) appear only once each
- **Conclusion:** `0x00519EC0` is the main script error handler

### Step 4: Dump Prologue

```
ScriptError: prologue bytes at 00519EC0:
6A FF 68 8B 69 C3 00 64 A1 00 00 00 00 50

Decode:
6A FF           push -1                 (2 bytes)
68 8B 69 C3 00  push offset __ehhandler (5 bytes)
64 A1 00 00 00 00  mov eax, fs:[0]      (6 bytes)
50              push eax                (1 byte)
Total: 14 bytes (MSVC SEH prologue)
```

### Step 5: Install Trampoline Hook

```cpp
// Allocate executable trampoline
s_trampoline = VirtualAlloc(32 bytes, PAGE_EXECUTE_READWRITE);

// Copy original 14-byte prologue
memcpy(s_trampoline, (void*)0x00519EC0, 14);

// Append JMP back to original function after prologue
s_trampoline[14] = 0xE9;  // JMP rel32
*(UInt32*)(s_trampoline+15) = 0x00519ECE - (trampoline+19);

// Hook: write 5-byte JMP to our handler, NOP remaining 9 bytes
WriteRelJump(0x00519EC0, (UInt32)&ScriptErrorHook);
SafeWrite8(0x00519EC0 + 5, 0x90);  // NOP
...
SafeWrite8(0x00519EC0 + 13, 0x90); // NOP
```

### Step 6: Handler Captures Full Error

```cpp
static void __stdcall ScriptErrorHandler(UInt32 retAddr, UInt32 ctx,
    UInt32 fmtAddr, UInt32 vararg0, UInt32 vararg1)
{
    // Format the full error message
    char formatted[512];
    _snprintf_s(formatted, sizeof(formatted), _TRUNCATE,
        (const char*)fmtAddr, vararg0, vararg1);

    _MESSAGE("ScriptError: %s", formatted);
    // Output: "Script command "xyz" not found."

    EventManager::DispatchEventByID(
        EventManager::kEventID_RuntimeScriptError, nullptr);
}
```

## API Reference

### ScanForScriptErrorHandlers()

Main entry point that runs all scanners.

**Location:** `fose/Hooks_SaveLoad.cpp`

**Output:** Logs to `fose.log`

### Reusable Scanner Components

```cpp
// Section header iteration
PIMAGE_SECTION_HEADER sec = IMAGE_FIRST_SECTION(nt);
for (UInt32 i = 0; i < nt->FileHeader.NumberOfSections; i++, sec++) {
    // sec->Name = ".text", ".rdata", etc.
    // sec->VirtualAddress = RVA
    // sec->Misc.VirtualSize = size
}

// String search in .rdata
for (UInt8* p = rdataBase; p < rdataBase + rdataSize; p++) {
    if (memcmp(p, targetString, strlen) == 0) {
        // Found string at p
    }
}

// Instruction scan in .text
for (UInt8* q = textBase; q < textBase + textSize; q++) {
    if (*q == 0x68) {  // push imm32
        UInt32 imm = *(UInt32*)(q+1);
        // imm is the pushed value
    }
    if (*q == 0xE8) {  // call rel32
        UInt32 rel = *(UInt32*)(q+1);
        UInt32 target = (UInt32)q + 5 + rel;
        // target is the called function
    }
}
```

## Pattern Reference

### Common x86 Instructions

| Opcode | Instruction | Size | Description |
|--------|-------------|------|-------------|
| 0xE8 XX XX XX XX | `call rel32` | 5 bytes | Call function at relative offset |
| 0x68 XX XX XX XX | `push imm32` | 5 bytes | Push 32-bit immediate |
| 0x6A XX | `push imm8` | 2 bytes | Push 8-bit immediate (sign-extended) |
| 0x57 | `push edi` | 1 byte | Push register |
| 0x50 | `push eax` | 1 byte | Push register |
| 0x89 XX | `mov r/m32, r32` | 2+ bytes | Register move |
| 0x8B XX | `mov r32, r/m32` | 2+ bytes | Register load |
| 0xE9 XX XX XX XX | `jmp rel32` | 5 bytes | Unconditional jump |
| 0xEB XX | `jmp rel8` | 2 bytes | Short jump |
| 0x90 | `nop` | 1 byte | No operation |

### MSVC SEH Prologue Pattern

```
6A FF                    push -1
68 XX XX XX XX           push offset __ehhandler
64 A1 00 00 00 00        mov eax, fs:[0]
50                       push eax
64 89 25 00 00 00 00     mov fs:[0], esp
```

**Total: 21 bytes** (but can be hooked after first 14 bytes)

## Future Applications

### Finding QQQ Hook

```cpp
static const char* candidates[] = {
    "qqq",
    "QQQ",
    "quit",
    "exit game",
    "console exit",
};
```

### Finding v1.7.0.4 Steam Addresses

Same scanner code, different base address (game auto-detects version). The discovered addresses will be different but the technique works identically.

### Finding Menu Click Dispatcher

```cpp
static const char* candidates[] = {
    "UIMenuOK",
    "click",
    "mouse1",
    "tile clicked",
    "menu action",
};
```

### Finding VTables

```cpp
// Pattern: mov [reg], vtable_address
// Common in constructors: mov [ecx], offset vtable
for (UInt8* p = textBase; p < textBase + textSize; p++) {
    if (p[0] == 0xC7 && p[1] == 0x01) {
        // mov [ecx], imm32
        UInt32 vtable = *(UInt32*)(p+2);
        if (vtable >= 0x00D00000 && vtable < 0x00E00000) {
            _MESSAGE("Possible vtable at %08X", vtable);
        }
    }
}
```

## Limitations

1. **Inlined functions** - Won't find functions that are inlined into callers
2. **Indirect calls** - Won't find `call [reg]` or `call [mem]` patterns
3. **Optimized builds** - May use different instruction patterns (LEA instead of PUSH for addresses)
4. **String deduplication** - Multiple identical strings share one address

## Tips for Success

1. **Use multiple candidate strings** - The more strings you search for, the more confident you can be about the results
2. **Look for frequency** - Functions called by many different strings are likely the main handlers
3. **Verify prologue** - Always dump and decode prologue bytes before installing hooks
4. **Test in Debug first** - Debug builds have clearer prologues and less optimization

## Comparison with Traditional Tools

| Feature | IDA/Ghidra | ErrorScanner |
|---------|-----------|--------------|
| Static analysis | ✅ Yes | ✅ Yes (at runtime) |
| Interactive | ✅ Yes | ❌ No (automated) |
| Version agnostic | ❌ No (need separate IDB) | ✅ Yes |
| No external tools | ❌ No | ✅ Yes |
| Works on any PC | ❌ No (need license/setup) | ✅ Yes |
| Real-time discovery | ❌ No | ✅ Yes |

**Verdict:** ErrorScanner is a lightweight alternative for common hook discovery tasks. Use IDA/Ghidra for complex analysis, ErrorScanner for quick runtime discovery.

## Credits

Developed for FOSE (Fallout 3 Script Extender) to enable version-agnostic hooking without relying on external reverse engineering tools.

**Key insight:** If you can describe what you're looking for (strings, patterns, behaviors), you can find it programmatically at runtime.
