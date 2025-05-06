#include "pch.h"
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include "Console.h"


static BOOL WINAPI HandlerRoutine(DWORD type)
{
	switch (type)
	{
		case CTRL_C_EVENT:      //Ctrl+Cを受け取った
		case CTRL_BREAK_EVENT:  //Ctrl+Breakを受け取った
		case CTRL_CLOSE_EVENT:  //コンソールを閉じた
			return TRUE;        //無効にするのでTRUEを返す
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			return FALSE;
	}
	return FALSE;           //それ以外の時は強制終了
}

bool Console::Alloc()
{
	if (instance_ != nullptr)
	{
		return false;
	}
	instance_ = _NEW Console;
	if (instance_ == nullptr)
	{
		return false;
	}
	return true;
}

bool Console::Free()
{
	if (instance_ != nullptr)
	{
		delete instance_;
	}
	instance_ = nullptr;
	return true;
}


Console::Console()
{
//	::FreeConsole();
	::AllocConsole();
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);
	FILE* fp;
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stderr);
}

Console::~Console()
{
//	::FreeConsole();
}
