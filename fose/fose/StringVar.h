#pragma once

#include "GameTypes.h"
#include "PluginAPI.h"
#include "CommandTable.h"
#include <unordered_map>
#include <string>

// String variable data structure
struct StringVarData
{
	UInt32		id;				// Unique ID for the string
	std::string	data;			// The actual string data
	void*		owningScript;	// Script that owns this string (for cleanup)

	StringVarData(UInt32 _id, const char* _data, void* _owningScript)
		: id(_id), data(_data ? _data : ""), owningScript(_owningScript)
	{
	}
};

// String variable manager
class StringVarManager
{
public:
	static StringVarManager& GetSingleton();

	// Create a new string variable
	UInt32	CreateString(const char* value, void* owningScript);

	// Get string data by ID
	const char*	GetString(UInt32 stringID);

	// Set string data by ID
	void	SetString(UInt32 stringID, const char* newValue);

	// Check if string ID is valid
	bool	IsValid(UInt32 stringID);

	// Clean up strings owned by a script
	void	CleanupScriptStrings(void* script);

private:
	StringVarManager();
	~StringVarManager();

	UInt32					m_nextID;							// Next available ID
	std::unordered_map<UInt32, StringVarData*>	m_stringMap;	// ID -> StringVarData map

	// Allow serialization callbacks to access private members
	friend void StringVar_SaveCallback(void * reserved);
	friend void StringVar_LoadCallback(void * reserved);
	friend void StringVar_NewGameCallback(void * reserved);
};

// Serialization callbacks
void StringVar_SaveCallback(void * reserved);
void StringVar_LoadCallback(void * reserved);
void StringVar_NewGameCallback(void * reserved);

// Initialize string variable serialization
void Init_StringVarSerialization();

// Interface functions for FOSEStringVarInterface
const char*		GetString(UInt32 stringID);
void			SetString(UInt32 stringID, const char* newValue);
UInt32			CreateString(const char* value, void* owningScript);
void			RegisterStringVarInterface(FOSEStringVarInterface* intfc);
bool			AssignToStringVar(COMMAND_ARGS, const char* newValue);
