#include "pch.h"
#include "Keyboard.h"

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
}

HRESULT Keyboard::CreateDevice(HINSTANCE hinst, HWND hwnd)
{
	HRESULT hr = S_OK;

	// IDirectInput8インターフェイスの取得
	hr = DirectInput8Create(
		hinst,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&pInterface_,
		NULL);
	if (FAILED(hr))
	{
		TRACE("fail to DirectInput8Create %s\n", HRESULT_MESSAGE(hr).c_str());
		return hr;
	}


	// デバイスの作成
	hr = pInterface_->CreateDevice(GUID_SysKeyboard, &pDevice_, NULL);
	if (FAILED(hr))
	{
		TRACE("fail to CreateDevice %s\n", HRESULT_MESSAGE(hr).c_str());
		return hr;
	}

	// デバイスのフォーマットの設定
	hr = pDevice_->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
	{
		TRACE("fail to SetDataFormat %s\n", HRESULT_MESSAGE(hr).c_str());
		return hr;
	}

	// 協調モードの設定
	hr = pDevice_->SetCooperativeLevel(hwnd, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
	{
		TRACE("fail to SetCooperativeLevel %s\n", HRESULT_MESSAGE(hr).c_str());
		return hr;
	}

	// デバイスの取得開始
	pDevice_->Acquire();

	return hr;
}

void Keyboard::Release()
{
	if (pDevice_)
	{
		pDevice_->Unacquire();
		pDevice_->Release();
		pDevice_ = nullptr;
	}
	if (pInterface_)
	{
		pInterface_->Release();
		pInterface_ = nullptr;
	}
}

bool Keyboard::Update()
{
	HRESULT hr = S_OK;

	// キーボードデバイスのゲッター
	if (pDevice_ == nullptr)
	{
		return false;
	}
	
	State old;
	MoveMemory(&old, InputState_.now, sizeof(State));
	hr = pDevice_->GetDeviceState(256, &InputState_.now);
	if (SUCCEEDED(hr))
	{
		for (auto i = 0; i < 256; i++)
		{
			InputState_.trg[i] = (InputState_.now[i] & (~old[i]));
			InputState_.ntrg[i] = (~InputState_.now[i]) & old[i];
		}
	}
	else if (hr == DIERR_INPUTLOST)
	{
		pDevice_->Acquire();
	}
	return true;
}

BYTE Keyboard::GetKey(BYTE key_code)
{
	return InputState_.now[key_code];
}

BYTE Keyboard::GetKeyDown(BYTE key_code)
{
	return InputState_.trg[key_code];
}

BYTE Keyboard::GetKeyUp(BYTE key_code)
{
	return InputState_.ntrg[key_code];
}


