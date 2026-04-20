#pragma once

#include "fose/CommandTable.h"
struct CommandInfo;
struct ParamInfo;
class TESObjectREFR;
class Script;
struct ScriptEventList;
struct PluginInfo;

typedef UInt32	PluginHandle;	// treat this as an opaque type

enum
{
	kPluginHandle_Invalid = 0xFFFFFFFF,
};

enum
{
	kPluginOpcode_Debug = kFoseOpCodeTest,
};

enum
{
	kInterface_Console = 0,

	// added in v0015
	kInterface_Serialization,

	// added v0016
	kInterface_StringVar,
	kInterface_IO,

	// Added v0002
	kInterface_Messaging,
	kInterface_CommandTable,

	// Added for event manager
	kInterface_EventManager,

	// Added for UI manipulation
	kInterface_UI,

	// Added for array variables
	kInterface_ArrayVar,

	// Added for script compilation and function calling
	kInterface_Script,

	// Added for data access
	kInterface_Data,

	kInterface_Max
};

struct FOSEInterface
{
	UInt32	foseVersion;
	UInt32	runtimeVersion;
	UInt32	editorVersion;
	UInt32	isEditor;
	bool	(* RegisterCommand)(CommandInfo * info);	// returns true for success, false for failure
	void	(* SetOpcodeBase)(UInt32 opcode);
	void *	(* QueryInterface)(UInt32 id);

	// call during your Query or Load functions to get a PluginHandle uniquely identifying your plugin
	// invalid if called at any other time, so call it once and save the result
	PluginHandle	(* GetPluginHandle)(void);

	// CommandReturnType enum defined in CommandTable.h
	// does the same as RegisterCommand but includes return type; *required* for commands returning arrays
	bool	(* RegisterTypedCommand)(CommandInfo * info, CommandReturnType retnType);
	// returns a full path the the game directory
	const char* (* GetRuntimeDirectory)();

	// Allows checking for nogore edition
	UInt32	isNogore;
};

struct FOSEConsoleInterface
{
	enum
	{
		kVersion = 1
	};

	UInt32	version;
	void	(* RunScriptLine)(const char * buf);
};

/***** string_var API *****************************
*
* string_var is FOSE's string pseudo-datatype. Strings are represented internally by
* integer IDs, however the details of the implementation are opaque to scripts and
* ideally to plugins as well.
*
* Plugin authors should rely primarily on Assign() to return a string as the result of a script function.
* It takes the COMMAND_ARGS passed to the script function followed by a pointer to the new string.
* i.e. Assign(PASS_COMMAND_ARGS, "some string") assigns "some string" to the  string variable on the lefthand
* side of the script assignment statement, initializing the variable if necessary. Generates a logged error if 
* the scripter does not provide a variable in which to store the result.
*
* GetString(), CreateString(), and SetString() are slightly lower-level functions; use them only if you have a 
* genuine need to directly create and manipulate new string variables outside of script commands. CreateString()
* returns the integer ID of the newly-created string var.
*
* If you want your script commands to support FOSE's %z format specifier (for inserting the contents of a string_var
* into another string), you must pass an FOSEStringVarInterface pointer to Register(). This only needs to be called
* once, preferably during plugin load.
*
**************************************************/

struct FOSEStringVarInterface
{
	enum {
		kVersion = 1
	};

	UInt32		version;
	const char* (* GetString)(UInt32 stringID);
	void		(* SetString)(UInt32 stringID, const char* newValue);
	UInt32		(* CreateString)(const char* value, void* owningScript);
	void		(* Register)(FOSEStringVarInterface* intfc);
	bool		(* Assign)(COMMAND_ARGS, const char* newValue);
};

// IsKeyPressed() takes a DirectInput scancode; values above 255 represent mouse buttons
// codes are the same as those used by OBSE's IsKeyPressed2 command
struct FOSEIOInterface
{
	enum {
		kVersion = 1
	};

	UInt32		version;
	bool		(* IsKeyPressed)(UInt32 scancode);
};

