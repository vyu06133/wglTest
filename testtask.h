#pragma once

#include "TaskBase.h"
#include "FBXAsset.h"

class GameInstance
{
public:
	inline static GameInstance* instance_ = nullptr;
	inline static void CreateInstance()
	{
		instance_ = _NEW GameInstance;
	}
	inline static void DestroyInstance()
	{
		delete instance_;
		instance_ = nullptr;
	}
	inline static GameInstance* GetInstance()
	{
		return instance_;
	}
};

class Camera : public TaskBase
{
public:
	vec3 eye_;
	vec3 center_;
	vec3 up_ = { 0.0f, 1.0f, 0.0f };
	float near_ = 0.1f;
	float far_ = 99999.9f;
	float fov_ = 45.0f;//deg
	vec3 orbit_ = { 0.785f, 0.91f, 30.0f };
	mat4 world_;
	mat4 view_;
	mat4 projection_;
	virtual void OnConstruct()
	{
		auto app = ts->GetApp();
		projection_ = glm::perspective(glm::radians(fov_), app->clientSize_.x / app->clientSize_.y, near_, far_);
		view_ = glm::lookAt(eye_, center_, up_);
	}
	virtual void OnDestruct()	{}

	vec2 lastMouse;
	virtual void OnTick(float deltaTime)
	{
		auto app = ts->GetApp();
		app->m_BasicShader.UseProg();
		auto csrPos = app->m_Mouse.m_csrPos;
		auto wheel = app->m_Mouse.m_CurrentMouseState.lZ;
		//TRACE("wheel:%d\n", wheel);
		orbit_.z = MyMath::Max(5.0f, orbit_.z - wheel * 0.01f);
		if (app->m_Mouse.m_CurrentMouseState.rgbButtons[0])
		{
			float deltaX = (float)csrPos.x - lastMouse.x;
			float deltaY = (float)csrPos.y - lastMouse.y;
			
			// カメラの回転角度を更新
			orbit_.x = MyMath::RadWrap2PAI(orbit_.x + deltaX * 0.01f);
			orbit_.y = MyMath::RadWrap2PAI(orbit_.y - deltaY * 0.01f);
		}
		// マウスの座標を更新
		lastMouse.x = (float)csrPos.x;
		lastMouse.y = (float)csrPos.y;
		
		eye_.x = orbit_.z * MyMath::Sin(orbit_.y) * MyMath::Cos(orbit_.x);
		eye_.y = orbit_.z * MyMath::Cos(orbit_.y);
		eye_.z = orbit_.z * MyMath::Sin(orbit_.y) * MyMath::Sin(orbit_.x);
		center_ = vec3(0.0f);
		up_ = vec3(0.0f, 1.0f, 0.0f);

		projection_ = glm::perspective(glm::radians(fov_), app->clientSize_.x/ app->clientSize_.y, near_, far_);
		view_ = glm::lookAt(eye_, center_, up_);
	}
	virtual void OnPostTick()
	{
		auto app = ts->GetApp();
		app->m_Constants.Data().SetView(view_);
		app->m_Constants.Data().SetProjection(projection_);
//		app->m_Constants.Data().eye.x = (float)app->m_Mouse.m_csrPos.x / 800.0f;
//		app->m_Constants.Data().eye.y = (float)app->m_Mouse.m_csrPos.y / 600.0f;
	}
	virtual void OnDraw(){}
	virtual void OnCreate(){}
	virtual void OnDestroy(){}
};

class Light : public TaskBase
{
public:
	int index_;
	vec3 intensity_;
	virtual void OnConstruct()
	{
		auto app = ts->GetApp();
		index_ = 0;
		intensity_ = vec3(1.0f);
	}
	virtual void OnDestruct()	{}
	virtual void OnTick(float deltaTime)
	{
		localMatrix = MyMath::CreateTranslation(vec3(10, 10, 10));
	}
	virtual void OnPostTick()
	{
		auto app = ts->GetApp();
		app->m_BasicShader.UseProg();
		App::Constants& constants = app->m_Constants.Data();
		App::LightInfo& lightinfo = app->m_LightInfo.Data();
		lightinfo.Position_EyeCoord[index_] = worldMatrix[3];//if(w==1)PointLight
		lightinfo.Range[index_] = 0.0f;
		lightinfo.Intensity[index_] = vec3(0.5f);

	}
	virtual void OnDraw(){}
	virtual void OnCreate(){}
	virtual void OnDestroy(){}
};

