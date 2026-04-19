# FOSE Event Manager API Guide

## Overview

The FOSE Event Manager API allows plugins to register handlers for game events without requiring individual hooks per event type. This provides a centralized, scalable infrastructure for event handling in Fallout 3.

## Available Events

### ScriptEventList Events
These events use `kEventParams_GameEvent` (2 parameters: source, target):

- `OnAdd` - Item added to container
- `OnEquip` - Item equipped
- `OnDrop` - Item dropped
- `OnUnequip` - Item unequipped
- `OnDeath` - Actor dies
- `OnMurder` - Actor is murdered
- `OnCombatEnd` - Combat ends
- `OnHit` - Actor is hit
- `OnHitWith` - Actor hit with specific weapon
- `OnPackageStart` - AI package starts
- `OnPackageDone` - AI package completes
- `OnPackageChange` - AI package changes
- `OnLoad` - Object loads into cell
- `OnMagicEffectHit` - Magic effect hits target
- `OnSell` - Item sold
- `OnStartCombat` - Combat starts
- `OnOpen` - Container/door opened
- `OnClose` - Container/door closed
- `SayToDone` - SayTo dialogue completes
- `OnGrab` - Object grabbed
- `OnRelease` - Object released
- `OnDestructionStageChange` - Destruction stage changes
- `OnFire` - Object catches fire
- `OnTrigger` - Trigger activated
- `OnTriggerEnter` - Actor enters trigger
- `OnTriggerLeave` - Actor leaves trigger
- `OnReset` - Object resets

### Special-Cased Events
- `OnActivate` - Object activated (uses `kEventParams_OnActivate`)
- `OnFrame` - Called every frame (no parameters)

## Getting the Interface

Query the Event Manager interface during your plugin's `FOSEPlugin_Load` function:

```cpp
bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    // Query the Event Manager interface
    FOSEEventManagerInterface* eventIntfc = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!eventIntfc || eventIntfc->version < FOSEEventManagerInterface::kVersion)
    {
        return false; // Incompatible version
    }

    // Save the interface pointer for later use
    g_eventManager = eventIntfc;

    return true;
}
```

## Registering an Event Handler

Define a callback function matching the `EventHandlerCallback` signature:

```cpp
void MyOnHitHandler(void** params, void* context)
{
    // params[0] = source (may be null for some events)
    // params[1] = target (the object the event occurred on)
    
    TESObjectREFR* target = (TESObjectREFR*)params[1];
    if (target)
    {
        // Handle the event
        _MESSAGE("OnHit event on object: %08X", target->formID);
    }
}
```

Register the handler during plugin load (or after receiving `kMessage_PostLoad`):

```cpp
bool success = g_eventManager->RegisterEventHandler(
    "OnHit",                    // Event name
    MyOnHitHandler,             // Callback function
    (void*)0xDEADBEEF,          // Optional context pointer
    100                         // Priority (higher = called first)
);

if (!success)
{
    _ERROR("Failed to register OnHit handler");
}
```

## Removing an Event Handler

To remove a handler, call `RemoveEventHandler` with the same parameters used during registration:

```cpp
bool success = g_eventManager->RemoveEventHandler(
    "OnHit",                    // Event name
    MyOnHitHandler,             // Callback function
    (void*)0xDEADBEEF           // Context pointer (must match registration)
);
```

## Event Parameters

### ScriptEventList Events (kEventParams_GameEvent)
```cpp
void** params = ...; // Passed to your callback
void* source = params[0];  // Source object (may be null)
void* target = params[1];  // Target object (the event occurred on this object)
```

### OnActivate (kEventParams_OnActivate)
```cpp
void** params = ...; // Passed to your callback
void* source = params[0];  // Activating reference
void* target = params[1];  // Activated reference
```

## Complete Example Plugin

```cpp
#include "fose/PluginAPI.h"
#include "fose/EventManager.h"

static FOSEEventManagerInterface* g_eventManager = nullptr;

// Event handler callback
void OnHitCallback(void** params, void* context)
{
    TESObjectREFR* target = (TESObjectREFR*)params[1];
    if (target)
    {
        _MESSAGE("OnHit: Target = %08X", target->formID);
    }
}

void OnDeathCallback(void** params, void* context)
{
    TESObjectREFR* target = (TESObjectREFR*)params[1];
    if (target)
    {
        _MESSAGE("OnDeath: Target = %08X", target->formID);
    }
}

bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info)
{
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "MyEventPlugin";
    info->version = 1;

    return true;
}

bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    // Query Event Manager interface
    g_eventManager = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!g_eventManager || g_eventManager->version < FOSEEventManagerInterface::kVersion)
    {
        return false;
    }

    // Register event handlers
    g_eventManager->RegisterEventHandler("OnHit", OnHitCallback, nullptr, 100);
    g_eventManager->RegisterEventHandler("OnDeath", OnDeathCallback, nullptr, 100);

    _MESSAGE("MyEventPlugin loaded successfully");
    return true;
}
```

## Notes

- **Context Pointer**: The context pointer is passed to your callback and can be used to identify which handler instance is being called. This is useful if you register multiple handlers for the same event.
- **Priority**: Handlers with higher priority are called first. Default priority is 0.
- **Thread Safety**: Event handlers are called from the main game thread. Do not perform long-running operations in handlers.
- **Deduplication**: The Event Manager automatically deduplicates events to prevent duplicate dispatch within the same frame.
- **Performance**: A fast-path check skips event dispatch when no handlers are registered for a given event type, minimizing overhead.

## Version History

- **Version 1**: Initial release with RegisterEventHandler and RemoveEventHandler
