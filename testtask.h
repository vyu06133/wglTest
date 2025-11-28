#pragma once

#include "TaskBase.h"
#include "MMDAsset.h"
#include "UFBXAsset.h"
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
		center_ = vec3(0.0f, 10.0f, 0.0f);
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
	DrawBuffer<VertexPNCT> m_vbo;
	std::vector<VertexPNCT> m_verts;
	virtual void OnTick(float deltaTime)
	{
		worldMatrix = mat4(1.0f);
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		app->m_BasicShader.UpdateUniformu("u_EnableTexture", 0);
		app->m_BasicShader.UpdateUniformu("u_EnablePrimitiveColor", 1);
		app->m_Constants.Data().SetWorld(worldMatrix);
//TRACE("%s	%s\n", __FUNCSIG__, glm::to_string(app->m_Constants.Data().eye).c_str());
		app->m_Constants.SendToGPU();
		m_vbo.Init(app->m_BasicShader.GetProgId());
		m_vbo.Begin(GL_TRIANGLES);
		m_vbo.Vertex(m_verts);
		m_vbo.End();
//		glDrawArrays(GL_LINE_STRIP, 0, m_vbo.GetVertexCount());
	}
	virtual void OnCreate()
	{
		GenerateCheckerPlaneZX(&m_verts, vec3(100.0f, 0.0f, 100.0f), vec3(10.0f), vec4(0.1f, 0.1f, 0.1f, 1.0f), vec4(0.9f, 0.9f, 0.9f, 1.0f));
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
	DrawBuffer<VertexPNCT> m_vbo;
	std::vector<VertexPNCT> m_verts;
	virtual void OnTick(float deltaTime)
	{
		auto t = vec3(0.0f, 3.0f, 0.0f);
		MyMath::SetTranslation(&localMatrix, t);
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		app->m_BasicShader.UpdateUniformmat4("WORLD", worldMatrix);
		app->m_BasicShader.UpdateUniformu("u_EnableTexture", 0);
		app->m_BasicShader.UpdateUniformu("u_EnableLighting", 0);
		m_vbo.Begin(GL_TRIANGLES);
		m_vbo.Vertex(m_verts);
		m_vbo.End();
	}
	virtual void OnCreate()
	{
		GenerateCube(&m_verts, vec3(3.0f));
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
	DrawBuffer<VertexPNCTAW> m_pnctaw;
	DrawBuffer<VertexPNCTA> m_pncta;
	DrawBuffer<VertexPNCT> m_pnct;
	DrawBuffer<VertexPC> m_pc;
#define FBX_DEFORM_SHADER true
	virtual void OnTick(float deltaTime)
	{
		auto app = ts->GetApp();
		auto& kb = app->m_Keyboard;
		fbx_.SetRootTransform(MyMath::CreateTranslation(vec3(0.0f, 0.0f, 20.0f))*MyMath::CreateScaling(vec3(0.2f)));

		fbx_.Update(deltaTime);
#if !FBX_DEFORM_SHADER
		fbx_.DeformBoneWeight();
#endif
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

#if FBX_DEFORM_SHADER
		prog.UpdateUniformu("u_EnableDeform", 1);
#else
		prog.UpdateUniformu("u_EnableDeform", 0);
#endif
		for (auto mesh = 0u; mesh < fbx_.GetMeshCount(); mesh++)
		{
#if FBX_DEFORM_SHADER
			//auto deformMatCnt =  ufbx_.GetDeformMatrixCount(mesh);
			//std::vector<mat4> deformMats(deformMatCnt);
			//ufbx_.GetDeformMatrix(mesh, deformMats.data(), deformMatCnt);
			//auto& matPalet = app->m_MatrixPalette;
			//std::memcpy(matPalet.Data().Matrices, deformMats.data(), sizeof(mat4) * std::min(matPalet.Data().MAX_PALETTE_SIZE, deformMatCnt));
			//matPalet.SendToGPU();
			fbx_.GetDeformation(mesh, app->m_MatrixPalette.Data().Matrices, App::MatrixPalette::MAX_PALETTE_SIZE);
			app->m_MatrixPalette.SendToGPU();
#endif
			auto minf = fbx_.GetMeshInfo(mesh);
			for (auto group = 0u; group < fbx_.GetMaterialGroupCount(mesh); group++)
			{
				auto geom = fbx_.GetMaterialGeometry(mesh, group);
				auto mat = fbx_.GetMaterialInfo(mesh,geom->material);
				//TRACE("%d %d %s\n", mesh, material, mat->textureFile.c_str());
				if (mat)
				{
					material.Data().ambientColor = mat->ambient;
					material.Data().diffuseColor = mat->diffuse;
					material.Data().emmisiveColor = mat->emmisive;
					material.Data().specularColor = mat->specular;
					material.Data().shininess = mat->shininess;
					material.SendToGPU();

					if (mat->texture)
					{
						mat->texture->BindTexture();
						prog.UpdateUniformu("u_EnableTexture", 1);
					}
					else
					{
						prog.UpdateUniformu("u_EnableTexture", 0);
					}
				}
#if FBX_DEFORM_SHADER
				std::vector<VertexPNCTAW> verts;
				fbx_.GetMeshPrim(mesh, group, &verts);
				m_pnctaw.Begin(GL_TRIANGLES);
				m_pnctaw.Vertex(verts);
				m_pnctaw.End();
#else
				std::vector<VertexPNCTA> verts;
				fbx_.GetMeshPrim(mesh, group, &verts);//note:deformバッファからgroup考慮アリで取得せねば
				m_pncta.Begin(GL_TRIANGLES);
				m_pncta.Vertex(verts);
				m_pncta.End();
#endif
			}
		}
		prog.UpdateUniformu("u_EnableDeform", 0);
		
		prog.UpdateUniformu("u_EnableFixedColor", 1);
		prog.UpdateUniformu("u_EnableFixedColor", 0);
		prog.UpdateUniformvec4("u_FixedColor", vec4(1.0f,1.0f,1.0f,1.0f));
		std::vector<VertexPC> pc;
		fbx_.RenderBone(&pc);//	fbx_.GetSkeleton(&pc);
		prog.UpdateUniformvec4("u_FixedColor", vec4(0.0f,1.0f,1.0f,1.0f));
		m_pc.Init(prog);
		m_pc.Begin(GL_LINES);
		m_pc.Vertex(pc);
		m_pc.End();
		prog.UpdateUniformu("u_EnableFixedColor", 0);
	}
	virtual void OnCreate()
	{
		ASSERT(ts);
		auto app = ts->GetApp();
		m_pncta.Init(app->m_BasicShader);
		m_pnctaw.Init(app->m_BasicShader);
		std::vector<VertexPC> pc;
		std::vector<VertexPNCTAW> pnctaw(6000);
//		ufbx_.LoadAsset("Assets\\fbx\\ruru - street dance.fbx", 1);
		fbx_.LoadAsset("Assets\\fbx\\ruru - Hip Hop Dancing (2).fbx");
//		fbx_.LoadAsset("Assets\\fbx\\The Boss - Hip Hop Dancing.fbx");
		fbx_.SetRootTransform(MyMath::CreateScaling(vec3(0.1f)));
		TRACE("Setup VertexPC for Bone\n");
		TRACE("Setup VertexPNCTAW for Mesh\n");
//		constants_.Gen();
//		constants_.Bind(app->m_BasicShader, "Constants");
//		material_.Gen();
//		material_.Bind(app->m_BasicShader, "Material");
		//return;
	}
	virtual void OnDestroy() {}
};

class UFBX : public TaskBase
{
public:
	UFBXAsset ufbx_;
	DrawBuffer<VertexPNCTAW> m_pnctaw;
	DrawBuffer<VertexPNCTA> m_pncta;
	DrawBuffer<VertexPNCT> m_pnct;
	DrawBuffer<VertexPC> m_pc;
	UFBXAsset::Geometry* geom0;
	UFBXAsset::Geometry* geom1;
	virtual void OnTick(float deltaTime)
	{
		auto app = ts->GetApp();
		auto& kb = app->m_Keyboard;
		ufbx_.SetRootTransform(MyMath::CreateTranslation(vec3(-15.0f, 0.0f, 0.0f)) * MyMath::CreateScaling(vec3(0.2f)));

		ufbx_.Update(deltaTime);
	}
#define UFBX_DEFORM_SHADER 1
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		auto& prog = app->m_BasicShader;
		app->m_LightInfo.SendToGPU();
		auto& constants = app->m_Constants;
		constants.Data().SetWorld(worldMatrix);
		constants.SendToGPU();

		auto& material = app->m_Material;

#if UFBX_DEFORM_SHADER
		prog.UpdateUniformu("u_EnableDeform", 1);
ufbx_.DeformBoneWeight();
#else
		prog.UpdateUniformu("u_EnableDeform", 0);
		ufbx_.DeformBoneWeight();
#endif
		auto meshCount = ufbx_.GetMeshCount();
		for (auto mesh = 0u; mesh < meshCount; mesh++)
		{
			auto meshinf = ufbx_.GetMeshInfo(mesh);
#if UFBX_DEFORM_SHADER
			ufbx_.GetDeformMatrix(mesh, app->m_MatrixPalette.Data().Matrices, App::MatrixPalette::MAX_PALETTE_SIZE);

			prog.UpdateUniformu("u_EnableDeform", 1);
			ufbx_.GetDeformMatrix(mesh, app->m_MatrixPalette.Data().Matrices, App::MatrixPalette::MAX_PALETTE_SIZE);
			app->m_MatrixPalette.SendToGPU();
#endif
			auto groupCount = ufbx_.GetMaterialGroupCount(mesh);
			for (auto group = 0u; group < groupCount; group++)
			{
				auto geom = ufbx_.GetMaterialGeometry(mesh, group);
				auto mat = ufbx_.GetMaterialInfo(geom->material);
				if (mat)
				{
					material.Data().ambientColor = mat->ambient;
					material.Data().diffuseColor = mat->diffuse;
					material.Data().emmisiveColor = mat->emmisive;
					material.Data().specularColor = mat->specular;
					material.Data().shininess = mat->shininess;
					material.SendToGPU();

					if (mat->texture)
					{
						mat->texture->BindTexture();
						prog.UpdateUniformu("u_EnableTexture", 1);
					}
					else
					{
						Texture2D::UnbindTexture();
						prog.UpdateUniformu("u_EnableTexture", 0);
					}
#if 0&&UFBX_DEFORM_SHADER
					prog.UpdateUniformu("u_debugFragColor", 1);
//					prog.UpdateUniformu("u_debugFragColor", 1);
//					prog.UpdateUniformu("u_EnableLighting", 0);
//					prog.UpdateUniformu("u_EnableTexture", 0);
					m_pnctaw.Begin(GL_TRIANGLES);
					m_pnctaw.Vertex(geom->vbuf);//bug：GLSLにboneWeight渡せてないかも？
					m_pnctaw.End();
					prog.UpdateUniformu("u_debugFragColor", 0);
					prog.UpdateUniformu("u_EnablePrimitiveColor", 0);
#else
					prog.UpdateUniformu("u_EnableDeform", 0);
					m_pncta.Begin(GL_TRIANGLES);
					m_pncta.Vertex(geom->deform);//note:デフォームをCPUで処理するこっちだと思わしい表示出来る
					m_pncta.End();
#endif
				}
			}
		}
		prog.UpdateUniformu("u_EnableDeform", 0);

		prog.UpdateUniformu("u_EnableFixedColor", 1);
		prog.UpdateUniformu("u_EnableFixedColor", 0);
		prog.UpdateUniformvec4("u_FixedColor", vec4(1.0f, 1.0f, 1.0f, 1.0f));
		std::vector<VertexPC> pc;
		ufbx_.GetSkeleton(&pc);
		prog.UpdateUniformvec4("u_FixedColor", vec4(0.0f, 1.0f, 1.0f, 1.0f));
		m_pc.Init(prog);
		m_pc.Begin(GL_LINES);
		m_pc.Vertex(pc);
		m_pc.End();
		prog.UpdateUniformu("u_EnableFixedColor", 0);
	}
	virtual void OnCreate()
	{
		ASSERT(ts);
		auto app = ts->GetApp();
		m_pncta.Init(app->m_BasicShader);
		m_pnctaw.Init(app->m_BasicShader);
		std::vector<VertexPC> pc;
		std::vector<VertexPNCTAW> pnctaw(6000);
		auto fname = "Assets\\fbx\\The Boss - BoomBoomBass.fbx";
		auto anim = 0;
		fname = "Assets\\fbx\\ruru - Hip Hop Dancing (2).fbx";
		ufbx_.LoadAsset(fname, anim);
		ufbx_.SetRootTransform(MyMath::CreateScaling(vec3(0.1f)));
		//		constants_.Gen();
		//		constants_.Bind(app->m_BasicShader, "Constants");
		//		material_.Gen();
		//		material_.Bind(app->m_BasicShader, "Material");
				//return;
		geom0 = ufbx_.GetMaterialGeometry(0, 0);
		geom1 = ufbx_.GetMaterialGeometry(1, 0);
	}
	virtual void OnDestroy() {}
};