class Field : public TaskBase
{
public:
	VertexBuffer<VertexPNCT> m_vbo;
	virtual void OnTick(float deltaTime)
	{
		worldMatrix = mat4(1.0f);
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		app->m_BasicShader.UpdateUniformu("u_EnableTexture", 0);
		app->m_Constants.Data().SetWorld(worldMatrix);
//TRACE("%s	%s\n", __FUNCSIG__, glm::to_string(app->m_Constants.Data().eye).c_str());
		app->m_Constants.SendToGPU();
		m_vbo.Bind();
		glDrawArrays(GL_TRIANGLES, 0, m_vbo.GetVertexCount());
	}
	virtual void OnCreate()
	{
		m_vbo.Setup(nullptr, 6*4*4, GL_DYNAMIC_DRAW);
		std::vector<VertexPNCT> v;
		GenerateCheckerPlaneZX(&v, vec3(10.0f, 0.0f, 10.0f), vec3(4.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), vec4(0.9f, 0.9f, 0.9f, 1.0f));
		m_vbo.UpdateData(v);
	}
	virtual void OnDestroy() {}
	void GenerateCheckerPlaneZX(std::vector<VertexPNCT>* vertices, const vec3& width, const vec3& grid, const vec4& color0, const vec4& color1)
	{
		ASSERT(vertices);
		int32_t gridz = static_cast<int32_t>(grid.z);
		int32_t gridx = static_cast<int32_t>(grid.x);
		float wZ = width.z / grid.z;
		float wX = width.x / grid.x;

		vec4 color[2] = { color0, color1 };

		for (int32_t j = 0; j < gridz; j++)
		{
			float z1 = -width.z * 0.5f + wZ * static_cast<float>(j);
			float z2 = z1 + wZ;
			float v1 = static_cast<float>(j + 0) / grid.z;
			float v2 = static_cast<float>(j + 1) / grid.z;
			for (int32_t i = 0; i < gridx; i++)
			{
				float x1 = -width.x * 0.5f + wX * static_cast<float>(i);
				float x2 = x1 + wX;
				float u1 = static_cast<float>(i + 0) / grid.x;
				float u2 = static_cast<float>(i + 1) / grid.x;
				auto n = vec3(0.0f, 1.0f, 0.0f);

				auto v11 = VertexPNCT(vec3(x1, 0.0f, z1), n, color[(i + j) & 1], vec2(u1, v1));
				auto v12 = VertexPNCT(vec3(x2, 0.0f, z1), n, color[(i + j) & 1], vec2(u2, v1));
				auto v21 = VertexPNCT(vec3(x1, 0.0f, z2), n, color[(i + j) & 1], vec2(u1, v2));
				auto v22 = VertexPNCT(vec3(x2, 0.0f, z2), n, color[(i + j) & 1], vec2(u2, v2));

				vertices->push_back(v11);
				vertices->push_back(v12);
				vertices->push_back(v22);
				vertices->push_back(v11);
				vertices->push_back(v22);
				vertices->push_back(v21);
			}
		}
	}

};