/**** Messaging API docs ********************************************************************
 *
 *	Messaging API allows inter-plugin communication at run-time. A plugin may register
 *	one callback for each plugin from which it wishes to receive messages, specifying
 *	the sender by name in the call to RegisterListener(). RegisterListener returns false
 *	if the specified plugin is not loaded, true otherwise. Any messages dispatched by
 *	the specified plugin will then be forwarded to the listener as they occur. Passing NULL as 
 *	the sender registers the calling plugin as a listener to every loaded plugin.
 *
 *	Messages may be dispatched via Dispatch() to either a specific listener (specified
 *	by name) or to all listeners (with NULL passed as the receiver). The contents and format of
 *	messageData are left up to the sender, and the receiver is responsible for casting the message
 *	to the expected type. If no plugins are registered as listeners for the sender, 
 *	Dispatch() returns false, otherwise it returns true.
 *
 *	Calling RegisterListener() or Dispatch() during plugin load is not advised as the requested plugin
 *	may not yet be loaded at that point. Instead, if you wish to register as a listener or dispatch a
 *	message immediately after plugin load, use RegisterListener() during load to register to receive
 *	messages from FOSE (sender name: "FOSE"). You will then receive a message from FOSE once 
 *	all plugins have been loaded, at which point it is safe to establish communications between
 *	plugins.
 *
 *	Some plugin authors may wish to use strings instead of integers to denote message type. In
 *	that case the receiver can pass the address of the string as an integer and require the receiver
 *	to cast it back to a char* on receipt.
 *
 *********************************************************************************************/

struct FOSEMessagingInterface
{
	struct Message {
		const char	* sender;
		UInt32		type;
		UInt32		dataLen;
		void		* data;
	};

	typedef void (* EventCallback)(Message* msg);

	enum {
		kVersion = 3
	};

	// FOSE messages
	enum {
		kMessage_PostLoad,				// sent to registered plugins once all plugins have been loaded (no data)

		kMessage_ExitGame,				// exit to windows from main menu or in-game menu

		kMessage_ExitToMainMenu,		// exit to main menu from in-game menu

		kMessage_LoadGame,				// Dispatched immediately before plugin serialization callbacks invoked, after savegame has been read by Fallout
										// dataLen: length of file path, data: char* file path of .fos savegame file
										// Receipt of this message does not *guarantee* the serialization callback will be invoked
										// as there may be no .fose file associated with the savegame

		kMessage_SaveGame,				// as above
	
		kMessage_Precompile,			// EDITOR: Dispatched when the user attempts to save a script in the script editor.
										// FOSE first does its pre-compile checks; if these pass the message is dispatched before
										// the vanilla compiler does its own checks. 
										// data: ScriptBuffer* to the buffer representing the script under compilation
		
		kMessage_PreLoadGame,			// dispatched immediately before savegame is read by Fallout
										// dataLen: length of file path, data: char* file path of .fos savegame file

		kMessage_ExitGame_Console,		// exit game using 'qqq' console command

		kMessage_PostLoadGame,			//dispatched after an attempt to load a saved game has finished (the game's LoadGame() routine
										//has returned). You will probably want to handle this event if your plugin uses a Preload callback
										//as there is a chance that after that callback is invoked the game will encounter an error
										//while loading the saved game (eg. corrupted save) which may require you to reset some of your
										//plugin state.
										//data: bool, true if game successfully loaded, false otherwise */

		kMessage_PostPostLoad,			// sent right after kMessage_PostLoad to facilitate the correct dispatching/registering of messages/listeners
										// plugins may register as listeners during the first callback while deferring dispatches until the next
		kMessage_RuntimeScriptError,	// dispatched when an FOSE script error is encountered during runtime/
										// data: char* errorMessageText
// added for kVersion = 2
		kMessage_DeleteGame,			// sent right before deleting the .fose cosave and the .fos save.
										// dataLen: length of file path, data: char* file path of .fos savegame file
		kMessage_RenameGame,			// sent right before renaming the .fose cosave and the .fos save.
										// dataLen: length of old file path, data: char* old file path of .fos savegame file
										// you are expected to save the data and wait for kMessage_RenameNewGame
		kMessage_RenameNewGame,			// sent right after kMessage_RenameGame.
										// dataLen: length of new file path, data: char* new file path of .fos savegame file
		kMessage_NewGame,				// sent right before iterating through plugins newGame.
										// dataLen: 0, data: NULL
// added for kVersion == 3
		kMessage_DeleteGameName,		// version of the messages sent with a save file name instead of a save file path.
		kMessage_RenameGameName,
		kMessage_RenameNewGameName,
	};

