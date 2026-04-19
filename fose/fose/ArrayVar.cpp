#include "ArrayVar.h"
#include "CommandTable.h"
#include "GameAPI.h"
#include "Serialization.h"
#include "common/IDebugLog.h"
#include <algorithm>

static const UInt32 kArrayVarRecordType = 'ARRY';

// ArrayVarManager Implementation
ArrayVarManager& ArrayVarManager::GetSingleton()
{
	static ArrayVarManager instance;
	return instance;
}

ArrayVarManager::ArrayVarManager()
	: m_nextID(1)
{
}

ArrayVarManager::~ArrayVarManager()
{
	// Clean up all array variables
	for (auto& pair : m_arrayMap)
	{
		delete pair.second;
	}
	m_arrayMap.clear();
}

UInt32 ArrayVarManager::CreateArray(void* owningScript)
{
	UInt32 id = m_nextID++;
	ArrayVarData* data = new ArrayVarData(id, owningScript);
	m_arrayMap[id] = data;
	
	_MESSAGE("ArrayVar: Created array ID %d", id);
	
	return id;
}

UInt32 ArrayVarManager::GetSize(UInt32 arrayID)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		return it->second->elements.size();
	}
	
	_MESSAGE("ArrayVar: GetSize failed - ID %d not found", arrayID);
	return 0;
}

bool ArrayVarManager::SetElement(UInt32 arrayID, UInt32 index, const ArrayElement& element)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		if (index < it->second->elements.size())
		{
			it->second->elements[index] = element;
			return true;
		}
		else
		{
			_MESSAGE("ArrayVar: SetElement failed - index %d out of bounds for array ID %d (size %d)", index, arrayID, it->second->elements.size());
		}
	}
	else
	{
		_MESSAGE("ArrayVar: SetElement failed - ID %d not found", arrayID);
	}
	return false;
}

bool ArrayVarManager::GetElement(UInt32 arrayID, UInt32 index, ArrayElement& outElement)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		if (index < it->second->elements.size())
		{
			outElement = it->second->elements[index];
			return true;
		}
		else
		{
			_MESSAGE("ArrayVar: GetElement failed - index %d out of bounds for array ID %d (size %d)", index, arrayID, it->second->elements.size());
		}
	}
	else
	{
		_MESSAGE("ArrayVar: GetElement failed - ID %d not found", arrayID);
	}
	return false;
}

bool ArrayVarManager::Push(UInt32 arrayID, const ArrayElement& element)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		it->second->elements.push_back(element);
		return true;
	}
	else
	{
		_MESSAGE("ArrayVar: Push failed - ID %d not found", arrayID);
	}
	return false;
}

bool ArrayVarManager::Pop(UInt32 arrayID, ArrayElement& outElement)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		if (!it->second->elements.empty())
		{
			outElement = it->second->elements.back();
			it->second->elements.pop_back();
			return true;
		}
		else
		{
			_MESSAGE("ArrayVar: Pop failed - array ID %d is empty", arrayID);
		}
	}
	else
	{
		_MESSAGE("ArrayVar: Pop failed - ID %d not found", arrayID);
	}
	return false;
}

bool ArrayVarManager::Remove(UInt32 arrayID, UInt32 index)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		if (index < it->second->elements.size())
		{
			it->second->elements.erase(it->second->elements.begin() + index);
			return true;
		}
		else
		{
			_MESSAGE("ArrayVar: Remove failed - index %d out of bounds for array ID %d (size %d)", index, arrayID, it->second->elements.size());
		}
	}
	else
	{
		_MESSAGE("ArrayVar: Remove failed - ID %d not found", arrayID);
	}
	return false;
}

bool ArrayVarManager::Clear(UInt32 arrayID)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		it->second->elements.clear();
		return true;
	}
	else
	{
		_MESSAGE("ArrayVar: Clear failed - ID %d not found", arrayID);
	}
	return false;
}

bool ArrayVarManager::IsValid(UInt32 arrayID)
{
	return m_arrayMap.find(arrayID) != m_arrayMap.end();
}

