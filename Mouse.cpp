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

	// �C���^�[�t�F�[�X�쐬
	hr = DirectInput8Create(
		hinst,						// �C���X�^���X�n���h��
		DIRECTINPUT_VERSION,		// DirectInput�̃o�[�W����
		IID_IDirectInput8,			// �g�p����@�\
		(void**)&pInterface_,		// �쐬���ꂽ�C���^�[�t�F�[�X����p
		NULL						// NULL�Œ�
	);

	if (FAILED(hr))
	{
		return hr;
	}

	// �f�o�C�X�쐬
	pDevice_ = nullptr;

	// �f�o�C�X����
	hr = pInterface_->CreateDevice(
		GUID_SysMouse,
		&pDevice_,
		NULL);

	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	// ���̓t�H�[�}�b�g�̎w��
	hr = pDevice_->SetDataFormat(&c_dfDIMouse2);
	if (FAILED(hr))
	{
		Release();
		return hr;
	}

	// �������x���̐ݒ�
	if (SetUpCooperativeLevel(pDevice_) == false)
	{
		Release();
		return E_FAIL;
	}

	int count = 0;
	// ����J�n
	while (StartControl() == false)
	{
		Sleep(100);
		count++;
		if (count >= 5)
		{
			break;
		}
	}

	// ���͏��̏�����
	ZeroMemory(&m_CurrentMouseState, sizeof(DIMOUSESTATE));
	ZeroMemory(&m_PrevMouseState, sizeof(DIMOUSESTATE));

	return S_OK;
}

void Mouse::Release()
{
	// �f�o�C�X�̉��
	if (pDevice_ != nullptr)
	{
		// ������~
		pDevice_->Unacquire();
		pDevice_->Release();
		pDevice_ = nullptr;
	}

	// �C���^�[�t�F�[�X�̉��
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

	// �X�V�O�ɍŐV�}�E�X����ۑ�����
	m_PrevMouseState = m_CurrentMouseState;

	// �}�E�X�̏�Ԃ��擾���܂�
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
		// �}�E�X���W(�X�N���[�����W)���擾����
		GetCursorPos(&p);

		// �X�N���[�����W�ɃN���C�A���g���W�ɕϊ�����
		ScreenToClient(hWnd_, &p);
	}

	// �ϊ��������W��ۑ�
	m_csrPos.x = p.x;
	m_csrPos.y = p.y;
	return true;
}

BOOL Mouse::SetUpCooperativeLevel(LPDIRECTINPUTDEVICE8 device)
{
	HRESULT hr = S_OK;
	// �������[�h�̐ݒ�
	hr = device->SetCooperativeLevel(hWnd_, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (FAILED(hr))
	{
		return false;
	}

	return true;
}

BOOL Mouse::StartControl()
{
	// �f�o�C�X����������ĂȂ�
	if (pDevice_ == nullptr)
	{
		return false;
	}
	// ����J�n
	if (FAILED(pDevice_->Acquire()))
	{
		return false;
	}

	DIDEVCAPS cap;
	pDevice_->GetCapabilities(&cap);
	// �|�[�����O����
	if (cap.dwFlags & DIDC_POLLEDDATAFORMAT)
	{
		DWORD error = GetLastError();
		// �|�[�����O�J�n
		///*
		//	Poll��Acquire�̑O�ɍs���Ƃ���Ă������A
		//	Acquire�̑O�Ŏ��s����Ǝ��s�����̂�
		//	��Ŏ��s����悤�ɂ���
		//*/
		if (FAILED(pDevice_->Poll()))
		{
			return false;
		}
	}

	return true;
}
