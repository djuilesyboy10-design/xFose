#ifndef PLAYER_CONTROLS_MANAGER_H
#define PLAYER_CONTROLS_MANAGER_H

class PlayerControlsManager
{
public:
	enum ControlType
	{
		kControlType_Keyboard = 0,
		kControlType_Mouse = 1,
		kControlType_Joystick = 2
	};

	// Singleton access
	static PlayerControlsManager* GetSingleton();

	// Key state functions
	bool IsKeyPressed(UInt32 keycode);
	void TapKey(UInt32 keycode);
	void HoldKey(UInt32 keycode);
	void ReleaseKey(UInt32 keycode);
	void DisableKey(UInt32 keycode);
	void EnableKey(UInt32 keycode);
	bool IsKeyDisabled(UInt32 keycode);

	// Control binding functions
	UInt32 GetControl(UInt32 whichControl, ControlType type);
	void SetControl(UInt32 whichControl, ControlType type, UInt32 keycode);
	bool IsControl(UInt32 key);
	bool IsControlPressed(UInt32 controlIndex);

	// Mouse functions
	UInt32 GetNumMouseButtonsPressed();
	UInt32 GetMouseButtonPress(UInt32 index);

private:
	PlayerControlsManager();
	~PlayerControlsManager();

	// Static wrapper functions for DataInterface
	static void Static_IsKeyPressed(void* instance, UInt32 keycode, bool* result);
	static void Static_TapKey(void* instance, UInt32 keycode);
	static void Static_HoldKey(void* instance, UInt32 keycode);
	static void Static_ReleaseKey(void* instance, UInt32 keycode);
	static void Static_DisableKey(void* instance, UInt32 keycode);
	static void Static_EnableKey(void* instance, UInt32 keycode);
	static void Static_IsKeyDisabled(void* instance, UInt32 keycode, bool* result);
	static void Static_GetControl(void* instance, UInt32 whichControl, ControlType type, UInt32* result);
	static void Static_SetControl(void* instance, UInt32 whichControl, ControlType type, UInt32 keycode);
	static void Static_IsControl(void* instance, UInt32 key, bool* result);
	static void Static_IsControlPressed(void* instance, UInt32 controlIndex, bool* result);
	static void Static_GetNumMouseButtonsPressed(void* instance, UInt32* result);
	static void Static_GetMouseButtonPress(void* instance, UInt32 index, UInt32* result);

	static PlayerControlsManager* s_singleton;
};

#endif // PLAYER_CONTROLS_MANAGER_H
