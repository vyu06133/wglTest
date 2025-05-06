#pragma once
#include "framework.h"

class Mouse
{
public:
	LPDIRECTINPUT8 pInterface_ = nullptr;	//!< DIRECTINPUT8�̃|�C���^
	LPDIRECTINPUTDEVICE8 pDevice_ = nullptr;//!< DIRECTINPUTDEVICE8�̃|�C���^
	DIMOUSESTATE2 m_CurrentMouseState;		//!< �}�E�X�̌��݂̓��͏��
	DIMOUSESTATE2 m_PrevMouseState;			//!< �}�E�X�̌��݂̓��͏��
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

