# xFOSE Event Manager API - Developer Guide

## Overview

xFOSE restores and modernizes the FOSE Event Manager, allowing plugin developers to register callbacks for game events such as OnHit, OnDeath, OnLoad, OnEquip, and more. This API was non-functional in the original FOSE release but is now fully operational.

## Getting Started

To use the Event Manager in your xFOSE plugin:

1. Include the FOSE Plugin API headers
2. Query the Event Manager interface during plugin load
3. Register event handlers for the events you're interested in
4. Handle the events in your callback functions

## API Reference

### Querying the Event Manager

```cpp
#include "fose/PluginAPI.h"

static FOSEEventManagerInterface* g_eventManager = nullptr;

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    g_eventManager = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!g_eventManager)
        return false;
    
    // Register handlers...
    return true;
}
```

### Registering Event Handlers

```cpp
bool RegisterEventHandler(
    const char* eventName,          // Event name (e.g., "OnHit", "OnDeath")
    EventHandlerCallback callback,   // Your callback function
    void* context,                   // Optional context pointer passed to callback
    UInt32 priority                 // Handler priority (higher = called first)
);
```

### Callback Function Signature

```cpp
void MyEventHandler(void** params, void* context);
```

- `params`: Array of parameters for the event (event-dependent)
- `context`: The context pointer you passed when registering

## Available Events

### ScriptEventList Events

These events fire on specific game objects when scripted conditions are met:

| Event Name | Description | Parameters |
|---|---|---|
| OnAdd | Object added to container | `params[0]`: source, `params[1]`: target |
| OnEquip | Item equipped | `params[0]`: source, `params[1]`: target |
| OnDrop | Item dropped | `params[0]`: source, `params[1]`: target |
| OnUnequip | Item unequipped | `params[0]`: source, `params[1]`: target |
| OnDeath | Actor died | `params[0]`: source, `params[1]`: target |
| OnHit | Actor hit | `params[0]`: source, `params[1]`: target |
| OnLoad | Object loaded | `params[0]`: source, `params[1]`: target |
| OnGrab | Object grabbed | `params[0]`: source, `params[1]`: target |
| OnRelease | Object released | `params[0]`: source, `params[1]`: target |
| OnDestructionStageChange | Destruction stage changed | `params[0]`: source, `params[1]`: target |
| OnFire | Weapon fired | `params[0]`: source, `params[1]`: target |
| OnTrigger | Trigger activated | `params[0]`: source, `params[1]`: target |
| OnTriggerEnter | Object entered trigger | `params[0]`: source, `params[1]`: target |
| OnTriggerLeave | Object left trigger | `params[0]`: source, `params[1]`: target |
| OnReset | Object reset | `params[0]`: source, `params[1]`: target |

### Special-Cased Events

| Event Name | Description |
|---|---|
| OnActivate | Object activated (special parameter layout) |

## Example Plugin

```cpp
#include "ITypes.h"
#include "fose/PluginAPI.h"
#include <stdio.h>

static FOSEEventManagerInterface* g_eventManager = nullptr;

// Log to file for debugging
static void Log(const char* fmt, ...)
{
    FILE* f = nullptr;
    fopen_s(&f, "MyPlugin.log", "a");
    if (f)
    {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);
        fprintf(f, "\n");
        fflush(f);
        fclose(f);
    }
}

// Event handler for OnHit
void OnHitHandler(void** params, void* context)
{
    UInt32 source = params ? (UInt32)params[0] : 0;
    UInt32 target = params ? (UInt32)params[1] : 0;
    Log("OnHit: source=0x%08X target=0x%08X", source, target);
}

// Event handler for OnDeath
void OnDeathHandler(void** params, void* context)
{
    UInt32 target = params ? (UInt32)params[1] : 0;
    Log("OnDeath: target=0x%08X", target);
}

extern "C" __declspec(dllexport) bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info)
{
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "MyPlugin";
    info->version = 1;
    return true;
}

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    Log("MyPlugin loading...");
    
    g_eventManager = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!g_eventManager)
    {
        Log("Failed to get EventManager interface");
        return false;
    }
    
    // Register event handlers
    bool r1 = g_eventManager->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0);
    bool r2 = g_eventManager->RegisterEventHandler("OnDeath", OnDeathHandler, nullptr, 0);
    
    Log("Registration results: OnHit=%d OnDeath=%d", r1, r2);
    
    return true;
}
```

## Notes

- Event parameters are passed as pointers. Cast them to the appropriate types based on the event.
- The `source` parameter may be null for some events depending on how they're triggered.
- Events can fire frequently (e.g., OnHit during combat), so keep your handlers efficient.
- Multiple handlers can be registered for the same event by different plugins.
- Use the `context` parameter to pass instance data to your callback if needed.

## Building Plugins

Build your plugin as a 32-bit DLL for Windows. Link against the FOSE headers and ensure your project targets:
- Platform: Win32 (x86)
- Configuration: Release or Debug

Place your compiled `.dll` file in `Data\FOSE\Plugins\` in your Fallout 3 installation.

## Support

For issues, questions, or contributions, visit the xFOSE project page.

---

**xFOSE** — Bringing the Event Manager back to Fallout 3 modding.
