#include "EventManager.h"
#include "Utilities.h"
#include "GameAPI.h"
#include "GameData.h"
#include "GameObjects.h"
#include "Hooks_DirectInput8Create.h"
#include <windows.h>
#include <stack>

namespace EventManager
{
	// Static storage for event information
	static std::list<EventInfo> s_eventInfos;
	static std::unordered_map<std::string, EventInfo*> s_eventNameMap;
	static std::unordered_map<EventID, EventInfo*> s_eventIDMap;
	
	// Static storage for event handlers
	static std::unordered_map<std::string, std::list<EventCallback>> s_eventHandlers;
	
	// Initialization flag
	static bool s_initialized = false;
	
	// Dedup tracking for events that fire multiple times per frame
	static void* s_lastObj = nullptr;
	static void* s_lastTarget = nullptr;
	static UInt32 s_lastEvent = 0;
	static void* s_lastOnHitWithActor = nullptr;
	static void* s_lastOnHitWithWeapon = nullptr;
	static void* s_lastOnHitVictim = nullptr;
	static void* s_lastOnHitAttacker = nullptr;
	
	// Event stack for tracking event nesting (for debugging)
	static std::stack<const char*> s_eventStack;
	
	// Deferred removal list for safe removal during event dispatch
	static std::list<std::pair<std::string, EventCallback>> s_deferredRemovals;
	static bool s_isDispatching = false;
	
	// Bitmask of events that have at least one handler registered
	UInt32 s_eventsInUse = 0;
	
	// Lookup table: EventID -> ScriptEventList event mask
	static const UInt32 s_eventMasks[] = {
		ScriptEventList::kEvent_OnAdd,
		ScriptEventList::kEvent_OnEquip,
		ScriptEventList::kEvent_OnDrop,
		ScriptEventList::kEvent_OnUnequip,
		ScriptEventList::kEvent_OnDeath,
		ScriptEventList::kEvent_OnMurder,
		ScriptEventList::kEvent_OnCombatEnd,
		ScriptEventList::kEvent_OnHit,
		ScriptEventList::kEvent_OnHitWith,
		ScriptEventList::kEvent_OnPackageStart,
		ScriptEventList::kEvent_OnPackageDone,
		ScriptEventList::kEvent_OnPackageChange,
		ScriptEventList::kEvent_OnLoad,
		ScriptEventList::kEvent_OnMagicEffectHit,
		ScriptEventList::kEvent_OnSell,
		ScriptEventList::kEvent_OnStartCombat,
		ScriptEventList::kEvent_OnOpen,
		ScriptEventList::kEvent_OnClose,
		ScriptEventList::kEvent_SayToDone,
		ScriptEventList::kEvent_OnGrab,
		ScriptEventList::kEvent_OnRelease,
		ScriptEventList::kEvent_OnDestructionStageChange,
		ScriptEventList::kEvent_OnFire,
		ScriptEventList::kEvent_OnTrigger,
		ScriptEventList::kEvent_OnTriggerEnter,
		ScriptEventList::kEvent_OnTriggerLeave,
		ScriptEventList::kEvent_OnReset,
	};
	
	UInt32 MaskForEventID(EventID evID)
	{
		if (evID < kEventID_ScriptEventListMAX)
			return s_eventMasks[evID];
		return 0;
	}
	
