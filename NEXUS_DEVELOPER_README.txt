xFOSE Developer Guide v1.4
===========================

This guide covers the xFOSE APIs available to plugin developers, including the Event 
Manager, DataInterface, ScriptInterface, and all new features added in xFOSE.

TABLE OF CONTENTS
================
1. Event Manager API
2. ScriptInterface API (NEW in v1.4)
3. DataInterface System
4. String Variable Commands
5. Array Commands
6. Faction Commands
7. Actor State Commands
8. Building Plugins

==============================================================================
1. EVENT MANAGER API
==============================================================================

OVERVIEW
--------
The xFOSE Event Manager allows plugin developers to register callbacks for game 
events such as OnHit, OnDeath, OnLoad, OnEquip, and more. This API was 
non-functional in the original FOSE but is now fully operational.

GETTING STARTED
--------------
To use the Event Manager in your xFOSE plugin:

1. Include the FOSE Plugin API headers
2. Query the Event Manager interface during plugin load
3. Register event handlers for the events you're interested in
4. Handle the events in your callback functions

QUERYING THE EVENT MANAGER
---------------------------
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

REGISTERING EVENT HANDLERS
----------------------------
bool RegisterEventHandler(
    const char* eventName,          // Event name (e.g., "OnHit", "OnDeath")
    EventHandlerCallback callback,   // Your callback function
    void* context,                   // Optional context pointer passed to callback
    UInt32 priority,                // Handler priority (higher = called first)
    const char* pluginName,         // Optional plugin name for debug logging
    const char* handlerName         // Optional handler name for debug logging
);

CALLBACK FUNCTION SIGNATURE
----------------------------
void MyEventHandler(void** params, void* context);

- params: Array of parameters for the event (event-dependent)
- context: The context pointer you passed when registering

AVAILABLE EVENTS
----------------

ScriptEventList Events
----------------------
These events fire on specific game objects when scripted conditions are met:

Event Name        | Description                    | Parameters
------------------|--------------------------------|------------------
OnAdd             | Object added to container       | params[0]: source, params[1]: target
OnEquip           | Item equipped                  | params[0]: source, params[1]: target
OnDrop            | Item dropped                   | params[0]: source, params[1]: target
OnUnequip         | Item unequipped                | params[0]: source, params[1]: target
OnDeath           | Actor died                     | params[0]: source, params[1]: target
OnHit             | Actor hit                      | params[0]: source, params[1]: target
OnLoad            | Object loaded                  | params[0]: source, params[1]: target
OnGrab            | Object grabbed                 | params[0]: source, params[1]: target
OnRelease         | Object released                | params[0]: source, params[1]: target
OnDestructionStageChange | Destruction stage changed | params[0]: source, params[1]: target
OnFire            | Weapon fired                   | params[0]: source, params[1]: target
OnTrigger         | Trigger activated              | params[0]: source, params[1]: target
OnTriggerEnter    | Object entered trigger         | params[0]: source, params[1]: target
OnTriggerLeave    | Object left trigger            | params[0]: source, params[1]: target
OnReset           | Object reset                   | params[0]: source, params[1]: target

Special Events
--------------
Event Name        | Description
------------------|------------------
OnActivate        | Object activated (special parameter layout)
OnFrame           | Fires every frame (no parameters)

Input Events
------------
Event Name        | Description                    | Parameters
------------------|--------------------------------|------------------
OnKeyDown         | Key pressed down                | params[0]: keycode (UInt32)
OnKeyUp           | Key released                   | params[0]: keycode (UInt32)
OnKeyPress        | Key pressed and released        | params[0]: keycode (UInt32)

Internal Events
---------------
Event Name        | Description                    | Parameters
------------------|--------------------------------|------------------
OnLoadGame        | Game loaded from save file     | None
OnSaveGame        | Game saved                     | None
OnExitGame        | Game is exiting to desktop     | None
OnExitToMainMenu  | Game is exiting to main menu   | None
OnPostLoadGame    | After game load completes      | None
OnNewGame         | New game started               | None
OnDeleteGame      | Save file deleted              | None
OnRenameGame      | Save file renamed              | None
OnPreLoadGame     | Before game load starts        | None

