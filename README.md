# xFOSE - Fallout 3 Script Extender

xFOSE is an updated, modernized continuation of the Fallout 3 Script Extender (FOSE) — the first major update in 15 years. It restores the broken Event Manager, modernizes the codebase with C++14 features, and provides new tools for modders, all while maintaining 100% backward compatibility with existing FOSE plugins.

## What's New

- **ScriptInterface API** - New interface for calling game scripts from plugins via `UDFCaller::CallUDF()`, enabling dynamic script execution with parameter passing and return values
- **ScriptRunner::ExecuteLine** - Fixed calling convention and parameter passing for the game's internal script execution function — plugins can now reliably call scripts via FOSE without crashes
- **Restored Event Manager** - The Event Manager was broken in the original FOSE and never worked. xFOSE fixes the initialization order bug and provides a fully working event system
- **Event Handlers** - Plugins can register handlers for game events: OnHit, OnDeath, OnLoad, OnEquip, OnActivate, OnKeyDown, OnKeyUp, OnKeyPress, and more
- **Phase 1 Event Manager Enhancements** - Event aliases, event flags system, internal events (LoadGame, SaveGame, ExitGame), better debug info with plugin/handler names, and event stack tracking
- **Phase 2 Event Manager Enhancements** - Advanced filtering system, enhanced priority system, handler inspection functions, and deferred removal for safe handler removal during dispatch
- **String Variables** - New script commands for string manipulation: create, set, get, concatenate, compare, and check validity with full save/load support
- **Arrays** - New script commands for array operations: create, set, get, push, pop, remove, clear, and size with full save/load support
- **Map and StringMap Containers** - New associative container types for key-value storage with full DataInterface support
- **Logging Interface** - New LoggingManager singleton for file-based logging with log levels (Info, Warning, Error) accessible via DataInterface
- **PlayerControls Interface** - New PlayerControlsManager singleton for input/control state checking and manipulation via DataInterface
- **Input Events** - New OnKeyDown, OnKeyUp, and OnKeyPress events for responding to keyboard input
- **DataInterface System** - Unified interface for accessing FOSE singletons and utility functions from plugins
- **Codebase Modernization** - C++14 support, STL-style containers, modern string handling, RAII hook management, and plugin API helpers
- **100% Backward Compatible** - Drop-in replacement for FOSE. All existing plugins and mods continue to work without modification
- **Open Source** - Free to use, modify, and upgrade by anyone

## Documentation

- **[MODERNIZATION.md](MODERNIZATION.md)** - Full modernization guide covering all C++14 upgrades, container improvements, and new helper APIs
- **[DEVELOPER_README.md](DEVELOPER_README.md)** - Developer guide for using the Event Manager API in your plugins
- **[EVENT_MANAGER_API.md](EVENT_MANAGER_API.md)** - Detailed Event Manager API reference
- **[DATAINTERFACE_API.md](DATAINTERFACE_API.md)** - DataInterface API for accessing FOSE singletons and utility functions
- **[STRING_VARIABLES.md](STRING_VARIABLES.md)** - String variable commands and usage guide for modders
- **[ARRAYS.md](ARRAYS.md)** - Array commands and usage guide for modders

## Installation (Users)

1. Copy `fose_loader.exe` and `fose_1_7.dll` to your Fallout 3 directory
2. Launch the game using `fose_loader.exe`
3. All existing FOSE plugins will continue to work as before

> **Note:** Windows Defender or Chrome may flag the download as a false positive. This is normal for script extenders that use DLL injection. Add the files to your antivirus exclusions to proceed.

## Building (Developers)

Requires Visual Studio and the Fallout 3 GOG or Steam version.

1. Open `fose.sln` in Visual Studio
2. Select Release|Win32 configuration
3. Build the solution
4. Output files will be in `bin\Release\`

## Event Manager Quick Start

```cpp
#include "fose/PluginAPI.h"

void OnHitHandler(void* params, void* context) {
    // Handle hit event
}

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    // Get Event Manager interface
    auto eventMgr = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!eventMgr) return false;
    
    // Register event handlers
    eventMgr->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0);
    eventMgr->RegisterEventHandler("OnDeath", OnDeathHandler, nullptr, 0);
    eventMgr->RegisterEventHandler("OnLoad", OnLoadHandler, nullptr, 0);
    eventMgr->RegisterEventHandler("OnEquip", OnEquipHandler, nullptr, 0);
    
    return true;
}
```

## Available Events

| Event | Description |
|-------|-------------|
| `OnHit` | Fired when an actor is hit |
| `OnDeath` | Fired when an actor dies |
| `OnLoad` | Fired when a game loads |
| `OnEquip` | Fired when an item is equipped |
| `OnActivate` | Fired when an object is activated |

## Modernization Highlights

- **String Class** - Modern constructors, std::string support, comparison operators
- **Containers** - STL-style `size()`, `empty()`, `begin()`/`end()`, range-based for loops on tList, NiTArray, BSSimpleArray, NiTPointerMap
- **Plugin API Helpers** - Type-safe interface queries, version checking, convenience wrappers
- **Game Data Helpers** - Form type checking, safe casting, lookup functions
- **Hook Helpers** - RAII-style hook management with automatic cleanup

See [MODERNIZATION.md](MODERNIZATION.md) for full details and code examples.

## License

This project is open source and free to use, modify, and distribute. See [LICENSE](LICENSE) for details.

## Credits

- **Original FOSE** by the FOSE Team
- **xFOSE** - Event Manager restoration and modernization by djuilesyboy10
- Inspired by xNVSE Event Manager implementation

## Support

For questions and support, see the [Nexus Mods page](https://www.nexusmods.com/fallout3) or open a GitHub Issue.