	UInt32	version;
	bool	(* RegisterListener)(PluginHandle listener, const char* sender, EventCallback handler);
	bool	(* Dispatch)(PluginHandle sender, UInt32 messageType, void * data, UInt32 dataLen, const char* receiver);
};

		
/**** command table API docs *******************************************************
*
*	Command table API gives plugins limited access to FOSE's internal command table.
*	The provided functionality mirrors that defined in the CommandTable class and
*	should be fairly self-explanatory. You may note that Start() and End() could be
*	used to alter the command table in memory. It probably needn't be said, but
*	doing so would be a Very Bad Idea.
*
*	GetRequiredFOSEVersion returns the minimum major release of FOSE required by
*	the specified command, i.e. the release in which it first appeared.
*	For non-FOSE commands, returns 0. For plugin commands, returns -1.
*
************************************************************************************/

struct FOSECommandTableInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;
	const CommandInfo*	(* Start)(void);
	const CommandInfo*	(* End)(void);
	const CommandInfo*	(* GetByOpcode)(UInt32 opcode);
	const CommandInfo*	(* GetByName)(const char* name);
	UInt32				(* GetReturnType)(const CommandInfo* cmd);		// return type enum defined in CommandTable.h
	UInt32				(* GetRequiredFOSEVersion)(const CommandInfo* cmd);
	const PluginInfo*	(* GetParentPlugin)(const CommandInfo* cmd);	// returns a pointer to the PluginInfo of the FOSE plugin that adds the command, if any. returns NULL otherwise
};

/**** event manager API docs **************************************************
*
*	The event manager API allows plugins to register handlers for game events.
*	Events include ScriptEventList events (OnHit, OnDeath, OnLoad, OnEquip, etc.)
*	and special-cased events (OnActivate).
*
*	To register an event handler:
*	1. Define a callback function matching the EventHandlerCallback signature
*	2. Call RegisterEventHandler with the event name, callback, optional context, and priority
*	3. The callback will be invoked when the event fires
*
*	To remove an event handler:
*	1. Call RemoveEventHandler with the same event name, callback, and context used during registration
*
*	Event parameters are passed as an array of void* pointers. The number and type
*	of parameters depends on the specific event. See EventParams.h for parameter definitions.
*
*	ScriptEventList events use kEventParams_GameEvent (2 params: source, target).
*	OnActivate uses kEventParams_OnActivate (2 params: source, target).
*
************************************************************************************/

struct FOSEEventManagerInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;

	// Event handler callback signature
	// params: array of parameter pointers (void*)
	// context: optional context pointer passed during registration
	typedef void (* EventHandlerCallback)(void** params, void* context);

	// Register an event handler
	// eventName: name of the event to register for (e.g., "OnHit", "OnDeath", "OnLoad")
	// callback: function to call when the event fires
	// context: optional context pointer passed to the callback
	// priority: handler priority (higher = called first)
	// pluginName: optional plugin name for debug logging (can be nullptr)
	// handlerName: optional handler name for debug logging (can be nullptr)
	// returns: true on success, false on failure
	bool (* RegisterEventHandler)(const char* eventName, EventHandlerCallback callback, void* context, UInt32 priority, const char* pluginName, const char* handlerName);

	// Remove an event handler
	// eventName: name of the event to unregister from
	// callback: the callback function to remove
	// context: the context pointer used during registration
	// returns: true on success, false on failure
	bool (* RemoveEventHandler)(const char* eventName, EventHandlerCallback callback, void* context);
	
	// Check if a handler is the first/last in the handler list for an event
	bool (* IsEventHandlerFirst)(const char* eventName, EventHandlerCallback callback, void* context);
	bool (* IsEventHandlerLast)(const char* eventName, EventHandlerCallback callback, void* context);
	
	// Get all handlers for an event
	// Returns count of handlers found
	UInt32 (* GetEventHandlers)(const char* eventName, void** outHandlers, UInt32 maxHandlers);
};