	EventID EventIDForMask(UInt32 eventMask)
	{
		switch (eventMask) {
			case ScriptEventList::kEvent_OnAdd:				return kEventID_OnAdd;
			case ScriptEventList::kEvent_OnEquip:			return kEventID_OnEquip;
			case ScriptEventList::kEvent_OnDrop:				return kEventID_OnDrop;
			case ScriptEventList::kEvent_OnUnequip:			return kEventID_OnUnequip;
			case ScriptEventList::kEvent_OnDeath:			return kEventID_OnDeath;
			case ScriptEventList::kEvent_OnMurder:			return kEventID_OnMurder;
			case ScriptEventList::kEvent_OnCombatEnd:		return kEventID_OnCombatEnd;
			case ScriptEventList::kEvent_OnHit:				return kEventID_OnHit;
			case ScriptEventList::kEvent_OnHitWith:			return kEventID_OnHitWith;
			case ScriptEventList::kEvent_OnPackageStart:		return kEventID_OnPackageStart;
			case ScriptEventList::kEvent_OnPackageDone:		return kEventID_OnPackageDone;
			case ScriptEventList::kEvent_OnPackageChange:	return kEventID_OnPackageChange;
			case ScriptEventList::kEvent_OnLoad:				return kEventID_OnLoad;
			case ScriptEventList::kEvent_OnMagicEffectHit:	return kEventID_OnMagicEffectHit;
			case ScriptEventList::kEvent_OnSell:				return kEventID_OnSell;
			case ScriptEventList::kEvent_OnStartCombat:		return kEventID_OnStartCombat;
			case ScriptEventList::kEvent_OnOpen:				return kEventID_OnOpen;
			case ScriptEventList::kEvent_OnClose:			return kEventID_OnClose;
			case ScriptEventList::kEvent_SayToDone:			return kEventID_SayToDone;
			case ScriptEventList::kEvent_OnGrab:			return kEventID_OnGrab;
			case ScriptEventList::kEvent_OnRelease:			return kEventID_OnRelease;
			case ScriptEventList::kEvent_OnDestructionStageChange:	return kEventID_OnDestructionStageChange;
			case ScriptEventList::kEvent_OnFire:			return kEventID_OnFire;
			case ScriptEventList::kEvent_OnTrigger:			return kEventID_OnTrigger;
			case ScriptEventList::kEvent_OnTriggerEnter:	return kEventID_OnTriggerEnter;
			case ScriptEventList::kEvent_OnTriggerLeave:	return kEventID_OnTriggerLeave;
			case ScriptEventList::kEvent_OnReset:			return kEventID_OnReset;
			default:										return kEventID_Invalid;
		}
	}
	
	void __stdcall HandleGameEvent(UInt32 eventMask, void* target)
	{
		EventID evID = EventIDForMask(eventMask);
		if (evID == kEventID_Invalid)
			return;
		
		// Frame boundary detection: reset dedup tracking when tick changes
		static DWORD s_lastTickCount = 0;
		DWORD currentTick = GetTickCount();
		if (currentTick != s_lastTickCount) {
			s_lastTickCount = currentTick;
			s_lastObj = nullptr;
			s_lastTarget = nullptr;
			s_lastEvent = 0;
			s_lastOnHitWithActor = nullptr;
			s_lastOnHitWithWeapon = nullptr;
			s_lastOnHitVictim = nullptr;
			s_lastOnHitAttacker = nullptr;
		}
		
		// Dedup: prevent duplicate events being processed in immediate succession
		if (eventMask == s_lastEvent && target == s_lastTarget)
			return;
		
		// Special dedup for OnHitWith (game marks it twice per event)
		if (eventMask == ScriptEventList::kEvent_OnHitWith) {
			if (target == s_lastOnHitWithWeapon)
				return;
			s_lastOnHitWithWeapon = target;
		}
		
		// Special dedup for OnHit (marked several times per frame for spells/enchanted weapons)
		if (eventMask == ScriptEventList::kEvent_OnHit) {
			if (target == s_lastOnHitAttacker)
				return;
			s_lastOnHitAttacker = target;
		}
		
		s_lastEvent = eventMask;
		s_lastTarget = target;
		
		// Dispatch by event ID
		// Note: source TESObjectREFR* not available from thiscall hook (ecx = ScriptEventList*, no fixed offset to owner)
		void* params[2] = { nullptr, target };
		DispatchEventByID(evID, params);
	}
	
