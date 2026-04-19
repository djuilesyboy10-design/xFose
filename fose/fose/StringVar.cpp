#include "StringVar.h"
#include "CommandTable.h"
#include "GameAPI.h"
#include "Serialization.h"
#include "common/IDebugLog.h"

static const UInt32 kStringVarRecordType = 'STRV';

// StringVarManager Implementation
StringVarManager& StringVarManager::GetSingleton()
{
	static StringVarManager instance;
	return instance;
}

StringVarManager::StringVarManager()
	: m_nextID(1)
{
}

StringVarManager::~StringVarManager()
{
	// Clean up all string variables
	for (auto& pair : m_stringMap)
	{
		delete pair.second;
	}
	m_stringMap.clear();
}

UInt32 StringVarManager::CreateString(const char* value, void* owningScript)
{
	UInt32 id = m_nextID++;
	StringVarData* data = new StringVarData(id, value, owningScript);
	m_stringMap[id] = data;
	
	_MESSAGE("StringVar: Created string ID %d with value '%s'", id, value ? value : "(null)");
	
	return id;
}

const char* StringVarManager::GetString(UInt32 stringID)
{
	auto it = m_stringMap.find(stringID);
	if (it != m_stringMap.end())
	{
		return it->second->data.c_str();
	}
	
	_MESSAGE("StringVar: GetString failed - ID %d not found", stringID);
	return "";
}

void StringVarManager::SetString(UInt32 stringID, const char* newValue)
{
	auto it = m_stringMap.find(stringID);
	if (it != m_stringMap.end())
	{
		it->second->data = newValue ? newValue : "";
		_MESSAGE("StringVar: Set string ID %d to '%s'", stringID, newValue ? newValue : "(null)");
	}
	else
	{
		_MESSAGE("StringVar: SetString failed - ID %d not found", stringID);
	}
}

bool StringVarManager::IsValid(UInt32 stringID)
{
	return m_stringMap.find(stringID) != m_stringMap.end();
}

void StringVarManager::CleanupScriptStrings(void* script)
{
	// Remove all strings owned by this script
	auto it = m_stringMap.begin();
	while (it != m_stringMap.end())
	{
		if (it->second->owningScript == script)
		{
			_MESSAGE("StringVar: Cleaning up string ID %d owned by script %p", it->first, script);
			delete it->second;
			it = m_stringMap.erase(it);
		}
		else
		{
			++it;
		}
	}
}

// Interface Functions
const char* GetString(UInt32 stringID)
{
	return StringVarManager::GetSingleton().GetString(stringID);
}

void SetString(UInt32 stringID, const char* newValue)
{
	StringVarManager::GetSingleton().SetString(stringID, newValue);
}

UInt32 CreateString(const char* value, void* owningScript)
{
	return StringVarManager::GetSingleton().CreateString(value, owningScript);
}

void RegisterStringVarInterface(FOSEStringVarInterface* intfc)
{
	// This would register the interface for %z format specifier support
	// For now, we'll just log that it was called
	_MESSAGE("StringVar: RegisterStringVarInterface called");
}

bool AssignToStringVar(COMMAND_ARGS, const char* newValue)
{
	// This assigns a string to the result of a script command
	// For now, we'll implement a basic version without script ownership
	
	// Create a new string and assign it to the result
	UInt32 stringID = CreateString(newValue, nullptr);
	
	// Assign the string ID to the result
	*result = stringID;
	
	_MESSAGE("StringVar: Assigned string ID %d to result", stringID);
	
	return true;
}

