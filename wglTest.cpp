// wglTest.cpp : アプリケーションのエントリ ポイントを定義します。
//

#include "pch.h"
#include "Console.h"
#include "App.h"
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
					 _In_opt_ HINSTANCE hPrevInstance,
					 _In_ LPWSTR    lpCmdLine,
					 _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

#if defined(_DEBUG)
	_CrtSetDbgFlag(0/*_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF*/);
#endif
	InitCommonControls();
	Console::Alloc();
	MyUtil::Init("JA-jp");
	auto app = App::CreateInstance();
	ASSERT(app);
	puts(project_revision);
	puts(project_branch);
	auto retcode = app->wWinMain(hInstance, hPrevInstance, lpCmdLine, nCmdShow);
	App::DestroyInstance();
	Console::Free();
#if defined(_DEBUG)
	_CrtDumpMemoryLeaks();
#endif
	return retcode;
}