	void Tick()
	{
		// Reset dedup tracking each frame
		s_lastObj = nullptr;
		s_lastTarget = nullptr;
		s_lastEvent = 0;
		s_lastOnHitWithActor = nullptr;
		s_lastOnHitWithWeapon = nullptr;
		s_lastOnHitVictim = nullptr;
		s_lastOnHitAttacker = nullptr;
		
		// Dispatch OnFrame event to all registered handlers
		DispatchEventByID(kEventID_OnFrame, nullptr);
		
		// Check for input events using DirectInput
		static UInt32 s_keyState[256] = {0};
		static bool s_keyInitialized = false;
		
		if (!s_keyInitialized)
		{
			// Initialize key state tracking
			for (UInt32 i = 0; i < 256; i++)
				s_keyState[i] = 0;
			s_keyInitialized = true;
		}
		
		// Check keys 0-255 (keyboard)
		for (UInt32 keycode = 0; keycode < 256; keycode++)
		{
			bool pressed = DI_data.IsKeyPressed(keycode);
			
			UInt32 newState = pressed ? 1 : 0;
			UInt32 oldState = s_keyState[keycode];
			
			if (newState != oldState)
			{
				// Key state changed
				s_keyState[keycode] = newState;
				
				void* params[1] = { (void*)keycode };
				
				if (newState)
				{
					// Key pressed
					DispatchEventByID(kEventID_OnKeyDown, params);
				}
				else
				{
					// Key released
					DispatchEventByID(kEventID_OnKeyUp, params);
					
					// Key press complete (down + up)
					DispatchEventByID(kEventID_OnKeyPress, params);
				}
			}
		}
	}
	
	void Initialize()
	{
		if (s_initialized)
			return;
		
		s_initialized = true;
		
		// Register ScriptEventList events
		RegisterEvent("OnAdd", kEventID_OnAdd, kEventParams_GameEvent, 2);
		RegisterEvent("OnEquip", kEventID_OnEquip, kEventParams_GameEvent, 2);
		RegisterEvent("OnDrop", kEventID_OnDrop, kEventParams_GameEvent, 2);
		RegisterEvent("OnUnequip", kEventID_OnUnequip, kEventParams_GameEvent, 2);
		RegisterEvent("OnDeath", kEventID_OnDeath, kEventParams_GameEvent, 2);
		RegisterEvent("OnMurder", kEventID_OnMurder, kEventParams_GameEvent, 2);
		RegisterEvent("OnCombatEnd", kEventID_OnCombatEnd, kEventParams_GameEvent, 2);
		RegisterEvent("OnHit", kEventID_OnHit, kEventParams_GameEvent, 2);
		RegisterEvent("OnHitWith", kEventID_OnHitWith, kEventParams_GameEvent, 2);
		RegisterEvent("OnPackageStart", kEventID_OnPackageStart, kEventParams_GameEvent, 2);
		RegisterEvent("OnPackageDone", kEventID_OnPackageDone, kEventParams_GameEvent, 2);
		RegisterEvent("OnPackageChange", kEventID_OnPackageChange, kEventParams_GameEvent, 2);
		RegisterEvent("OnLoad", kEventID_OnLoad, kEventParams_GameEvent, 2);
		RegisterEvent("OnMagicEffectHit", kEventID_OnMagicEffectHit, kEventParams_GameEvent, 2);
		RegisterEvent("OnSell", kEventID_OnSell, kEventParams_GameEvent, 2);
		RegisterEvent("OnStartCombat", kEventID_OnStartCombat, kEventParams_GameEvent, 2);
		RegisterEvent("OnOpen", kEventID_OnOpen, kEventParams_GameEvent, 2);
		RegisterEvent("OnClose", kEventID_OnClose, kEventParams_GameEvent, 2);
		RegisterEvent("SayToDone", kEventID_SayToDone, kEventParams_GameEvent, 2);
		RegisterEvent("OnGrab", kEventID_OnGrab, kEventParams_GameEvent, 2);
		RegisterEvent("OnRelease", kEventID_OnRelease, kEventParams_GameEvent, 2);
		RegisterEvent("OnDestructionStageChange", kEventID_OnDestructionStageChange, kEventParams_GameEvent, 2);
		RegisterEvent("OnFire", kEventID_OnFire, kEventParams_GameEvent, 2);
		RegisterEvent("OnTrigger", kEventID_OnTrigger, kEventParams_GameEvent, 2);
		RegisterEvent("OnTriggerEnter", kEventID_OnTriggerEnter, kEventParams_GameEvent, 2);
		RegisterEvent("OnTriggerLeave", kEventID_OnTriggerLeave, kEventParams_GameEvent, 2);
		RegisterEvent("OnReset", kEventID_OnReset, kEventParams_GameEvent, 2);
		
		// Special-cased game events
		RegisterEvent("OnActivate", kEventID_OnActivate, kEventParams_OnActivate, 2);
		RegisterEvent("OnFrame", kEventID_OnFrame, nullptr, 0);
		
		// Input events
		RegisterEvent("OnKeyDown", kEventID_OnKeyDown, kEventParams_InputEvent, 1);
		RegisterEvent("OnKeyUp", kEventID_OnKeyUp, kEventParams_InputEvent, 1);
		RegisterEvent("OnKeyPress", kEventID_OnKeyPress, kEventParams_InputEvent, 1);
		
		// Internal events (save/load, game state)
		RegisterEvent("OnLoadGame", kEventID_LoadGame, nullptr, 0);
		RegisterEvent("OnSaveGame", kEventID_SaveGame, nullptr, 0);
		RegisterEvent("OnExitGame", kEventID_ExitGame, nullptr, 0);
		RegisterEvent("OnExitToMainMenu", kEventID_ExitToMainMenu, nullptr, 0);
		RegisterEvent("OnPostLoadGame", kEventID_PostLoadGame, nullptr, 0);
		RegisterEvent("OnDeleteGame", kEventID_DeleteGame, nullptr, 0);
		RegisterEvent("OnRenameGame", kEventID_RenameGame, nullptr, 0);
		RegisterEvent("OnNewGame", kEventID_NewGame, nullptr, 0);
		RegisterEvent("OnPreLoadGame", kEventID_PreLoadGame, nullptr, 0);
		
	}
	