class Cube : public TaskBase
{
public:
	VertexBuffer<VertexPNCT> m_vbo;
	virtual void OnTick(float deltaTime)
	{
		auto t = vec3(0.0f, 3.0f, 0.0f);
		MyMath::SetTranslation(&localMatrix, t);
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		app->m_BasicShader.UpdateUniformmat4("WORLD", worldMatrix);
		m_vbo.Bind();
		glDrawArrays(GL_TRIANGLES, 0, m_vbo.GetVertexCount());
	}
	virtual void OnCreate()
	{
		m_vbo.Setup(nullptr, 6 * 6, GL_DYNAMIC_DRAW);
		std::vector<VertexPNCT> v;
		GenerateCube(&v, vec3(3.0f));
		m_vbo.UpdateData(v);
	}
	virtual void OnDestroy() {}
	void GenerateCube(std::vector<VertexPNCT>* vertices, const vec3& size)
	{
		vertices->clear();

		// A box has six faces, each one pointing in a different direction.
		constexpr int FaceCount = 6;

		static const vec3 faceNormals[FaceCount] =
		{
			{  0.0f,  0.0f,  1.0f },
			{  0.0f,  0.0f, -1.0f },
			{  1.0f,  0.0f,  0.0f },
			{ -1.0f,  0.0f,  0.0f },
			{  0.0f,  1.0f,  0.0f },
			{  0.0f, -1.0f,  0.0f },
		};

		static const vec2 texCoords[4] =
		{
			{ 1.0f, 0.0f },
			{ 1.0f, 1.0f },
			{ 0.0f, 1.0f },
			{ 0.0f, 0.0f },
		};

		auto tsize = size * 0.5f;

		// Create each face in turn.
		for (int i = 0; i < FaceCount; i++)
		{
			auto normal = faceNormals[i];

			// Get two vectors perpendicular both to the face normal and to each other.
			const auto basis = (i >= 4) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);

			const auto side1 = glm::cross(normal, basis);
			const auto side2 = glm::cross(normal, side1);

			{
			}

			vec4 col(MyMath::Abs(normal.x), MyMath::Abs(normal.y), MyMath::Abs(normal.z), 1.0f);
			// Four vertices per face.
			// (normal - side1 - side2) * tsize // normal // t0
		//	vertices.push_back(VertexPositionNormalTexture(XMVectorMultiply(XMVectorSubtract(XMVectorSubtract(normal, side1), side2), tsize), normal, textureCoordinates[0]));
			auto v0 = VertexPNCT((normal - side1 - side2) * tsize, normal, col, texCoords[0]);

			// (normal - side1 + side2) * tsize // normal // t1
		//	vertices.push_back(VertexPositionNormalTexture(XMVectorMultiply(XMVectorAdd(XMVectorSubtract(normal, side1), side2), tsize), normal, textureCoordinates[1]));
			auto v1 = VertexPNCT((normal - side1 + side2) * tsize, normal, col, texCoords[1]);

			// (normal + side1 + side2) * tsize // normal // t2
		//	vertices.push_back(VertexPositionNormalTexture(XMVectorMultiply(XMVectorAdd(normal, XMVectorAdd(side1, side2)), tsize), normal, textureCoordinates[2]));
			auto v2 = VertexPNCT((normal + side1 + side2) * tsize, normal, col, texCoords[2]);

			// (normal + side1 - side2) * tsize // normal // t3
		//	vertices.push_back(VertexPositionNormalTexture(XMVectorMultiply(XMVectorSubtract(XMVectorAdd(normal, side1), side2), tsize), normal, textureCoordinates[3]));
			auto v3 = VertexPNCT((normal + side1 - side2) * tsize, normal, col, texCoords[3]);

			(*vertices).push_back(v0);
			(*vertices).push_back(v1);
			(*vertices).push_back(v2);
			(*vertices).push_back(v0);
			(*vertices).push_back(v2);
			(*vertices).push_back(v3);
		}
	}

};

