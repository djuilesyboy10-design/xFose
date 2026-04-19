#pragma once

#include "PluginAPI.h"
#include "GameForms.h"
#include "GameAPI.h"

// --- PluginFormExtraData ---
// Base class for plugins to attach custom data to forms
// Extend this class and allocate it with the game's heap
// Use the static methods to add, get, and remove it from a form
// Class is reference counted
//
// Example class:
// class MyFormExtraData : public PluginFormExtraData {
// public:
//     MyFormExtraData() : PluginFormExtraData("MyFormExtraData") {}
//     virtual ~MyFormExtraData() override = default;
//     float myAttachedData;
// };
//
// Example creation:
// auto* data = (MyFormExtraData*)FormHeap_Allocate(sizeof(MyFormExtraData));
// new (data) MyFormExtraData();
// data->myAttachedData = 1.0f;
// PluginFormExtraData::Add(dataApi, actor, data);
//
// Example retrieval:
// auto* extraData = (MyFormExtraData*)PluginFormExtraData::Get(dataApi, actor, "MyFormExtraData");
//
// Example removal:
// PluginFormExtraData::Remove(dataApi, actor, "MyFormExtraData");
class PluginFormExtraData
{
public:
	const char*	name;
	UInt32		refCount;

	PluginFormExtraData(const char* aName) : name(aName), refCount(0) {}
	virtual ~PluginFormExtraData() {}
	virtual void DeleteThis() {
		this->~PluginFormExtraData();
		FormHeap_Free(this);
	}

	void IncRefCount() {
		InterlockedIncrement((LONG*)&refCount);
	}

	void DecRefCount() {
		if (InterlockedDecrement((LONG*)&refCount) == 0) {
			DeleteThis();
		}
	}

	// Retrieves extra data from a form by name
	static inline PluginFormExtraData* Get(FOSEDataInterface* dataApi, const TESForm* form, const char* name)
	{
		static auto* get = (PluginFormExtraData*(*)(const TESForm*, const char*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataGet);
		return get(form, name);
	}

	// Adds extra data to a form
	// Returns true if the extra data was added successfully, false if it already exists or arguments are null
	static inline bool Add(FOSEDataInterface* dataApi, TESForm* form, PluginFormExtraData* extraData)
	{
		static auto* add = (bool(*)(TESForm*, PluginFormExtraData*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataAdd);
		return add(form, extraData);
	}

	// Removes extra data from a form by name
	static inline void Remove(FOSEDataInterface* dataApi, TESForm* form, const char* name)
	{
		static auto* remove = (void (*)(TESForm*, const char*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataRemoveByName);
		remove(form, name);
	}

	// Removes extra data from a form by pointer to the data
	static inline void RemoveByPtr(FOSEDataInterface* dataApi, TESForm* form, PluginFormExtraData* extraData)
	{
		static auto* remove = (void (*)(TESForm*, PluginFormExtraData*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataRemoveByPtr);
		remove(form, extraData);
	}

	// Retrieves all extra data from a form
	// First query the data count with an empty outData pointer, then call again with an appropriately sized outData array
	static inline UInt32 GetAllExtraData(FOSEDataInterface* dataApi, const TESForm* form, PluginFormExtraData** outData) {
		static auto* getAll = (UInt32(*)(const TESForm*, PluginFormExtraData**)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataGetAll);
		return getAll(form, outData);
	}

	// Counts the number of extra data entries on a form
	static inline UInt32 Count(FOSEDataInterface* dataApi, const TESForm* form) {
		static auto* count = (UInt32(*)(const TESForm*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataCount);
		return count(form);
	}

	// Checks if extra data with a specific name exists on a form
	static inline bool Exists(FOSEDataInterface* dataApi, const TESForm* form, const char* name) {
		static auto* exists = (bool(*)(const TESForm*, const char*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataExists);
		return exists(form, name);
	}

	// Callback type for iteration
	typedef void (*IterateCallback)(const TESForm* form, PluginFormExtraData* data, void* userData);

	// Iterates through all extra data on a form, calling the callback for each entry
	static inline void Iterate(FOSEDataInterface* dataApi, const TESForm* form, IterateCallback callback, void* userData) {
		static auto* iterate = (void(*)(const TESForm*, IterateCallback, void*)) dataApi->GetFunc(FOSEDataInterface::kFOSEData_FormExtraDataIterate);
		iterate(form, callback, userData);
	}
};

// FormExtraData function declarations
void FormExtraData_Initialize();
bool FormExtraData_Add(TESForm* form, PluginFormExtraData* formExtraData);
void FormExtraData_RemoveByName(TESForm* form, const char* name);
void FormExtraData_RemoveByPtr(TESForm* form, PluginFormExtraData* formExtraData);
PluginFormExtraData* FormExtraData_Get(const TESForm* form, const char* name);
UInt32 FormExtraData_GetAll(const TESForm* form, PluginFormExtraData** outData);
UInt32 FormExtraData_Count(const TESForm* form);
bool FormExtraData_Exists(const TESForm* form, const char* name);
void FormExtraData_Iterate(const TESForm* form, PluginFormExtraData::IterateCallback callback, void* userData);