PHASE 1 FEATURES
----------------

Event Aliases
-------------
Events can have aliases for backward compatibility or alternative naming:
- Both the event name and alias can be used to register handlers
- Example: "OnActorDeath" is an alias for "OnDeath"
- Example: "OnEquipped" is an alias for "OnEquip"

Event Flags System
------------------
Events can have flags that control their behavior:

Flag                              | Description
----------------------------------|----------------------------------
kEventFlag_FlushOnLoad             | Handlers are cleared when the game loads
kEventFlag_IsUserDefined          | Event was defined by a plugin
kEventFlag_AllowScriptDispatch    | Event can be dispatched from scripts
kEventFlag_HasUnknownArgTypes     | Event has dynamic parameter types

Use ClearFlushOnLoadEventHandlers() to clear all handlers for events marked with FlushOnLoad.

Better Debug Info
-----------------
Event handlers support optional plugin and handler names for better debugging:
- Pass pluginName and handlerName when registering handlers
- These names are logged when events are dispatched
- Helps identify which plugin/handler is being called during debugging

Example:
g_eventManager->RegisterEventHandler("OnLoadGame", MyHandler, nullptr, 0, "MyPlugin", "MyLoadHandler");

Event Stack Tracking
--------------------
The Event Manager tracks event nesting for debugging:
- GetCurrentEventName() returns the name of the currently dispatching event
- Useful for debugging nested event handling
- Empty string if no event is currently being dispatched

PHASE 2 FEATURES
----------------

Advanced Filtering System
-------------------------
Event handlers can filter based on parameter values:
- Use RegisterEventHandlerWithFilter() to register a handler with a filter
- Filters can be single values or arrays of values
- Filters are checked before the handler is called
- Useful for only handling events with specific targets or sources

Enhanced Priority System
-------------------------
Special priority values are available for common use cases:
- kPriority_Highest (1000000) - Handlers called first
- kPriority_Default (0) - Default priority
- kPriority_Lowest (-1000000) - Handlers called last

Example:
g_eventManager->RegisterEventHandler("OnHit", MyHandler, nullptr, EventManager::kPriority_Highest, "MyPlugin", "MyHandler");

Handler Inspection Functions
----------------------------
Functions to inspect handler ordering and priority:
- IsEventHandlerFirst(eventName, callback, context) - Check if handler is first in list
- IsEventHandlerLast(eventName, callback, context) - Check if handler is last in list
- GetEventHandlers(eventName, outHandlers, maxHandlers) - Get all handlers for an event

Deferred Removal
----------------
Handlers can be safely removed during event dispatch:
- Removal is deferred until after dispatch completes
- Prevents iterator invalidation and crashes
- Marked handlers are skipped during current dispatch

==============================================================================
2. SCRIPTINTERFACE API (NEW in v1.4)
==============================================================================

OVERVIEW
--------
The ScriptInterface API allows plugins to call game scripts directly from code. 
This enables dynamic script execution with parameter passing and return values.

GETTING STARTED
--------------
To use the ScriptInterface:

1. Query the ScriptInterface during plugin load
2. Use UDFCaller::CallUDF() to execute scripts
3. Pass parameters and receive return values

QUERYING THE SCRIPTINTERFACE
----------------------------
#include "fose/PluginAPI.h"
#include "fose/ScriptRunner.h"

static ScriptInterface g_scriptInterface;

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    g_scriptInterface = (ScriptInterface)fose->QueryInterface(kInterface_Script);
    if (!g_scriptInterface)
    {
        // ScriptInterface may not be available in all FOSE versions
        return true; // Continue loading without ScriptInterface
    }
    return true;
}

CALLING SCRIPTS
---------------
bool UDFCaller::CallUDF(
    Script* funcScript,           // Script to execute
    TESObjectREFR* callingObj,   // Calling object (can be NULL)
    TESObjectREFR* container,     // Container object (can be NULL)
    double* result,               // Pointer to store return value
    UInt8 numArgs,               // Number of parameters
    ...                          // Variable arguments (double values)
);

