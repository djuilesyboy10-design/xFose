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

    bool r1 = g_eventManager->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0);
    bool r2 = g_eventManager->RegisterEventHandler("OnDeath", OnDeathHandler, nullptr, 0);
    bool r3 = g_eventManager->RegisterEventHandler("OnLoad", OnLoadHandler, nullptr, 0);
    bool r4 = g_eventManager->RegisterEventHandler("OnEquip", OnEquipHandler, nullptr, 0);

    Log("RegisterEventHandler results: OnHit=%d OnDeath=%d OnLoad=%d OnEquip=%d", r1, r2, r3, r4);

    return true;
}
