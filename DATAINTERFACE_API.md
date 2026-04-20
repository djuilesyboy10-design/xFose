# xFOSE DataInterface API - Developer Guide

## Overview

The DataInterface is a unified system for accessing FOSE singletons and utility functions from plugins. It provides a consistent interface for plugin developers to access core FOSE functionality without needing to know internal implementation details.

## Getting Started

To use the DataInterface in your xFOSE plugin:

1. Include the FOSE Plugin API headers
2. Query the DataInterface during plugin load
3. Retrieve singletons and functions using their IDs
4. Call the retrieved functions as needed

## API Reference

### Querying the DataInterface

```cpp
#include "fose/PluginAPI.h"

static FOSEDataInterface* g_dataInterface = nullptr;

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    g_dataInterface = (FOSEDataInterface*)fose->QueryInterface(kInterface_Data);
    if (!g_dataInterface)
        return false;
    
    // Use DataInterface...
    return true;
}
```

### Retrieving Singletons

```cpp
void* GetSingleton(UInt32 singletonID);
```

Returns a pointer to a singleton object.

**Available Singleton IDs:**

| ID | Name | Description |
|----|------|-------------|
| 1 | ArrayMap | Array variable manager for integer-keyed arrays |
| 2 | StringMap | String variable manager for string-keyed arrays |
| 3 | LoggingManager | File-based logging manager |
| 4 | PlayerControlsManager | Input/control state manager |

### Retrieving Functions

```cpp
void* GetFunc(UInt32 funcID);
```

Returns a pointer to a utility function.

**Available Function IDs:**

| ID Range | Category | Functions |
|----------|----------|-----------|
| 1-8 | FormExtraData | Get, Add, Remove, Count, Exists, Iterate |
| 9-10 | Map/StringMap | DeleteBySelf |
| 11-26 | Array/String Operations | InsertAt, FindByValue, CountByType, string ops |
| 27-31 | Map/StringMap Operations | CreateMap, CreateStringMap, GetContainerType, HasKey, SetElementByKey, GetElementByKey, RemoveByKey |
| 32-35 | Logging | Log, LogInfo, LogWarning, LogError |
| 36-48 | PlayerControls | IsKeyPressed, TapKey, HoldKey, ReleaseKey, DisableKey, EnableKey, IsKeyDisabled, GetControl, SetControl, IsControl, IsControlPressed, GetNumMouseButtonsPressed, GetMouseButtonPress |

## Using the LoggingManager

The LoggingManager provides file-based logging with multiple log levels.

### Retrieving the LoggingManager

```cpp
void* loggingManager = g_dataInterface->GetSingleton(FOSEDataInterface::kFOSEData_LoggingManager);
```

### Retrieving Logging Functions

```cpp
void* logInfo = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_LoggingLogInfo);
void* logWarning = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_LoggingLogWarning);
void* logError = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_LoggingLogError);
```

### Calling Logging Functions

```cpp
void (*LogInfo)(void*, const char*) = (void (*)(void*, const char*))logInfo;
LogInfo(loggingManager, "MyPlugin: Info message");

void (*LogWarning)(void*, const char*) = (void (*)(void*, const char*))logWarning;
LogWarning(loggingManager, "MyPlugin: Warning message");

void (*LogError)(void*, const char*) = (void (*)(void*, const char*))logError;
LogError(loggingManager, "MyPlugin: Error message");
```

**Log File Location:** `fose_plugin.log` in the Fallout 3 game directory

## Using the PlayerControlsManager

The PlayerControlsManager provides access to input and control state.

### Retrieving the PlayerControlsManager

```cpp
void* playerControls = g_dataInterface->GetSingleton(FOSEDataInterface::kFOSEData_PlayerControlsManager);
```

### Checking Key State