class MMD : public TaskBase
{
public:
	MMDAsset mmd_;
	DrawBuffer<VertexPNT> pnt_;
	DrawBuffer<VertexPC> pc_;
	virtual void OnConstruct() override
	{
		mmd_.Init();
	}
	virtual void OnDestruct() override
	{
		mmd_.Term();
	}
	virtual void OnTick(float deltaTime)
	{
		auto app = ts->GetApp();
		auto& kb = app->m_Keyboard;

		///mmd_.Update(deltaTime * 30.0f);
		mmd_.Update(deltaTime);
		//mmd_.CalcBbox();
	}
	virtual void OnDraw()
	{
		auto app = ts->GetApp();
		auto& prog = app->m_BasicShader;
		auto& material = app->m_Material;
		prog.UpdateUniformu("u_EnableTexture", 0);
		prog.UpdateUniformu("u_EnableLighting", 0);
#if 0//bone
		{
			std::vector<VertexPC> pc;
			mmd_.DrawBone(&pc);
			glLineWidth(3.0f);
			pc_.Begin(GL_LINES);
			pc_.Vertex(pc);
			pc_.End();
		}
#endif

		//		glEnable(GL_DEPTH_TEST);
#if 1//Mesh
		{
			prog.UpdateUniformu("u_EnablePrimitiveColor", 0);
			for (auto m = 0u; m < mmd_.GetMaterialCount(); m++)
			{
				auto mat = mmd_.GetMaterial(m);
				//TRACE("%u %u %s\n", m, mat.vertexCount, mat.texname.c_str());
				material.Data().ambientColor = mat.ambient.rgb;
				material.Data().diffuseColor = vec3(1.0f);// mat.diffuse.rgb;
				//material.Data().emmisiveColor = mat.emmisive;
				material.Data().specularColor = mat.specular.rgb;
				material.Data().shininess = mat.specular.a;
				material.SendToGPU();

				auto tex2d = mat.tex2d;
				if (tex2d)
				{
					prog.UpdateUniformu("u_EnableTexture", 1);
					tex2d->BindTexture();
				}
				else
				{
					prog.UpdateUniformu("u_EnableTexture", 0);
				}
				std::vector<VertexPNT> pnt;
				mmd_.DrawMesh(m, &pnt);
				pnt_.Begin(GL_TRIANGLES);
				pnt_.Vertex(pnt);
				pnt_.End();
			}
		}
#endif

#if 1//bounding box
		{
			prog.UpdateUniformu("u_EnablePrimitiveColor", 1);
			std::vector<VertexPC> pc;
			mmd_.DrawBoneBbox(&pc);
			glLineWidth(1.0f);
			pc_.Begin(GL_LINES);
			pc_.Vertex(pc);
			pc_.End();
		}
#endif
		return;
	}
	virtual void OnCreate()
	{
		auto app = ts->GetApp();
		std::vector<VertexPC> pc;
		auto pmdFile = "Assets\\mmd\\Lat式ミク\\Lat式ミクVer2.31_Normalエッジ無し専用.pmd";
		std::string texdir = "Assets\\mmd\\Lat式ミク\\";
		pmdFile = "Assets\\mmd\\alicia_solid\\alicia_solid.pmd";
		texdir = "Assets\\mmd\\alicia_solid\\";
//		pmdFile = "Assets\\mmd\\制服JK(素体&モデル)v0.62\\水着JK_ビキニ.pmd";
//		texdir = "Assets\\mmd\\制服JK(素体&モデル)v0.62\\";
//		pmdFile = "Assets\\mmd\\20171102素体\\Ma_v20170227sam.pmd";
//		texdir = "Assets\\mmd\\20171102素体\\";
		mmd_.LoadPMDFromFile(pmdFile);
		auto vmdFile="Assets\\mmd\\bibbidiba_Short_Last〆付き_表情付き.vmd";
//		vmdFile = "Assets\\mmd\\OKPアバン（朝潮）.vmd";
//		vmdFile = "Assets\\mmd\\MMD_HOWL_ShortMotion.vmd";
//		vmdFile = "Assets\\mmd\\MMD_KonKoKonkonDanceMotion.vmd";
//		vmdFile = "Assets\\mmd\\ビリ モーション_01.vmd";
		mmd_.LoadVMDFromFile(vmdFile);
		mmd_.Prepare();
		for (auto m = 0u; m < mmd_.GetMaterialCount(); m++)
		{
			auto mat = mmd_.GetMaterial(m);
			std::string texname(mat.texname);
			if (texname.length())
			{
				std::replace(texname.begin(), texname.end(), '*', '\0');
				auto texfile = texdir + texname;
				auto tex2d = Texture2D::CreateFromFile(texfile.c_str());
				ASSERT(tex2d);
				mmd_.SetTexture(m, tex2d);
			}
		}
		pc_.Init(app->m_BasicShader, 3600);
		pnt_.Init(app->m_BasicShader, 30000/*mmd_.GetVerticesCount()*/);
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
	DrawBuffer <VertexPCT>m_vbFont;
	virtual void OnTick(float deltaTime)
	{
	}
	virtual void OnPostTick()
	{
	}
	virtual void OnCreate()
	{
		m_vbFont.Init(ts->GetApp()->m_HUD.GetProgId(), 2048);
	}
	virtual void OnDraw()
	{
#if _FONT_H_
		auto app = ts->GetApp();
		std::vector<VertexPCT> verts;
		app->m_font.RenderText(&verts, 0.0f, 0.0f, 1.0f, L"%d,%d,%016llx\n", app->m_Mouse.m_csrPos.x, app->m_Mouse.m_csrPos.y, *(uint64_t*)app->m_Mouse.m_CurrentMouseState.rgbButtons);
		auto loc = app->m_HUD.UpdateUniformu("u_tex",app->m_font.atlas_.TextureUnits());
		app->m_font.atlas_.BindTexture();
		app->m_vboFont.Begin(GL_TRIANGLES);
		app->m_vboFont.Vertex(verts);
		app->m_vboFont.End();

		auto t = app->FindTaskByName("MMD");
		if (t.size() && t[0])
		{
			auto m = dynamic_cast<MMD*>(t[0]);
			app->m_font.RenderText(&verts, 0.0f, 48.0f, 0.5f, L"%.2f", m->mmd_.GetFrameTime());
			auto view = app->m_Constants.Data().view;
			auto viewInv = glm::inverse(view);
			vec3 eye = viewInv[3].xyz;
 			app->m_font.RenderText(&verts, 0.0f, 48.0f, 0.5f, L"%S", glm::to_string(eye/30).c_str());
			app->m_vboFont.Begin(GL_TRIANGLES);
			app->m_vboFont.Vertex(verts);
			app->m_vboFont.End();
		}
#endif
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
//		ts3d->CreateTask<FBX>(nullptr, "FBX");
		ts3d->CreateTask<UFBX>(nullptr, "UFBX");
		ts3d->CreateTask<MMD>(nullptr, "MMD");
	}
};
