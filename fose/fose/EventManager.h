#pragma once

#include "EventParams.h"
#include "GameForms.h"
#include "GameTypes.h"
#include <functional>
#include <list>
#include <unordered_map>
#include <string>

namespace EventManager
{
	// Event handler function signature for native (C++) handlers
	// params: array of parameter pointers (void*)
	// context: optional context pointer (can be null)
	typedef void (*EventHandlerCallback)(void** params, void* context);

	// Event flags - control event behavior
	enum EventFlags
	{
		kEventFlag_None = 0,
		kEventFlag_FlushOnLoad = 1 << 0,		// Clear handlers on game load
		kEventFlag_IsUserDefined = 1 << 1,		// User-defined event
		kEventFlag_AllowScriptDispatch = 1 << 2,	// Can be dispatched from scripts
		kEventFlag_HasUnknownArgTypes = 1 << 3,	// Dynamic parameter types
	};

	// Event ID enumeration - ScriptEventList events must come first in mask order
	enum EventID
	{
		// ScriptEventList events (correspond to event mask bits)
		kEventID_OnAdd = 0,
		kEventID_OnEquip,
		kEventID_OnDrop,
		kEventID_OnUnequip,
		kEventID_OnDeath,
		kEventID_OnMurder,
		kEventID_OnCombatEnd,
		kEventID_OnHit,
		kEventID_OnHitWith,
		kEventID_OnPackageStart,
		kEventID_OnPackageDone,
		kEventID_OnPackageChange,
		kEventID_OnLoad,
		kEventID_OnMagicEffectHit,
		kEventID_OnSell,
		kEventID_OnStartCombat,
		kEventID_OnOpen,
		kEventID_OnClose,
		kEventID_SayToDone,
		kEventID_OnGrab,
		kEventID_OnRelease,
		kEventID_OnDestructionStageChange,
		kEventID_OnFire,
		kEventID_OnTrigger,
		kEventID_OnTriggerEnter,
		kEventID_OnTriggerLeave,
		kEventID_OnReset,
		
		kEventID_ScriptEventListMAX,
		
		// Special-cased game events (not in ScriptEventList)
		kEventID_OnActivate = kEventID_ScriptEventListMAX,
		kEventID_OnFrame,
		
		kEventID_GameEventMAX,
		
		// Input events (using PlayerControls system)
		kEventID_OnKeyDown = kEventID_GameEventMAX,
		kEventID_OnKeyUp,
		kEventID_OnKeyPress,
		
		kEventID_InputEventMAX,
		
		// FOSE messaging events
		kEventID_LoadGame = kEventID_InputEventMAX,
		kEventID_SaveGame,
		kEventID_ExitGame,
		kEventID_ExitToMainMenu,
		kEventID_PostLoadGame,
		kEventID_DeleteGame,
		kEventID_RenameGame,
		kEventID_NewGame,
		kEventID_PreLoadGame,
		
		// User-defined events
		kEventID_UserDefined,
		
		kEventID_MAX,
		kEventID_Invalid = 0xFFFFFFFF
	};

	// Event information structure
	struct EventInfo
	{
		std::string			evName;			// Event name
		std::string			alias;			// Event alias (optional, for backward compatibility)
		EventID				evID;			// Unique event ID
		ParamType*			paramTypes;		// Array of parameter types
		UInt8				numParams;		// Number of parameters
		UInt32				flags;			// Event flags (reserved for future use)
		
		EventInfo() : evID(kEventID_Invalid), paramTypes(nullptr), numParams(0), flags(0) {}
	};

	// Native event handler information
	struct NativeEventHandlerInfo
	{
		EventHandlerCallback	m_func;			// Handler function
		void*					m_context;		// Optional context pointer
		UInt32					m_priority;		// Handler priority (higher = first)
		std::string				m_pluginName;	// Plugin that registered this handler
		std::string				m_handlerName;	// Name of this handler (for debugging)
		
		NativeEventHandlerInfo() : m_func(nullptr), m_context(nullptr), m_priority(0) {}
		NativeEventHandlerInfo(EventHandlerCallback func, void* context = nullptr, UInt32 priority = 0)
			: m_func(func), m_context(context), m_priority(priority) {}
		
		bool operator==(const NativeEventHandlerInfo& rhs) const
		{
			return m_func == rhs.m_func && m_context == rhs.m_context;
		}
		
		bool operator!=(const NativeEventHandlerInfo& rhs) const
		{
			return !(*this == rhs);
		}
		
		operator bool() const { return m_func != nullptr; }
	};

	// Event callback structure
	struct EventCallback
	{
		NativeEventHandlerInfo	nativeHandler;	// Native handler (for now, only this is used)
		UInt32					priority;		// Handler priority
		bool					removed;		// Flag for removal during iteration
		
		EventCallback() : priority(0), removed(false) {}
		EventCallback(const NativeEventHandlerInfo& handler, UInt32 prio = 0)
			: nativeHandler(handler), priority(prio), removed(false) {}
		
		bool operator==(const EventCallback& rhs) const
		{
			return nativeHandler == rhs.nativeHandler;
		}
	};

	// Core event manager functions
	
	// Initialize the event manager
	void Initialize();
	
	// Shutdown the event manager
	void Shutdown();
	
	// Clear handlers for events marked with FlushOnLoad flag
	void ClearFlushOnLoadEventHandlers();
	
	// Register a new event type
	// Returns true if registration succeeded
	bool RegisterEvent(const char* eventName, EventID evID, ParamType* paramTypes, UInt8 numParams, const char* alias = nullptr, EventFlags flags = kEventFlag_None);
	
	// Register a native event handler
	// Returns true if handler was registered successfully
	bool RegisterEventHandler(const char* eventName, EventHandlerCallback callback, void* context = nullptr, UInt32 priority = 0, const char* pluginName = nullptr, const char* handlerName = nullptr);
	
	// Remove a native event handler
	// Returns true if handler was found and removed
	bool RemoveEventHandler(const char* eventName, EventHandlerCallback callback, void* context = nullptr);
	
	// Dispatch an event to all registered handlers
	void DispatchEvent(const char* eventName, void** params);
	
	// Dispatch an event by ID
	void DispatchEventByID(EventID evID, void** params);
	
	// Get event info by name
	const EventInfo* GetEventInfo(const char* eventName);
	
	// Get event info by ID
	const EventInfo* GetEventInfoByID(EventID evID);
	
	// Check if an event is registered
	bool IsEventRegistered(const char* eventName);
	
	// Get event alias (returns empty string if no alias)
	const char* GetEventAlias(const char* eventName);
	
	// Get current event name (for debugging nested event handling)
	const char* GetCurrentEventName();
	
	// Install hooks for game events (called during initialization)
	void InstallGameHooks();
	
	// Per-frame tick to reset dedup tracking
	void Tick();
	
	// Map a ScriptEventList event mask to an EventID
	EventID EventIDForMask(UInt32 eventMask);
	
	// Map an EventID to a ScriptEventList event mask (0 if not a ScriptEventList event)
	UInt32 MaskForEventID(EventID evID);
	
	// Handle a game event from the MarkEvent hook
	void __stdcall HandleGameEvent(UInt32 eventMask, void* target);
	
	// Bitmask of events that have at least one handler registered
	extern UInt32 s_eventsInUse;
}
