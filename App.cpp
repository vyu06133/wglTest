#include "pch.h"
#include "App.h"
#include "Resource.h"
#include "Vertex.h"
#include "testtask.h"

void GLAPIENTRY MessageCallback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	if (type == GL_DEBUG_TYPE_ERROR)
	{
		TRACE_QUIET("[OpenGL Error]: (%08x): %s\n", id, message);
		DBGBREAK();
	}
	else if (severity == GL_DEBUG_SEVERITY_HIGH)
	{
		TRACE_QUIET("[OpenGL ERROR]: (%08x): %s\n", id, message);
		DBGBREAK();
	}
	else
	{
		TRACE_QUIET("[OpenGL WARNING]: (%08x): %s\n", id, message);
	}
}

App::App() :hInst_(nullptr), hWnd_(nullptr), hGLRC_(nullptr), szTitle_(L""), szWindowClass_(L"")
{
}

App::~App()
{
}

int APIENTRY App::wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	hInst_ = hInstance; // �O���[�o���ϐ��ɃC���X�^���X �n���h�����i�[����

	// �O���[�o�������������������
	LoadStringW(hInst_, IDS_APP_TITLE, szTitle_, MAX_LOADSTRING);
	LoadStringW(hInst_, IDC_WGLTEST, szWindowClass_, MAX_LOADSTRING);
	MyRegisterClass();
	auto path = MyUtil::ModulePath();
	MyUtil::PathA temp(path);
	auto assetPathTokenized = temp.Tokenize();

	// �A�v���P�[�V�����������̎��s:
	if (!InitInstance(nCmdShow, DEFAULT_CLIENT_WIDTH, DEFAULT_CLIENT_HEIGHT))
	{
		return FALSE;
	}
	HDC hDC = GetDC(hWnd_);
	InitWGL(hDC);
	ReleaseDC(hWnd_, hDC);

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WGLTEST));

	// ���C�� ���b�Z�[�W ���[�v:
	MSG msg = {};
	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
//			IdleFunc();
			TickFunc(tickClock.GetDelta());
			tickClock.Period();
			DrawFunc(drawClock.GetDelta());
			drawClock.Period();
		}
	}
	TermWGL();
	ReleaseDC(hWnd_, hDC);
	DestroyWindow(hWnd_);

	return (int)msg.wParam;
}

ATOM App::MyRegisterClass()
{
	WNDCLASSEXW wcex = {};

	wcex.cbSize = sizeof(WNDCLASSEX);
	//wcex.style = CS_HREDRAW | CS_VREDRAW;//�p�t�H�[�}���X�d���i�Q�[����OpenGL�Ȃǁj�Ȃ�A�蓮�ōĕ`��Ǘ����x�^�[�B 
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInst_;
	wcex.hIcon = LoadIcon(hInst_, MAKEINTRESOURCE(IDI_WGLTEST));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WGLTEST);
	wcex.lpszClassName = szWindowClass_;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

bool App::InitInstance(int nCmdShow, int clientWidth, int clientHeight)
{
	// �E�B���h�E�X�^�C��
	DWORD dwStyle = WS_OVERLAPPEDWINDOW;

	// �E�B���h�E�T�C�Y�𒲐�
	RECT rect = { 0, 0, clientWidth, clientHeight };
	AdjustWindowRect(&rect, dwStyle, TRUE);//todo:���j���[�o�[���܂ޏꍇ�� TRUE

	int windowWidth = rect.right - rect.left;
	int windowHeight = rect.bottom - rect.top;

	hWnd_ = CreateWindowEx(
		0,								// �g���X�^�C��
		szWindowClass_,					// �E�B���h�E�N���X��
		szTitle_,						// �^�C�g���o�[�̃e�L�X�g
		dwStyle,						// �E�B���h�E�X�^�C��
		CW_USEDEFAULT, CW_USEDEFAULT,	// �ʒu
		windowWidth, windowHeight,		// �T�C�Y
		nullptr,						// �e�E�B���h�E
		nullptr,						// ���j���[�n���h��
		hInst_,							// �C���X�^���X�n���h��
		nullptr);						// �ǉ��p�����[�^
	if (!hWnd_)
	{
		return false;
	}

	ShowWindow(hWnd_, nCmdShow);
	UpdateWindow(hWnd_);

	return true;
}