	void Shutdown()
	{
		if (!s_initialized)
			return;
		
		// Clear all event handlers
		s_eventHandlers.clear();
		
		// Clear all event info
		s_eventNameMap.clear();
		s_eventIDMap.clear();
		s_eventInfos.clear();
		
		s_initialized = false;
	}
	
	void ClearFlushOnLoadEventHandlers()
	{
		if (!s_initialized)
			return;
		
		// Iterate through all events and clear handlers for those with FlushOnLoad flag
		for (auto& eventInfo : s_eventInfos)
		{
			if (eventInfo.flags & kEventFlag_FlushOnLoad)
			{
				s_eventHandlers[eventInfo.evName].clear();
			}
		}
	}
	
	bool RegisterEvent(const char* eventName, EventID evID, ParamType* paramTypes, UInt8 numParams, const char* alias, EventFlags flags)
	{
		if (!s_initialized || !eventName)
			return false;
		
		// Check if event already registered
		if (s_eventNameMap.find(eventName) != s_eventNameMap.end())
			return false;
		
		if (evID != kEventID_UserDefined && s_eventIDMap.find(evID) != s_eventIDMap.end())
			return false;
		
		// Check if alias is already used
		if (alias && s_eventNameMap.find(alias) != s_eventNameMap.end())
			return false;
		
		// Create new event info
		EventInfo newEvent;
		newEvent.evName = eventName;
		if (alias)
			newEvent.alias = alias;
		newEvent.evID = evID;
		newEvent.paramTypes = paramTypes;
		newEvent.numParams = numParams;
		newEvent.flags = flags;
		
		// Add to storage
		s_eventInfos.push_back(newEvent);
		EventInfo* eventPtr = &s_eventInfos.back();
		
		s_eventNameMap[eventName] = eventPtr;
		
		// Also register alias if provided
		if (alias)
		{
			s_eventNameMap[alias] = eventPtr;
		}
		
		if (evID != kEventID_UserDefined)
		{
			s_eventIDMap[evID] = eventPtr;
		}
		
		return true;
	}
	