class FBX : public TaskBase
{
public:
	FBXAsset fbx_;
	VertexBuffer<VertexPNCTAW> m_pnctaw;
	VertexBuffer<VertexPNCT> m_pnct;
	VertexBuffer<VertexPC> m_pc;
	virtual void OnTick(float deltaTime)
	{
		auto app = ts->GetApp();
		auto& kb = app->m_Keyboard;
		if (kb.GetKeyDown(DIK_0))fbx_.SelectAnim(0);
		if (kb.GetKeyDown(DIK_1))fbx_.SelectAnim(1);
		if (kb.GetKeyDown(DIK_2))fbx_.SelectAnim(2);
		if (kb.GetKeyDown(DIK_3))fbx_.SelectAnim(3);
		if (kb.GetKeyDown(DIK_4))fbx_.SelectAnim(4);
		if (kb.GetKeyDown(DIK_5))fbx_.SelectAnim(5);
		if (kb.GetKeyDown(DIK_6))fbx_.SelectAnim(6);
		if (kb.GetKeyDown(DIK_7))fbx_.SelectAnim(7);
		if (kb.GetKeyDown(DIK_8))fbx_.SelectAnim(8);

		fbx_.Update(deltaTime);
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		auto& prog = app->m_BasicShader;
		app->m_LightInfo.SendToGPU();
		auto& constants = app->m_Constants;
		constants.Data().SetWorld(worldMatrix);
		constants.SendToGPU();
		
		auto& material = app->m_Material;
//		app->m_Material.Data().diffuseColor = vec4(1.0f);//test:
//		app->m_Material.Data().specularColor = vec4(0.0f, 1.0f, 0.0f,1.0f);//test:
//		app->m_Material.Data().shininess = 30.0f;//test:
		app->m_Material.SendToGPU();//test:
		//app->m_Material.Data().ambientColor = vec4(MyMath::Abs(MyMath::Sin(elapsed) * 5.f), 0, MyMath::Abs(MyMath::Sin(elapsed * 1.f)), 1.0f);
		//app->m_Material.Data().emmisiveColor;// = vec4(MyMath::mt.randf(), MyMath::mt.randf(), MyMath::mt.randf(), 1.0f);

		std::vector<VertexPNCT> wire;
		fbx_.RenderWire(&wire);
		m_pnct.UpdateData(wire);
		m_pnct.Bind();
//		glDrawArrays(GL_TRIANGLES, 0, m_pnct.GetVertexCount());

		std::vector<VertexPNCT> m;
		std::vector<VertexPNCTAW> pnctaw;
		prog.UpdateUniformu("u_EnableTexture", 0);
		for (auto i = 0u; i < fbx_.GetMeshCount(); i++)
		{
			for (auto j = 0u; j < fbx_.GetMaterialCount(i); j++)
			{
				auto mi = fbx_.GetMaterial(i, j);
				material.Data().ambientColor = vec4(mi->ambient.rgb, 1.0f);
				material.Data().diffuseColor = vec4(mi->diffuse.rgb, 1.0f);
				material.Data().emmisiveColor = vec4(mi->emmisive.rgb, 1.0f);
				material.Data().specularColor = vec4(mi->specular.rgb, 1.0f);
				material.Data().shininess = mi->shininess;
				if (mi && mi->texture)
				{
					auto d = material.Data();
					material.Data().diffuseColor = vec4(1.0f);//note:0では計算されぬ？
					prog.UpdateUniformu("u_EnableTexture", 1);
					prog.UpdateUniformu("u_TextureUnit", mi->texture->TextureUnits());
					mi->texture->ApplyParameter();
					mi->texture->BindTexture();
				}
		material.Data().ambientColor = vec4(0.5);//test:
		material.Data().diffuseColor = vec4(0.5);//test:
				material.SendToGPU();


				
#if SOFTWARE_DEFORM
				prog.UpdateUniformu("u_EnableDeform", 0);
				fbx_.GetWirePrim(i, j, &m);
				m_pnct.UpdateData(wire);
				m_pnct.Bind();
//				glDrawArrays(GL_TRIANGLES, 0, m_pnct.GetVertexCount());
				glDrawArrays(GL_TRIANGLES, 0, m_pnct.GetVertexCount());
#else
				prog.UpdateUniformu("u_EnableDeform", 1);
				fbx_.GetDeformation(i, app->m_MatrixPalette.Data().Matrices, App::MatrixPalette::MAX_PALETTE_SIZE);
				app->m_MatrixPalette.SendToGPU();
				fbx_.GetMeshPrim(i, j, &pnctaw);
				m_pnctaw.UpdateData(pnctaw);
				m_pnctaw.Bind();
				glDrawArrays(GL_TRIANGLES, 0, m_pnctaw.GetVertexCount());
//				glDrawArrays(GL_LINE_STRIP, 0, m_pnctaw.GetVertexCount());
#endif
		}
		}

		prog.UpdateUniformu("u_EnableDeform", 0);
		std::vector<VertexPC> v;
		fbx_.RenderBone(&v);
		m_pc.UpdateData(v);
		m_pc.Bind();
		glDrawArrays(GL_LINES, 0, m_pc.GetVertexCount());
	}
	virtual void OnCreate()
	{
		auto app = ts->GetApp();
		std::vector<VertexPC> pc;
		std::vector<VertexPNCTAW> pnctaw;
		fbx_.LoadAsset("Assets\\fbx\\256Ruru\\fbx\\Thriller Idle.fbx");
		fbx_.AddAnim("Assets\\fbx\\256Ruru\\fbx\\Thriller Part 1.fbx");
		fbx_.AddAnim("Assets\\fbx\\256Ruru\\fbx\\Thriller Part 2.fbx");
		fbx_.AddAnim("Assets\\fbx\\256Ruru\\fbx\\Thriller Part 3.fbx");
		fbx_.AddAnim("Assets\\fbx\\256Ruru\\fbx\\Thriller Part 4.fbx");
		fbx_.AddAnim("Assets\\fbx\\256Ruru\\fbx\\Hip Hop Dancing.fbx");
		fbx_.AddAnim("Assets\\fbx\\256Ruru\\fbx\\Punching Bag.fbx");
		TRACE("Setup VertexPC for Bone\n");
		pc.resize(fbx_.GetBonePrimCount());
		m_pc.Setup(pc, GL_DYNAMIC_DRAW);
		m_pnct.Setup(nullptr, fbx_.GetWirePrimCount(), GL_DYNAMIC_DRAW);
		TRACE("Setup VertexPNCTAW for Mesh\n");
		pnctaw.resize(fbx_.GetMeshPrimCount());
		m_pnctaw.ClearAttribPointer();
		m_pnctaw.Setup(pnctaw, GL_DYNAMIC_DRAW);
//		constants_.Gen();
//		constants_.Bind(app->m_BasicShader, "Constants");
//		material_.Gen();
//		material_.Bind(app->m_BasicShader, "Material");
	}
	virtual void OnDestroy() {}
};