/**** UI manipulation API docs **************************************************
*
*	The UI manipulation API allows plugins to modify HUD and menu elements directly.
*	This provides access to the same functionality as the SetUIFloat/SetUIString
*	console commands, but callable from C++ code.
*
*	Component path format: "MenuType\\tile\\tile\\...\\traitName"
*	Example: "HUDMainMenu\\xHM\\_xHMAlpha"
*
*	Returns error value (-999.0f) if the trait is not found.
*
************************************************************************************/

struct FOSEUIManagerInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;

	// Get the value of a float UI trait
	// componentPath: path to the UI trait (e.g., "HUDMainMenu\\xHM\\_xHMAlpha")
	// returns: the float value, or -999.0f if trait not found
	float (* GetUIFloat)(const char* componentPath);

	// Set the value of a float UI trait
	// componentPath: path to the UI trait (e.g., "HUDMainMenu\\xHM\\_xHMAlpha")
	// value: the new float value to set
	// returns: true on success, false on failure
	bool (* SetUIFloat)(const char* componentPath, float value);

	// Set the value of a string UI trait
	// componentPath: path to the UI trait (e.g., "HUDMainMenu\\xHM\\filename")
	// value: the new string value to set
	// returns: true on success, false on failure
	bool (* SetUIString)(const char* componentPath, const char* value);
};

/**** array variable API docs **************************************************
*
*	The array variable API allows plugins to create and manipulate arrays of data.
*	Arrays support integer, float, and string elements.
*
************************************************************************************/

struct FOSEArrayVarInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;

	// Create a new array
	// owningScript: optional script that owns this array
	// returns: the ID of the created array
	UInt32 (* CreateArray)(void* owningScript);

	// Get the size of an array
	// arrayID: the ID of the array
	// returns: the number of elements in the array, or 0 if invalid
	UInt32 (* GetArraySize)(UInt32 arrayID);

	// Set an element in an array (integer)
	// arrayID: the ID of the array
	// index: the index to set
	// value: the integer value to set
	// returns: true on success, false on failure
	bool (* SetArrayElement)(UInt32 arrayID, UInt32 index, UInt32 value);

	// Get an element from an array (integer)
	// arrayID: the ID of the array
	// index: the index to get
	// outValue: pointer to receive the value
	// returns: true on success, false on failure
	bool (* GetArrayElement)(UInt32 arrayID, UInt32 index, UInt32* outValue);

	// Check if an array ID is valid
	// arrayID: the ID to check
	// returns: true if valid, false if not
	bool (* IsArrayValid)(UInt32 arrayID);
};

/**** script API docs **********************************************************
 *
 *	Provides general functionality for interacting with scripts.
 *
 *	CallFunction() attempts to execute a script defined as a user-defined function.
 *	A calling object and containing object can be specified, or passed as NULL.
 *	If successful, it returns true, and the result is passed back from the script.
 *
 *	GetFunctionParams() returns the number of parameters expected by a function
 *	script. Returns -1 if the script is not a valid function script.
 *
 ************************************************************************************/

#if RUNTIME

struct FOSEScriptInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;

	// Call a user-defined function script
	// funcScript: the script to execute
	// callingObj: the calling object (can be NULL)
	// container: the containing object (can be NULL)
	// result: pointer to receive the result
	// numArgs: number of arguments
	// ...: variable arguments (int, float, char*)
	// returns: true on success, false on failure
	bool (* CallFunction)(Script* funcScript, TESObjectREFR* callingObj, TESObjectREFR* container,
		double* result, UInt8 numArgs, ...);

	// Get the number of parameters expected by a function script
	// funcScript: the script to check
	// paramTypesOut: optional array to receive parameter types
	// returns: number of parameters, or -1 if not a valid function script
	UInt32 (* GetFunctionParams)(Script* funcScript, UInt8* paramTypesOut);

	// Extract arguments with extended functionality
	// Similar to game's ExtractArgs() but without type checking for TESForm parameters
	bool (* ExtractArgsEx)(ParamInfo * paramInfo, void * scriptDataIn, UInt32 * scriptDataOffset, Script * scriptObj,
		ScriptEventList * eventList, ...);

	// Extract format string and arguments
	bool (* ExtractFormatStringArgs)(UInt32 fmtStringPos, char* buffer, ParamInfo * paramInfo, void * scriptDataIn, 
		UInt32 * scriptDataOffset, Script * scriptObj, ScriptEventList * eventList, UInt32 maxParams, ...);
};