	bool RegisterEventHandler(const char* eventName, EventHandlerCallback callback, void* context, UInt32 priority, const char* pluginName, const char* handlerName)
	{
		if (!s_initialized || !eventName || !callback)
			return false;
		
		// Check if event exists
		auto it = s_eventNameMap.find(eventName);
		if (it == s_eventNameMap.end())
			return false;
		
		// Create handler
		NativeEventHandlerInfo handler(callback, context, priority);
		if (pluginName)
			handler.m_pluginName = pluginName;
		if (handlerName)
			handler.m_handlerName = handlerName;
		EventCallback eventCallback(handler, priority);
		
		// Add to handler list
		auto& handlerList = s_eventHandlers[eventName];
		
		// Check if handler already exists
		for (const auto& existing : handlerList)
		{
			if (existing == eventCallback)
				return false;
		}
		
		// Insert in priority order (higher priority first)
		handlerList.push_back(eventCallback);
		handlerList.sort([](const EventCallback& a, const EventCallback& b) {
			return a.priority > b.priority;
		});
		
		// Update s_eventsInUse bitmask for ScriptEventList events
		EventInfo* eventInfo = it->second;
		if (eventInfo->evID < kEventID_ScriptEventListMAX) {
			s_eventsInUse |= MaskForEventID(eventInfo->evID);
			_MESSAGE("RegisterEventHandler: '%s' -> s_eventsInUse = 0x%08X", eventName, s_eventsInUse);
		}
		
		return true;
	}
	
	bool RegisterEventHandlerWithFilter(const char* eventName, EventHandlerCallback callback, EventFilter* filter, void* context, UInt32 priority, const char* pluginName, const char* handlerName)
	{
		if (!s_initialized || !eventName || !callback)
			return false;
		
		// Check if event exists
		auto it = s_eventNameMap.find(eventName);
		if (it == s_eventNameMap.end())
			return false;
		
		// Create handler
		NativeEventHandlerInfo handler(callback, context, priority);
		if (pluginName)
			handler.m_pluginName = pluginName;
		if (handlerName)
			handler.m_handlerName = handlerName;
		EventCallback eventCallback(handler, priority, filter);
		
		// Add to handler list
		auto& handlerList = s_eventHandlers[eventName];
		
		// Check if handler already exists
		for (const auto& existing : handlerList)
		{
			if (existing == eventCallback)
				return false;
		}
		
		// Insert in priority order (higher priority first)
		handlerList.push_back(eventCallback);
		handlerList.sort([](const EventCallback& a, const EventCallback& b) {
			return a.priority > b.priority;
		});
		
		// Update s_eventsInUse bitmask for ScriptEventList events
		EventInfo* eventInfo = it->second;
		if (eventInfo->evID < kEventID_ScriptEventListMAX) {
			s_eventsInUse |= MaskForEventID(eventInfo->evID);
			_MESSAGE("RegisterEventHandlerWithFilter: '%s' -> s_eventsInUse = 0x%08X", eventName, s_eventsInUse);
		}
		
		return true;
	}
	
	bool RemoveEventHandler(const char* eventName, EventHandlerCallback callback, void* context)
	{
		if (!s_initialized || !eventName || !callback)
			return false;
		
		// Check if event exists
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
			return false;
		
		// Find and remove handler
		auto& handlerList = handlersIt->second;
		for (auto it = handlerList.begin(); it != handlerList.end(); ++it)
		{
			if (it->nativeHandler.m_func == callback && 
				it->nativeHandler.m_context == context)
			{
				// If dispatching, defer removal to prevent iterator invalidation
				if (s_isDispatching)
				{
					it->removed = true; // Mark for removal
					s_deferredRemovals.push_back(std::make_pair(std::string(eventName), *it));
					return true;
				}
				else
				{
					// Remove immediately if not dispatching
					handlerList.erase(it);
					return true;
				}
			}
		}
		
		return false;
	}
	