```cpp
void* isKeyPressed = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_PlayerControlsIsKeyPressed);
bool (*IsKeyPressed)(void*, UInt32, bool*) = (bool (*)(void*, UInt32, bool*))isKeyPressed;

bool pressed = false;
IsKeyPressed(playerControls, 0x57, &pressed); // Check if 'W' key is pressed
```

### Simulating Key Presses

```cpp
void* tapKey = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_PlayerControlsTapKey);
void (*TapKey)(void*, UInt32) = (void (*)(void*, UInt32))tapKey;

TapKey(playerControls, 0x57); // Tap 'W' key
```

### Checking Control State

```cpp
void* isControlPressed = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_PlayerControlsIsControlPressed);
bool (*IsControlPressed)(void*, UInt32, bool*) = (bool (*)(void*, UInt32, bool*))isControlPressed;

bool pressed = false;
IsControlPressed(playerControls, 0, &pressed); // Check if forward control is pressed
```

## Using ArrayMap and StringMap

The ArrayMap and StringMap managers provide associative container functionality.

### Retrieving the Managers

```cpp
void* arrayMap = g_dataInterface->GetSingleton(FOSEDataInterface::kFOSEData_ArrayMap);
void* stringMap = g_dataInterface->GetSingleton(FOSEDataInterface::kFOSEData_StringMap);
```

### Map Operations

```cpp
// Create a map
void* createMap = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_ArrayVarCreateMap);
UInt32 (*CreateMap)(void*) = (UInt32 (*)(void*))createMap;
UInt32 mapID = CreateMap(arrayMap);

// Set element by key
void* setElement = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_ArrayVarSetElementByKey);
bool (*SetElementByKey)(void*, UInt32, UInt32, double) = (bool (*)(void*, UInt32, UInt32, double))setElement;
SetElementByKey(arrayMap, mapID, 42, 3.14);

// Get element by key
void* getElement = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_ArrayVarGetElementByKey);
bool (*GetElementByKey)(void*, UInt32, UInt32, double*) = (bool (*)(void*, UInt32, UInt32, double*))getElement;
double value = 0;
GetElementByKey(arrayMap, mapID, 42, &value);
```

## Example Plugin

```cpp
#include "ITypes.h"
#include "fose/PluginAPI.h"
#include <stdio.h>

static FOSEDataInterface* g_dataInterface = nullptr;

extern "C" __declspec(dllexport) bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info)
{
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = "MyPlugin";
    info->version = 1;
    return true;
}

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    // Get DataInterface
    g_dataInterface = (FOSEDataInterface*)fose->QueryInterface(kInterface_Data);
    if (!g_dataInterface)
        return false;
    
    // Get LoggingManager
    void* loggingManager = g_dataInterface->GetSingleton(FOSEDataInterface::kFOSEData_LoggingManager);
    void* logInfo = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_LoggingLogInfo);
    
    // Log a message
    void (*LogInfo)(void*, const char*) = (void (*)(void*, const char*))logInfo;
    LogInfo(loggingManager, "MyPlugin loaded successfully");
    
    // Get PlayerControlsManager
    void* playerControls = g_dataInterface->GetSingleton(FOSEDataInterface::kFOSEData_PlayerControlsManager);
    void* isKeyPressed = g_dataInterface->GetFunc(FOSEDataInterface::kFOSEData_PlayerControlsIsKeyPressed);
    
    // Check key state
    bool (*IsKeyPressed)(void*, UInt32, bool*) = (bool (*)(void*, UInt32, bool*))isKeyPressed;
    bool pressed = false;
    IsKeyPressed(playerControls, 0x57, &pressed);
    
    return true;
}
```

## Notes

- All singleton and function pointers are valid for the lifetime of the plugin
- The DataInterface version is currently 1
- Check function pointers for NULL before use
- Key codes use DirectInput scancodes (e.g., 0x57 = 'W')
- Control indices correspond to game control bindings

## Support

For issues, questions, or contributions, visit the xFOSE project page.

---

**xFOSE** — Modern plugin interfaces for Fallout 3 modding.
