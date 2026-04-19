#include "fose/PluginAPI.h"
#include "fose/fose/GameInterface.h"
#include "fose/fose/GameTiles.h"

static float fErrorReturnValue = -999.0f;

static float UI_GetUIFloat(const char* componentPath)
{
	Tile::Value* val = InterfaceManager::GetMenuComponentValue((char*)componentPath);
	if (val)
		return val->num;
	return fErrorReturnValue;
}

static bool UI_SetUIFloat(const char* componentPath, float value)
{
	Tile::Value* val = InterfaceManager::GetMenuComponentValue((char*)componentPath);
	if (val)
	{
		CALL_MEMBER_FN(val->parent, SetFloatValue)(val->id, value, true);
		return true;
	}
	return false;
}

static bool UI_SetUIString(const char* componentPath, const char* value)
{
	Tile::Value* val = InterfaceManager::GetMenuComponentValue((char*)componentPath);
	if (val)
	{
		CALL_MEMBER_FN(val->parent, SetStringValue)(val->id, value, true);
		return true;
	}
	return false;
}

static FOSEUIManagerInterface g_UIInterface = {
	FOSEUIManagerInterface::kVersion,
	UI_GetUIFloat,
	UI_SetUIFloat,
	UI_SetUIString
};

FOSEUIManagerInterface* GetUIInterface()
{
	return &g_UIInterface;
}