EXAMPLE
-------
// Find a script and call it
Script* script = FindFirstScript();  // Exported function to find a script
if (script)
{
    double result = 0;
    bool success = UDFCaller::CallUDF(script, nullptr, nullptr, &result, 2, 42.0, 3.14);
    if (success)
    {
        // Script executed successfully
        // result contains the return value
    }
}

NOTES
-----
- The ScriptInterface automatically parses script bytecode to extract block offsets
- Supports quest scripts and object scripts
- Zeroed ScriptRunner is used internally (no complex initialization needed)
- Parameters are passed as double values
- Variable values are set on the script's variables in declaration order

==============================================================================
3. DATAINTERFACE SYSTEM
==============================================================================

OVERVIEW
--------
The DataInterface provides a unified way for plugins to access FOSE singletons 
and utility functions.

QUERYING THE DATAINTERFACE
---------------------------
#include "fose/PluginAPI.h"

static void* g_dataInterface = nullptr;

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    g_dataInterface = fose->QueryInterface(kInterface_Data);
    if (!g_dataInterface)
        return false;
    
    // Access DataInterface functions
    return true;
}

DATAINTERFACE STRUCTURE
------------------------
The DataInterface is a struct with the following layout:
- Offset 0: UInt32 version
- Offset 4: GetSingleton function pointer
- Offset 8: GetFunc function pointer

GETSINGLETON
------------
void* GetSingleton(UInt32 dataID);

Available data IDs:
- kFOSEData_ArrayMap (1) - ArrayVarManager singleton
- kFOSEData_StringMap (2) - StringVarManager singleton
- kFOSEData_LoggingManager (35) - LoggingManager singleton
- kFOSEData_PlayerControlsManager (40) - PlayerControlsManager singleton

GETFUNC
-------
void* GetFunc(UInt32 funcID);

Available function IDs include:
FormExtraData Functions:
- kFOSEData_FormExtraDataGet (1)
- kFOSEData_FormExtraDataAdd (3)
- kFOSEData_FormExtraDataRemoveByName (4)
- kFOSEData_FormExtraDataRemoveByPtr (5)
- kFOSEData_FormExtraDataCount (6)
- kFOSEData_FormExtraDataExists (7)
- kFOSEData_FormExtraDataIterate (8)

ArrayVar Functions:
- kFOSEData_ArrayVarMapDeleteBySelf (9)
- kFOSEData_StringVarMapDeleteBySelf (10)
- kFOSEData_ArrayVarInsertAt (11)
- kFOSEData_ArrayVarFindByValue (12)
- kFOSEData_ArrayVarCountByType (13)
- kFOSEData_ArrayVarSort (18)
- kFOSEData_ArrayVarReverse (19)
- kFOSEData_ArrayVarShuffle (20)

StringVar Functions:
- kFOSEData_StringVarGetLength (14)
- kFOSEData_StringVarCompare (15)
- kFOSEData_StringVarConcatenate (16)
- kFOSEData_StringVarSubstring (17)
- kFOSEData_StringVarUppercase (21)
- kFOSEData_StringVarLowercase (22)
- kFOSEData_StringVarTrim (23)
- kFOSEData_StringVarReplace (24)

PlayerControls Functions:
- kFOSEData_PlayerControlsIsKeyPressed (36)
- kFOSEData_PlayerControlsTapKey (37)
- kFOSEData_PlayerControlsHoldKey (38)
- kFOSEData_PlayerControlsReleaseKey (39)
- kFOSEData_PlayerControlsIsControlPressed (45)

Logging Functions:
- kFOSEData_LoggingLog (41)
- kFOSEData_LoggingLogInfo (42)
- kFOSEData_LoggingLogWarning (43)
- kFOSEData_LoggingLogError (44)

==============================================================================
4. FACTION COMMANDS
==============================================================================

FOSE provides commands for managing actor factions and faction relationships.

