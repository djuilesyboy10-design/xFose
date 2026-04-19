#include "FormExtraData.h"
#include "common/IDebugLog.h"
#include <map>
#include <vector>
#include <windows.h>

// FormExtraData implementation for FOSE
// Stores plugin-attached data on forms

namespace
{
	// Map of forms to their extra data
	std::map<TESForm*, std::vector<PluginFormExtraData*>> g_formExtraDataMap;
	CRITICAL_SECTION g_formExtraDataCS;

	void InitializeCriticalSection()
	{
		InitializeCriticalSection(&g_formExtraDataCS);
	}
}

// Initialize the critical section on DLL load
static bool g_initialized = false;
void FormExtraData_Initialize()
{
	if (!g_initialized)
	{
		InitializeCriticalSection();
		g_initialized = true;
	}
}

bool FormExtraData_Add(TESForm* form, PluginFormExtraData* formExtraData)
{
	if (!form || !formExtraData)
		return false;

	if (!formExtraData->name)
		return false;

	if (!g_initialized)
		FormExtraData_Initialize();

	EnterCriticalSection(&g_formExtraDataCS);

	auto iter = g_formExtraDataMap.find(form);
	if (iter != g_formExtraDataMap.end())
	{
		auto& dataList = iter->second;
		for (auto* data : dataList)
		{
			if (data && data->name == formExtraData->name)
			{
				LeaveCriticalSection(&g_formExtraDataCS);
				return false; // Already exists
			}
		}
	}

	g_formExtraDataMap[form].push_back(formExtraData);
	formExtraData->IncRefCount();

	LeaveCriticalSection(&g_formExtraDataCS);
	return true;
}

void FormExtraData_RemoveByName(TESForm* form, const char* name)
{
	if (!form || !name)
		return;

	if (!g_initialized)
		FormExtraData_Initialize();

	EnterCriticalSection(&g_formExtraDataCS);

	auto iter = g_formExtraDataMap.find(form);
	if (iter != g_formExtraDataMap.end())
	{
		auto& dataList = iter->second;

		for (auto it = dataList.begin(); it != dataList.end(); )
		{
			auto* data = *it;
			if (data && data->name == name)
			{
				data->DecRefCount();
				it = dataList.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (dataList.empty())
			g_formExtraDataMap.erase(iter);
	}

	LeaveCriticalSection(&g_formExtraDataCS);
}

void FormExtraData_RemoveByPtr(TESForm* form, PluginFormExtraData* formExtraData)
{
	if (!form || !formExtraData)
		return;

	if (!g_initialized)
		FormExtraData_Initialize();

	EnterCriticalSection(&g_formExtraDataCS);

	auto iter = g_formExtraDataMap.find(form);
	if (iter != g_formExtraDataMap.end())
	{
		auto& dataList = iter->second;

		for (auto it = dataList.begin(); it != dataList.end(); )
		{
			if (*it == formExtraData)
			{
				formExtraData->DecRefCount();
				it = dataList.erase(it);
			}
			else
			{
				++it;
			}
		}

		if (dataList.empty())
			g_formExtraDataMap.erase(iter);
	}

	LeaveCriticalSection(&g_formExtraDataCS);
}

PluginFormExtraData* FormExtraData_Get(const TESForm* form, const char* name)
{
	if (!form || !name)
		return nullptr;

	if (!g_initialized)
		FormExtraData_Initialize();

	EnterCriticalSection(&g_formExtraDataCS);

	auto iter = g_formExtraDataMap.find(const_cast<TESForm*>(form));
	if (iter != g_formExtraDataMap.end())
	{
		for (auto* data : iter->second)
		{
			if (data && data->name == name)
			{
				LeaveCriticalSection(&g_formExtraDataCS);
				return data;
			}
		}
	}

	LeaveCriticalSection(&g_formExtraDataCS);
	return nullptr;
}

UInt32 FormExtraData_GetAll(const TESForm* form, PluginFormExtraData** outData)
{
	if (!form)
		return 0;

	if (!g_initialized)
		FormExtraData_Initialize();

	EnterCriticalSection(&g_formExtraDataCS);

	auto iter = g_formExtraDataMap.find(const_cast<TESForm*>(form));
	if (iter != g_formExtraDataMap.end())
	{
		const auto& dataList = iter->second;
		UInt32 count = static_cast<UInt32>(dataList.size());
		if (outData)
		{
			for (UInt32 i = 0; i < count; ++i)
			{
				outData[i] = dataList[i];
			}
		}
		LeaveCriticalSection(&g_formExtraDataCS);
		return count;
	}

	LeaveCriticalSection(&g_formExtraDataCS);
	return 0;
}

UInt32 FormExtraData_Count(const TESForm* form)
{
	return FormExtraData_GetAll(form, nullptr);
}

bool FormExtraData_Exists(const TESForm* form, const char* name)
{
	return FormExtraData_Get(form, name) != nullptr;
}

void FormExtraData_Iterate(const TESForm* form, PluginFormExtraData::IterateCallback callback, void* userData)
{
	if (!form || !callback)
		return;

	if (!g_initialized)
		FormExtraData_Initialize();

	EnterCriticalSection(&g_formExtraDataCS);

	auto iter = g_formExtraDataMap.find(const_cast<TESForm*>(form));
	if (iter != g_formExtraDataMap.end())
	{
		const auto& dataList = iter->second;
		for (auto* data : dataList)
		{
			if (data)
			{
				callback(form, data, userData);
			}
		}
	}

	LeaveCriticalSection(&g_formExtraDataCS);
}