void ArrayVarManager::CleanupScriptArrays(void* script)
{
	// Remove all arrays owned by this script
	auto it = m_arrayMap.begin();
	while (it != m_arrayMap.end())
	{
		if (it->second->owningScript == script)
		{
			_MESSAGE("ArrayVar: Cleaning up array ID %d owned by script %p", it->first, script);
			delete it->second;
			it = m_arrayMap.erase(it);
		}
		else
		{
			++it;
		}
	}
}

void ArrayVarManager::Delete(UInt32 arrayID)
{
	auto it = m_arrayMap.find(arrayID);
	if (it != m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: Deleting array ID %d", arrayID);
		delete it->second;
		m_arrayMap.erase(it);
	}
	else
	{
		_MESSAGE("ArrayVar: Delete failed - ID %d not found", arrayID);
	}
}

void ArrayVarManager::DeleteBySelf(ArrayVarManager* self, UInt32 arrayID)
{
	if (self)
		self->Delete(arrayID);
}

bool ArrayVarManager::InsertAt(UInt32 arrayID, UInt32 index, const ArrayElement& element)
{
	auto it = m_arrayMap.find(arrayID);
	if (it == m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: InsertAt failed - ID %d not found", arrayID);
		return false;
	}

	ArrayVarData* data = it->second;
	UInt32 size = data->elements.size();

	if (index > size)
	{
		_MESSAGE("ArrayVar: InsertAt failed - index %d out of bounds (size %d)", index, size);
		return false;
	}

	data->elements.insert(data->elements.begin() + index, element);
	return true;
}

SInt32 ArrayVarManager::FindByValue(UInt32 arrayID, const ArrayElement& element)
{
	auto it = m_arrayMap.find(arrayID);
	if (it == m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: FindByValue failed - ID %d not found", arrayID);
		return -1;
	}

	ArrayVarData* data = it->second;
	for (UInt32 i = 0; i < data->elements.size(); i++)
	{
		const ArrayElement& current = data->elements[i];
		if (current.type != element.type)
			continue;

		bool match = false;
		switch (element.type)
		{
		case kArrayElement_Integer:
			match = (current.intValue == element.intValue);
			break;
		case kArrayElement_Float:
			match = (current.floatValue == element.floatValue);
			break;
		case kArrayElement_String:
			match = (current.stringID == element.stringID);
			break;
		default:
			break;
		}

		if (match)
			return (SInt32)i;
	}

	return -1;
}

UInt32 ArrayVarManager::CountByType(UInt32 arrayID, ArrayElementType type)
{
	auto it = m_arrayMap.find(arrayID);
	if (it == m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: CountByType failed - ID %d not found", arrayID);
		return 0;
	}

	UInt32 count = 0;
	ArrayVarData* data = it->second;
	for (const auto& element : data->elements)
	{
		if (element.type == type)
			count++;
	}

	return count;
}

bool ArrayVarManager::InsertAtBySelf(ArrayVarManager* self, UInt32 arrayID, UInt32 index, const ArrayElement& element)
{
	if (self)
		return self->InsertAt(arrayID, index, element);
	return false;
}

SInt32 ArrayVarManager::FindByValueBySelf(ArrayVarManager* self, UInt32 arrayID, const ArrayElement& element)
{
	if (self)
		return self->FindByValue(arrayID, element);
	return -1;
}

UInt32 ArrayVarManager::CountByTypeBySelf(ArrayVarManager* self, UInt32 arrayID, ArrayElementType type)
{
	if (self)
		return self->CountByType(arrayID, type);
	return 0;
}

bool ArrayVarManager::Sort(UInt32 arrayID)
{
	auto it = m_arrayMap.find(arrayID);
	if (it == m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: Sort failed - ID %d not found", arrayID);
		return false;
	}

	ArrayVarData* data = it->second;
	std::sort(data->elements.begin(), data->elements.end(),
		[](const ArrayElement& a, const ArrayElement& b) {
			if (a.type != b.type)
				return a.type < b.type;
			switch (a.type)
			{
			case kArrayElement_Integer:
				return a.intValue < b.intValue;
			case kArrayElement_Float:
				return a.floatValue < b.floatValue;
			case kArrayElement_String:
				return a.stringID < b.stringID;
			default:
				return false;
			}
		});

	return true;
}

