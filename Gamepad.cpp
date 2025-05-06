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
	// �C���^�[�t�F�[�X�쐬
	if (!CreateInterface(hinst))
	{
		return false;
	}

	// �f�o�C�X�쐬
	if (!CreateGamePadDevice(hinst))
	{
		Release();
		return false;
	}

	// ���͏��̏�����
	for (int i = 0; i < ButtonKind::ButtonKindMax; i++)
	{
		buttonStates_[i] = ButtonState::ButtonStateNone;
	}

	return S_OK;
}

bool Gamepad::CreateInterface(HINSTANCE hinst)
{
	HRESULT hr = S_OK;

	// �C���^�[�t�F�[�X�쐬
	hr = DirectInput8Create(
		hinst,						// �C���X�^���X�n���h��
		DIRECTINPUT_VERSION,		// DirectInput�̃o�[�W����
		IID_IDirectInput8,			// �g�p����@�\
		(void**)&pInterface_,		// �쐬���ꂽ�C���^�[�t�F�[�X����p
		NULL);						// NULL�Œ�

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

	// GAMEPAD�𒲂ׂ�
//	TRACE("DI8DEVTYPE_GAMEPAD{\n");
	pInterface_->EnumDevices(
		DI8DEVTYPE_GAMEPAD,			// ��������f�o�C�X�̎��
		DeviceFindCallBackProxy,			// �������Ɏ��s����֐�
		this,						// �֐��ɓn���l
		DIEDFL_ATTACHEDONLY			// �������@
	);
//	TRACE("}=%d\n", inputDevises_.size());

	// JOYSTICK�𒲂ׂ�
//	TRACE("DI8DEVTYPE_JOYSTICK{\n");
	pInterface_->EnumDevices(
		DI8DEVTYPE_JOYSTICK,
		DeviceFindCallBackProxy,
		this,
		DIEDFL_ATTACHEDONLY
	);
//	TRACE("}=%d\n", inputDevises_.size());

	// �ǂ���������邱�Ƃ��o���Ȃ������玸�s
	//if (parameter_.FindCount == 0)
	if (inputDevises_.size() == 0)
	{
		return false;
	}

	int count = 0;
	// ����J�n
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
	//�������[�h�̐ݒ�
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
	//// �f�o�C�X����������ĂȂ�
	//if (pDevice_ == nullptr)
	//{
	//	return false;
	//}

	// ����J�n
	hr = device->Acquire();
	if (FAILED(hr))
	{
		return false;
	}

	DIDEVCAPS cap;
	device->GetCapabilities(&cap);
	// �|�[�����O����
	if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
	{
		DWORD error = GetLastError();
		// �|�[�����O�J�n
		/*
			Poll��Acquire�̑O�ɍs���Ƃ���Ă������A
			Acquire �̑O�Ŏ��s����Ǝ��s�����̂�
			��Ŏ��s����悤�ɂ���
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
	// �����[�h���Βl���[�h�Ƃ��Đݒ�
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

	// X���̒l�͈̔͐ݒ�
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

	// Y���̒l�͈̔͐ݒ�
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

	//// ���ɔ������Ă���Ȃ�I��
	//if (parameter_.FindCount >= 1)
	//{
	//	return DIENUM_STOP;
	//}
	InputDevice dev = {};
	dev.guidInstance = lpddi->guidInstance;
	lstrcpy(dev.tszInstanceName, lpddi->tszInstanceName);

	// �f�o�C�X����
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

	// ���̓t�H�[�}�b�g�̎w��
	device = *parameter_.GamePadDevice;
	hr = device->SetDataFormat(&c_dfDIJoystick);

	if (FAILED(hr))
	{
		return DIENUM_STOP;
	}

	// �v���p�e�B�̐ݒ�
	if (SetUpGamePadProperty(device) == false)
	{
		return DIENUM_STOP;
	}

	// �������x���̐ݒ�
	if (SetUpCooperativeLevel(device) == false)
	{
		return DIENUM_STOP;
	}

	// ���������J�E���g
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
	
	// �f�o�C�X�擾
	HRESULT hr = device->GetDeviceState(sizeof(DIJOYSTATE), &pad_data);
	if (FAILED(hr))
	{
		// �ēx����J�n
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

	// �X�e�B�b�N����
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

	// �\���L�[����
	if (pad_data.rgdwPOV[0] != 0xFFFFFFFF)
	{
		float rad = (pad_data.rgdwPOV[0] / 100.0f)*3.14f/180.0f;
		// �{����x��cos�Ay��sin�����ǁArgdwPOV��0���ォ��n�܂�̂ŁA
		// cos��sin���t�ɂ��������s��������
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

	// �{�^������
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

	// ���͏�񂩂�{�^���̏�Ԃ��X�V����
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
