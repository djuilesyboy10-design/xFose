#include "PluginAPI.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "ArrayVar.h"
#include "StringVar.h"
#include "FormExtraData.h"
#include "common/IDebugLog.h"

// Implementations for FOSEDataInterface functions

void* DataInterface_GetSingleton(UInt32 singletonID)
{
	switch (singletonID)
	{
	case FOSEDataInterface::kFOSEData_ArrayMap:
		return &ArrayVarManager::GetSingleton();
	case FOSEDataInterface::kFOSEData_StringMap:
		return &StringVarManager::GetSingleton();
	default:
		_MESSAGE("DataInterface_GetSingleton called with unknown ID %d", singletonID);
		return NULL;
	}
}

void* DataInterface_GetFunc(UInt32 funcID)
{
	switch (funcID)
	{
	case FOSEDataInterface::kFOSEData_FormExtraDataGet:
		return (void*)FormExtraData_Get;
	case FOSEDataInterface::kFOSEData_FormExtraDataGetAll:
		return (void*)FormExtraData_GetAll;
	case FOSEDataInterface::kFOSEData_FormExtraDataAdd:
		return (void*)FormExtraData_Add;
	case FOSEDataInterface::kFOSEData_FormExtraDataRemoveByName:
		return (void*)FormExtraData_RemoveByName;
	case FOSEDataInterface::kFOSEData_FormExtraDataRemoveByPtr:
		return (void*)FormExtraData_RemoveByPtr;
	case FOSEDataInterface::kFOSEData_FormExtraDataCount:
		return (void*)FormExtraData_Count;
	case FOSEDataInterface::kFOSEData_FormExtraDataExists:
		return (void*)FormExtraData_Exists;
	case FOSEDataInterface::kFOSEData_FormExtraDataIterate:
		return (void*)FormExtraData_Iterate;
	case FOSEDataInterface::kFOSEData_ArrayVarMapDeleteBySelf:
		return (void*)ArrayVarManager::DeleteBySelf;
	case FOSEDataInterface::kFOSEData_StringVarMapDeleteBySelf:
		return (void*)StringVarManager::DeleteBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarInsertAt:
		return (void*)ArrayVarManager::InsertAtBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarFindByValue:
		return (void*)ArrayVarManager::FindByValueBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarCountByType:
		return (void*)ArrayVarManager::CountByTypeBySelf;
	case FOSEDataInterface::kFOSEData_StringVarGetLength:
		return (void*)StringVarManager::GetLengthBySelf;
	case FOSEDataInterface::kFOSEData_StringVarCompare:
		return (void*)StringVarManager::CompareBySelf;
	case FOSEDataInterface::kFOSEData_StringVarConcatenate:
		return (void*)StringVarManager::ConcatenateBySelf;
	case FOSEDataInterface::kFOSEData_StringVarSubstring:
		return (void*)StringVarManager::SubstringBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarSort:
		return (void*)ArrayVarManager::SortBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarReverse:
		return (void*)ArrayVarManager::ReverseBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarShuffle:
		return (void*)ArrayVarManager::ShuffleBySelf;
	case FOSEDataInterface::kFOSEData_StringVarUppercase:
		return (void*)StringVarManager::UppercaseBySelf;
	case FOSEDataInterface::kFOSEData_StringVarLowercase:
		return (void*)StringVarManager::LowercaseBySelf;
	case FOSEDataInterface::kFOSEData_StringVarTrim:
		return (void*)StringVarManager::TrimBySelf;
	case FOSEDataInterface::kFOSEData_StringVarReplace:
		return (void*)StringVarManager::ReplaceBySelf;
	default:
		_MESSAGE("DataInterface_GetFunc called with unknown ID %d", funcID);
		return NULL;
	}
}

void* DataInterface_GetData(UInt32 dataID)
{
	static UInt32 numPreloadMods = 0;
	static UInt32 numLoadedPlugins = 0;
	static UInt32 foseVersion = 0x01030032; // FOSE 1.3.2

	switch (dataID)
	{
	case FOSEDataInterface::kFOSEData_NumPreloadMods:
		// Return pointer to number of preload mods
		// This is a placeholder - actual implementation would need to access game data
		return &numPreloadMods;
	case FOSEDataInterface::kFOSEData_NumLoadedPlugins:
		// Return pointer to number of loaded plugins
		// This is a placeholder - actual implementation would need to access game data
		return &numLoadedPlugins;
	case FOSEDataInterface::kFOSEData_FOSEVersion:
		// Return pointer to FOSE version
		return &foseVersion;
	default:
		_MESSAGE("DataInterface_GetData called with unknown ID %d", dataID);
		return NULL;
	}
}

void DataInterface_ClearScriptDataCache()
{
	_MESSAGE("DataInterface_ClearScriptDataCache called (stub - not yet implemented)");
}

FOSEDataInterface g_dataInterface = {
	1, // kVersion
	DataInterface_GetSingleton,
	DataInterface_GetFunc,
	DataInterface_GetData,
	DataInterface_ClearScriptDataCache
};
