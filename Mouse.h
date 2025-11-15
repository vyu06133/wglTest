#pragma once
#include "framework.h"

class Mouse
{
public:
	LPDIRECTINPUT8 pInterface_ = nullptr;	//!< DIRECTINPUT8のポインタ
	LPDIRECTINPUTDEVICE8 pDevice_ = nullptr;//!< DIRECTINPUTDEVICE8のポインタ
	DIMOUSESTATE2 m_CurrentMouseState;		//!< マウスの現在の入力情報
	DIMOUSESTATE2 m_PrevMouseState;			//!< マウスの現在の入力情報
	POINT m_csrPos;
	HWND hWnd_ = nullptr;

	Mouse();
	~Mouse();

	HRESULT CreateDevice(HINSTANCE hinst, HWND hwnd);
	void Release();
	bool Update();
	BOOL SetUpCooperativeLevel(LPDIRECTINPUTDEVICE8 device);
	BOOL StartControl();
};

