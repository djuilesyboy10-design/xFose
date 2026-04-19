#pragma once

// Minimal PluginAPI header for external plugins
// This version doesn't include CommandTable.h to avoid pulling in internal FOSE types

#include <cstdint>

// Basic type definitions
typedef uint32_t	UInt32;
typedef uint8_t		UInt8;
typedef uint16_t	UInt16;
typedef int32_t		SInt32;
typedef float		Float32;

typedef UInt32	PluginHandle;	// treat this as an opaque type

enum
{
	kPluginHandle_Invalid = 0xFFFFFFFF,
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

	kInterface_Max
};

struct FOSEInterface
{
	UInt32	foseVersion;
	UInt32	runtimeVersion;
	UInt32	editorVersion;
	UInt32	isEditor;
	bool	(* RegisterCommand)(void * info);	// returns true for success, false for failure
	void	(* SetOpcodeBase)(UInt32 opcode);
	void *	(* QueryInterface)(UInt32 id);

	// call during your Query or Load functions to get a PluginHandle uniquely identifying your plugin
	// invalid if called at any other time, so call it once and save the result
	PluginHandle	(* GetPluginHandle)(void);

	// CommandReturnType enum defined in CommandTable.h
	// does the same as RegisterCommand but includes return type; *required* for commands returning arrays
	bool	(* RegisterTypedCommand)(void * info, UInt32 retnType);
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
	bool		(* Assign)(void* args, const char* newValue);
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

struct FOSECommandTableInterface
{
	enum {
		kVersion = 1
	};

	UInt32	version;
	const void*	(* Start)(void);
	const void*	(* End)(void);
	const void*	(* GetByOpcode)(UInt32 opcode);
	const void*	(* GetByName)(const char* name);
	UInt32				(* GetReturnType)(const void* cmd);		// return type enum defined in CommandTable.h
	UInt32				(* GetRequiredFOSEVersion)(const void* cmd);
	const void*		(* GetParentPlugin)(const void* cmd);	// returns a pointer to the PluginInfo of the FOSE plugin that adds the command, if any. returns NULL otherwise
};

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
	// returns: true on success, false on failure
	bool (* RegisterEventHandler)(const char* eventName, EventHandlerCallback callback, void* context, UInt32 priority);

	// Remove an event handler
	// eventName: name of the event to unregister from
	// callback: the callback function to remove
	// context: the context pointer used during registration
	// returns: true on success, false on failure
	bool (* RemoveEventHandler)(const char* eventName, EventHandlerCallback callback, void* context);
};

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
	// if the mod owning that ID was unloaded.
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