#endif

/**** data API docs **********************************************************
 *
 *	Provides access to internal data structures without reverse engineering.
 *
 *	GetSingleton() returns pointers to internal singleton objects.
 *	GetFunc() returns pointers to internal utility functions.
 *	GetData() returns pointers to internal data values.
 *
 ************************************************************************************/

#if RUNTIME

struct FOSEDataInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;

	// Singleton IDs for GetSingleton
	enum {
		kFOSEData_ArrayMap = 1,
		kFOSEData_StringMap,
		kFOSEData_LoggingManager,
		kFOSEData_PlayerControlsManager,
		kFOSEData_SingletonMax,
	};

	// Data IDs for GetData
	enum {
		kFOSEData_NumPreloadMods = 1,
		kFOSEData_NumLoadedPlugins,
		kFOSEData_FOSEVersion,
		kFOSEData_DataMax,
	};

	// Function IDs for GetFunc
	enum {
		kFOSEData_FormExtraDataGet = 1,
		kFOSEData_FormExtraDataGetAll,
		kFOSEData_FormExtraDataAdd,
		kFOSEData_FormExtraDataRemoveByName,
		kFOSEData_FormExtraDataRemoveByPtr,
		kFOSEData_FormExtraDataCount,
		kFOSEData_FormExtraDataExists,
		kFOSEData_FormExtraDataIterate,
		kFOSEData_ArrayVarMapDeleteBySelf,
		kFOSEData_StringVarMapDeleteBySelf,
		kFOSEData_ArrayVarInsertAt,
		kFOSEData_ArrayVarFindByValue,
		kFOSEData_ArrayVarCountByType,
		kFOSEData_StringVarGetLength,
		kFOSEData_StringVarCompare,
		kFOSEData_StringVarConcatenate,
		kFOSEData_StringVarSubstring,
		kFOSEData_ArrayVarSort,
		kFOSEData_ArrayVarReverse,
		kFOSEData_ArrayVarShuffle,
		kFOSEData_StringVarUppercase,
		kFOSEData_StringVarLowercase,
		kFOSEData_StringVarTrim,
		kFOSEData_StringVarReplace,
		kFOSEData_ArrayVarCreateMap,
		kFOSEData_ArrayVarCreateStringMap,
		kFOSEData_ArrayVarGetContainerType,
		kFOSEData_ArrayVarHasKey,
		kFOSEData_ArrayVarSetElementByKey,
		kFOSEData_ArrayVarGetElementByKey,
		kFOSEData_ArrayVarRemoveByKey,
		kFOSEData_LoggingLog,
		kFOSEData_LoggingLogInfo,
		kFOSEData_LoggingLogWarning,
		kFOSEData_LoggingLogError,
		kFOSEData_PlayerControlsIsKeyPressed,
		kFOSEData_PlayerControlsTapKey,
		kFOSEData_PlayerControlsHoldKey,
		kFOSEData_PlayerControlsReleaseKey,
		kFOSEData_PlayerControlsDisableKey,
		kFOSEData_PlayerControlsEnableKey,
		kFOSEData_PlayerControlsIsKeyDisabled,
		kFOSEData_PlayerControlsGetControl,
		kFOSEData_PlayerControlsSetControl,
		kFOSEData_PlayerControlsIsControl,
		kFOSEData_PlayerControlsIsControlPressed,
		kFOSEData_PlayerControlsGetNumMouseButtonsPressed,
		kFOSEData_PlayerControlsGetMouseButtonPress,
		kFOSEData_FuncMax,
	};

	// Get internal singleton objects
	void * (* GetSingleton)(UInt32 singletonID);

	// Get internal utility functions
	void * (* GetFunc)(UInt32 funcID);

	// Get internal data values
	void * (* GetData)(UInt32 dataID);

	// Clear script data cache (for performance)
	void (* ClearScriptDataCache)();
};