class Hud : public TaskBase
{
public:
	virtual void OnTick(float deltaTime)
	{
	}
	virtual void OnPostTick()
	{
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		std::vector<VertexPCT> verts;
		app->m_font.RenderText(&verts, 0.0f, 0.0f, 1.0f, L"%d,%d,%016llx\n", app->m_Mouse.m_csrPos.x, app->m_Mouse.m_csrPos.y, *(uint64_t*)app->m_Mouse.m_CurrentMouseState.rgbButtons);
		app->m_vbFont.UpdateData(verts);
		auto loc = app->m_HUD.UpdateUniformu("u_tex",app->m_font.atlas_.TextureUnits());
		app->m_vbFont.Bind();
		app->m_font.atlas_.BindTexture();
		glDrawArrays(GL_TRIANGLES, 0, app->m_vbFont.GetVertexCount());

		auto t = app->FindTaskByName("FBX");
		if (t.size() && t[0])
		{
			auto f = dynamic_cast<FBX*>(t[0]);
//			app->m_font.RenderText(&verts, 0.0f, 48.0f, 0.5f, L"%.2f", f->fbx_.m_time);
			auto view = app->m_Constants.Data().view;
			auto viewInv = glm::inverse(view);
			vec3 eye = viewInv[3].xyz;
 			app->m_font.RenderText(&verts, 0.0f, 48.0f, 0.5f, L"%S", glm::to_string(eye/30).c_str());
			app->m_vbFont.UpdateData(verts);
			app->m_vbFont.Bind();
			glDrawArrays(GL_TRIANGLES, 0, app->m_vbFont.GetVertexCount());
		}
	}
};

class TestTask
{
public:
	inline static void Setup(TaskSystem* ts3d, TaskSystem* ts2d)
	{
		ts2d->CreateTask<Hud>(nullptr, "Hud");
		ts3d->CreateTask<Camera>(nullptr, "Camera");
		ts3d->CreateTask<Light>(nullptr, "Light");
		ts3d->CreateTask<Field>(nullptr, "Field");
		ts3d->CreateTask<FBX>(nullptr, "FBX");
	}
};