LRESULT CALLBACK App::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// �I�����ꂽ���j���[�̉��:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(App::GetInstance()->hInst_, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// TODO: HDC ���g�p����`��R�[�h�������ɒǉ����Ă�������...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SIZE:
	case WM_SIZING:
	{
		auto w = (float)LOWORD(lParam);
		auto h = (float)HIWORD(lParam);
		auto app = App::GetInstance();
		if (app)
		{
			auto& csize = App::GetInstance()->clientSize_;
			csize = vec2(w, h);
			// �E�B���h�E�̍ŏ��T�C�Y�𐧌�
			if (csize.x < app->minclientSize_.x)
			{
				csize.x = app->minclientSize_.x;
			}
			if (csize.y < app->minclientSize_.y)
			{
				csize.y = app->minclientSize_.y;
			}
			InvalidateRect(App::GetInstance()->hWnd_, NULL, FALSE);  // �ĕ`��v��
			if (App::GetInstance()->hGLRC_)
			{
//				App::GetInstance()->DrawFunc(0.0f);
				App::GetInstance()->Present(nullptr);
			}

			TRACE("client size=%s\n", glm::to_string(App::GetInstance()->clientSize_).c_str());
		}
		break;
	}
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE)
		{
			PostQuitMessage(777);
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

INT_PTR CALLBACK App::About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

bool App::InitWGL(HDC hDC)
{
	if (hDC == NULL)
	{
		return false;
	}
	// �s�N�Z���t�H�[�}�b�g�ݒ�
	PIXELFORMATDESCRIPTOR pfd = {};
	pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pfd.nVersion = 1;
	pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	pfd.iPixelType = PFD_TYPE_RGBA;
	pfd.cColorBits = 32;
	pfd.cDepthBits = 24;
	pfd.iLayerType = PFD_MAIN_PLANE;
	int pixelFormat = ChoosePixelFormat(hDC, &pfd);
	if (pixelFormat == 0)
	{
		return false;
	}
	if (!SetPixelFormat(hDC, pixelFormat, &pfd))
	{
		return false;
	}
	// OpenGL�R���e�L�X�g�쐬
	hGLRC_ = wglCreateContext(hDC);
	if (hGLRC_ == NULL)
	{
		return false;
	}
	if (!wglMakeCurrent(hDC, hGLRC_))
	{
		return false;
	}

	// API ���o�� 
	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)wglGetProcAddress("wglCreateContextAttribsARB");
	ASSERT(wglCreateContextAttribsARB);

	// OpenGL �̃o�[�W�����ƃv���t�@�C���̎w��
	static const int  att[] = {
	   WGL_CONTEXT_MAJOR_VERSION_ARB,   4,
	   WGL_CONTEXT_MINOR_VERSION_ARB,   5,
	   WGL_CONTEXT_FLAGS_ARB,           0,
	   WGL_CONTEXT_PROFILE_MASK_ARB,    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
	   0,
	};

	// �V���� HGLRC �̍쐬
	HGLRC   hglrc = wglCreateContextAttribsARB(hDC, NULL, att);
	ASSERT(hglrc);
	wglMakeCurrent(hDC, hglrc);

	// �Â� HGLRC �̍폜�ƒu������
	wglDeleteContext(hGLRC_);
	hGLRC_ = hglrc;
	
	// GLEW������
	auto err = glewInit();
	ASSERT(err == GLEW_OK);

	// �G���[�R�[���o�b�N�ݒ�
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(MessageCallback, nullptr);
	glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);

	// VSync��L���� (wglew.h�̋@�\)
	if (wglewIsSupported("WGL_EXT_swap_control"))
	{
		wglSwapIntervalEXT(1); // 1: VSync ON, 0: OFF
	}
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	glClearColor(0.0f, 0.0f, 0.5f, 0.0f);//test:wglCreateContext���������Ă邩�m�F���邽�ߕ�����₷���F�ŃN���A
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	SwapBuffers(hDC);
	
	// �o�[�W��������\��
	auto versionString = glGetString(GL_VERSION);
	if (versionString)
	{
		TRACE("OpenGL Version:[%s]\n", (const char*)versionString);
	}

	TRACE("Keyboard\n");
	ASSERT(SUCCEEDED(m_Keyboard.CreateDevice(hInst_, hWnd_)));
	TRACE("Gamepad\n");
	ASSERT(SUCCEEDED(m_Gamepad.Create(hInst_, hWnd_)));
	TRACE("Mouse\n");
	ASSERT(SUCCEEDED(m_Mouse.CreateDevice(hInst_, hWnd_)));

	HUDSetup();
	BasicSetup();
	ts2d_.SetApp(this);
	ts3d_.SetApp(this);
	TestTask::Setup(&ts3d_, &ts2d_);