	void DispatchEvent(const char* eventName, void** params)
	{
		if (!s_initialized || !eventName)
			return;
		
		// Push event name to stack for tracking
		s_eventStack.push(eventName);
		
		// Set dispatching flag for deferred removal
		s_isDispatching = true;
		
		// Check if event has handlers
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
		{
			s_eventStack.pop();
			s_isDispatching = false;
			return;
		}
		
		// Get event info
		auto eventIt = s_eventNameMap.find(eventName);
		if (eventIt == s_eventNameMap.end())
		{
			s_eventStack.pop();
			s_isDispatching = false;
			return;
		}
		
		const EventInfo* eventInfo = eventIt->second;
		
		// Dispatch to all handlers
		auto& handlerList = handlersIt->second;
		for (auto& handler : handlerList)
		{
			if (handler.removed)
				continue;
			
			// Check filter before calling handler
			if (handler.filter && !FilterMatches(handler.filter, params))
				continue; // Filter doesn't match, skip this handler
			
			if (handler.nativeHandler.m_func)
			{
				// Log handler invocation with debug info
				const char* pluginName = handler.nativeHandler.m_pluginName.empty() ? "[Unknown]" : handler.nativeHandler.m_pluginName.c_str();
				const char* handlerName = handler.nativeHandler.m_handlerName.empty() ? "[Unnamed]" : handler.nativeHandler.m_handlerName.c_str();
				_MESSAGE("DispatchEvent: Calling handler '%s::%s' for event '%s'", pluginName, handlerName, eventName);
				
				handler.nativeHandler.m_func(params, handler.nativeHandler.m_context);
			}
		}
		
		// Process deferred removals
		for (const auto& deferred : s_deferredRemovals)
		{
			const std::string& evName = deferred.first;
			const EventCallback& callback = deferred.second;
			
			auto deferredHandlersIt = s_eventHandlers.find(evName);
			if (deferredHandlersIt != s_eventHandlers.end())
			{
				auto& deferredHandlerList = deferredHandlersIt->second;
				deferredHandlerList.remove_if([&callback](const EventCallback& cb) {
					return cb == callback;
				});
			}
		}
		s_deferredRemovals.clear();
		
		// Clean up removed handlers
		handlerList.remove_if([](const EventCallback& cb) { return cb.removed; });
		
		// Reset dispatching flag
		s_isDispatching = false;
		
		// Pop event name from stack
		s_eventStack.pop();
	}
	
	void DispatchEventByID(EventID evID, void** params)
	{
		if (!s_initialized)
			return;
		
		// Get event info by ID
		auto eventIt = s_eventIDMap.find(evID);
		if (eventIt == s_eventIDMap.end())
			return;
		
		const EventInfo* eventInfo = eventIt->second;
		
		// Dispatch by name
		DispatchEvent(eventInfo->evName.c_str(), params);
	}
	
	const EventInfo* GetEventInfo(const char* eventName)
	{
		if (!s_initialized || !eventName)
			return nullptr;
		
		auto it = s_eventNameMap.find(eventName);
		if (it == s_eventNameMap.end())
			return nullptr;
		
		return it->second;
	}
	
	const EventInfo* GetEventInfoByID(EventID evID)
	{
		if (!s_initialized)
			return nullptr;
		
		auto it = s_eventIDMap.find(evID);
		if (it == s_eventIDMap.end())
			return nullptr;
		
		return it->second;
	}
	
	bool IsEventRegistered(const char* eventName)
	{
		if (!s_initialized || !eventName)
			return false;
		
		return s_eventNameMap.find(eventName) != s_eventNameMap.end();
	}
	
	const char* GetEventAlias(const char* eventName)
	{
		if (!s_initialized || !eventName)
			return "";
		
		const EventInfo* eventInfo = GetEventInfo(eventName);
		if (eventInfo && !eventInfo->alias.empty())
			return eventInfo->alias.c_str();
		
		return "";
	}
	
	const char* GetCurrentEventName()
	{
		if (!s_initialized || s_eventStack.empty())
			return "";
		
		return s_eventStack.top();
	}
	
	bool FilterMatches(const EventFilter* filter, void** params)
	{
		if (!filter || !params)
			return true; // No filter means always match
		
		// Check if parameter index is valid
		UInt32 paramIndex = filter->paramIndex;
		if (!params[paramIndex])
			return false; // Parameter is null, can't match
		
		// Simple pointer comparison for now
		// TODO: Implement proper value comparison based on parameter type
		if (filter->isArray)
		{
			// Array-based filter: check if param matches any value in array
			void** array = (void**)filter->filterValue;
			for (UInt32 i = 0; i < filter->arraySize; i++)
			{
				if (array[i] == params[paramIndex])
					return true; // Match found
			}
			return false; // No match in array
		}
		else
		{
			// Single value filter: direct comparison
			return filter->filterValue == params[paramIndex];
		}
	}
	
	bool IsEventHandlerFirst(const char* eventName, EventHandlerCallback callback, void* context)
	{
		if (!s_initialized || !eventName || !callback)
			return false;
		
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
			return false;
		
		auto& handlerList = handlersIt->second;
		if (handlerList.empty())
			return false;
		
		// Check if the first handler matches
		auto& firstHandler = handlerList.front();
		return firstHandler.nativeHandler.m_func == callback && firstHandler.nativeHandler.m_context == context;
	}
	