bool ArrayVarManager::Reverse(UInt32 arrayID)
{
	auto it = m_arrayMap.find(arrayID);
	if (it == m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: Reverse failed - ID %d not found", arrayID);
		return false;
	}

	ArrayVarData* data = it->second;
	std::reverse(data->elements.begin(), data->elements.end());
	return true;
}

bool ArrayVarManager::Shuffle(UInt32 arrayID)
{
	auto it = m_arrayMap.find(arrayID);
	if (it == m_arrayMap.end())
	{
		_MESSAGE("ArrayVar: Shuffle failed - ID %d not found", arrayID);
		return false;
	}

	ArrayVarData* data = it->second;
	std::random_shuffle(data->elements.begin(), data->elements.end());
	return true;
}

bool ArrayVarManager::SortBySelf(ArrayVarManager* self, UInt32 arrayID)
{
	if (self)
		return self->Sort(arrayID);
	return false;
}

bool ArrayVarManager::ReverseBySelf(ArrayVarManager* self, UInt32 arrayID)
{
	if (self)
		return self->Reverse(arrayID);
	return false;
}

bool ArrayVarManager::ShuffleBySelf(ArrayVarManager* self, UInt32 arrayID)
{
	if (self)
		return self->Shuffle(arrayID);
	return false;
}

// Serialization Callbacks
void ArrayVar_SaveCallback(void * reserved)
{
	FOSESerializationInterface* fose = &g_FOSESerializationInterface;
	ArrayVarManager& manager = ArrayVarManager::GetSingleton();

	// Open the array variable record
	fose->OpenRecord(kArrayVarRecordType, 1);

	// Write the number of arrays
	UInt32 numArrays = manager.m_arrayMap.size();
	fose->WriteRecordData(&numArrays, sizeof(numArrays));

	// Write the next ID
	UInt32 nextID = manager.m_nextID;
	fose->WriteRecordData(&nextID, sizeof(nextID));

	// Write each array
	for (auto& pair : manager.m_arrayMap)
	{
		ArrayVarData* data = pair.second;

		// Write the ID
		UInt32 id = data->id;
		fose->WriteRecordData(&id, sizeof(id));

		// Write the number of elements
		UInt32 numElements = data->elements.size();
		fose->WriteRecordData(&numElements, sizeof(numElements));

		// Write each element
		for (const auto& element : data->elements)
		{
			// Write the element type
			UInt32 type = element.type;
			fose->WriteRecordData(&type, sizeof(type));

			// Write the element data based on type
			switch (element.type)
			{
			case kArrayElement_Integer:
				fose->WriteRecordData(&element.intValue, sizeof(element.intValue));
				break;
			case kArrayElement_Float:
				fose->WriteRecordData(&element.floatValue, sizeof(element.floatValue));
				break;
			case kArrayElement_String:
				fose->WriteRecordData(&element.stringID, sizeof(element.stringID));
				fose->WriteRecordData(&element.stringLen, sizeof(element.stringLen));
				break;
			default:
				break;
			}
		}

		// Write the owning script pointer
		void* owningScript = data->owningScript;
		fose->WriteRecordData(&owningScript, sizeof(owningScript));
	}

	_MESSAGE("ArrayVar: Saved %d arrays", numArrays);
}

