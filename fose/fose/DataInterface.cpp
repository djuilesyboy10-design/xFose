#include "PluginAPI.h"
#include "GameAPI.h"
#include "GameForms.h"
#include "GameObjects.h"
#include "ArrayVar.h"
#include "StringVar.h"
#include "FormExtraData.h"
#include "LoggingManager.h"
#include "PlayerControlsManager.h"
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
	case FOSEDataInterface::kFOSEData_LoggingManager:
		return &LoggingManager::GetSingleton();
	case FOSEDataInterface::kFOSEData_PlayerControlsManager:
		return PlayerControlsManager::GetSingleton();
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
	case FOSEDataInterface::kFOSEData_ArrayVarCreateMap:
		return (void*)ArrayVarManager::CreateMapBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarCreateStringMap:
		return (void*)ArrayVarManager::CreateStringMapBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarGetContainerType:
		return (void*)ArrayVarManager::GetContainerTypeBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarHasKey:
		return (void*)ArrayVarManager::HasKeyBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarSetElementByKey:
		return (void*)ArrayVarManager::SetElementByKeyBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarGetElementByKey:
		return (void*)ArrayVarManager::GetElementByKeyBySelf;
	case FOSEDataInterface::kFOSEData_ArrayVarRemoveByKey:
		return (void*)ArrayVarManager::RemoveByKeyBySelf;
	case FOSEDataInterface::kFOSEData_LoggingLog:
		return (void*)LoggingManager::LogBySelf;
	case FOSEDataInterface::kFOSEData_LoggingLogInfo:
		return (void*)LoggingManager::LogInfoBySelf;
	case FOSEDataInterface::kFOSEData_LoggingLogWarning:
		return (void*)LoggingManager::LogWarningBySelf;
	case FOSEDataInterface::kFOSEData_LoggingLogError:
		return (void*)LoggingManager::LogErrorBySelf;
	case FOSEDataInterface::kFOSEData_PlayerControlsIsKeyPressed:
		return (void*)PlayerControlsManager::Static_IsKeyPressed;
	case FOSEDataInterface::kFOSEData_PlayerControlsTapKey:
		return (void*)PlayerControlsManager::Static_TapKey;
	case FOSEDataInterface::kFOSEData_PlayerControlsHoldKey:
		return (void*)PlayerControlsManager::Static_HoldKey;
	case FOSEDataInterface::kFOSEData_PlayerControlsReleaseKey:
		return (void*)PlayerControlsManager::Static_ReleaseKey;
	case FOSEDataInterface::kFOSEData_PlayerControlsDisableKey:
		return (void*)PlayerControlsManager::Static_DisableKey;
	case FOSEDataInterface::kFOSEData_PlayerControlsEnableKey:
		return (void*)PlayerControlsManager::Static_EnableKey;
	case FOSEDataInterface::kFOSEData_PlayerControlsIsKeyDisabled:
		return (void*)PlayerControlsManager::Static_IsKeyDisabled;
	case FOSEDataInterface::kFOSEData_PlayerControlsGetControl:
		return (void*)PlayerControlsManager::Static_GetControl;
	case FOSEDataInterface::kFOSEData_PlayerControlsSetControl:
		return (void*)PlayerControlsManager::Static_SetControl;
	case FOSEDataInterface::kFOSEData_PlayerControlsIsControl:
		return (void*)PlayerControlsManager::Static_IsControl;
	case FOSEDataInterface::kFOSEData_PlayerControlsIsControlPressed:
		return (void*)PlayerControlsManager::Static_IsControlPressed;
	case FOSEDataInterface::kFOSEData_PlayerControlsGetNumMouseButtonsPressed:
		return (void*)PlayerControlsManager::Static_GetNumMouseButtonsPressed;
	case FOSEDataInterface::kFOSEData_PlayerControlsGetMouseButtonPress:
		return (void*)PlayerControlsManager::Static_GetMouseButtonPress;
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