// PluginFormExtraData class is defined in FormExtraData.h
// Include FormExtraData.h to use PluginFormExtraData functionality

#endif


/**** serialization API docs ***************************************************
 *	
 *	The serialization API adds a separate save data file plugins can use to save
 *	data along with a game save. It is be stored separately from the main save
 *	(.ess) file to prevent any potential compatibility issues. The actual
 *	implementation is opaqe, so it can be changed if needed, but the APIs
 *	provided will always remain the same and will operate the same way.
 *	
 *	Each plugin that has registered the proper callbacks will be able to write
 *	typed and versioned records in to the file, similarly to the way Fallout
 *	.esp files work. Chunk types are just simple 32-bit values, you can use
 *	them for whatever you want as they are only meaningful to your code.
 *	Multiple records of the same type can be added, and records will be
 *	returned to you in the order they were originally written. Versioning is
 *	important so you can update the format of your data without breaking
 *	people's save files. Note that this means that if you have created multiple
 *	versions of a record you will have to check the version number and do some
 *	data conversion on load. Of course it isn't strictly /mandatory/ that you do
 *	this, but I consider not breaking people's save files to be very important.
 *	Also, note that your record data will be uniquely identified by your
 *	assigned opcode base, so make sure that is set up correctly (you really
 *	have to be doing that anyway, but I thought I'd mention it again).
 *	
 *	At any point, a plugin can call the
 *	FOSEStorageInterface::SetSave/Load/NewGameCallback functions to register a
 *	callback that will be run when a game is being saved, loaded, or started
 *	fresh. You must provide the PluginHandle you were given during your startup
 *	function so the core code knows which plugin to associate with the callback.
 *	
 *	When the save callback is called, there are two APIs you can use to write
 *	your records. WriteRecord is the simplest API - just give it your record
 *	type, the version, and a buffer storing your data. This is good when you can
 *	write your entire record all at one time. If you have more complex needs,
 *	use the OpenRecord/WriteRecordData APIs. When starting to write a new
 *	record, call OpenRecord, passing in the record type and version. Then call
 *	WriteRecordData as many times as needed to fill in the data for the record.
 *	To start the next record, just call OpenRecord again. Calling OpenRecord or
 *	exiting your save callback will automatically close the record and perform
 *	behind-the-scenes cleanup.
 *	
 *	The load callback is simpler. First call GetNextRecordInfo. It will move to
 *	the next record (or the first record if it is the first time it has been
 *	called) and return information about it. GetNextRecordInfo returns true if
 *	it found a "next" record for your plugin, so you know you are done reading
 *	when it returns false. After calling GetNextRecordInfo, call ReadRecordData
 *	to retrieve the data stored in the record. It may be called multiple times,
 *	and returns the number of bytes read from the record (really only useful if
 *	you accidentally try to read off the end of the record).
 *	
 *	The new game callback should be used to reset all of your internal data
 *	structures. It is called when a new game is started or a game is loaded with
 *	no save file.
 *	
 *	RefIDs stored in this file involve one complication. The upper 8 bits of the
 *	ID store the index of the mod that "owns" the form. This index may change in
 *	between save and load if a user changes their mod list. To fix this, run the
 *	ID through the ResolveRefID API. It fixes up the ID to reflect any changes
 *	in the mod list. If the mod owning that ID is no longer loaded, the function
 *	returns false.
 *	
 ******************************************************************************/

struct FOSESerializationInterface
{
	enum
	{
		kVersion = 2,
	};

	typedef void (* EventCallback)(void * reserved);

	UInt32	version;
	void	(* SetSaveCallback)(PluginHandle plugin, EventCallback callback);
	void	(* SetLoadCallback)(PluginHandle plugin, EventCallback callback);
	void	(* SetNewGameCallback)(PluginHandle plugin, EventCallback callback);

