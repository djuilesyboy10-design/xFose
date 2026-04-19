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

	// Delete string by ID
	void	Delete(UInt32 stringID);

	// Static wrapper for Delete (for DataInterface)
	static void DeleteBySelf(StringVarManager* self, UInt32 stringID);

	// Get string length
	UInt32	GetLength(UInt32 stringID);

	// Compare two strings (returns 0 if equal, negative if first < second, positive if first > second)
	SInt32	Compare(UInt32 stringID1, UInt32 stringID2);

	// Concatenate two strings (creates a new string)
	UInt32	Concatenate(UInt32 stringID1, UInt32 stringID2, void* owningScript);

	// Extract substring (creates a new string)
	UInt32	Substring(UInt32 stringID, UInt32 startIndex, UInt32 length, void* owningScript);

	// Uppercase string (modifies in-place)
	void	Uppercase(UInt32 stringID);

	// Lowercase string (modifies in-place)
	void	Lowercase(UInt32 stringID);

	// Trim whitespace from string (modifies in-place)
	void	Trim(UInt32 stringID);

	// Replace occurrences of substring (modifies in-place, returns number of replacements)
	UInt32	Replace(UInt32 stringID, const char* search, const char* replace);

	// Static wrappers for DataInterface
	static UInt32 GetLengthBySelf(StringVarManager* self, UInt32 stringID);
	static SInt32 CompareBySelf(StringVarManager* self, UInt32 stringID1, UInt32 stringID2);
	static UInt32 ConcatenateBySelf(StringVarManager* self, UInt32 stringID1, UInt32 stringID2, void* owningScript);
	static UInt32 SubstringBySelf(StringVarManager* self, UInt32 stringID, UInt32 startIndex, UInt32 length, void* owningScript);
	static void UppercaseBySelf(StringVarManager* self, UInt32 stringID);
	static void LowercaseBySelf(StringVarManager* self, UInt32 stringID);
	static void TrimBySelf(StringVarManager* self, UInt32 stringID);
	static UInt32 ReplaceBySelf(StringVarManager* self, UInt32 stringID, const char* search, const char* replace);

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
