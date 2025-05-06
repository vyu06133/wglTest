#pragma once
#include "framework.h"
#include "App.h"
#include "Clock.h"
#include "Shader.h"
#include "Vertex.h"
#include "Texture2D.h"
#include "Keyboard.h"
#include "Gamepad.h"
#include "Mouse.h"
#include "font.h"
#include "TaskSystem.h"
#include "UBO.h"

class App
{
public:
	inline static App* instance_ = nullptr;
	inline static App* GetInstance()
	{
		return instance_;
	}
	inline static App* CreateInstance()
	{
		instance_ = _NEW App;
		return instance_;
	}
	inline static void DestroyInstance()
	{
		if (instance_)
		{
			delete instance_;
			instance_ = nullptr;
		}
	}
	App();
	~App();
	int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow);
	ATOM MyRegisterClass();
	bool InitInstance(int nCmdShow, int clientWidth, int clientHeight);
	static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
	bool InitWGL(HDC hDC);
	void TermWGL();
	void Present(HDC hDC);
//	void IdleFunc();
	void TickFunc(float delta);
	void DrawFunc(float delta);

private:
	static const int MAX_LOADSTRING = 100;
	static const int DEFAULT_CLIENT_WIDTH = 800;
	static const int DEFAULT_CLIENT_HEIGHT = 600;
	HINSTANCE hInst_ = nullptr;
	WCHAR szTitle_[MAX_LOADSTRING];
	WCHAR szWindowClass_[MAX_LOADSTRING];
	HWND hWnd_ = nullptr;
	HGLRC hGLRC_ = nullptr;
public:
	vec2 clientSize_ = { 0.0f, 0.0f };
	vec2 minclientSize_ = { 1.0f, 1.0f };

	TaskSystem ts2d_;
	TaskSystem ts3d_;
	std::vector<TaskBase*> FindTaskByName(const std::string& name)
	{
		std::vector<TaskBase*> result;
		auto r2 = ts2d_.FindTaskByName(name);
		for(auto t : r2)
			result.push_back(t);
		auto r3 = ts3d_.FindTaskByName(name);
		for (auto t : r3)
			result.push_back(t);
		return result;
	}
	Clock tickClock;
	Clock drawClock;

	//uniform block objects
	struct alignas(16) Material
	{
		vec3 ambientColor;
		vec3 diffuseColor;
		vec3 emmisiveColor;
		vec3 specularColor;
		float shininess;
		void SetColors(const vec3& a, const vec3& d, const vec3& e, const vec3& sp, const float& sh)
		{
			ambientColor = a;
			diffuseColor = d;
			emmisiveColor = e;
			specularColor = sp;
			shininess = sh;
		}
	};
	static_assert((sizeof(Material)%16) == 0, "Material struct size%16 != 0");
	UBO<Material> m_Material;

	struct alignas(16) MatrixPalette
	{
		static const size_t MAX_PALETTE_SIZE = 96;//TODO:GLSLÇ∆çáÇÌÇπÇÈ
		mat4 Matrices[MAX_PALETTE_SIZE];
	};
	UBO<MatrixPalette> m_MatrixPalette;

	struct alignas(16) Constants
	{
		mat4 worldViewProj = mat4(1.0f);
		mat4 world = mat4(1.0f);
		mat4 view = mat4(1.0f);
		mat4 proj = mat4(1.0f);
		vec3 eye = vec3(0.0f);
		mat3 Normal = mat3(1.0f);
		inline void SetWVP(const mat4& w, const mat4& v, const mat4& p )
		{
			worldViewProj = p * v * w;
			world = w;
			view = v;
			proj = p;
			Normal = mat3(glm::transpose(glm::inverse(mat3(view * world))));
			eye = glm::inverse(view)[3].xyz;
//			TRACE("%s	eye=%s\n", __FUNCSIG__, glm::to_string(eye).c_str());
		}
		inline void SetWorld(const mat4& w)
		{
			SetWVP(w, view, proj);
		}
		inline void SetView(const mat4& v)
		{
			SetWVP(world, v, proj);
		}
		inline void SetProjection(const mat4& p)
		{
			SetWVP(world, view, p);
		}
	};
	UBO<Constants> m_Constants;

	struct alignas(16) LightInfo
	{
		static const int MAX_LIGHT = 8;//TODO:GLSLÇ∆çáÇÌÇπÇÈ
		vec4 Position_EyeCoord[MAX_LIGHT]; // Light position in eye coords.
		float Range[MAX_LIGHT];
		vec3 Intensity[MAX_LIGHT];
		inline void Clear()
		{
			ZeroMemory(Intensity, sizeof(Intensity));
			ZeroMemory(Range, sizeof(Range));
		}
	};
	UBO<LightInfo> m_LightInfo;

	std::string	assetPath;

	// devices
	Keyboard m_Keyboard;
	Gamepad m_Gamepad;
	Mouse m_Mouse;
	Font m_font;

	// shader setup
	void HUDSetup();
	void BasicSetup();
	ShaderProg m_HUD;
	Shader m_HUDVert;
	Shader m_HUDFrag;
	ShaderProg m_BasicShader;
	Shader m_BasicVert;
	Shader m_BasicFrag;

	//vertex bufer objects
	VertexBuffer<VertexPCT> m_vboPCT;
	VertexBuffer<VertexPCT> m_vbFont;
	Texture2D m_tex;
	std::vector<VertexPCT> vertices_;

	//glm::mat4 model_;
	//glm::mat4 view_;
	//glm::mat4 proj_;
	//glm::mat4 mvp_;
};

