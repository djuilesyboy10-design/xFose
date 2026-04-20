#include "PlayerControlsManager.h"
#include "Hooks_DirectInput8Create.h"
#include "GameOSDepend.h"
#include "GameAPI.h"

PlayerControlsManager* PlayerControlsManager::s_singleton = nullptr;

PlayerControlsManager::PlayerControlsManager()
{
}

PlayerControlsManager::~PlayerControlsManager()
{
}

PlayerControlsManager* PlayerControlsManager::GetSingleton()
{
	if (!s_singleton)
		s_singleton = new PlayerControlsManager();
	return s_singleton;
}

bool PlayerControlsManager::IsKeyPressed(UInt32 keycode)
{
	return DI_data.IsKeyPressed(keycode);
}

void PlayerControlsManager::TapKey(UInt32 keycode)
{
	DI_data.TapKey(keycode);
}

void PlayerControlsManager::HoldKey(UInt32 keycode)
{
	DI_data.SetKeyHeldState(keycode, true);
}

void PlayerControlsManager::ReleaseKey(UInt32 keycode)
{
	DI_data.SetKeyHeldState(keycode, false);
}

void PlayerControlsManager::DisableKey(UInt32 keycode)
{
	DI_data.SetKeyDisableState(keycode, true);
}

void PlayerControlsManager::EnableKey(UInt32 keycode)
{
	DI_data.SetKeyDisableState(keycode, false);
}

bool PlayerControlsManager::IsKeyDisabled(UInt32 keycode)
{
	return DI_data.IsKeyDisabled(keycode);
}

UInt32 PlayerControlsManager::GetControl(UInt32 whichControl, ControlType type)
{
	OSInputGlobals* globs = *g_OSInputGlobals;
	if (!globs || whichControl >= globs->kMaxControlBinds)
		return 0xFF;

	switch (type)
	{
	case kControlType_Keyboard:
		return globs->keyBinds[whichControl];
	case kControlType_Mouse:
		{
			UInt32 button = globs->mouseBinds[whichControl];
			if (button != 0xFF)
				button += 0x100;
			return button;
		}
	case kControlType_Joystick:
		return globs->joystickBinds[whichControl];
	default:
		return 0xFF;
	}
}

void PlayerControlsManager::SetControl(UInt32 whichControl, ControlType type, UInt32 keycode)
{
	OSInputGlobals* globs = *g_OSInputGlobals;
	if (!globs || whichControl >= globs->kMaxControlBinds)
		return;

	UInt8* binds = (type == kControlType_Mouse) ? globs->mouseBinds : globs->keyBinds;
	keycode = (keycode >= 0x100) ? keycode - 0x100 : keycode;

	// if specified key already used by another control, swap with the new one
	for (UInt32 i = 0; i < OSInputGlobals::kMaxControlBinds; i++)
	{
		if (binds[i] == keycode)
		{
			binds[i] = binds[whichControl];
			break;
		}
	}

	binds[whichControl] = keycode;
}

bool PlayerControlsManager::IsControl(UInt32 key)
{
	OSInputGlobals* globs = *g_OSInputGlobals;
	if (!globs)
		return false;

	UInt8* binds = key >= 0x100 ? globs->mouseBinds : globs->keyBinds;
	key = key >= 0x100 ? key - 0x100 : key;

	for (UInt32 i = 0; i < OSInputGlobals::kMaxControlBinds; i++)
		if (binds[i] == key)
			return true;

	return false;
}

bool PlayerControlsManager::IsControlPressed(UInt32 controlIndex)
{
	UInt32 keycode = GetControl(controlIndex, kControlType_Keyboard);
	if (keycode != 0xFF && DI_data.IsKeyPressed(keycode))
		return true;
	else
	{
		keycode = GetControl(controlIndex, kControlType_Mouse);
		if (keycode != 0xFF && DI_data.IsKeyPressed(keycode))
			return true;
	}
	return false;
}

UInt32 PlayerControlsManager::GetNumMouseButtonsPressed()
{
	UInt32 count = 0;
	for (UInt32 d = 256; d < 0x1000; d++)
		if (DI_data.IsKeyPressed(d))
			count++;
	return count;
}

UInt32 PlayerControlsManager::GetMouseButtonPress(UInt32 index)
{
	UInt32 count = 0;
	for (UInt32 d = 256; d < 0x1000; d++)
		if (DI_data.IsKeyPressed(d) && (!count--))
			return d;
	return 0xFFFFFFFF;
}

// Static wrapper functions for DataInterface
void PlayerControlsManager::Static_IsKeyPressed(void* instance, UInt32 keycode, bool* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->IsKeyPressed(keycode);
}

void PlayerControlsManager::Static_TapKey(void* instance, UInt32 keycode)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr)
		mgr->TapKey(keycode);
}

void PlayerControlsManager::Static_HoldKey(void* instance, UInt32 keycode)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr)
		mgr->HoldKey(keycode);
}

void PlayerControlsManager::Static_ReleaseKey(void* instance, UInt32 keycode)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr)
		mgr->ReleaseKey(keycode);
}

void PlayerControlsManager::Static_DisableKey(void* instance, UInt32 keycode)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr)
		mgr->DisableKey(keycode);
}

void PlayerControlsManager::Static_EnableKey(void* instance, UInt32 keycode)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr)
		mgr->EnableKey(keycode);
}

void PlayerControlsManager::Static_IsKeyDisabled(void* instance, UInt32 keycode, bool* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->IsKeyDisabled(keycode);
}

void PlayerControlsManager::Static_GetControl(void* instance, UInt32 whichControl, ControlType type, UInt32* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->GetControl(whichControl, type);
}

void PlayerControlsManager::Static_SetControl(void* instance, UInt32 whichControl, ControlType type, UInt32 keycode)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr)
		mgr->SetControl(whichControl, type, keycode);
}

void PlayerControlsManager::Static_IsControl(void* instance, UInt32 key, bool* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->IsControl(key);
}

void PlayerControlsManager::Static_IsControlPressed(void* instance, UInt32 controlIndex, bool* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->IsControlPressed(controlIndex);
}

void PlayerControlsManager::Static_GetNumMouseButtonsPressed(void* instance, UInt32* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->GetNumMouseButtonsPressed();
}

void PlayerControlsManager::Static_GetMouseButtonPress(void* instance, UInt32 index, UInt32* result)
{
	PlayerControlsManager* mgr = static_cast<PlayerControlsManager*>(instance);
	if (mgr && result)
		*result = mgr->GetMouseButtonPress(index);
}
