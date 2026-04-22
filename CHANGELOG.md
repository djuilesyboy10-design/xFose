# FOSE Changelog

## [Script Event Handler Implementation] - 2026-04-22

### Summary
Re-implemented script event handler functionality to allow plugins to register script-based event handlers. The implementation uses a safe, incremental approach that preserves ABI compatibility and has been thoroughly tested without crashes.

### Changes

#### EventManager.h
- Added `ScriptEventHandlerInfo` struct to store script handler details:
  - `m_script`: Script function to call
  - `m_target`: Target object for script execution
  - `m_priority`: Handler priority (higher = first)
  - `m_handlerName`: Name for debugging
- Added function declarations:
  - `RegisterScriptEventHandler(const char* eventName, Script* script, TESObjectREFR* target, UInt32 priority, const char* handlerName)`
  - `RemoveScriptEventHandler(const char* eventName, Script* script, TESObjectREFR* target)`

#### EventManager.cpp
- Added static storage: `s_scriptEventHandlers` (std::unordered_map<std::string, std::list<ScriptEventHandlerInfo>>)
- Implemented `RegisterScriptEventHandler`:
  - Validates event exists
  - Creates handler with script, target, priority, and name
  - Checks for duplicate handlers
  - Inserts in priority order (higher priority first)
  - Updates s_eventsInUse bitmask for ScriptEventList events
- Implemented `RemoveScriptEventHandler`:
  - Validates event has script handlers
  - Finds and removes handler by script and target
- Modified `DispatchEvent` to call script handlers:
  - Iterates through script handlers for the event
  - Calls UDFCaller::CallUDF for each script handler
  - Logs errors if script call fails
  - Note: Parameter conversion is simplified (passes no parameters for now) - TODO for future enhancement

#### PluginAPI.h
- Added to `FOSEEventManagerInterface`:
  - `RegisterScriptEventHandler` function pointer
  - `RemoveScriptEventHandler` function pointer

#### PluginManager.cpp
- Added wrapper functions:
  - `PluginAPI_RegisterScriptEventHandler`
  - `PluginAPI_RemoveScriptEventHandler`
- Updated `g_FOSEEventManagerInterface` to include new script handler functions

#### TestEventPlugin (main.cpp)
- Added Phase 4.0 test in `OnLoadGameHandler`:
  - Uses FindFirstScript to get a valid script from game data
  - Registers script handler for OnHit event
  - Defers registration to OnLoadGame to ensure game data is available
- Removed old disabled Phase 3.0 test code (F1 key test)
- Simplified `OnKeyPressHandler` (removed F1 test code)

### Testing

#### Build Tests
- Debug configuration: ✅ PASSED
- Release configuration: ✅ PASSED (DLL compiled successfully)
- Note: Post-build copy commands fail in both configs (pre-existing issue, unrelated to changes)

#### In-Game Tests
- Game loads without crash
- Existing event handlers continue to work (OnHit, OnDeath, OnLoad, OnEquip, OnKeyDown/Up/Press)
- Script handler registration successful:
  - `RegisterScriptEventHandler: 'OnHit' -> s_eventsInUse = 0x00001092`
- Script handler dispatch successful:
  - `UDFCaller::CallUDF: Script refID=66001F96 type=1 dataLen=3883 numArgs=0`
  - `UDFCaller::CallUDF: ExecuteLine returned 1, opcodeOffset=1008`
- No crashes or stability issues

### Technical Notes

#### ABI Compatibility
- Script handler storage uses separate static map (`s_scriptEventHandlers`) to avoid ABI changes
- PluginAPI interface version remains at kVersion = 1
- New functions added to end of interface struct (backward compatible)

#### Known Limitations
- Script parameter conversion is simplified (currently passes no parameters)
- TODO: Implement proper parameter conversion based on event param types
- This is safe for basic script handler functionality but limits advanced use cases

#### Design Decisions
- Separate storage for script handlers to avoid ABI changes to existing event handler structures
- Priority ordering for script handlers (higher priority called first)
- Script handler registration deferred to OnLoadGame to ensure game data availability
- UDFCaller used for script execution (same mechanism as existing ScriptInterface)

### Future Enhancements
- Implement proper parameter conversion in DispatchEvent for script handlers
- Add script handler query functions (GetScriptHandlers, IsScriptHandlerRegistered, etc.)
- Add script handler name-based removal
- Consider adding script handler statistics/counters

### Files Modified
- `fose/fose/EventManager.h`
- `fose/fose/EventManager.cpp`
- `fose/fose/PluginAPI.h`
- `fose/fose/PluginManager.cpp`
- `TestEventPlugin/main.cpp`

### Files Added
- `CHANGELOG.md` (this file)

### Build Status
- Debug: ✅ Compiles successfully
- Release: ✅ Compiles successfully
- TestEventPlugin: ✅ Compiles successfully (both configs)

### Backup Information
- Both Debug and Release builds tested and working
- Backup includes complete solution with both configuration outputs
- Baseline stable with no crashes
