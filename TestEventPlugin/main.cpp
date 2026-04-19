#include "ITypes.h"
#include <map>
#include "fose/PluginAPI.h"
#include <stdio.h>
#include <stdarg.h>

// Plugin info
static const char* g_pluginName = "TestEventPlugin";
static UInt32 g_pluginVersion = 1;

// Event Manager interface pointer
static FOSEEventManagerInterface* g_eventManager = nullptr;

// Data interface pointer
static void* g_dataInterface = nullptr;

// Log to own file in game working directory - no dependencies, no conflicts
static void Log(const char* fmt, ...)
{
    FILE* f = nullptr;
    fopen_s(&f, "TestEventPlugin.log", "a");
    if (f)
    {
        va_list args;
        va_start(args, fmt);
        vfprintf(f, fmt, args);
        va_end(args);
        fprintf(f, "\n");
        fflush(f);
        fclose(f);
    }
}

// Event handler callbacks
void OnHitHandler(void** params, void* context)
{
    Log("OnHit fired! source=%08X target=%08X", params ? params[0] : 0, params ? params[1] : 0);
}

void OnDeathHandler(void** params, void* context)
{
    Log("OnDeath fired! source=%08X target=%08X", params ? params[0] : 0, params ? params[1] : 0);
}

void OnLoadHandler(void** params, void* context)
{
    Log("OnLoad fired! source=%08X target=%08X", params ? params[0] : 0, params ? params[1] : 0);
}

void OnEquipHandler(void** params, void* context)
{
    Log("OnEquip fired! source=%08X target=%08X", params ? params[0] : 0, params ? params[1] : 0);
}

extern "C" __declspec(dllexport) bool FOSEPlugin_Query(const FOSEInterface* fose, PluginInfo* info)
{
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name = g_pluginName;
    info->version = g_pluginVersion;
    return true;
}