	bool	(* WriteRecord)(UInt32 type, UInt32 version, const void * buf, UInt32 length);
	bool	(* OpenRecord)(UInt32 type, UInt32 version);
	bool	(* WriteRecordData)(const void * buf, UInt32 length);

	bool	(* GetNextRecordInfo)(UInt32 * type, UInt32 * version, UInt32 * length);
	UInt32	(* ReadRecordData)(void * buf, UInt32 length);

	// take a refid as stored in the loaded save file and resolve it using the currently
	// loaded list of mods. All refids stored in a save file must be run through this
	// function to account for changing mod lists. This returns true on success, and false
	// if the mod owning the RefID was unloaded.
	bool	(* ResolveRefID)(UInt32 refID, UInt32 * outRefID);

	void	(* SetPreLoadCallback)(PluginHandle plugin, EventCallback callback);

	// returns a full path to the last loaded save game
	const char* (* GetSavePath)();

	// Peeks at the data without interfiring with the current position
	UInt32	(* PeekRecordData)(void * buf, UInt32 length);
};

struct PluginInfo
{
	enum
	{
		kInfoVersion = 1
	};

	UInt32			infoVersion;
	const char *	name;
	UInt32			version;
};

typedef bool (* _FOSEPlugin_Query)(const FOSEInterface * fose, PluginInfo * info);
typedef bool (* _FOSEPlugin_Load)(const FOSEInterface * fose);

/**** plugin API docs **********************************************************
 *	
 *	IMPORTANT: Before releasing a plugin, you MUST contact the FOSE team at the
 *	contact addresses listed in fose_readme.txt to register a range of opcodes.
 *	This is required to prevent conflicts between multiple plugins, as each
 *	command must be assigned a unique opcode.
 *	
 *	The base API is pretty simple. Create a project based on the
 *	fose_plugin_example project included with the FOSE source code, then define
 *	and export these functions:
 *	
 *	bool FOSEPlugin_Query(const FOSEInterface * fose, PluginInfo * info)
 *	
 *	This primary purposes of this function are to fill out the PluginInfo
 *	structure, and to perform basic version checks based on the info in the
 *	FOSEInterface structure. Return false if your plugin is incompatible with
 *	the version of FOSE or Fallout passed in, otherwise return true. In either
 *	case, fill out the PluginInfo structure.
 *	
 *	If the plugin is being loaded in the context of the editor, isEditor will be
 *	non-zero, editorVersion will contain the current editor version, and
 *	falloutVersion will be zero. In this case you can probably just return
 *	true, however if you have multiple DLLs implementing the same behavior, for
 *	example one for each version of Fallout, only one of them should return
 *	true.
 *	
 *	The PluginInfo fields should be filled out as follows:
 *	- infoVersion should be set to PluginInfo::kInfoVersion
 *	- name should be a pointer to a null-terminated string uniquely identifying
 *	  your plugin, it will be used in the plugin querying API
 *	- version is only used by the plugin query API, and will be returned to
 *	  scripts requesting the current version of your plugin
 *	
 *	bool FOSEPlugin_Load(const FOSEInterface * fose)
 *	
 *	In this function, use the SetOpcodeBase callback in FOSEInterface to set the
 *	opcode base to your assigned value, then use RegisterCommand to register all
 *	of your commands. FOSE will fix up your CommandInfo structure when loaded
 *	in the context of the editor, and will fill in any NULL callbacks with their
 *	default values, so don't worry about having a unique 'execute' callback for
 *	the editor, and don't provide a 'parse' callback unless you're actually
 *	overriding the default behavior. The opcode field will also be automatically
 *	updated with the next opcode in the sequence started by SetOpcodeBase.
 *	
 *	At this time, or at any point forward you can call the QueryInterface
 *	callback to retrieve an interface structure for the base services provided
 *	by the FOSE core.
 *	
 *	You may optionally return false from this function to unload your plugin,
 *	but make sure that you DO NOT register any commands if you do.
 *	
 *	Note that all structure versions are backwards-compatible, so you only need
 *	to check against the latest version that you need. New fields will be only
 *	added to the end, and all old fields will remain compatible with their
 *	previous implementations.
 *	
 ******************************************************************************/
