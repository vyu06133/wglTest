// header.h : 標準のシステム インクルード ファイルのインクルード ファイル、
// またはプロジェクト専用のインクルード ファイル
//

#pragma once

#include "targetver.h"
#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーからほとんど使用されていない部分を除外する
// Windows ヘッダー ファイル
#include <windows.h>

// デバッグNEW
#ifdef _DEBUG
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#define _NEW ::new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define _FREE(p) _free_dbg(p, _NORMAL_BLOCK)
#define _MALLOC(s) _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define _REALLOC(p, s) _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#pragma warning(push)
#pragma warning(disable : 4996)  // strcpyの警告を無視
#define WEDGE(s) memcpy(_NEW char[strlen(s)+1],(s),strlen(s)+1)//　メモリリーク追跡用
#pragma warning(pop)
#else
#define _NEW ::new
#define _FREE(p) _free
#define _MALLOC(s) _malloc
#define _REALLOC(p, s) _realloc
#define WEDGE(s) ((void)(s))
#endif
// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <algorithm>
#include <atomic>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <cwchar>
#include <exception>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string>
#include <system_error>
#include <tuple>
#include <stdio.h>
#include <wchar.h>
#include <memory>
#include <vector>
#include <array>
#include <queue>
#include <stack>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <bitset>
#include <algorithm>
#include <thread>
#include <future>
#include <chrono>
#include <cassert>
#include <functional>
#include <vcruntime_exception.h>
#include <concrt.h>
#include <cstdarg> // for va_list

#include <cwchar>
#include <string>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>
#include <locale>
#include <codecvt>
#include <variant>
#include <any>
#include <filesystem>
#include <future>
#define F32_MAX		(3.402823466e+38f)
#define F32_MINP	(1.175494351e-38f)
#define	numof(x)	((int32_t)(sizeof(x) / sizeof((x)[0])))

#if defined(_DEBUG)
inline static std::string HRESULT_MESSAGE(HRESULT h) { return std::system_category().message(h); }
#define DBGBREAK()	_CrtDbgBreak()
#define ASSERT(f)	assert((f))
#define VERIFY(f)	assert((f))
#define PAUSE(f)	do{if(!(f)){_CrtDbgBreak();assert(0);}}while(0)

template <size_t BUFSIZE = 4096>
inline void TRACE_QUIET(const char* format, ...)
{
	va_list arg;
	va_start(arg, format);
	static char msg[BUFSIZE] = {};
	vsprintf_s(msg, sizeof(msg), format, arg);
	va_end(arg);
	printf(msg);
	OutputDebugStringA(msg);
}
template <size_t BUFSIZE = 4096>
inline void TRACE_VERBOSE(const char* f, int l, const char* format, ...)
{
	auto findFindFileNameA = [](const char* path)
		{
			const char* last = path;
			while (path && path[0])
			{
				if ((path[0] == '\\' || path[0] == '/' || path[0] == ':') && path[1] && path[1] != '\\' && path[1] != '/')
					last = path + 1;
				path = CharNextA(path);
			}
			return last;
		};
	va_list arg;
	va_start(arg, format);
	static char msg1[4096] = "";
	static char msg2[4096];
	vsprintf_s(msg1, sizeof(msg1), format, arg);
	va_end(arg);
	sprintf_s(msg2, sizeof(msg2), "%s(%d):%s", findFindFileNameA(f), l, msg1);
	printf(msg2);
	OutputDebugStringA(msg2);
}
#define TRACE(...) TRACE_VERBOSE<>(__FILE__,__LINE__,__VA_ARGS__)
#else   // _DEBUG
inline static std::string HRESULT_MESSAGE(HRESULT h) std::string()
#define ASSERT(f)	((void)(f))
#define VERIFY(f)	((void)(f))
#define PAUSE(f)	((void)(f))

#define TRACE( FORMAT, ... )	((void)FORMAT)
#endif  // _DEBUG

// OpenGL
#include <GL/glew.h>
#include <GL/wglew.h>
#include <GL/GL.h>
#pragma comment( lib, "opengl32.lib" )
#include "glenummap.inc"

// GLM
#define USE_GLM 1
#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_SWIZZLE
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <glm/gtx/string_cast.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/quaternion.hpp>
using glm::vec2;
using glm::vec3;
using glm::vec4;
using glm::mat3;
using glm::mat4;
using glm::quat;
#include <glm/gtx/scalar_relational.hpp> // glm::sum() を使用するために必要

// FreeType
#include <ft2build.h>
#include FT_FREETYPE_H

//input device
#define DIRECTINPUT_VERSION 0x0800 
#include <dinput.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// fbxsdk
#pragma warning(push)
#pragma warning(disable:26812)	// enum Class
#pragma warning(disable:26451)	// 演算オーバーフロー
#pragma warning(disable:26495)	// メンバ初期化
#include <fbxsdk.h>
#include <fbxsdk/core/math/fbxmath.h>
#include <fbxsdk/core/math/fbxdualquaternion.h>
#include <fbxsdk/core/math/fbxmatrix.h>
#include <fbxsdk/core/math/fbxquaternion.h>
#pragma warning(pop)
using fbxsdk::FbxAMatrix;
using fbxsdk::FbxMatrix;

#include "revision.inc"
#include "MyMath.h"
#include "MyUtil.h"
#include "Console.h"

#include <tinyxml.h>


