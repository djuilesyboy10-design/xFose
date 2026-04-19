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

void StringVarManager::Delete(UInt32 stringID)
{
	auto it = m_stringMap.find(stringID);
	if (it != m_stringMap.end())
	{
		_MESSAGE("StringVar: Deleting string ID %d", stringID);
		delete it->second;
		m_stringMap.erase(it);
	}
	else
	{
		_MESSAGE("StringVar: Delete failed - ID %d not found", stringID);
	}
}

void StringVarManager::DeleteBySelf(StringVarManager* self, UInt32 stringID)
{
	if (self)
		self->Delete(stringID);
}

UInt32 StringVarManager::GetLength(UInt32 stringID)
{
	auto it = m_stringMap.find(stringID);
	if (it == m_stringMap.end())
	{
		_MESSAGE("StringVar: GetLength failed - ID %d not found", stringID);
		return 0;
	}

	return (UInt32)it->second->data.length();
}

SInt32 StringVarManager::Compare(UInt32 stringID1, UInt32 stringID2)
{
	auto it1 = m_stringMap.find(stringID1);
	auto it2 = m_stringMap.find(stringID2);

	if (it1 == m_stringMap.end() || it2 == m_stringMap.end())
	{
		_MESSAGE("StringVar: Compare failed - one or both IDs not found (%d, %d)", stringID1, stringID2);
		return -1;
	}

	return strcmp(it1->second->data.c_str(), it2->second->data.c_str());
}

UInt32 StringVarManager::Concatenate(UInt32 stringID1, UInt32 stringID2, void* owningScript)
{
	auto it1 = m_stringMap.find(stringID1);
	auto it2 = m_stringMap.find(stringID2);

	if (it1 == m_stringMap.end() || it2 == m_stringMap.end())
	{
		_MESSAGE("StringVar: Concatenate failed - one or both IDs not found (%d, %d)", stringID1, stringID2);
		return 0;
	}

	std::string concatenated = it1->second->data + it2->second->data;
	return CreateString(concatenated.c_str(), owningScript);
}

UInt32 StringVarManager::Substring(UInt32 stringID, UInt32 startIndex, UInt32 length, void* owningScript)
{
	auto it = m_stringMap.find(stringID);
	if (it == m_stringMap.end())
	{
		_MESSAGE("StringVar: Substring failed - ID %d not found", stringID);
		return 0;
	}

	const std::string& str = it->second->data;
	UInt32 strLen = (UInt32)str.length();

	if (startIndex >= strLen)
	{
		_MESSAGE("StringVar: Substring failed - startIndex %d out of bounds (length %d)", startIndex, strLen);
		return 0;
	}

	// Adjust length if it exceeds string bounds
	UInt32 actualLength = length;
	if (startIndex + length > strLen)
		actualLength = strLen - startIndex;

	std::string substr = str.substr(startIndex, actualLength);
	return CreateString(substr.c_str(), owningScript);
}

UInt32 StringVarManager::GetLengthBySelf(StringVarManager* self, UInt32 stringID)
{
	if (self)
		return self->GetLength(stringID);
	return 0;
}

SInt32 StringVarManager::CompareBySelf(StringVarManager* self, UInt32 stringID1, UInt32 stringID2)
{
	if (self)
		return self->Compare(stringID1, stringID2);
	return -1;
}

UInt32 StringVarManager::ConcatenateBySelf(StringVarManager* self, UInt32 stringID1, UInt32 stringID2, void* owningScript)
{
	if (self)
		return self->Concatenate(stringID1, stringID2, owningScript);
	return 0;
}

UInt32 StringVarManager::SubstringBySelf(StringVarManager* self, UInt32 stringID, UInt32 startIndex, UInt32 length, void* owningScript)
{
	if (self)
		return self->Substring(stringID, startIndex, length, owningScript);
	return 0;
}

void StringVarManager::Uppercase(UInt32 stringID)
{
	auto it = m_stringMap.find(stringID);
	if (it == m_stringMap.end())
	{
		_MESSAGE("StringVar: Uppercase failed - ID %d not found", stringID);
		return;
	}

	std::string& str = it->second->data;
	for (char& c : str)
	{
		c = toupper(c);
	}
}

void StringVarManager::Lowercase(UInt32 stringID)
{
	auto it = m_stringMap.find(stringID);
	if (it == m_stringMap.end())
	{
		_MESSAGE("StringVar: Lowercase failed - ID %d not found", stringID);
		return;
	}

	std::string& str = it->second->data;
	for (char& c : str)
	{
		c = tolower(c);
	}
}

void StringVarManager::Trim(UInt32 stringID)
{
	auto it = m_stringMap.find(stringID);
	if (it == m_stringMap.end())
	{
		_MESSAGE("StringVar: Trim failed - ID %d not found", stringID);
		return;
	}

	std::string& str = it->second->data;

	// Trim leading whitespace
	size_t start = str.find_first_not_of(" \t\n\r");
	if (start != std::string::npos)
	{
		str = str.substr(start);
	}

	// Trim trailing whitespace
	size_t end = str.find_last_not_of(" \t\n\r");
	if (end != std::string::npos)
	{
		str = str.substr(0, end + 1);
	}
}

UInt32 StringVarManager::Replace(UInt32 stringID, const char* search, const char* replace)
{
	auto it = m_stringMap.find(stringID);
	if (it == m_stringMap.end())
	{
		_MESSAGE("StringVar: Replace failed - ID %d not found", stringID);
		return 0;
	}

	if (!search || !replace)
		return 0;

	std::string& str = it->second->data;
	std::string searchStr(search);
	std::string replaceStr(replace);

	UInt32 count = 0;
	size_t pos = 0;
	while ((pos = str.find(searchStr, pos)) != std::string::npos)
	{
		str.replace(pos, searchStr.length(), replaceStr);
		pos += replaceStr.length();
		count++;
	}

	return count;
}

void StringVarManager::UppercaseBySelf(StringVarManager* self, UInt32 stringID)
{
	if (self)
		self->Uppercase(stringID);
}

void StringVarManager::LowercaseBySelf(StringVarManager* self, UInt32 stringID)
{
	if (self)
		self->Lowercase(stringID);
}

void StringVarManager::TrimBySelf(StringVarManager* self, UInt32 stringID)
{
	if (self)
		self->Trim(stringID);
}

UInt32 StringVarManager::ReplaceBySelf(StringVarManager* self, UInt32 stringID, const char* search, const char* replace)
{
	if (self)
		return self->Replace(stringID, search, replace);
	return 0;
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
