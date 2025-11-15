#pragma once
#include "framework.h"

class Gamepad
{
public:
	enum ButtonKind
	{
		UpButton,
		DownButton,
		LeftButton,
		RightButton,
		Button01,
		Button02,
		ButtonKindMax,
	};

	enum ButtonState
	{
		ButtonStateNone,
		ButtonStateDown,
		ButtonStatePush,
		ButtonStateUp,
		ButtonStateMax,
	};
	ButtonState buttonStates_[ButtonKind::ButtonKindMax];

	// DIRECTINPUT8のポインタ
	LPDIRECTINPUT8 pInterface_ = nullptr;

	// DIRECTINPUTDEVICE8のポインタ
	LPDIRECTINPUTDEVICE8 pDevice_ = nullptr;
	struct InputDevice
	{
		LPDIRECTINPUTDEVICE8 pDevice;
		WCHAR	tszInstanceName[MAX_PATH];
		GUID	guidInstance;
	};
	std::vector<InputDevice> inputDevises_;
	
	DIJOYSTATE pad_data = {};
	bool is_push[ButtonKind::ButtonKindMax] = {};
	HWND hWnd_ = nullptr;

	struct DeviceEnumParameter
	{
		LPDIRECTINPUTDEVICE8* GamePadDevice;
		int FindCount;
	};
	DeviceEnumParameter parameter_;

public:
	Gamepad();
	~Gamepad();

	HRESULT Create(HINSTANCE hinst, HWND hwnd);
	bool CreateInterface(HINSTANCE hinst);
	bool CreateGamePadDevice(HINSTANCE hinst);
	bool StartGamePadControl();
	bool SetUpCooperativeLevel(LPDIRECTINPUTDEVICE8 device);
	bool SetUpGamePadProperty(LPDIRECTINPUTDEVICE8 device);
	static BOOL CALLBACK DeviceFindCallBackProxy(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);
	bool DeviceFindCallBack(LPCDIDEVICEINSTANCE lpddi);
	void Release();
	bool Update();
};

