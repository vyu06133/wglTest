#pragma once
#include "framework.h"

class Keyboard
{
public:
	// DIRECTINPUT8のポインタ
	LPDIRECTINPUT8 pInterface_ = nullptr;

	// DIRECTINPUTDEVICE8のポインタ
	LPDIRECTINPUTDEVICE8 pDevice_ = nullptr;

	// キー情報
	using State = BYTE[256];
	struct INPUTSTATE
	{
		State now;
		State trg;
		State ntrg;
	};
	INPUTSTATE InputState_ = {};

public:
	Keyboard();
	~Keyboard();

	HRESULT CreateDevice(HINSTANCE hinst, HWND hwnd);
	void Release();
	bool Update();
	BYTE GetKey(BYTE key_code);
	BYTE GetKeyDown(BYTE key_code);
	BYTE GetKeyUp(BYTE key_code);
};