#if MAKE_FONT_ATLAS
	font_.Init("assets\\font\\�݂������.ttf", 0, 32);
#else
	m_font.Init("Assets\\font\\�݂������.png", "Assets\\font\\�݂������.fontmap", 24);
#endif
	m_tex.SetActiveTexture(GL_TEXTURE0);
	m_tex.LoadFile("assets\\image\\�d���L�L.png");
	m_tex.BindTexture();

	m_vbFont.Setup(nullptr, 120*6, GL_DYNAMIC_DRAW);
	vertices_.push_back(VertexPCT(0.0f, 500.0f, 0.0f,	1,0,1,1, 0.0f, 1.0f)); // ����
	vertices_.push_back(VertexPCT(800.0f, 500.0f, 0.0f,	0,1,1,1, 1.0f, 1.0f));  // �E��
	vertices_.push_back(VertexPCT(0.0f, 0.0f, 0.0f,		1,1,0,1, 0.0f, 0.0f));   // ��

	vertices_.push_back(VertexPCT(800.0f, 500.0f, 0.0f,	1,1,0,1, 1.0f, 1.0f));   // ��
	vertices_.push_back(VertexPCT(800.0f, 0.0f, 0.0f,	0,1,1,1, 1.0f, 0.0f));  // �E��
	vertices_.push_back(VertexPCT(0.0f, 0.0f, 0.0f,		1,0,1,1, 0.0f, 0.0f)); // ����
	m_vboPCT.Setup(vertices_, GL_STATIC_DRAW);

	m_Constants.Data().SetWVP(
		glm::mat4(1.0f), // ���f���s�� (�P�ʍs��)
		glm::lookAt(
			glm::vec3(0.0f, 0.0f, 5.0f), // �J�����̈ʒu
			glm::vec3(0.0f, 0.0f, 0.0f), // �����_
			glm::vec3(0.0f, 1.0f, 0.0f)  // ������x�N�g��
		),
		glm::orthoRH(0.0f, 1600.0f, 1200.0f, 0.0f, 0.1f, 100.0f)); // ���ˉe�s��@����������font�łĂ邩��

	return true;
}

void App::TermWGL()
{
	if (hGLRC_)
	{
		wglMakeCurrent(nullptr, nullptr);
		wglDeleteContext(hGLRC_);
		hGLRC_ = nullptr;
	}
}

void App::Present(HDC hDC)
{
	if (hDC)
	{
		SwapBuffers(hDC);
	}
	else
	{
		hDC = GetDC(hWnd_);
		SwapBuffers(hDC);
		ReleaseDC(hWnd_, hDC);
	}
}

void App::TickFunc(float delta)
{
	m_Keyboard.Update();
	m_Gamepad.Update();
	m_Mouse.Update();
	m_LightInfo.Data().Clear();
	ts3d_.Tick(delta);
	ts2d_.Tick(delta);
#if 0
#if MAKE_FONT_ATLAS
	static wchar_t ch = 0;
	for(auto i=0;i<90;i++)	font_.GetCh(ch++);
	TRACE("used %d\n", font_.used_);
	if (ch > 0x9ffc)
	{
		font_.WriteCache();
	}
	if (GetAsyncKeyState(32)<0)
	{
		DBGBREAK();
		font_.WriteCache();
	}
#endif
#endif
}

void App::DrawFunc(float delta)
{
	glDepthMask(GL_TRUE);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);   /* ��ʂ����� */
	glViewport(0, 0, (int)clientSize_.x, (int)clientSize_.y);
	BasicSetup();
	m_LightInfo.SendToGPU();
	ts3d_.Draw();
	HUDSetup();
	ts2d_.Draw();
#if 0
	proj_ = glm::ortho(-1.0f, 10.0f, -1.0f, 10.0f, 0.1f, 100.0f); // ���ˉe�s��@����������Ȃ��ƎO�p�`���łȂ�
	proj_ = glm::orthoRH(0.0f, clientSize_.x, clientSize_.y, 0.0f, 0.1f, 32767.0f);
	mvp_ = proj_ * view_ * model_;
	GLuint mvpLoc = m_shaderProg.FindUniformLoc("u_mvp");
//	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp_));
	auto loc = m_shaderProg.FindUniformLoc("u_tex");
//	glUniform1i(loc, font_.atlas_.TextureUnits());
	m_vboPCT.Bind();
	font_.atlas_.BindTexture();
