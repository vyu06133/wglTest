#include "pch.h"
#include "Mouse.h"

Mouse::Mouse()
{
}

Mouse::~Mouse()
{
}

HRESULT Mouse::CreateDevice(HINSTANCE hinst, HWND hwnd)
{
	HRESULT hr = S_OK;

	hWnd_ = hwnd;

	// インターフェース作成
	hr = DirectInput8Create(
		hinst,						// インスタンスハンドル
		DIRECTINPUT_VERSION,		// DirectInputのバージョン
		IID_IDirectInput8,			// 使用する機能
		(void**)&pInterface_,		// 作成されたインターフェース代入用
		NULL						// NULL固定
	);

	if (FAILED(hr))
	{
		return hr;
	}

	// デバイス作成
	pDevice_ = nullptr;

	// デバイス生成
	hr = pInterface_->CreateDevice(
		GUID_SysMouse,
		&pDevice_,
		NULL);

	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	// 入力フォーマットの指定
	hr = pDevice_->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	// 協調レベルの設定
	if (SetUpCooperativeLevel(pDevice_) == false)
	{
		Release();
		return E_FAIL;
	}

	int count = 0;
	// 制御開始
	while (StartControl() == false)
	{
		Sleep(100);
		count++;
		if (count >= 5)
		{
			break;
		}
	}

	// 入力情報の初期化
	ZeroMemory(&m_CurrentMouseState, sizeof(DIMOUSESTATE));
	ZeroMemory(&m_PrevMouseState, sizeof(DIMOUSESTATE));

	return S_OK;
}

void Mouse::Release()
{
	// デバイスの解放
	if (pDevice_ != nullptr)
	{
		// 制御を停止
		pDevice_->Unacquire();
		pDevice_->Release();
		pDevice_ = nullptr;
	}

	// インターフェースの解放
	if (pInterface_ != nullptr)
	{
		pInterface_->Release();
		pInterface_ = nullptr;
	}
}

bool Mouse::Update()
{
	if (pDevice_ == nullptr)
	{
		return false;
	}

	// 更新前に最新マウス情報を保存する
	m_PrevMouseState = m_CurrentMouseState;

	// マウスの状態を取得します
	HRESULT	hr = pDevice_->GetDeviceState(sizeof(DIMOUSESTATE2), &m_CurrentMouseState);
	if (FAILED(hr))
	{
		pDevice_->Acquire();
		hr = pDevice_->GetDeviceState(sizeof(DIMOUSESTATE2), &m_CurrentMouseState);
	}

	POINT p;
	p.x = m_CurrentMouseState.lX;
	p.y = m_CurrentMouseState.lY;
	
	{
		// マウス座標(スクリーン座標)を取得する
		GetCursorPos(&p);

		// スクリーン座標にクライアント座標に変換する
		ScreenToClient(hWnd_, &p);
	}

	// 変換した座標を保存
	m_csrPos.x = p.x;
	m_csrPos.y = p.y;
	return true;
}

BOOL Mouse::SetUpCooperativeLevel(LPDIRECTINPUTDEVICE8 device)
{
	HRESULT hr = S_OK;
	// 協調モードの設定
	hr = device->SetCooperativeLevel(hWnd_, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

BOOL Mouse::StartControl()
{
	// デバイスが生成されてない
	if (pDevice_ == nullptr)
	{
		return false;
	}
	// 制御開始
	if (FAILED(pDevice_->Acquire()))
	{
		return false;
	}

	DIDEVCAPS cap;
	pDevice_->GetCapabilities(&cap);
	// ポーリング判定
	if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
	{
		DWORD error = GetLastError();
		// ポーリング開始
		///*
		//	PollはAcquireの前に行うとされていたが、
		//	Acquireの前で実行すると失敗したので
		//	後で実行するようにした
		//*/
		if (FAILED(pDevice_->Poll()))
		{
			return false;
		}
	}

	return true;
}