// Serialization Callbacks
void StringVar_SaveCallback(void * reserved)
{
	FOSESerializationInterface* fose = &g_FOSESerializationInterface;
	StringVarManager& manager = StringVarManager::GetSingleton();

	// Open the string variable record
	fose->OpenRecord(kStringVarRecordType, 1);

	// Write the number of strings
	UInt32 numStrings = manager.m_stringMap.size();
	fose->WriteRecordData(&numStrings, sizeof(numStrings));

	// Write the next ID
	UInt32 nextID = manager.m_nextID;
	fose->WriteRecordData(&nextID, sizeof(nextID));

	// Write each string
	for (auto& pair : manager.m_stringMap)
	{
		StringVarData* data = pair.second;

		// Write the ID
		UInt32 id = data->id;
		fose->WriteRecordData(&id, sizeof(id));

		// Write the string length
		UInt32 strLen = data->data.length();
		fose->WriteRecordData(&strLen, sizeof(strLen));

		// Write the string data
		if (strLen > 0)
		{
			fose->WriteRecordData(data->data.c_str(), strLen);
		}

		// Write the owning script pointer (may not be useful across saves, but save it anyway)
		void* owningScript = data->owningScript;
		fose->WriteRecordData(&owningScript, sizeof(owningScript));
	}

	_MESSAGE("StringVar: Saved %d strings", numStrings);
}

void StringVar_LoadCallback(void * reserved)
{
	FOSESerializationInterface* fose = &g_FOSESerializationInterface;
	StringVarManager& manager = StringVarManager::GetSingleton();
	UInt32 type, version, length;

	// Clear existing strings
	for (auto& pair : manager.m_stringMap)
	{
		delete pair.second;
	}
	manager.m_stringMap.clear();

	// Read the string variable record
	if (!fose->GetNextRecordInfo(&type, &version, &length))
	{
		_MESSAGE("StringVar: No string variable record found");
		return;
	}

	if (type != kStringVarRecordType)
	{
		_MESSAGE("StringVar: Unexpected record type %08x", type);
		return;
	}

	// Read the number of strings
	UInt32 numStrings = 0;
	UInt32 bytesRead = fose->ReadRecordData(&numStrings, sizeof(numStrings));
	if (bytesRead != sizeof(numStrings))
	{
		_MESSAGE("StringVar: Failed to read numStrings");
		return;
	}

	// Read the next ID
	UInt32 nextID = 1;
	bytesRead = fose->ReadRecordData(&nextID, sizeof(nextID));
	if (bytesRead != sizeof(nextID))
	{
		_MESSAGE("StringVar: Failed to read nextID");
		return;
	}
	manager.m_nextID = nextID;

	// Read each string
	for (UInt32 i = 0; i < numStrings; i++)
	{
		// Read the ID
		UInt32 id = 0;
		bytesRead = fose->ReadRecordData(&id, sizeof(id));
		if (bytesRead != sizeof(id))
		{
			_MESSAGE("StringVar: Failed to read string ID");
			break;
		}

		// Read the string length
		UInt32 strLen = 0;
		bytesRead = fose->ReadRecordData(&strLen, sizeof(strLen));
		if (bytesRead != sizeof(strLen))
		{
			_MESSAGE("StringVar: Failed to read string length");
			break;
		}

		// Read the string data
		std::string strData;
		if (strLen > 0)
		{
			strData.resize(strLen);
			bytesRead = fose->ReadRecordData(&strData[0], strLen);
			if (bytesRead != strLen)
			{
				_MESSAGE("StringVar: Failed to read string data");
				break;
			}
		}

		// Read the owning script pointer
		void* owningScript = nullptr;
		bytesRead = fose->ReadRecordData(&owningScript, sizeof(owningScript));
		if (bytesRead != sizeof(owningScript))
		{
			_MESSAGE("StringVar: Failed to read owning script");
			break;
		}

		// Create the string variable
		StringVarData* data = new StringVarData(id, strData.c_str(), owningScript);
		manager.m_stringMap[id] = data;
	}

	_MESSAGE("StringVar: Loaded %d strings", numStrings);
}

void StringVar_NewGameCallback(void * reserved)
{
	StringVarManager& manager = StringVarManager::GetSingleton();

	// Clear all strings
	for (auto& pair : manager.m_stringMap)
	{
		delete pair.second;
	}
	manager.m_stringMap.clear();

	// Reset the next ID
	manager.m_nextID = 1;

	_MESSAGE("StringVar: Cleared all strings for new game");
}

void Init_StringVarSerialization()
{
	Serialization::InternalSetSaveCallback(0, StringVar_SaveCallback);
	Serialization::InternalSetLoadCallback(0, StringVar_LoadCallback);
	Serialization::InternalSetNewGameCallback(0, StringVar_NewGameCallback);

	_MESSAGE("StringVar: Serialization callbacks registered");
}
