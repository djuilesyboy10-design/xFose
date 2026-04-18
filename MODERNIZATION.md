# FOSE Modernization Guide

This document describes the modernization improvements made to the FOSE codebase while maintaining strict backward compatibility.

## Overview

The FOSE codebase has been modernized to support modern C++ features while maintaining full binary compatibility with existing plugins. All changes are designed to improve developer ergonomics without affecting performance or breaking existing functionality.

## Modernization Phases

### Phase 1: C++14 and std::string Support
- Updated project to use C++14 standard
- Added std::string compatibility throughout the codebase

### Phase 2: String Class Modernization
The `String` class in `GameTypes.h` has been enhanced with modern C++ methods:

**Constructors:**
- `String(const char* str)` - Construct from C-string
- `String(const std::string& str)` - Construct from std::string
- `String(const String& other)` - Copy constructor

**Assignment:**
- `String& operator=(const String& other)` - Copy assignment
- `String& operator=(const std::string& str)` - std::string assignment
- `String& operator=(const char* str)` - C-string assignment

**Comparison:**
- `bool operator==(const String& other) const` - Equality comparison
- `bool operator!=(const String& other) const` - Inequality comparison
- `bool operator<(const String& other) const` - Less-than comparison

**Helper Methods:**
- `const char* c_str() const` - Get C-string representation
- `UInt32 length() const` - Get string length
- `bool empty() const` - Check if string is empty
- `String& operator+=(const String& other)` - Append operator

### Phase 3: Container Modernization

#### tList Container
The `tList` class now supports STL-like methods:
- `size()` - Returns number of elements
- `empty()` - Checks if list is empty
- `front()` - Returns first element
- `back()` - Returns last element
- `clear()` - Clears the list
- `push_back()` - Adds element to end
- `push_front()` - Adds element to beginning
- `operator[]` - Indexed access
- `begin()` / `end()` - Range-based for loop support

#### NiTArray Container
- `size()` - Returns array size
- `empty()` - Checks if array is empty
- `at()` - Bounds-checked access
- `front()` - Returns first element
- `back()` - Returns last element
- `capacity()` - Returns array capacity

#### BSSimpleArray Container
- `empty()` - Checks if array is empty
- `at()` - Bounds-checked access
- `front()` - Returns first element
- `back()` - Returns last element
- `capacity()` - Returns array capacity

#### NiTPointerMap Container
- `size()` - Returns number of entries
- `empty()` - Checks if map is empty
- `begin()` / `end()` - Iterator support

#### NiTMapBase Container
- `size()` - Returns number of entries
- `empty()` - Checks if map is empty
- `bucket_count()` - Returns number of buckets

### Phase 4: Command System Modernization
The `CommandTable` class now supports STL-like methods:
- `size()` - Returns number of commands
- `empty()` - Checks if table is empty
- `operator[]` - Indexed access to commands
- `begin()` / `end()` - Iterator support (const and non-const)

### Phase 5: Plugin API Helpers
Created `PluginAPIHelpers.h` - an opt-in header with helper functions for the Plugin API:
- Version checking helpers
- Type-safe interface query templates
- Convenience wrappers for common interfaces (Console, Messaging, Serialization)

**Note:** This is a separate header file that must be explicitly included. It does not affect core FOSE DLL performance.

### Phase 6: Utility Function Improvements
The `Tokenizer` class in `Utilities.h` now supports:
- `has_more()` - Checks if more tokens are available
- `reset()` - Resets tokenizer to beginning
- `get_data()` - Returns the source string
- `get_delims()` - Returns the delimiter string
- `get_offset()` - Returns the current offset

### Phase 7: Game Data Structure Helpers
Created `GameDataHelpers.h` - an opt-in header with helper functions for game data:
- Form type checking helpers (`IsFormType`, `IsActor`, `IsItem`)
- Type-safe form casting template (`DynamicCastForm`)
- Form lookup helpers (`LookupFormByID`, `LookupFormByEditorID`)
- Form flag helpers (`IsQuestItem`, `IsInitialized`)
- Form name helpers (`GetFormName`, `GetFormNameSafe`)
- Reference helpers (`GetReference`)
- Cell helpers (`GetContainingCell`)
- Base form helpers (`GetBaseForm`)

**Note:** This is a separate header file that must be explicitly included.

### Phase 8: Hook System Modernization
Created `HookHelpers.h` - an opt-in header with RAII-style hook management:
- `TempHookGuard` class - Automatic hook restoration on scope exit
- `InstallHookSafe()` - Safe hook installation with error checking
- `RemoveHookSafe()` - Safe hook removal with error checking
- `MakeTempHook()` - Factory function for creating hook guards

**Note:** This is a separate header file that must be explicitly included.

## Binary Compatibility

All modernization changes maintain strict binary compatibility:
- No changes to existing class layouts or memory structures
- Only method additions (no modifications or removals)
- All new helper headers are opt-in and don't affect core DLL
- No performance regressions in core FOSE functionality

## Usage Examples

### String Class
```cpp
String str1("Hello");
String str2 = std::string("World");

if (str1 == str2) { /* ... */ }
str1 += str2;
const char* cstr = str1.c_str();
```

### tList Container
```cpp
tList<SomeType> list;
list.push_back(item);

for (auto& item : list) {
    // Process item
}

if (!list.empty()) {
    auto first = list.front();
}
```

### CommandTable
```cpp
CommandTable& table = CommandTable::Get();

for (auto& cmd : table) {
    // Process command
}

if (!table.empty()) {
    CommandInfo& cmd = table[0];
}
```

### Plugin API Helpers
```cpp
#include "PluginAPIHelpers.h"

using namespace fose;

if (IsFOSEVersionCompatible(fose, 1, 7)) {
    auto console = GetConsoleInterface(fose);
    if (console) {
        // Use console interface
    }
}
```

### Game Data Helpers
```cpp
#include "GameDataHelpers.h"

using namespace fose;

TESForm* form = LookupFormByID(formID);
if (IsActor(form)) {
    std::string name = GetFormNameSafe(form);
}
```

### Hook Helpers
```cpp
#include "HookHelpers.h"

using namespace fose;

{
    TempHookGuard hook(hookAddr, jmpAddr);
    // Hook is active in this scope
    // Automatically restored when guard goes out of scope
}
```

## Performance Considerations

- All core FOSE changes maintain original performance characteristics
- Helper headers are opt-in and don't affect core DLL
- Inline helpers are only used in opt-in headers to avoid performance impact
- No virtual function overhead added to core classes

## Testing

All modernization phases have been tested:
- Compilation successful
- In-game testing confirms no performance regressions
- All existing functionality preserved
