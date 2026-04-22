xFOSE - Fallout 3 Script Extender v1.4
=====================================

xFOSE is an updated, modernized continuation of the Fallout 3 Script Extender (FOSE) - 
the first major update in 15 years. It restores the broken Event Manager, modernizes 
the codebase with C++14 features, and provides new tools for modders, all while 
maintaining 100% backward compatibility with existing FOSE plugins.

WHAT'S NEW IN v1.4
==================

ScriptInterface API (NEW)
-------------------------
- New interface for calling game scripts from plugins via UDFCaller::CallUDF()
- Enables dynamic script execution with parameter passing and return values
- FindFirstScript export for locating suitable scripts in game data
- Supports quest scripts and object scripts with automatic type selection

ScriptRunner::ExecuteLine Fix
------------------------------
- Fixed calling convention and parameter passing for the game's internal script execution
- Plugins can now reliably call scripts via FOSE without crashes
- Proper bytecode offset parsing for correct script execution

FEATURES
========

Restored Event Manager
----------------------
- The Event Manager was broken in the original FOSE and never worked
- xFOSE fixes the initialization order bug and provides a fully working event system
- Plugins can register handlers for game events: OnHit, OnDeath, OnLoad, OnEquip, OnActivate, OnKeyDown, OnKeyUp, OnKeyPress, and more

Event Manager Enhancements
--------------------------
- Event aliases (OnActorDeath, OnEquipped, etc.)
- Event flags system for controlling handler behavior
- Internal events (LoadGame, SaveGame, ExitGame, NewGame, etc.)
- Enhanced priority system with special values (Highest, Default, Lowest)
- Handler inspection functions for debugging
- Deferred removal for safe handler removal during dispatch
- Advanced filtering system for parameter-based filtering

String Variables
----------------
- New script commands for string manipulation
- Create, set, get, concatenate, compare, and check validity
- Full save/load support
- Commands: sv_create, sv_set, sv_get, sv_concat, sv_compare, sv_length, sv_uppercase, sv_lowercase, sv_trim, sv_replace

Arrays
------
- New script commands for array operations
- Create, set, get, push, pop, remove, clear, and size
- Full save/load support
- Advanced operations: sort, reverse, shuffle, insertAt, findByValue, countByType
- Map and StringMap container types for key-value storage

Logging Interface
-----------------
- New LoggingManager singleton for file-based logging
- Log levels: Info, Warning, Error
- Accessible via DataInterface

PlayerControls Interface
-------------------------
- New PlayerControlsManager singleton for input/control state checking
- Check if specific keys are pressed
- Check if game controls are active (Forward, Back, etc.)
- Tap, hold, and release key functions

Input Events
------------
- OnKeyDown - Key pressed down
- OnKeyUp - Key released
- OnKeyPress - Key pressed and released

DataInterface System
--------------------
- Unified interface for accessing FOSE singletons and utility functions from plugins
- GetSingleton for ArrayMap, StringMap, LoggingManager, PlayerControlsManager
- GetFunc for all utility functions

Codebase Modernization
----------------------
- C++14 support throughout
- STL-style containers with size(), empty(), begin()/end()
- Modern string handling
- RAII hook management
- Plugin API helpers

BACKWARD COMPATIBILITY
======================
- 100% backward compatible with existing FOSE plugins
- Drop-in replacement for FOSE
- All existing plugins and mods continue to work without modification

INSTALLATION (USERS)
=====================
1. Copy fose_loader.exe and fose_1_7.dll to your Fallout 3 directory
2. Launch the game using fose_loader.exe
3. All existing FOSE plugins will continue to work as before

Note: Windows Defender or Chrome may flag the download as a false positive. This is 
normal for script extenders that use DLL injection. Add the files to your antivirus 
exclusions to proceed.

AVAILABLE EVENTS
================

ScriptEventList Events
----------------------
OnAdd      - Object added to container
OnEquip     - Item equipped
OnDrop      - Item dropped
OnUnequip   - Item unequipped
OnDeath     - Actor died
OnHit       - Actor hit
OnLoad      - Object loaded
OnGrab      - Object grabbed
OnRelease   - Object released
OnDestructionStageChange - Destruction stage changed
OnFire      - Weapon fired
OnTrigger   - Trigger activated
OnTriggerEnter - Object entered trigger
OnTriggerLeave - Object left trigger
OnReset     - Object reset

Special Events
--------------
OnActivate  - Object activated
OnFrame     - Fires every frame

Input Events
------------
OnKeyDown   - Key pressed down
OnKeyUp     - Key released
OnKeyPress  - Key pressed and released

Internal Events
---------------
OnLoadGame      - Game loaded from save file
OnSaveGame      - Game saved
OnExitGame      - Game is exiting to desktop
OnExitToMainMenu - Game is exiting to main menu
OnPostLoadGame  - After game load completes
OnNewGame       - New game started
OnDeleteGame    - Save file deleted
OnRenameGame    - Save file renamed
OnPreLoadGame   - Before game load starts

STRING VARIABLE COMMANDS
========================
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

ARRAY COMMANDS
==============
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

LICENSE
=======
This project is open source and free to use, modify, and distribute.

CREDITS
=======
- Original FOSE by the FOSE Team
- xFOSE - Event Manager restoration and modernization by djuilesyboy10
- Inspired by xNVSE Event Manager implementation

SUPPORT
=======
For questions and support, see the Nexus Mods page or open a GitHub Issue.

========================
Version 1.4 - April 2026
========================