extern "C" __declspec(dllexport) bool FOSEPlugin_Load(const FOSEInterface* fose)
{
    Log("FOSEPlugin_Load called");

    g_eventManager = (FOSEEventManagerInterface*)fose->QueryInterface(kInterface_EventManager);
    if (!g_eventManager)
    {
        Log("ERROR: QueryInterface for EventManager returned NULL");
        return false;
    }
    Log("EventManager interface obtained");

    g_dataInterface = fose->QueryInterface(kInterface_Data);
    if (!g_dataInterface)
    {
        Log("WARNING: QueryInterface for DataInterface returned NULL (may not be available)");
    }
    else
    {
        Log("DataInterface interface obtained");
        // Cast to access version field (first field in struct)
        UInt32 version = *((UInt32*)g_dataInterface);
        Log("DataInterface version=%d", version);

        // Get the function pointer for GetSingleton (second field in struct)
        void* (*GetSingletonFunc)(UInt32) = *((void* (**)(UInt32))((char*)g_dataInterface + 4));

        // Test GetSingleton with ArrayMap
        void* arrayMap = GetSingletonFunc(1); // kFOSEData_ArrayMap = 1
        Log("DataInterface GetSingleton(ArrayMap) returned: %08X", arrayMap);

        // Test GetSingleton with StringMap
        void* stringMap = GetSingletonFunc(2); // kFOSEData_StringMap = 2
        Log("DataInterface GetSingleton(StringMap) returned: %08X", stringMap);

        // Test GetFunc for FormExtraData functions
        void* (*GetFuncFunc)(UInt32) = *((void* (**)(UInt32))((char*)g_dataInterface + 8));

        void* formExtraDataGet = GetFuncFunc(1); // kFOSEData_FormExtraDataGet = 1
        Log("DataInterface GetFunc(FormExtraDataGet) returned: %08X", formExtraDataGet);

        void* formExtraDataAdd = GetFuncFunc(3); // kFOSEData_FormExtraDataAdd = 3
        Log("DataInterface GetFunc(FormExtraDataAdd) returned: %08X", formExtraDataAdd);

        // Test GetFunc for new FormExtraData functions
        void* formExtraDataCount = GetFuncFunc(6); // kFOSEData_FormExtraDataCount = 6
        Log("DataInterface GetFunc(FormExtraDataCount) returned: %08X", formExtraDataCount);

        void* formExtraDataExists = GetFuncFunc(7); // kFOSEData_FormExtraDataExists = 7
        Log("DataInterface GetFunc(FormExtraDataExists) returned: %08X", formExtraDataExists);

        void* formExtraDataIterate = GetFuncFunc(8); // kFOSEData_FormExtraDataIterate = 8
        Log("DataInterface GetFunc(FormExtraDataIterate) returned: %08X", formExtraDataIterate);

        // Test GetFunc for cleanup functions
        void* arrayVarMapDeleteBySelf = GetFuncFunc(9); // kFOSEData_ArrayVarMapDeleteBySelf = 9
        Log("DataInterface GetFunc(ArrayVarMapDeleteBySelf) returned: %08X", arrayVarMapDeleteBySelf);

        void* stringVarMapDeleteBySelf = GetFuncFunc(10); // kFOSEData_StringVarMapDeleteBySelf = 10
        Log("DataInterface GetFunc(StringVarMapDeleteBySelf) returned: %08X", stringVarMapDeleteBySelf);

        // Test GetFunc for new ArrayVarManager functions
        void* arrayVarInsertAt = GetFuncFunc(11); // kFOSEData_ArrayVarInsertAt = 11
        Log("DataInterface GetFunc(ArrayVarInsertAt) returned: %08X", arrayVarInsertAt);

        void* arrayVarFindByValue = GetFuncFunc(12); // kFOSEData_ArrayVarFindByValue = 12
        Log("DataInterface GetFunc(ArrayVarFindByValue) returned: %08X", arrayVarFindByValue);

        void* arrayVarCountByType = GetFuncFunc(13); // kFOSEData_ArrayVarCountByType = 13
        Log("DataInterface GetFunc(ArrayVarCountByType) returned: %08X", arrayVarCountByType);

        // Test GetFunc for new StringVarManager functions
        void* stringVarGetLength = GetFuncFunc(14); // kFOSEData_StringVarGetLength = 14
        Log("DataInterface GetFunc(StringVarGetLength) returned: %08X", stringVarGetLength);

        void* stringVarCompare = GetFuncFunc(15); // kFOSEData_StringVarCompare = 15
        Log("DataInterface GetFunc(StringVarCompare) returned: %08X", stringVarCompare);

        void* stringVarConcatenate = GetFuncFunc(16); // kFOSEData_StringVarConcatenate = 16
        Log("DataInterface GetFunc(StringVarConcatenate) returned: %08X", stringVarConcatenate);

        void* stringVarSubstring = GetFuncFunc(17); // kFOSEData_StringVarSubstring = 17
        Log("DataInterface GetFunc(StringVarSubstring) returned: %08X", stringVarSubstring);

        // Test GetFunc for new ArrayVarManager medium enhancement functions
        void* arrayVarSort = GetFuncFunc(18); // kFOSEData_ArrayVarSort = 18
        Log("DataInterface GetFunc(ArrayVarSort) returned: %08X", arrayVarSort);

        void* arrayVarReverse = GetFuncFunc(19); // kFOSEData_ArrayVarReverse = 19
        Log("DataInterface GetFunc(ArrayVarReverse) returned: %08X", arrayVarReverse);

        void* arrayVarShuffle = GetFuncFunc(20); // kFOSEData_ArrayVarShuffle = 20
        Log("DataInterface GetFunc(ArrayVarShuffle) returned: %08X", arrayVarShuffle);

        // Test GetFunc for new StringVarManager medium enhancement functions
        void* stringVarUppercase = GetFuncFunc(21); // kFOSEData_StringVarUppercase = 21
        Log("DataInterface GetFunc(StringVarUppercase) returned: %08X", stringVarUppercase);

        void* stringVarLowercase = GetFuncFunc(22); // kFOSEData_StringVarLowercase = 22
        Log("DataInterface GetFunc(StringVarLowercase) returned: %08X", stringVarLowercase);

        void* stringVarTrim = GetFuncFunc(23); // kFOSEData_StringVarTrim = 23
        Log("DataInterface GetFunc(StringVarTrim) returned: %08X", stringVarTrim);

        void* stringVarReplace = GetFuncFunc(24); // kFOSEData_StringVarReplace = 24
        Log("DataInterface GetFunc(StringVarReplace) returned: %08X", stringVarReplace);
    }

    bool r1 = g_eventManager->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0);
    bool r2 = g_eventManager->RegisterEventHandler("OnDeath", OnDeathHandler, nullptr, 0);
    bool r3 = g_eventManager->RegisterEventHandler("OnLoad", OnLoadHandler, nullptr, 0);
    bool r4 = g_eventManager->RegisterEventHandler("OnEquip", OnEquipHandler, nullptr, 0);

    Log("RegisterEventHandler results: OnHit=%d OnDeath=%d OnLoad=%d OnEquip=%d", r1, r2, r3, r4);

    return true;
}