void ArrayVar_LoadCallback(void * reserved)
{
	FOSESerializationInterface* fose = &g_FOSESerializationInterface;
	ArrayVarManager& manager = ArrayVarManager::GetSingleton();
	UInt32 type, version, length;

	// Clear existing arrays
	for (auto& pair : manager.m_arrayMap)
	{
		delete pair.second;
	}
	manager.m_arrayMap.clear();

	// Read the array variable record
	if (!fose->GetNextRecordInfo(&type, &version, &length))
	{
		_MESSAGE("ArrayVar: No array variable record found");
		return;
	}

	if (type != kArrayVarRecordType)
	{
		_MESSAGE("ArrayVar: Unexpected record type %08x", type);
		return;
	}

	// Read the number of arrays
	UInt32 numArrays = 0;
	UInt32 bytesRead = fose->ReadRecordData(&numArrays, sizeof(numArrays));
	if (bytesRead != sizeof(numArrays))
	{
		_MESSAGE("ArrayVar: Failed to read numArrays");
		return;
	}

	// Read the next ID
	UInt32 nextID = 1;
	bytesRead = fose->ReadRecordData(&nextID, sizeof(nextID));
	if (bytesRead != sizeof(nextID))
	{
		_MESSAGE("ArrayVar: Failed to read nextID");
		return;
	}
	manager.m_nextID = nextID;

	// Read each array
	for (UInt32 i = 0; i < numArrays; i++)
	{
		// Read the ID
		UInt32 id = 0;
		bytesRead = fose->ReadRecordData(&id, sizeof(id));
		if (bytesRead != sizeof(id))
		{
			_MESSAGE("ArrayVar: Failed to read array ID");
			break;
		}

		// Create the array
		ArrayVarData* data = new ArrayVarData(id, nullptr);
		manager.m_arrayMap[id] = data;

		// Read the number of elements
		UInt32 numElements = 0;
		bytesRead = fose->ReadRecordData(&numElements, sizeof(numElements));
		if (bytesRead != sizeof(numElements))
		{
			_MESSAGE("ArrayVar: Failed to read numElements");
			break;
		}

		// Read each element
		for (UInt32 j = 0; j < numElements; j++)
		{
			ArrayElement element;

			// Read the element type
			UInt32 type = 0;
			bytesRead = fose->ReadRecordData(&type, sizeof(type));
			if (bytesRead != sizeof(type))
			{
				_MESSAGE("ArrayVar: Failed to read element type");
				break;
			}
			element.type = (ArrayElementType)type;

			// Read the element data based on type
			switch (element.type)
			{
			case kArrayElement_Integer:
				bytesRead = fose->ReadRecordData(&element.intValue, sizeof(element.intValue));
				if (bytesRead != sizeof(element.intValue))
				{
					_MESSAGE("ArrayVar: Failed to read integer element");
					break;
				}
				break;
			case kArrayElement_Float:
				bytesRead = fose->ReadRecordData(&element.floatValue, sizeof(element.floatValue));
				if (bytesRead != sizeof(element.floatValue))
				{
					_MESSAGE("ArrayVar: Failed to read float element");
					break;
				}
				break;
			case kArrayElement_String:
				bytesRead = fose->ReadRecordData(&element.stringID, sizeof(element.stringID));
				if (bytesRead != sizeof(element.stringID))
				{
					_MESSAGE("ArrayVar: Failed to read string ID element");
					break;
				}
				bytesRead = fose->ReadRecordData(&element.stringLen, sizeof(element.stringLen));
				if (bytesRead != sizeof(element.stringLen))
				{
					_MESSAGE("ArrayVar: Failed to read string length element");
					break;
				}
				break;
			default:
				break;
			}

			data->elements.push_back(element);
		}

		// Read the owning script pointer
		void* owningScript = nullptr;
		bytesRead = fose->ReadRecordData(&owningScript, sizeof(owningScript));
		if (bytesRead != sizeof(owningScript))
		{
			_MESSAGE("ArrayVar: Failed to read owning script");
			break;
		}
		data->owningScript = owningScript;
	}

	_MESSAGE("ArrayVar: Loaded %d arrays", numArrays);
}

void ArrayVar_NewGameCallback(void * reserved)
{
	ArrayVarManager& manager = ArrayVarManager::GetSingleton();

	// Clear all arrays
	for (auto& pair : manager.m_arrayMap)
	{
		delete pair.second;
	}
	manager.m_arrayMap.clear();

	// Reset the next ID
	manager.m_nextID = 1;

	_MESSAGE("ArrayVar: Cleared all arrays for new game");
}

void Init_ArrayVarSerialization()
{
	Serialization::InternalSetSaveCallback(1, ArrayVar_SaveCallback);
	Serialization::InternalSetLoadCallback(1, ArrayVar_LoadCallback);
	Serialization::InternalSetNewGameCallback(1, ArrayVar_NewGameCallback);

	_MESSAGE("ArrayVar: Serialization callbacks registered");
}