	bool IsEventHandlerLast(const char* eventName, EventHandlerCallback callback, void* context)
	{
		if (!s_initialized || !eventName || !callback)
			return false;
		
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
			return false;
		
		auto& handlerList = handlersIt->second;
		if (handlerList.empty())
			return false;
		
		// Check if the last handler matches
		auto& lastHandler = handlerList.back();
		return lastHandler.nativeHandler.m_func == callback && lastHandler.nativeHandler.m_context == context;
	}
	
	UInt32 GetHigherPriorityEventHandlers(const char* eventName, EventHandlerCallback callback, void* context, EventCallback** outHandlers, UInt32 maxHandlers)
	{
		if (!s_initialized || !eventName || !callback)
			return 0;
		
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
			return 0;
		
		auto& handlerList = handlersIt->second;
		
		// Find the handler to compare against
		UInt32 targetPriority = 0;
		bool found = false;
		for (const auto& handler : handlerList)
		{
			if (handler.nativeHandler.m_func == callback && handler.nativeHandler.m_context == context)
			{
				targetPriority = handler.priority;
				found = true;
				break;
			}
		}
		
		if (!found)
			return 0;
		
		// Count and collect handlers with higher priority
		UInt32 count = 0;
		if (outHandlers && maxHandlers > 0)
		{
			for (const auto& handler : handlerList)
			{
				if (handler.priority > targetPriority)
				{
					if (count < maxHandlers)
						outHandlers[count] = const_cast<EventCallback*>(&handler);
					count++;
				}
			}
		}
		else
		{
			// Just count if no output buffer
			for (const auto& handler : handlerList)
			{
				if (handler.priority > targetPriority)
					count++;
			}
		}
		
		return count;
	}
	
	UInt32 GetLowerPriorityEventHandlers(const char* eventName, EventHandlerCallback callback, void* context, EventCallback** outHandlers, UInt32 maxHandlers)
	{
		if (!s_initialized || !eventName || !callback)
			return 0;
		
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
			return 0;
		
		auto& handlerList = handlersIt->second;
		
		// Find the handler to compare against
		UInt32 targetPriority = 0;
		bool found = false;
		for (const auto& handler : handlerList)
		{
			if (handler.nativeHandler.m_func == callback && handler.nativeHandler.m_context == context)
			{
				targetPriority = handler.priority;
				found = true;
				break;
			}
		}
		
		if (!found)
			return 0;
		
		// Count and collect handlers with lower priority
		UInt32 count = 0;
		if (outHandlers && maxHandlers > 0)
		{
			for (const auto& handler : handlerList)
			{
				if (handler.priority < targetPriority)
				{
					if (count < maxHandlers)
						outHandlers[count] = const_cast<EventCallback*>(&handler);
					count++;
				}
			}
		}
		else
		{
			// Just count if no output buffer
			for (const auto& handler : handlerList)
			{
				if (handler.priority < targetPriority)
					count++;
			}
		}
		
		return count;
	}
	
	UInt32 GetEventHandlers(const char* eventName, EventCallback** outHandlers, UInt32 maxHandlers)
	{
		if (!s_initialized || !eventName)
			return 0;
		
		auto handlersIt = s_eventHandlers.find(eventName);
		if (handlersIt == s_eventHandlers.end())
			return 0;
		
		auto& handlerList = handlersIt->second;
		
		// Count handlers
		UInt32 count = 0;
		if (outHandlers && maxHandlers > 0)
		{
			for (const auto& handler : handlerList)
			{
				if (count < maxHandlers)
					outHandlers[count] = const_cast<EventCallback*>(&handler);
				count++;
			}
		}
		else
		{
			// Just count if no output buffer
			count = handlerList.size();
		}
		
		return count;
	}
	
	void InstallGameHooks()
	{
		// Hook installation is done in Hooks_Gameplay.cpp
		// This function is called during FOSE initialization
		// The actual hooks are installed in Hook_Gameplay_Init()
	}
}
