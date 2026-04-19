#pragma once

#include "GameTypes.h"
#include "PluginAPI.h"
#include "CommandTable.h"
#include <unordered_map>
#include <vector>
#include <string>

// Array element types
enum ArrayElementType
{
	kArrayElement_None = 0,
	kArrayElement_Integer,
	kArrayElement_Float,
	kArrayElement_String,
};

// Array element data
struct ArrayElement
{
	ArrayElementType	type;
	union
	{
		UInt32	intValue;
		float	floatValue;
		UInt32	stringID;	// Reference to string variable
	};
	UInt32				stringLen;	// For inline strings (optional)

	ArrayElement() : type(kArrayElement_None), intValue(0), stringLen(0) {}
	ArrayElement(UInt32 val) : type(kArrayElement_Integer), intValue(val), stringLen(0) {}
	ArrayElement(float val) : type(kArrayElement_Float), floatValue(val), stringLen(0) {}
	ArrayElement(UInt32 strID, UInt32 len) : type(kArrayElement_String), stringID(strID), stringLen(len) {}
};

// Array variable data structure
struct ArrayVarData
{
	UInt32						id;				// Unique ID for the array
	std::vector<ArrayElement>	elements;		// Array elements
	void*						owningScript;	// Script that owns this array (for cleanup)

	ArrayVarData(UInt32 _id, void* _owningScript)
		: id(_id), owningScript(_owningScript)
	{
	}
};

// Array variable manager
class ArrayVarManager
{
public:
	static ArrayVarManager& GetSingleton();

	// Create a new array variable
	UInt32	CreateArray(void* owningScript);

	// Get array size
	UInt32	GetSize(UInt32 arrayID);

	// Set array element at index
	bool	SetElement(UInt32 arrayID, UInt32 index, const ArrayElement& element);

	// Get array element at index
	bool	GetElement(UInt32 arrayID, UInt32 index, ArrayElement& outElement);

	// Push element to end of array
	bool	Push(UInt32 arrayID, const ArrayElement& element);

	// Pop element from end of array
	bool	Pop(UInt32 arrayID, ArrayElement& outElement);

	// Remove element at index
	bool	Remove(UInt32 arrayID, UInt32 index);

	// Clear all elements
	bool	Clear(UInt32 arrayID);

	// Check if array ID is valid
	bool	IsValid(UInt32 arrayID);

	// Clean up arrays owned by a script
	void	CleanupScriptArrays(void* script);

	// Delete array by ID
	void	Delete(UInt32 arrayID);

	// Static wrapper for Delete (for DataInterface)
	static void DeleteBySelf(ArrayVarManager* self, UInt32 arrayID);

	// Insert element at specific index
	bool	InsertAt(UInt32 arrayID, UInt32 index, const ArrayElement& element);

	// Find element by value (returns index, or -1 if not found)
	SInt32	FindByValue(UInt32 arrayID, const ArrayElement& element);

	// Count elements by type
	UInt32	CountByType(UInt32 arrayID, ArrayElementType type);

	// Sort array elements (ascending order)
	bool	Sort(UInt32 arrayID);

	// Reverse array elements
	bool	Reverse(UInt32 arrayID);

	// Shuffle array elements randomly
	bool	Shuffle(UInt32 arrayID);

	// Static wrappers for DataInterface
	static bool InsertAtBySelf(ArrayVarManager* self, UInt32 arrayID, UInt32 index, const ArrayElement& element);
	static SInt32 FindByValueBySelf(ArrayVarManager* self, UInt32 arrayID, const ArrayElement& element);
	static UInt32 CountByTypeBySelf(ArrayVarManager* self, UInt32 arrayID, ArrayElementType type);
	static bool SortBySelf(ArrayVarManager* self, UInt32 arrayID);
	static bool ReverseBySelf(ArrayVarManager* self, UInt32 arrayID);
	static bool ShuffleBySelf(ArrayVarManager* self, UInt32 arrayID);

private:
	ArrayVarManager();
	~ArrayVarManager();

	UInt32					m_nextID;							// Next available ID
	std::unordered_map<UInt32, ArrayVarData*>	m_arrayMap;	// ID -> ArrayVarData map

	// Allow serialization callbacks to access private members
	friend void ArrayVar_SaveCallback(void * reserved);
	friend void ArrayVar_LoadCallback(void * reserved);
	friend void ArrayVar_NewGameCallback(void * reserved);
};

// Serialization callbacks
void ArrayVar_SaveCallback(void * reserved);
void ArrayVar_LoadCallback(void * reserved);
void ArrayVar_NewGameCallback(void * reserved);

// Initialize array variable serialization
void Init_ArrayVarSerialization();
