# Event Manager Upgrade Roadmap

This document outlines the planned upgrades to the FOSE Event Manager to bring it closer to xNVSE's level of functionality.

## Current State

The FOSE Event Manager currently provides:
- Basic event registration and dispatch
- Native event handlers only (C++ callbacks)
- Simple parameter types
- Basic priority system (numeric priority)
- 30+ ScriptEventList events
- Special-cased events (OnActivate, OnFrame)
- Input events (OnKeyDown, OnKeyUp, OnKeyPress)

## xNVSE Advanced Features

xNVSE's Event Manager includes:
- Advanced filtering system (array-based, nested filters)
- Event flags system (FlushOnLoad, UserDefined, etc.)
- Lambda support for anonymous scripts
- Advanced dispatch with callbacks
- Thread-safe dispatch
- Event aliases
- Better debug info (plugin/handler names)
- Event stack tracking
- More internal events
- Script event handlers
- Priority system with special priorities
- Handler conflict detection

## Upgrade Plan

### Phase 1 - Easy Wins (No script system required)

These features can be implemented without requiring the script system and provide immediate value.

#### 1.1 Event Aliases
- Add alias field to EventInfo structure
- Allow events to have aliases for backward compatibility
- Update event registration to support aliases
- Update event lookup to search by alias

**Status:** Completed

#### 1.2 Event Flags System
- Add EventFlags enum (FlushOnLoad, UserDefined, AllowScriptDispatch, etc.)
- Add flags field to EventInfo structure
- Implement flag-based behavior (e.g., FlushOnLoad clears handlers on game load)
- Update event registration to accept flags

**Status:** Completed

#### 1.3 More Internal Events
- Add LoadGame event
- Add SaveGame event
- Add ExitGame event
- Add ExitToMainMenu event
- Add PostLoadGame event
- Add DeleteGame event
- Add RenameGame event
- Add NewGame event
- Wire up hooks for these events

**Status:** Completed

#### 1.4 Better Debug Info
- Add plugin name field to NativeEventHandlerInfo
- Add handler name field to NativeEventHandlerInfo
- Update RegisterEventHandler to accept plugin/handler names
- Update logging to include plugin/handler names
- Add GetNativeHandlerInfo function

**Status:** Completed

#### 1.5 Event Stack Tracking
- Add s_eventStack to track event nesting
- Push/pop events during dispatch
- Add GetCurrentEventName function
- Useful for debugging nested event handling

**Status:** Completed

### Phase 2 - Medium Wins (Moderate complexity)

These features require moderate complexity but don't require the full script system.

#### 2.1 Advanced Filtering System
- Add array-based filter support
- Allow filters to be arrays of values
- Add nested filter support (arrays of arrays)
- Implement filter matching by parameter index
- Add filter validation

**Status:** Completed

#### 2.2 Enhanced Priority System
- Add special priorities (Highest, Default, Lowest)
- Implement handler conflict detection
- Add IsEventHandlerFirst/Last functions
- Add GetHigher/LowerPriorityEventHandlers functions

**Status:** Completed

#### 2.3 GetEventHandlers with Filtering
- Add GetEventHandlers function
- Support filtering by scripts, plugins, handlers
- Return array of registered handlers
- Useful for debugging and inspection

**Status:** Completed

#### 2.4 Deferred Remove
- Add deferred removal list
- Safe removal during iteration
- Prevents crashes when removing handlers during dispatch

**Status:** Completed

### Phase 3 - Hard Wins (Requires script system)

These features require the script system (Script calling/evaluation, Lambda functions).

#### 3.1 Script Event Handlers
- Allow scripts as event callbacks
- Integrate with script calling system
- Support UDF (User-Defined Function) callbacks
- Add SetEventHandler for scripts

**Status:** Pending (requires Script calling/evaluation)

#### 3.2 Lambda Support
- Integrate with LambdaManager
- Allow LambdaManager::Maybe_Lambda as callback
- Implement lambda context saving
- Add lambda-specific event handling

**Status:** Pending (requires Lambda functions)

#### 3.3 Advanced Dispatch
- Add DispatchEventRaw with type checking
- Add DispatchEventAlt with callbacks
- Support result callbacks
- Add post-dispatch callbacks

**Status:** Pending (requires script system)

#### 3.4 Thread-Safe Dispatch
- Add thread-safe dispatch functions
- Implement deferred callback queue
- Add DispatchEventThreadSafe
- Add DispatchEventAltThreadSafe

**Status:** Pending (requires multithreading support)

## Implementation Order

1. **Phase 1.1** - Event Aliases (simplest, high value)
2. **Phase 1.2** - Event Flags (enables other features)
3. **Phase 1.4** - Better Debug Info (improves developer experience)
4. **Phase 1.5** - Event Stack (debugging aid)
5. **Phase 1.3** - More Internal Events (requires hooks, moderate effort)

After Phase 1 is complete, we can proceed to Phase 2 or tackle other systems (Math commands, Console commands) to build momentum.

## Dependencies

- Phase 3 depends on: Script calling/evaluation, Lambda functions
- Phase 2.1 (Advanced Filtering) benefits from: Array system (already complete)
- Phase 1.3 (More Internal Events) requires: Hook installation for save/load events

## Notes

- All changes should maintain backward compatibility with existing plugins
- Each feature should be tested independently
- Documentation should be updated after each phase
- Consider adding unit tests for complex features
