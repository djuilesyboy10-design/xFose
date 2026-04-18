# xFOSE - Fallout 3 Script Extender

xFOSE is a continuation of the Fallout 3 Script Extender (FOSE), restoring the broken Event Manager and enabling modders to respond to game events like hits, deaths, equips, and loads.

## Features

- **Restored Event Manager** - The Event Manager, broken in the original FOSE, now works correctly
- **Event Handlers** - Plugins can register handlers for events: OnHit, OnDeath, OnLoad, OnEquip, OnActivate, and more
- **100% Backward Compatible** - Existing FOSE plugins continue to work without modification
- **Open Source** - Free to use, modify, and upgrade by anyone

## Building

Requires Visual Studio and the Fallout 3 GOG or Steam version.

1. Open `fose.sln` in Visual Studio
2. Select Release|Win32 configuration
3. Build the solution
4. The output files will be in `bin\Release\`

## Installation

1. Copy `fose_loader.exe` and `fose_1_7.dll` to your Fallout 3 directory
2. Launch the game using `fose_loader.exe`

## Event Manager API

Plugins can use the Event Manager interface to register event handlers:

```cpp
#include "fose/PluginAPI.h"

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    g_fose = fose;
    
    // Get Event Manager interface
    g_eventManager = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!g_eventManager)
        return false;
    
    // Register event handlers
    g_eventManager->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0);
    g_eventManager->RegisterEventHandler("OnDeath", OnDeathHandler, nullptr, 0);
    g_eventManager->RegisterEventHandler("OnLoad", OnLoadHandler, nullptr, 0);
    g_eventManager->RegisterEventHandler("OnEquip", OnEquipHandler, nullptr, 0);
    
    return true;
}
```

## Available Events

- `OnHit` - Fired when an actor is hit
- `OnDeath` - Fired when an actor dies
- `OnLoad` - Fired when a game loads
- `OnEquip` - Fired when an item is equipped
- `OnActivate` - Fired when an object is activated

## License

This project is open source and free to use, modify, and distribute. See LICENSE file for details.

## Credits

- Original FOSE by the FOSE Team
- xFOSE Event Manager restoration by djuilesyboy10
- Based on xNVSE Event Manager implementation

## Support

For modding questions and support, see the Nexus Mods page or GitHub Issues.
