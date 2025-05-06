#include "pch.h"
#include "Gamepad.h"

Gamepad::Gamepad()
{
}

Gamepad::~Gamepad()
{
}

HRESULT Gamepad::Create(HINSTANCE hinst, HWND hwnd)
{
	hWnd_ = hwnd;
	// インターフェース作成
	if (!CreateInterface(hinst))
	{
		return false;
	}

	// デバイス作成
	if (!CreateGamePadDevice(hinst))
	{
		Release();
		return false;
	}

	// 入力情報の初期化
	for (int i = 0; i < ButtonKind::ButtonKindMax; i++)
	{
		buttonStates_[i] = ButtonState::ButtonStateNone;
	}

	return S_OK;
}

bool Gamepad::CreateInterface(HINSTANCE hinst)
{
	HRESULT hr = S_OK;

	// インターフェース作成
	hr = DirectInput8Create(
		hinst,						// インスタンスハンドル
		DIRECTINPUT_VERSION,		// DirectInputのバージョン
		IID_IDirectInput8,			// 使用する機能
		(void**)&pInterface_,		// 作成されたインターフェース代入用
		NULL);						// NULL固定

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Gamepad::CreateGamePadDevice(HINSTANCE hinst)
{
	parameter_.FindCount = 0;
	parameter_.GamePadDevice = &pDevice_;

	// GAMEPADを調べる
//	TRACE("DI8DEVTYPE_GAMEPAD{\n");
	pInterface_->EnumDevices(
		DI8DEVTYPE_GAMEPAD,			// 検索するデバイスの種類
		DeviceFindCallBackProxy,			// 発見時に実行する関数
		this,						// 関数に渡す値
		DIEDFL_ATTACHEDONLY			// 検索方法
	);
//	TRACE("}=%d\n", inputDevises_.size());

	// JOYSTICKを調べる
//	TRACE("DI8DEVTYPE_JOYSTICK{\n");
	pInterface_->EnumDevices(
		DI8DEVTYPE_JOYSTICK,
		DeviceFindCallBackProxy,
		this,
		DIEDFL_ATTACHEDONLY
	);
//	TRACE("}=%d\n", inputDevises_.size());

	// どちらも見つけることが出来なかったら失敗
	//if (parameter_.FindCount == 0)
	if (inputDevises_.size() == 0)
	{
		return false;
	}

	int count = 0;
	// 制御開始
	while (StartGamePadControl() == false)
	{
		Sleep(100);
		count++;
		if (count >= 5)
		{
			break;
		}
	}

	return true;
}

bool Gamepad::SetUpCooperativeLevel(LPDIRECTINPUTDEVICE8 device)
{
	HRESULT hr = S_OK;
	//協調モードの設定
	hr = device->SetCooperativeLevel(hWnd_, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
//	hr = device->SetCooperativeLevel(hWnd_, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

bool Gamepad::StartGamePadControl()
{
	HRESULT hr = S_OK;
	if (inputDevises_.size() == 0)
	{
		return false;
	}
	auto device = inputDevises_[0].pDevice;
	//// デバイスが生成されてない
	//if (pDevice_ == nullptr)
	//{
	//	return false;
	//}

	// 制御開始
	hr = device->Acquire();
	if (FAILED(hr))
	{
		return false;
	}

	DIDEVCAPS cap;
	device->GetCapabilities(&cap);
	// ポーリング判定
	if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
	{
		DWORD error = GetLastError();
		// ポーリング開始
		/*
			PollはAcquireの前に行うとされていたが、
			Acquire の前で実行すると失敗したので
			後で実行するようにした
		*/
		if (FAILED(device->Poll()))
		{
			return false;
		}
	}

	return true;
}

bool Gamepad::SetUpGamePadProperty(LPDIRECTINPUTDEVICE8 device)
{
	// 軸モードを絶対値モードとして設定
	DIPROPDWORD diprop;
	ZeroMemory(&diprop, sizeof(diprop));
	diprop.diph.dwSize = sizeof(diprop);
	diprop.diph.dwHeaderSize = sizeof(diprop.diph);
	diprop.diph.dwHow = DIPH_DEVICE;
	diprop.diph.dwObj = 0;
	diprop.dwData = DIPROPAXISMODE_ABS;
	if (FAILED(device->SetProperty(DIPROP_AXISMODE, &diprop.diph)))
	{
		return false;
	}

	// X軸の値の範囲設定
	DIPROPRANGE diprg;
	ZeroMemory(&diprg, sizeof(diprg));
	diprg.diph.dwSize = sizeof(diprg);
	diprg.diph.dwHeaderSize = sizeof(diprg.diph);
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.diph.dwObj = DIJOFS_X;
	diprg.lMin = -1000;
	diprg.lMax = 1000;
	if (FAILED(device->SetProperty(DIPROP_RANGE, &diprg.diph)))
	{
		return false;
	}

	// Y軸の値の範囲設定
	diprg.diph.dwObj = DIJOFS_Y;
	if (FAILED(device->SetProperty(DIPROP_RANGE, &diprg.diph)))
	{
		return false;
	}

	return true;
}

/*static*/ BOOL CALLBACK Gamepad::DeviceFindCallBackProxy(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef)
{
	Gamepad* T = reinterpret_cast<Gamepad*>(pvRef);
	return T->DeviceFindCallBack(lpddi);
}

bool Gamepad::DeviceFindCallBack(LPCDIDEVICEINSTANCE lpddi)
{
	LPDIRECTINPUTDEVICE8 device = nullptr;
	TRACE("%S\n", lpddi->tszInstanceName);

	//// 既に発見しているなら終了
	//if (parameter_.FindCount >= 1)
	//{
	//	return DIENUM_STOP;
	//}
	InputDevice dev = {};
	dev.guidInstance = lpddi->guidInstance;
	lstrcpy(dev.tszInstanceName, lpddi->tszInstanceName);

	// デバイス生成
	HRESULT hr = pInterface_->CreateDevice(
		//GUID_Joystick,
		lpddi->guidInstance,
		//parameter_.GamePadDevice,
		&dev.pDevice,
		NULL);

	if (FAILED(hr))
	{
		return DIENUM_STOP;
	}
	*parameter_.GamePadDevice = dev.pDevice;

	TRACE("push_back(%S)\n", dev.tszInstanceName);
	inputDevises_.push_back(dev);

	// 入力フォーマットの指定
	device = *parameter_.GamePadDevice;
	hr = device->SetDataFormat(&c_dfDIJoystick);

	if (FAILED(hr))
	{
		return DIENUM_STOP;
	}

	// プロパティの設定
	if (SetUpGamePadProperty(device) == false)
	{
		return DIENUM_STOP;
	}

	// 協調レベルの設定
	if (SetUpCooperativeLevel(device) == false)
	{
		return DIENUM_STOP;
	}

	// 発見数をカウント
	parameter_.FindCount++;

	return DIENUM_CONTINUE;
}


void Gamepad::Release()
{
	for (auto& i : inputDevises_)
	{
		i.pDevice->Release();
		i.pDevice = nullptr;
	}
	pInterface_->Release();
	pInterface_ = nullptr;
}

bool Gamepad::Update()
{
	if (inputDevises_.size() == 0)
	{
		return false;
	}
	auto device = inputDevises_[0].pDevice;
	if (device == nullptr)
	{
		return false;
	}
	
	// デバイス取得
	HRESULT hr = device->GetDeviceState(sizeof(DIJOYSTATE), &pad_data);
	if (FAILED(hr))
	{
		// 再度制御開始
		if (FAILED(device->Acquire()))
		{
			for (int i = 0; i < ButtonKind::ButtonKindMax; i++)
			{
				buttonStates_[i] = ButtonState::ButtonStateNone;
			}
			device->Poll();
		}
		return false;
	}

	// スティック判定
//	TRACE("%d %d %d\n", pad_data.lX, pad_data.lY, pad_data.lZ);
	int unresponsive_range = 200;
	if (pad_data.lX < -unresponsive_range)
	{
		is_push[ButtonKind::LeftButton] = true;
	}
	else if (pad_data.lX > unresponsive_range)
	{
		is_push[ButtonKind::RightButton] = true;
	}

	if (pad_data.lY < -unresponsive_range)
	{
		is_push[ButtonKind::UpButton] = true;
	}
	else if (pad_data.lY > unresponsive_range)
	{
		is_push[ButtonKind::DownButton] = true;
	}

	// 十字キー判定
	if (pad_data.rgdwPOV[0] != 0xFFFFFFFF)
	{
		float rad = (pad_data.rgdwPOV[0] / 100.0f)*3.14f/180.0f;
		// 本来はxがcos、yがsinだけど、rgdwPOVは0が上から始まるので、
		// cosとsinを逆にした方が都合がいい
		float x = sinf(rad);
		float y = cosf(rad);

		if (x < -0.01f)
		{
			is_push[ButtonKind::LeftButton] = true;
		}
		else if (x > 0.01f)
		{
			is_push[ButtonKind::RightButton] = true;
		}

		if (y > 0.01f)
		{
			is_push[ButtonKind::UpButton] = true;
		}
		else if (y < -0.01f)
		{
			is_push[ButtonKind::DownButton] = true;
		}
	}

	// ボタン判定
	for (int i = 0; i < 32; i++)
	{
		if (!(pad_data.rgbButtons[i] & 0x80))
		{
			continue;
		}

		switch (i)
		{
		case 0:
			is_push[ButtonKind::Button01] = true;
			break;
		case 1:
			is_push[ButtonKind::Button02] = true;
			break;
		}
	}

	// 入力情報からボタンの状態を更新する
	for (int i = 0; i < ButtonKind::ButtonKindMax; i++)
	{
		if (is_push[i] == true)
		{
			if (buttonStates_[i] == ButtonState::ButtonStateNone)
			{
				buttonStates_[i] = ButtonState::ButtonStateDown;
			}
			else
			{
				buttonStates_[i] = ButtonState::ButtonStatePush;
			}
		}
		else
		{
			if (buttonStates_[i] == ButtonState::ButtonStatePush)
			{
				buttonStates_[i] = ButtonState::ButtonStateUp;
			}
			else
			{
				buttonStates_[i] = ButtonState::ButtonStateNone;
			}
		}
	}
	return true;
}