#if 0
	glDepthMask(GL_FALSE); // �[�x�o�b�t�@�ւ̏������݂�������
	glDrawArrays(GL_TRIANGLES, 0, 6);

	m_shaderProg.UpdateUniformvec4("u_fontColor", vec4(1.0f,1.0f,1.0f,1.0f));
	proj_ = glm::orthoRH(0.0f, clientSize_.x, clientSize_.y, 0.0f, 0.1f, 32767.0f);
	mvp_ = proj_ * view_ * model_;
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, glm::value_ptr(mvp_));
	std::vector<VertexPCT> verts;
	wchar_t text[30];
	swprintf_s(text, _countof(text), L"%d,%d\n", m_Mouse.m_csrPos.x, m_Mouse.m_csrPos.y);// , * (uint64_t*)m_Mouse.m_CurrentMouseState.rgbButtons);
	font_.RenderText(&verts, 0.0f, 500.0f, 1.0f, text);
	m_vbFont.UpdateData(verts);
	font_.atlas_.SetParameter(GL_CLAMP, GL_LINEAR);
	font_.atlas_.BindTexture();
	loc = m_shaderProg.FindUniformLoc("u_tex");
	glUniform1i(loc, font_.atlas_.TextureUnits());
	m_vbFont.Bind();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDisable(GL_DEPTH_TEST);
	glDrawArrays(GL_TRIANGLES, 0, m_vbFont.GetVertexCount());
#endif
	HUDSetup();
	std::vector<VertexPCT> verts;
	font_.RenderText(&verts, 0.0f, 0.0f, 1.0f, L"%d,%d\n", m_Mouse.m_csrPos.x, m_Mouse.m_csrPos.y);
	m_vbFont.UpdateData(verts);
	loc = m_HUD.FindUniformLoc("u_tex");
	glUniform1i(loc, font_.atlas_.TextureUnits());
	m_vbFont.Bind();
	glDrawArrays(GL_TRIANGLES, 0, m_vbFont.GetVertexCount());
#endif
	Present(nullptr);
}

void App::HUDSetup()
{
	if (m_HUD.GetProgId() == 0)
	{
		ASSERT(m_HUD.CreateProgram());
		m_HUDVert.CompileShaderFromFile(GL_VERTEX_SHADER, "glsl\\HUD.vert");
		m_HUDFrag.CompileShaderFromFile(GL_FRAGMENT_SHADER, "glsl\\HUD.frag");
		m_HUD.AttachShader(&m_HUDVert);
		m_HUD.AttachShader(&m_HUDFrag);
		m_HUD.LinkProg();
	}
	m_HUD.UseProg();
	auto u_tex = m_font.atlas_.TextureUnits();
	m_HUD.UpdateUniformu("u_tex", u_tex);
	m_HUD.UpdateUniformvec4("u_fontColor", vec4(1.0f, 1.0f, 1.0f, 1.0f));
	auto p = glm::orthoRH(0.0f, clientSize_.x, clientSize_.y, 0.0f, 0.1f, 32767.0f);
	auto v = glm::lookAt(
		glm::vec3(0.0f, 0.0f, 100.0f), // �J�����̈ʒu
		glm::vec3(0.0f, 0.0f, 0.0f), // �����_
		glm::vec3(0.0f, 1.0f, 0.0f)  // ������x�N�g��
	);
	auto mvp = p * v;
	m_HUD.UpdateUniformmat4("u_mvp", mvp);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_FALSE); // �[�x�o�b�t�@�ւ̏������݂�������
	glDisable(GL_DEPTH_TEST);
}

void App::BasicSetup()
{
	if (m_BasicShader.GetProgId() == 0)
	{
		m_BasicShader.CreateProgram();
		m_BasicVert.CompileShaderFromFile(GL_VERTEX_SHADER, "glsl\\basic.vert");
		m_BasicFrag.CompileShaderFromFile(GL_FRAGMENT_SHADER, "glsl\\basic.frag");
		m_BasicShader.AttachShader(&m_BasicVert);
		m_BasicShader.AttachShader(&m_BasicFrag);
		m_BasicShader.LinkProg();
		m_BasicShader.UseProg();
		m_Constants.GenAndBind(m_BasicShader, "Constants");
		m_Material.GenAndBind(m_BasicShader, "Material");
		m_MatrixPalette.GenAndBind(m_BasicShader, "Palette");
		m_LightInfo.GenAndBind(m_BasicShader, "LightInfo");
	}
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}

