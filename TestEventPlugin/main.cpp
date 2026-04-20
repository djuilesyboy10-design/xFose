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

void OnKeyDownHandler(void** params, void* context)
{
    UInt32 keycode = params ? (UInt32)params[0] : 0;
    Log("OnKeyDown fired! keycode=%d (0x%02X)", keycode, keycode);
}

void OnKeyUpHandler(void** params, void* context)
{
    UInt32 keycode = params ? (UInt32)params[0] : 0;
    Log("OnKeyUp fired! keycode=%d (0x%02X)", keycode, keycode);
}

void OnKeyPressHandler(void** params, void* context)
{
    UInt32 keycode = params ? (UInt32)params[0] : 0;
    Log("OnKeyPress fired! keycode=%d (0x%02X)", keycode, keycode);
}

// Phase 1 test handlers
void OnLoadGameHandler(void** params, void* context)
{
    Log("OnLoadGame fired!");
}

void OnSaveGameHandler(void** params, void* context)
{
    Log("OnSaveGame fired!");
}

void OnExitGameHandler(void** params, void* context)
{
    Log("OnExitGame fired!");
}

void OnExitToMainMenuHandler(void** params, void* context)
{
    Log("OnExitToMainMenu fired!");
}

void OnNewGameHandler(void** params, void* context)
{
    Log("OnNewGame fired!");
}

void OnDeleteGameHandler(void** params, void* context)
{
    Log("OnDeleteGame fired!");
}