FACTION RANK MANAGEMENT
-----------------------
GetFactionRank(faction, reference)  - Returns the rank of a reference in a faction
SetFactionRank(faction, rank, reference) - Sets the rank of a reference in a faction
AddFaction(faction, rank, reference) - Adds a reference to a faction with a specified rank
RemoveFaction(faction, reference) - Removes a reference from a faction

Example:
AddFaction(RaiderFactionRef, 5, PlayerRef)
SetFactionRank(EnclaveFactionRef, 0, PlayerRef)
int rank = GetFactionRank(RaiderFactionRef, PlayerRef)

FACTION REACTION MANAGEMENT
----------------------------
GetFactionReaction(factionA, factionB) - Returns the reaction modifier between two factions
SetFactionReaction(factionA, factionB, modifier) - Sets the reaction modifier between two factions

Example:
SetFactionReaction(BrotherhoodFactionRef, EnclaveFactionRef, -100)
int reaction = GetFactionReaction(BrotherhoodFactionRef, EnclaveFactionRef)

==============================================================================
5. ACTOR STATE COMMANDS
==============================================================================

FOSE provides commands for checking actor states:

IsUnconscious(reference) - Returns 1 if the actor is unconscious, 0 otherwise
IsAlive(reference) - Returns 1 if the actor is alive, 0 if dead
GetGameDaysPassed() - Returns the number of game days passed

Example:
if IsUnconscious(ActorRef)
    Print "Actor is unconscious"

if IsAlive(ActorRef)
    Print "Actor is alive"
else
    Print "Actor is dead"

==============================================================================
6. STRING VARIABLE COMMANDS
==============================================================================

sv_create name [value]     - Create a string variable
sv_set name value          - Set a string variable
sv_get name                - Get a string variable
sv_concat name1 name2      - Concatenate two strings
sv_compare name1 name2     - Compare two strings (0 if equal)
sv_length name             - Get string length
sv_uppercase name          - Convert to uppercase
sv_lowercase name          - Convert to lowercase
sv_trim name               - Trim whitespace
sv_replace name find rep   - Replace substring

==============================================================================
7. ARRAY COMMANDS
==============================================================================

ar_create name              - Create an array
ar_set name index value     - Set array element
ar_get name index           - Get array element
ar_push name value          - Push value to end
ar_pop name                 - Pop value from end
ar_remove name index        - Remove element at index
ar_clear name               - Clear array
ar_size name                - Get array size
ar_sort name                - Sort array
ar_reverse name             - Reverse array
ar_shuffle name             - Shuffle array
ar_insertAt name idx val    - Insert at index
ar_findByValue name val     - Find index of value
ar_countByType name type    - Count elements by type

==============================================================================
8. BUILDING PLUGINS
==============================================================================

Build your plugin as a 32-bit DLL for Windows. Link against the FOSE headers and 
ensure your project targets:
- Platform: Win32 (x86)
- Configuration: Release or Debug

Place your compiled .dll file in Data\FOSE\Plugins\ in your Fallout 3 installation.

EXAMPLE PLUGIN
--------------
#include "ITypes.h"
#include "fose/PluginAPI.h"
#include <stdio.h>

static FOSEEventManagerInterface* g_eventManager = nullptr;

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

void OnHitHandler(void** params, void* context)
{
    UInt32 source = params ? (UInt32)params[0] : 0;
    UInt32 target = params ? (UInt32)params[1] : 0;
    Log("OnHit: source=0x%08X target=0x%08X", source, target);
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
    
    g_eventManager->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0, "MyPlugin", "OnHitHandler");
    
    return true;
}

NOTES
-----
- Event parameters are passed as pointers. Cast them to the appropriate types based on the event.
- The source parameter may be null for some events depending on how they're triggered.
- Events can fire frequently (e.g., OnHit during combat), so keep your handlers efficient.
- Multiple handlers can be registered for the same event by different plugins.
- Use the context parameter to pass instance data to your callback if needed.

SUPPORT
=======
For issues, questions, or contributions, visit the xFOSE project page.

========================
Version 1.4 - April 2026
========================