void OnRenameGameHandler(void** params, void* context)
{
    Log("OnRenameGame fired!");
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

        // Test GetFunc for new Map/StringMap functions
        void* arrayVarCreateMap = GetFuncFunc(25); // kFOSEData_ArrayVarCreateMap = 25
        Log("DataInterface GetFunc(ArrayVarCreateMap) returned: %08X", arrayVarCreateMap);

        void* arrayVarCreateStringMap = GetFuncFunc(26); // kFOSEData_ArrayVarCreateStringMap = 26
        Log("DataInterface GetFunc(ArrayVarCreateStringMap) returned: %08X", arrayVarCreateStringMap);

        void* arrayVarGetContainerType = GetFuncFunc(27); // kFOSEData_ArrayVarGetContainerType = 27
        Log("DataInterface GetFunc(ArrayVarGetContainerType) returned: %08X", arrayVarGetContainerType);

        void* arrayVarHasKey = GetFuncFunc(28); // kFOSEData_ArrayVarHasKey = 28
        Log("DataInterface GetFunc(ArrayVarHasKey) returned: %08X", arrayVarHasKey);

        void* arrayVarSetElementByKey = GetFuncFunc(29); // kFOSEData_ArrayVarSetElementByKey = 29
        Log("DataInterface GetFunc(ArrayVarSetElementByKey) returned: %08X", arrayVarSetElementByKey);

        void* arrayVarGetElementByKey = GetFuncFunc(30); // kFOSEData_ArrayVarGetElementByKey = 30
        Log("DataInterface GetFunc(ArrayVarGetElementByKey) returned: %08X", arrayVarGetElementByKey);

        void* arrayVarRemoveByKey = GetFuncFunc(31); // kFOSEData_ArrayVarRemoveByKey = 31
        Log("DataInterface GetFunc(ArrayVarRemoveByKey) returned: %08X", arrayVarRemoveByKey);

        // Test GetSingleton for LoggingManager
        void* loggingManager = GetSingletonFunc(3); // kFOSEData_LoggingManager = 3
        Log("DataInterface GetSingleton(LoggingManager) returned: %08X", loggingManager);

        // Test GetFunc for Logging functions
        void* loggingLog = GetFuncFunc(32); // kFOSEData_LoggingLog = 32
        Log("DataInterface GetFunc(LoggingLog) returned: %08X", loggingLog);

        void* loggingLogInfo = GetFuncFunc(33); // kFOSEData_LoggingLogInfo = 33
        Log("DataInterface GetFunc(LoggingLogInfo) returned: %08X", loggingLogInfo);

        void* loggingLogWarning = GetFuncFunc(34); // kFOSEData_LoggingLogWarning = 34
        Log("DataInterface GetFunc(LoggingLogWarning) returned: %08X", loggingLogWarning);

        void* loggingLogError = GetFuncFunc(35); // kFOSEData_LoggingLogError = 35
        Log("DataInterface GetFunc(LoggingLogError) returned: %08X", loggingLogError);

        // Test logging functions
        if (loggingManager && loggingLogInfo)
        {
            void (*LogInfoFunc)(void*, const char*) = (void (*)(void*, const char*))loggingLogInfo;
            LogInfoFunc(loggingManager, "TestEventPlugin: Logging test - Info message");
        }
        if (loggingManager && loggingLogWarning)
        {
            void (*LogWarningFunc)(void*, const char*) = (void (*)(void*, const char*))loggingLogWarning;
            LogWarningFunc(loggingManager, "TestEventPlugin: Logging test - Warning message");
        }
        if (loggingManager && loggingLogError)
        {
            void (*LogErrorFunc)(void*, const char*) = (void (*)(void*, const char*))loggingLogError;
            LogErrorFunc(loggingManager, "TestEventPlugin: Logging test - Error message");
        }

        // Test GetSingleton for PlayerControlsManager
        void* playerControlsManager = GetSingletonFunc(4); // kFOSEData_PlayerControlsManager = 4
        Log("DataInterface GetSingleton(PlayerControlsManager) returned: %08X", playerControlsManager);

        // Test GetFunc for PlayerControls functions
        void* pcIsKeyPressed = GetFuncFunc(36); // kFOSEData_PlayerControlsIsKeyPressed = 36
        Log("DataInterface GetFunc(PlayerControlsIsKeyPressed) returned: %08X", pcIsKeyPressed);

        void* pcTapKey = GetFuncFunc(37); // kFOSEData_PlayerControlsTapKey = 37
        Log("DataInterface GetFunc(PlayerControlsTapKey) returned: %08X", pcTapKey);

        void* pcHoldKey = GetFuncFunc(38); // kFOSEData_PlayerControlsHoldKey = 38
        Log("DataInterface GetFunc(PlayerControlsHoldKey) returned: %08X", pcHoldKey);

        void* pcReleaseKey = GetFuncFunc(39); // kFOSEData_PlayerControlsReleaseKey = 39
        Log("DataInterface GetFunc(PlayerControlsReleaseKey) returned: %08X", pcReleaseKey);

        void* pcIsControlPressed = GetFuncFunc(45); // kFOSEData_PlayerControlsIsControlPressed = 45
        Log("DataInterface GetFunc(PlayerControlsIsControlPressed) returned: %08X", pcIsControlPressed);

        // Test PlayerControls functions
        if (playerControlsManager && pcIsKeyPressed)
        {
            void (*IsKeyPressedFunc)(void*, UInt32, bool*) = (void (*)(void*, UInt32, bool*))pcIsKeyPressed;
            bool pressed = false;
            IsKeyPressedFunc(playerControlsManager, 0x57, &pressed); // Check if 'W' key is pressed
            Log("PlayerControls: IsKeyPressed('W') = %d", pressed);
        }
        if (playerControlsManager && pcIsControlPressed)
        {
            void (*IsControlPressedFunc)(void*, UInt32, bool*) = (void (*)(void*, UInt32, bool*))pcIsControlPressed;
            bool pressed = false;
            IsControlPressedFunc(playerControlsManager, 0, &pressed); // Check if forward control is pressed
            Log("PlayerControls: IsControlPressed(Forward) = %d", pressed);
        }
    }

    bool r1 = g_eventManager->RegisterEventHandler("OnHit", OnHitHandler, nullptr, 0, nullptr, nullptr);
    bool r2 = g_eventManager->RegisterEventHandler("OnDeath", OnDeathHandler, nullptr, 0, nullptr, nullptr);
    bool r3 = g_eventManager->RegisterEventHandler("OnLoad", OnLoadHandler, nullptr, 0, nullptr, nullptr);
    bool r4 = g_eventManager->RegisterEventHandler("OnEquip", OnEquipHandler, nullptr, 0, nullptr, nullptr);

    // Register input event handlers
    bool r5 = g_eventManager->RegisterEventHandler("OnKeyDown", OnKeyDownHandler, nullptr, 0, nullptr, nullptr);
    bool r6 = g_eventManager->RegisterEventHandler("OnKeyUp", OnKeyUpHandler, nullptr, 0, nullptr, nullptr);
    bool r7 = g_eventManager->RegisterEventHandler("OnKeyPress", OnKeyPressHandler, nullptr, 0, nullptr, nullptr);

    // Register Phase 1 test handlers with plugin/handler names for debug info
    bool r8 = g_eventManager->RegisterEventHandler("OnLoadGame", OnLoadGameHandler, nullptr, 0, "TestEventPlugin", "OnLoadGameHandler");
    bool r9 = g_eventManager->RegisterEventHandler("OnSaveGame", OnSaveGameHandler, nullptr, 0, "TestEventPlugin", "OnSaveGameHandler");
    bool r10 = g_eventManager->RegisterEventHandler("OnExitGame", OnExitGameHandler, nullptr, 0, "TestEventPlugin", "OnExitGameHandler");
    bool r11 = g_eventManager->RegisterEventHandler("OnExitToMainMenu", OnExitToMainMenuHandler, nullptr, 0, "TestEventPlugin", "OnExitToMainMenuHandler");
    bool r12 = g_eventManager->RegisterEventHandler("OnNewGame", OnNewGameHandler, nullptr, 0, "TestEventPlugin", "OnNewGameHandler");
    bool r13 = g_eventManager->RegisterEventHandler("OnDeleteGame", OnDeleteGameHandler, nullptr, 0, "TestEventPlugin", "OnDeleteGameHandler");
    bool r14 = g_eventManager->RegisterEventHandler("OnRenameGame", OnRenameGameHandler, nullptr, 0, "TestEventPlugin", "OnRenameGameHandler");

    Log("RegisterEventHandler results: OnHit=%d OnDeath=%d OnLoad=%d OnEquip=%d OnKeyDown=%d OnKeyUp=%d OnKeyPress=%d", r1, r2, r3, r4, r5, r6, r7);
    Log("Phase 1 event handlers: OnLoadGame=%d OnSaveGame=%d OnExitGame=%d OnExitToMainMenu=%d OnNewGame=%d OnDeleteGame=%d OnRenameGame=%d", r8, r9, r10, r11, r12, r13, r14);

    return true;
}
