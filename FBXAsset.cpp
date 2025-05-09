#include "pch.h"
#include "MyUtil.h"
#include "FBXAsset.h"

//#include "FBXAsset_DX.h"

#pragma warning(disable: 4244)
#define USE_ROOT_TRANSFORM 1
#define GEN_FLAT_NORMAL 1
#define USE_SAMPLEPOINT 1
#define MOTION_INTERP_RATE 30.0f
#define MAKE_OBB_PER_CLUSTER 0
#define MAKE_OBB_THRESHOLD 0.5f


inline static glm::vec2 ToGlmVec2(const fbxsdk::FbxDouble2& fbxVec)
{
	return glm::vec2(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]));
}

inline static glm::vec3 ToGlmVec3(const fbxsdk::FbxDouble3& fbxVec)
{
	return glm::vec3(static_cast<float>(fbxVec[0]), static_cast<float>(fbxVec[1]), static_cast<float>(fbxVec[2]));
}

//inline static glm::vec3 ToGlmVec3(const Vector3& dxVec)
//{
//	return glm::vec3(static_cast<float>(dxVec.x), static_cast<float>(dxVec.y), static_cast<float>(dxVec.z));
//}



inline uint32_t GetNodeAttributeFlags(fbxsdk::FbxNode* node)
{
	uint32_t flags = 0;
	auto attrCount = node->GetNodeAttributeCount();
	for (auto i = 0; i < attrCount; i++)
	{
		auto attr = node->GetNodeAttributeByIndex(i);
		flags |= 1 << (attr ? attr->GetAttributeType() : 0);
	}
	return flags;
}

//------------------------------------------

FBXAsset::FBXAsset()
{
#if USE_GLM
	m_rootTransform = glm::mat4(1.0f);
#else
	m_rootTransform = Matrix::Identity;
#endif // !USE_GLM
}

FBXAsset::~FBXAsset()
{
}

bool FBXAsset::LoadAsset(const std::string& Filename)
{
	m_currentAssetPath = Filename;

	// マネージャを生成
	m_manager = fbxsdk::FbxManager::Create();
	ASSERT(m_manager);

	// IOSettingを生成
	auto ioSettings = fbxsdk::FbxIOSettings::Create(m_manager, IOSROOT);
	ASSERT(ioSettings);

	// Importerを生成
	auto importer = fbxsdk::FbxImporter::Create(m_manager, "");
	ASSERT(importer);
	if (!importer->Initialize(Filename.c_str(), -1, m_manager->GetIOSettings()))
	{
		// インポートエラー
		auto stat = importer->GetStatus();
		auto err = stat.GetErrorString();
		TRACE("%s : %s\n", err, Filename.c_str());
		DBGBREAK();
		return false;
	}

	// SceneオブジェクトにFBXファイル内の情報を流し込む
	m_scene = fbxsdk::FbxScene::Create(m_manager, "scene");
	if (!m_scene)
	{
		TRACE("Failed to create scene : %s", Filename.c_str());
		return false;
	}

	if (!importer->Import(m_scene))
	{
		m_scene->Destroy();
		m_scene = nullptr;
			
		TRACE("Failed to import file: %s", Filename.c_str());
		return false;
	}
	importer->Destroy(); // シーンを流し込んだらImporterは解放してOK
	
	
	fbxsdk::FbxGeometryConverter geomconv(m_manager);
	// シーン全体のメッシュを三角形化
	if (!geomconv.Triangulate(m_scene, true))
	{
		TRACE("Failed to Triangulate mesh : %s", Filename.c_str());
		return false;
	}

	auto root = m_scene->GetRootNode();
	ASSERT(root);
	TRACE("BuildNodeTree{{{\n");
	BuildNodeTree(&m_nodeTree, 0, root, nullptr);
	TRACE("}}}\n");
	TRACE("m_nodeTree: %d\n", m_nodeTree.GetCount());
	BuildNodeMap(&m_nodeMap, &m_nodeTree);

	//	今のところ未使用だがポーズを取得
	auto poseCount = m_scene->GetPoseCount();
	if (poseCount > 0)
	{
		for (auto i = 0; i < poseCount; i++)
		{
			m_poses.push_back(m_scene->GetPose(i));
		}
	}

	if (!ParsePose(&m_nodeTree, m_scene, MyUtil::StripPath(Filename)))
	{
		TRACE("Failed to parse pose : %s", Filename.c_str());
		return false;
	}

	if (!ParseAnimStack(&m_nodeTree, m_scene, MyUtil::StripPath(Filename)))
	{
		TRACE("Failed to parse animstack : %s", Filename.c_str());
		return false;
	}
	SelectAnim(1);

	// メッシュノードを取得
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		ParseMesh(itr);
	}
	//if (m_meshes.size() != 1)
	//{
	//	DBGBREAK();
	//	TRACE("found multi meshes\n");
	//	return false;
	//}

	return true;
}

bool FBXAsset::AddAnim(const std::string& Filename)
{
	if (!m_manager||!m_scene)
	{
		TRACE("Manager,Scene is not initialized\n");
		return false;
	}
	// Importerを生成
	auto importer = fbxsdk::FbxImporter::Create(m_manager, "");
	ASSERT(importer);
	if (!importer->Initialize(Filename.c_str(), -1, m_manager->GetIOSettings()))
	{
		// インポートエラー
		TRACE("Failed to init importer : %s", Filename.c_str());
		return false;
	}

	// SceneオブジェクトにFBXファイル内の情報を流し込む
	fbxsdk::FbxScene* scene = fbxsdk::FbxScene::Create(m_manager, "scene");
	if (!scene)
	{
		TRACE("Failed to create scene : %s", Filename.c_str());
		return false;
	}

	if (!importer->Import(scene))
	{
		scene->Destroy();
		scene = nullptr;

		TRACE("Failed to import file: %s", Filename.c_str());
		return false;
	}
	importer->Destroy(); // シーンを流し込んだらImporterは解放してOK
	
	NodeTree temp;
	BuildNodeTree(&temp, 0, scene->GetRootNode(), nullptr);

	return ParseAnimStack(&temp, scene, MyUtil::StripPath(Filename));
}

bool FBXAsset::ParseMaterial(MeshInfo* mi)
{
	ASSERT(mi);
	auto node = mi->node;
	ASSERT(node);
	auto materialCount = node->GetMaterialCount();

	for (auto i = 0; i < materialCount; i++)
	{
		fbxsdk::FbxSurfaceMaterial* material = node->GetMaterial(i);

		if (material)
		{
			TRACE("material[%d/%d]: '%s'\n", i, materialCount, material->GetName());
			auto materialInfo = _NEW MaterialInfo;
			PAUSE(materialInfo);
			fbxsdk::FbxDouble3 diffuse(1.0, 0.0, 1.0), specular, ambient, emissive;//note:<1,0,1>is magenta
			fbxsdk::FbxDouble shininess;
			fbxsdk::FbxProperty diffuseProp = material->FindProperty(fbxsdk::FbxSurfaceMaterial::sDiffuse);
			if (diffuseProp.IsValid())
			{
				diffuse = diffuseProp.Get<fbxsdk::FbxDouble3>();
				TRACE("diffuse: %f, %f, %f\n", diffuse[0], diffuse[1], diffuse[2]);
				materialInfo->diffuse = vec4(diffuse[0], diffuse[1], diffuse[2], 1.0f);
				int textureCount = diffuseProp.GetSrcObjectCount<fbxsdk::FbxTexture>();
				if (textureCount > 0)
				{
					fbxsdk::FbxTexture* texture = diffuseProp.GetSrcObject<fbxsdk::FbxTexture>(0);
					if (texture && texture->GetClassId().Is(fbxsdk::FbxFileTexture::ClassId))
					{
						auto fileTexture = static_cast<fbxsdk::FbxFileTexture*>(texture);
						auto fileName = MyUtil::StripPath(MyUtil::SlashToBackSlash(fileTexture->GetFileName()));
						TRACE("difffuse texture: '%s'\n", fileName.c_str());
						materialInfo->textureFile = MyUtil::RemoveFileSpec(m_currentAssetPath) + "\\" + fileName;
						TRACE("modify path: '%s'\n", materialInfo->textureFile.c_str());
						// OpenGLにテクスチャをロードしてバインド
						materialInfo->texture = Texture2D::CreateFromFile(materialInfo->textureFile.c_str());
						PAUSE(materialInfo->texture);
						if (materialInfo->texture)
						{
							auto wrapU = texture->GetWrapModeU();
							auto wrapV = texture->GetWrapModeV();
							materialInfo->texture->SetParameter(GL_CLAMP_TO_EDGE, GL_NEAREST);
						}
					}
				}
			}
			fbxsdk::FbxProperty specularProp = material->FindProperty(fbxsdk::FbxSurfaceMaterial::sSpecular);
			if (specularProp.IsValid())
			{
				specular = specularProp.Get<fbxsdk::FbxDouble3>();
				TRACE("specular: %f, %f, %f\n", specular[0], specular[1], specular[2]);
				materialInfo->specular = vec4(specular[0], specular[1], specular[2], 1.0f);
				fbxsdk::FbxProperty shininessProp = material->FindProperty(fbxsdk::FbxSurfaceMaterial::sShininess);
				if (shininessProp.IsValid())
				{
					shininess = shininessProp.Get<fbxsdk::FbxDouble>();
					TRACE("shininess: %f\n", shininess);
					materialInfo->shininess = static_cast<float>(shininess);
				}
				else
				{
					materialInfo->shininess = 0.0f;
				}
			}
			else
			{
				materialInfo->shininess = 0.0f;
			}
			fbxsdk::FbxProperty ambientProp = material->FindProperty(fbxsdk::FbxSurfaceMaterial::sAmbient);
			if (ambientProp.IsValid())
			{
				ambient = ambientProp.Get<fbxsdk::FbxDouble3>();
				TRACE("ambient: %f, %f, %f\n", ambient[0], ambient[1], ambient[2]);
				materialInfo->ambient = vec4(ambient[0], ambient[1], ambient[2], 1.0f);
			}
			fbxsdk::FbxProperty emissiveProp = material->FindProperty(fbxsdk::FbxSurfaceMaterial::sEmissive);
			if (emissiveProp.IsValid())
			{
				emissive = emissiveProp.Get<fbxsdk::FbxDouble3>();
				TRACE("emissive: %f, %f, %f\n", emissive[0], emissive[1], emissive[2]);
				materialInfo->emmisive = vec4(emissive[0], emissive[1], emissive[2], 1.0f);
			}
			TRACE("\n%s\n", materialInfo->ToString().c_str());
			mi->materials_.push_back(materialInfo);
			//materialInfo->Command();
			return true;
		}
	}
	return false;
}

bool FBXAsset::ParsePose(NodeTree* tree, fbxsdk::FbxScene* scene, const std::string& prefix)
{
	ASSERT(tree);
	ASSERT(scene);
	std::string name("initial");
	name = prefix + "/" + name;

	auto sheet = _NEW DopeSheet;
	ASSERT(sheet);
	sheet->name = name;
	sheet->stack_ = nullptr;
	sheet->start_ = 0.0f;
	sheet->stop_ = 0.0f;
	m_animStackNames.push_back(name);
	for (auto itr = (*tree).Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		auto node = ni.node;
		if (!node)
		{
			continue;
		}
		auto lclTranslation = node->LclTranslation;
		auto lclRotation = node->LclRotation;
		auto lclScaling = node->LclScaling;

		if (lclScaling.IsValid())
		{
			auto animName = MakeAnimName(node, &lclScaling);
			m_samplePoint.answers[animName].xyz = vec3(1.0f);
			auto curve = _NEW AnimCurveVec3;
			ASSERT(curve);
			curve->SetDefaultValue(vec3(lclScaling.Get()[0], lclScaling.Get()[1], lclScaling.Get()[2]));
			sheet->curves[animName] = curve;
		}
		if (lclRotation.IsValid())
		{
			auto animName = MakeAnimName(node, &lclRotation);
			m_samplePoint.answers[animName].q = quat();
			auto curve = _NEW AnimCurveQuat;
			ASSERT(curve);
			vec3 deg(lclRotation.Get()[0], lclRotation.Get()[1], lclRotation.Get()[2]);
			curve->SetDefaultValue(quat(glm::radians(deg)));
			sheet->curves[animName] = curve;
		}
		if (lclTranslation.IsValid())
		{
			auto animName = MakeAnimName(node, &lclTranslation);
			m_samplePoint.answers[animName].xyz = vec3(0.0f);
			auto curve = _NEW AnimCurveVec3;
			ASSERT(curve);
			curve->SetDefaultValue(vec3(lclTranslation.Get()[0], lclTranslation.Get()[1], lclTranslation.Get()[2]));
			sheet->curves[animName] = curve;
		}
	}
	m_dopeSheets[name] = sheet;
	// select
	m_currentDopeSheet = sheet;
	m_start = sheet->start_;
	m_stop = sheet->stop_;
	m_sampleAlpha = 0.0f;
	UpdateSamplePoint(m_start);//モーション補間の為SamplePointを計算しておく
	fbxsdk::FbxTime frameTime;
	frameTime.SetTime(0, 0, 0, 1, 0, m_scene->GetGlobalSettings().GetTimeMode());
	m_secPerFrame = frameTime.GetSecondDouble();
	//m_prevSamplePoint = m_samplePoint;
	//m_sampleAlpha = 0.0f;
	m_time = m_start;

	return true;
}

bool FBXAsset::ParseAnimStack(NodeTree* tree, fbxsdk::FbxScene* scene, const std::string& prefix)
{
	ASSERT(scene);
	TRACE("ParseAnimStack(%s)\n", prefix.c_str());
	fbxsdk::FbxArray<fbxsdk::FbxString*> AnimStackNameArray;
	scene->FillAnimStackNameArray(AnimStackNameArray);
	for (auto index = 0; index < AnimStackNameArray.GetCount(); index++)
	{
		std::string name = AnimStackNameArray[index]->Buffer();
		auto animStack = scene->FindMember<fbxsdk::FbxAnimStack>(name.c_str());
		TRACE("[%s]=%p\n", name.c_str(), animStack);
		name = prefix + "/" + name;

		auto sheet = _NEW DopeSheet;
		ASSERT(sheet);
		sheet->name = name;
		sheet->stack_ = animStack;
		sheet->start_ = animStack->LocalStart.Get().GetSecondDouble();
		sheet->stop_ = animStack->LocalStop.Get().GetSecondDouble();
		scene->SetCurrentAnimationStack(animStack);
		m_animStackNames.push_back(name);
		for (auto itr = (*tree).Begin(); itr; itr = itr->next)
		{
			auto& ni = itr->data;
			auto node = ni.node;
			if (!node)
			{
				continue;
			}
			auto lclTranslation = node->LclTranslation;
			auto lclRotation = node->LclRotation;
			auto lclScaling = node->LclScaling;

			auto sclCurve = ParseProperty(node, lclScaling);
			auto rotCurve = ParseProperty(node, lclRotation);
			auto posCurve = ParseProperty(node, lclTranslation);
			if (sclCurve)
			{
				//TRACE("sclCurve[%s]: %d\n", sclCurve->name.c_str(), sclCurve->keys.size());
				m_samplePoint.answers[sclCurve->name].xyz = vec3(1.0f);
				sheet->curves[sclCurve->name] = sclCurve;
				ni.lclScaling = sclCurve->name;
			}
			if (rotCurve)
			{
				//TRACE("rotCurve[%s]: %d\n", rotCurve->name.c_str(), rotCurve->keys.size());
				m_samplePoint.answers[rotCurve->name].q = quat();
				auto quatCurve = _NEW AnimCurveQuat();
				ASSERT(quatCurve);
				auto Vec3CurveToQuatCurve = [](AnimCurveQuat* dst, const AnimCurveVec3* src)
					{
						dst->name = src->name;
					//	dst->SetDefaultValue(MyMath::CreateFromYawPitchRollDeg(src->defaultValue));
						dst->SetDefaultValue(quat(glm::radians(src->defaultValue)));
						for (const auto& itr : src->keys)
						{
							dst->Insert(itr.first, quat(glm::radians(itr.second)));
						}
					};

				Vec3CurveToQuatCurve(quatCurve, rotCurve);
				sheet->curves[rotCurve->name] = quatCurve;
				ni.lclRotation = rotCurve->name;
				delete rotCurve;
			}
			if (posCurve)
			{
				//TRACE("posCurve[%s]: %d\n", posCurve->name.c_str(), posCurve->keys.size());
				m_samplePoint.answers[posCurve->name].xyz = vec3(0.0f);
				sheet->curves[posCurve->name] = posCurve;
				ni.lclTranslation = posCurve->name;
			}
		}
		m_dopeSheets[name] = sheet;
		// select
		m_currentDopeSheet = sheet;
		m_start = sheet->start_;
		m_stop = sheet->stop_;
	}
	m_sampleAlpha = 0.0f;
	UpdateSamplePoint(m_start);//モーション補間の為SamplePointを計算しておく
	fbxsdk::FbxTime frameTime;
	frameTime.SetTime(0, 0, 0, 1, 0, m_scene->GetGlobalSettings().GetTimeMode());
	m_secPerFrame = frameTime.GetSecondDouble();
	SelectAnim(0);
	m_time = m_start;
	m_sampleAlpha = 0.0f;//SelectAnimで1になってるはずだがparse時点で補間は不要
	
	return true;
}

inline FBXAsset::AnimCurveVec3* FBXAsset::ParseProperty(fbxsdk::FbxNode* node, fbxsdk::FbxPropertyT<fbxsdk::FbxDouble3>& property)
{
	if (!property.IsValid())
	{
		return nullptr;
	}
	auto result = _NEW AnimCurveVec3;
	ASSERT(result);
	result->SetDefaultValue(vec3(property.Get()[0], property.Get()[1], property.Get()[2]));
	result->name = MakeAnimName(node, &property);
	fbxsdk::FbxAnimCurveNode* curveNode = property.GetCurveNode();
	if (!curveNode)
	{
		//curveが無くてもDefaultValueコンテナとしてつかうのでAnimCurveを戻す
		return result;
	}
	fbxsdk::FbxAnimCurve* curveX = curveNode->GetCurve(0);
	fbxsdk::FbxAnimCurve* curveY = curveNode->GetCurve(1);
	fbxsdk::FbxAnimCurve* curveZ = curveNode->GetCurve(2);
	if (curveX && curveY && curveZ)
	{
		std::set<float> times;
		auto xCount = curveX->KeyGetCount();
		for (auto i = 0; i < xCount; i++)
		{
			auto time = curveX->KeyGetTime(i);
			times.insert(time.GetSecondDouble());
		}
		auto yCount = curveY->KeyGetCount();
		for (auto i = 0; i < yCount; i++)
		{
			auto time = curveY->KeyGetTime(i);
			times.insert(time.GetSecondDouble());
		}
		auto zCount = curveZ->KeyGetCount();
		for (auto i = 0; i < zCount; i++)
		{
			auto time = curveZ->KeyGetTime(i);
			times.insert(time.GetSecondDouble());
		}
		for (auto itr = times.begin(); itr != times.end(); itr++)
		{
			fbxsdk::FbxTime fbxtime;
			fbxtime.SetSecondDouble(*itr);
			result->Insert(*itr, vec3(curveX->Evaluate(fbxtime), curveY->Evaluate(fbxtime), curveZ->Evaluate(fbxtime)));
		}
		return result;
	}
	return result;
}

bool FBXAsset::SelectAnim(int number)
{
	if (number < 0 || number >= m_animStackNames.size())
	{
		TRACE("SelectAnim(%d) : Out Of Range\n", number);
		return false;
	}
	auto name = m_animStackNames[number];
	PAUSE(m_dopeSheets.find(name) != m_dopeSheets.end());
	auto& sheet = m_dopeSheets[name];
	m_scene->SetCurrentAnimationStack(sheet->stack_);
	m_currentDopeSheet = sheet;
	m_start = sheet->start_;
	m_stop = sheet->stop_;

	m_prevSamplePoint = m_samplePoint;
	m_sampleAlpha = 1.0f;
	return true;
}

bool FBXAsset::ParseMesh(NodeTree::Item* item)
{
	ASSERT(item && item->data.node);
	auto node = item->data.node;
	if (item->data.flags & ATTRIBUTE_FLAG_MESH)
	{
		auto m = _NEW MeshInfo;
		ASSERT(m);
		auto GeometricTranslation = node->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot);
		auto GeometricRotation = node->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot);
		auto GeometricScaling = node->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot);
		fbxsdk::FbxAMatrix GeometryOffset(GeometricTranslation, GeometricRotation, GeometricScaling);
		m->GeometryOffset = ToMyMat(GeometryOffset);
		m->node = node;
		m->name = node->GetName();
		m->mesh = node->GetMesh();
		ASSERT(m->mesh);

		//マテリアル
		ParseMaterial(m);
		TRACE("ParseMaterial : %d\n", m->materials_.size());

		if (!ParseMeshCP(m))
		{
			return false;
		}
		if (!ParseMeshAttr(m))
		{
			return false;
		}
		
		m_meshes.push_back(m);
		return true;
	}
	return false;
}

bool FBXAsset::ParseMeshCP(MeshInfo* meshinfo)
{
	ASSERT(meshinfo);
	auto fbxNode = meshinfo->node;
	auto fbxMesh = meshinfo->mesh;

	auto normalElement = meshinfo->mesh->GetElementNormal();
	auto normalMappingMode = normalElement ? normalElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto normalReferenceMode = normalElement ? normalElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto uvElement = meshinfo->mesh->GetElementUV();
	auto uvMappingMode = uvElement ? uvElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto uvReferenceMode = uvElement ? uvElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto tangentElement = meshinfo->mesh->GetElementTangent();
	auto tangentMappingMode = tangentElement ? tangentElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto tangentReferenceMode = tangentElement ? tangentElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto colorElement = meshinfo->mesh->GetElementVertexColor();
	auto colorMappingMode = colorElement ? colorElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto colorReferenceMode = colorElement ? colorElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto materialElement = meshinfo->mesh->GetElementMaterial(0);
	auto materialMappingMode = materialElement ? materialElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto materialReferenceMode = materialElement ? materialElement->GetReferenceMode() : FbxGeometryElement::eNone;

	// コントロールポイント数
	auto pointsCount = meshinfo->mesh->GetControlPointsCount();
	meshinfo->pointsCount = static_cast<uint32_t>(pointsCount);
	meshinfo->vbuf_.resize(pointsCount);
	meshinfo->cp_.resize(pointsCount);
	meshinfo->deform_.resize(pointsCount);
	
	// インデックスバッファ
	auto faceCount = meshinfo->mesh->GetPolygonCount();
	meshinfo->faceCount = faceCount;
	meshinfo->ibuf_.resize(faceCount * 3);
	meshinfo->MaterialIndex_.resize(faceCount);
	for (auto fi = 0; fi < faceCount; fi++)
	{
		// ポリゴンの頂点数 (通常は3で三角形)
		auto polygonSize = meshinfo->mesh->GetPolygonSize(fi);
		PAUSE(polygonSize == 3);//trianglateしているはず

		// ibuf
		PAUSE(fi * 3 + 0 < meshinfo->ibuf_.size());
		PAUSE(fi * 3 + 1 < meshinfo->ibuf_.size());
		PAUSE(fi * 3 + 2 < meshinfo->ibuf_.size());
		meshinfo->ibuf_[fi * 3 + 0] = meshinfo->mesh->GetPolygonVertex(fi, 0);
		meshinfo->ibuf_[fi * 3 + 1] = meshinfo->mesh->GetPolygonVertex(fi, 1);
		meshinfo->ibuf_[fi * 3 + 2] = meshinfo->mesh->GetPolygonVertex(fi, 2);
		
		// N/Poly
		if (normalElement)
		{
			if (normalMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				for (auto vi = 0; vi < polygonSize; vi++)
				{
					auto cpi = meshinfo->mesh->GetPolygonVertex(fi, vi);
					FbxVector4 normal = normalElement->GetDirectArray().GetAt(vi);
					meshinfo->cp_[cpi].normal = vec3((float)normal[0], (float)normal[1], (float)normal[2]);
				}
			}
			else if (normalMappingMode == FbxGeometryElement::eByControlPoint)
			{
				//次のループでやる
			}
			else
			{
				DBGBREAK();
				ASSERT(0);
			}
		}

		// C/Poly
		if (colorElement)
		{
			if (colorMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				for (auto vi = 0; vi < polygonSize; vi++)
				{
					auto cpi = meshinfo->mesh->GetPolygonVertex(fi, vi);
					FbxColor color = colorElement->GetDirectArray().GetAt(vi);
					meshinfo->cp_[cpi].color = vec4((float)color[0], (float)color[1], (float)color[2], (float)color[3]);
				}
			}
			else if (colorMappingMode == FbxGeometryElement::eByControlPoint)
			{
				//次のループでやる
			}
			else
			{
				DBGBREAK();
				ASSERT(0);
			}
		}
	}

	// N/CP
	if (normalElement)
	{
		if (normalMappingMode == FbxGeometryElement::eByControlPoint)
		{
			if (normalReferenceMode == FbxGeometryElement::eDirect)
			{
				for (auto cpi = 0u; cpi < meshinfo->pointsCount; cpi++)
				{
					FbxVector4 normal = normalElement->GetDirectArray().GetAt(cpi);
					meshinfo->cp_[cpi].normal = vec3((float)normal[0], (float)normal[1], (float)normal[2]);
				}
			}
			else if (normalReferenceMode == FbxGeometryElement::eIndexToDirect)
			{
				for (auto cpi = 0u; cpi < meshinfo->pointsCount; cpi++)
				{
					int index = normalElement->GetIndexArray().GetAt(cpi);
					FbxVector4 normal = normalElement->GetDirectArray().GetAt(index);
					meshinfo->cp_[cpi].normal = vec3((float)normal[0], (float)normal[1], (float)normal[2]);
				}
			}
		}
		else if (normalMappingMode == FbxGeometryElement::eByPolygonVertex)
		{
			//前のループでやった
		}
		else
		{
			DBGBREAK();
			ASSERT(0);
		}
	}
	
	// C/CP
	if (colorElement)
	{
		if (colorMappingMode == FbxGeometryElement::eByControlPoint)
		{
			if (colorReferenceMode == FbxGeometryElement::eDirect)
			{
				for (auto cpi = 0u; cpi < meshinfo->pointsCount; cpi++)
				{
					FbxColor color = colorElement->GetDirectArray().GetAt(cpi);
					meshinfo->cp_[cpi].color = vec4((float)color[0], (float)color[1], (float)color[2], (float)color[3]);
				}
			}
			else if (colorReferenceMode == FbxGeometryElement::eIndexToDirect)
			{
				for (auto cpi = 0u; cpi < meshinfo->pointsCount; cpi++)
				{
					int index = colorElement->GetIndexArray().GetAt(cpi);
					FbxColor color = colorElement->GetDirectArray().GetAt(index);
					meshinfo->cp_[cpi].color = vec4((float)color[0], (float)color[1], (float)color[2], (float)color[3]);
				}
			}
		}
		else if (colorMappingMode == FbxGeometryElement::eByPolygonVertex)
		{
			//前のループでやった
		}
		else
		{
			DBGBREAK();
			ASSERT(0);
		}
	}

	// コントロールポイント
	for (auto i = 0; i < pointsCount; i++)
	{
		auto p = meshinfo->mesh->GetControlPointAt(i);
		meshinfo->cp_[i].pos.x = p[0];
		meshinfo->cp_[i].pos.y = p[1];
		meshinfo->cp_[i].pos.z = p[2];
		meshinfo->cp_[i].weight.Reset();
		meshinfo->vbuf_[i].Reset();
		meshinfo->deform_[i].Reset();
	}

#pragma region BoneWeight
	auto si = &meshinfo->skin;

	auto deformer = meshinfo->mesh->GetDeformer(0, FbxDeformer::eSkin);
	// deformer が無いシーンもあり得る
	auto skinDeformer = FbxCast<FbxSkin>(deformer);
	if (skinDeformer)
	{
		auto GeometricTranslation = fbxNode->GetGeometricTranslation(fbxsdk::FbxNode::eSourcePivot);
		auto GeometricRotation = fbxNode->GetGeometricRotation(fbxsdk::FbxNode::eSourcePivot);
		auto GeometricScaling = fbxNode->GetGeometricScaling(fbxsdk::FbxNode::eSourcePivot);
		fbxsdk::FbxAMatrix GeometryOffset(GeometricTranslation, GeometricRotation, GeometricScaling);

		auto skinningType = skinDeformer->GetSkinningType();
		//		PAUSE(skinningType == FbxSkin::eLinear || skinningType == FbxSkin::eRigid);
		si->skin = skinDeformer;
		auto clusterCount = skinDeformer->GetClusterCount();
		//ここまでにFbxNode*から行列パレットのIDを見つけられるようにしとく
		//Nameをキーにすることは可能なはず
		for (int clusterIndex = 0; clusterIndex < clusterCount; clusterIndex++)
		{
			ClusterInfo ci;
			// クラスタ(ボーン)の取り出し
			fbxsdk::FbxCluster* cluster = skinDeformer->GetCluster(clusterIndex);
			ASSERT(cluster);
			ci.cluster = cluster;
			PAUSE(cluster->GetLinkMode() == FbxCluster::eNormalize);
			//ウェイトが正規化されており、すべての影響するボーンのウェイトの合計が1になるように調整されます。
			//一般的なスキニングモードです。
			fbxsdk::FbxAMatrix clusterTransform;
			fbxsdk::FbxAMatrix clusterTransformLink;
			cluster->GetTransformMatrix(clusterTransform);
			ci.SetTransformMatrix(ToMyMat(clusterTransform));
			clusterTransform *= GeometryOffset;
			cluster->GetTransformLinkMatrix(clusterTransformLink);
			ci.SetTransformLinkMatrix(ToMyMat(clusterTransformLink));
			auto link = cluster->GetLink();
			PAUSE(link != nullptr);
			if (m_nodeMap.find(link->GetName()) == m_nodeMap.end())
			{
				TRACE("bone: '%s' not found\n", link->GetName());
				continue;
			}
			ci.boneName = link->GetName();
			auto pointCount = cluster->GetControlPointIndicesCount();
			auto indicesPtr = cluster->GetControlPointIndices();
			auto weightsPtr = cluster->GetControlPointWeights();
			//todo:クラスタ毎にOBBを作って保持する∵コリジョンに使える
#if MAKE_OBB_PER_CLUSTER
			if (pointCount)
			{
				auto bone = m_nodeMap[ci.boneName];
				auto boneTransform = CalcWorldTransform(bone);
				auto invBoneTransform = glm::inverse(boneTransform);

				OBB& obb = ci.obb;
				std::vector<vec3> obbPoints;
				obbPoints.reserve(pointCount);
				for (int i = 0; i < pointCount; i++)
				{
					//if (weightsPtr[i] > MAKE_OBB_THRESHOLD)
					{
						obbPoints.push_back(MyTransform(invBoneTransform, meshinfo->vbuf_[indicesPtr[i]].pos));
					}
				}
				obb.Calc(obbPoints);
				//TRACE("%s\n", obb.ToString().c_str());
			}
#endif
#if USE_CLUSTER_INDICES
			ci.indicesCount = pointCount;
			ci.indices.reserve(pointCount);
			ci.weights.reserve(pointCount);
#endif
			PAUSE(si->clusters.size() == clusterIndex);
			for (int i = 0; i < pointCount; i++)
			{
				auto w = weightsPtr[i];
				if (!meshinfo->cp_[indicesPtr[i]].weight.AddBoneData(clusterIndex, w))
				{
					TRACE("BoneData Overflow\n");
					continue;
				}
#if USE_CLUSTER_INDICES
				ci.indices.push_back(indicesPtr[i]);
				ci.weights.push_back(static_cast<float>(weightsPtr[i]));
#endif
			}
			for (int i = 0; i < pointCount; i++)
			{
				meshinfo->cp_[indicesPtr[i]].weight.Sort();
			}
			si->clusters.push_back(ci);
		}
	}
	else
	{
		si->skin = nullptr;
	}
#pragma endregion

	return true;
}

int32_t FBXAsset::AddOrFindVbuf(MeshInfo* meshinfo, const VertexPNCTAW& v)
{
	for (uint32_t i = 0u; i < meshinfo->vbuf_.size(); i++)
	{
		auto& vbuf = meshinfo->vbuf_[i];
		if (vbuf.pos == v.pos &&
			vbuf.normal == v.normal &&
			vbuf.color == v.color &&
			vbuf.tex == v.tex &&
			vbuf.weight==v.weight)
		{
			return i;
		}
	}
	uint32_t newIdx = (uint32_t)meshinfo->vbuf_.size();
	meshinfo->vbuf_.push_back(v);
	return newIdx;
}

bool FBXAsset::ParseMeshAttr(MeshInfo* meshinfo)
{
	/*
	* note:meshinfoのvbuf.texを取得、格納していく
	* 同一コントロールポイントで異なるUVの場合は共有頂点に出来ないので丸ごと作り直す
	* タンジェントはFbxSceneから取得せず計算してしまう
	* vbuf作り直しするのでibufも作り直す
	*/
	ASSERT(meshinfo);
	auto fbxNode = meshinfo->node;
	auto fbxMesh = meshinfo->mesh;

	auto normalElement = meshinfo->mesh->GetElementNormal();
	auto normalMappingMode = normalElement ? normalElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto normalReferenceMode = normalElement ? normalElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto uvElement = meshinfo->mesh->GetElementUV();
	auto uvMappingMode = uvElement ? uvElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto uvReferenceMode = uvElement ? uvElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto tangentElement = meshinfo->mesh->GetElementTangent();
	auto tangentMappingMode = tangentElement ? tangentElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto tangentReferenceMode = tangentElement ? tangentElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto colorElement = meshinfo->mesh->GetElementVertexColor();
	auto colorMappingMode = colorElement ? colorElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto colorReferenceMode = colorElement ? colorElement->GetReferenceMode() : FbxGeometryElement::eNone;

	auto materialElement = meshinfo->mesh->GetElementMaterial(0);
	auto materialMappingMode = materialElement ? materialElement->GetMappingMode() : FbxGeometryElement::eNone;
	auto materialReferenceMode = materialElement ? materialElement->GetReferenceMode() : FbxGeometryElement::eNone;

	TRACE("vbuf&ibuf reset\n");
	auto pointsCount = meshinfo->pointsCount;
	auto faceCount = meshinfo->mesh->GetPolygonCount();
	meshinfo->faceCount = faceCount;
	meshinfo->vbuf_.clear();
	meshinfo->vbuf_.reserve(pointsCount);
	meshinfo->ibuf_.clear();
	meshinfo->ibuf_.reserve(faceCount * 3);//多分faceCount*3～triangle～がibufサイズの下限ではないか
	meshinfo->MaterialIndex_.clear();
	meshinfo->MaterialIndex_.reserve(faceCount);
	for (auto fi = 0; fi < faceCount; fi++)
	{
		//新しくvbuf用vertexを準備
		auto cpi0 = meshinfo->mesh->GetPolygonVertex(fi, 0);
		auto cpi1 = meshinfo->mesh->GetPolygonVertex(fi, 1);
		auto cpi2 = meshinfo->mesh->GetPolygonVertex(fi, 2);
		auto& cp0 = meshinfo->cp_[cpi0];
		auto& cp1 = meshinfo->cp_[cpi1];
		auto& cp2 = meshinfo->cp_[cpi2];
		VertexPNCTAW newVtx[3] = {
			VertexPNCTAW(cp0.pos, cp0.normal, cp0.color, vec2(0.0f), vec3(0.0f), cp0.weight),
			VertexPNCTAW(cp1.pos, cp1.normal, cp1.color, vec2(0.0f), vec3(0.0f), cp1.weight),
			VertexPNCTAW(cp2.pos, cp2.normal, cp2.color, vec2(0.0f), vec3(0.0f), cp2.weight),
		};
		// ポリゴン毎のマテリアル割り当て
		int materialIndex = 0;
		if (materialMappingMode == FbxGeometryElement::eByPolygon)
		{
			auto materialindices = materialElement->GetIndexArray();
			materialIndex = materialindices.GetAt(fi);
		}
		else
		{
			if (materialMappingMode == FbxGeometryElement::eAllSame)
			{
				materialIndex = 0;
			}
			else
			{
				assert(0);
				materialIndex = 0;
			}
		}
		if (uvElement)
		{
			if (uvMappingMode == FbxGeometryElement::eByPolygonVertex)
			{
				auto polygonSize = meshinfo->mesh->GetPolygonSize(fi);
				ASSERT(polygonSize == 3);

				fbxsdk::FbxVector2 uv;
				for (auto vidx = 0; vidx < polygonSize; vidx++)
				{
					// コントロールポイントインデックス
					if (uvReferenceMode == FbxGeometryElement::eDirect)
					{
						auto cpi = meshinfo->mesh->GetPolygonVertex(fi, vidx);
						uv = uvElement->GetDirectArray().GetAt(cpi);
						uv[1] = 1.0 - uv[1];
						newVtx[vidx].tex = ToGlmVec2(uv);
					}
					else if (uvReferenceMode == FbxGeometryElement::eIndexToDirect)
					{
						auto cpi = uvElement->GetIndexArray().GetAt(fi * polygonSize + vidx);
						uv = uvElement->GetDirectArray().GetAt(cpi);
						uv[1] = 1.0 - uv[1];
						newVtx[vidx].tex = ToGlmVec2(uv);
					}
					else
					{
						ASSERT(0);
					}
				}
			}
			else
			{
				DBGBREAK();
				ASSERT(0);
			}
		}
		auto vi0 = AddOrFindVbuf(meshinfo, newVtx[0]);
		auto vi1 = AddOrFindVbuf(meshinfo, newVtx[1]);
		auto vi2 = AddOrFindVbuf(meshinfo, newVtx[2]);
		meshinfo->MaterialIndex_.push_back(materialIndex);
		meshinfo->ibuf_.push_back(vi0);
		meshinfo->ibuf_.push_back(vi1);
		meshinfo->ibuf_.push_back(vi2);
	}
	meshinfo->faceCount = (uint32_t)meshinfo->MaterialIndex_.size();
	meshinfo->pointsCount = (uint32_t)meshinfo->vbuf_.size();
	TRACE("faceCount=%d pointsCont=%d\n", meshinfo->faceCount, meshinfo->pointsCount);
	meshinfo->deform_.resize(meshinfo->pointsCount);
	return true;
#pragma region TANGENT
#if 1
	// マッピングモードとリファレンスモードを確認
	{
		// タンジェントはここで作る
		for (auto& v : meshinfo->vbuf_)
		{
			v.tangent.x = 0.0f;
			v.tangent.y = 0.0f;
			v.tangent.z = 0.0f;
		}
		for (auto i = 0; i < meshinfo->ibuf_.size(); i += 3)
		{
			auto i0 = meshinfo->ibuf_[i + 0];
			auto i1 = meshinfo->ibuf_[i + 1];
			auto i2 = meshinfo->ibuf_[i + 2];

			const auto& v0 = meshinfo->vbuf_[i0].pos;
			const auto& v1 = meshinfo->vbuf_[i1].pos;
			const auto& v2 = meshinfo->vbuf_[i2].pos;

			auto& uv0 = meshinfo->vbuf_[i0].tex;
			auto& uv1 = meshinfo->vbuf_[i1].tex;
			auto& uv2 = meshinfo->vbuf_[i2].tex;

			auto deltaPos1 = v1 - v0;
			auto deltaPos2 = v2 - v0;

			auto deltaUV1 = uv1 - uv0;
			auto deltaUV2 = uv2 - uv0;

			float r = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV1.y * deltaUV2.x);
			auto tangent = (deltaPos1 * deltaUV2.y - deltaPos2 * deltaUV1.y) * r;

			meshinfo->vbuf_[i0].tangent += tangent;
			meshinfo->vbuf_[i1].tangent += tangent;
			meshinfo->vbuf_[i2].tangent += tangent;
		}
		// 各頂点のタンジェントを正規化
		for (auto& v : meshinfo->vbuf_)
		{
			v.tangent = glm::normalize(v.tangent);
		}
	}
#endif
#pragma endregion
//for (auto i = 0u; i < pointsCount; i++)
//{
//	meshinfo->vbuf_[i].color = meshinfo->vbuf_[i].weight.Weights;//noto:BONEWEIGHTをCOLORに移動
//}

	return true;
}

void FBXAsset::DeleteAsset()
{
	if (m_scene)
	{
		m_scene->Destroy();
		m_scene = nullptr;
	}
	if (m_manager)
	{
		m_manager->Destroy();
		m_manager = nullptr;
	}
	for (auto& ds : m_dopeSheets)
	{
		if (ds.second)
		{
			delete ds.second;
			ds.second = nullptr;
		}
	}
	m_dopeSheets.clear();
	for (auto& m : m_meshes)
	{
		if (m)
		{
			delete m;
			m = nullptr;
		}
	}
	m_meshes.clear();
	m_nodeTree.Clear();
	if (m_scene)
	{
		m_scene->Destroy();
		m_scene = nullptr;
	}
}

void FBXAsset::Update(float delta)
{
	m_time += m_secPerFrame;
	if (m_time > m_stop)
	{
		m_time -= (m_stop - m_start);
	}
	
	m_sampleAlpha -= delta * MOTION_INTERP_RATE;
	if (m_sampleAlpha < 0.0f)
	{
		m_sampleAlpha = 0.0f;
	}
	
#if USE_GLM
	m_rootTransform = glm::scale(mat4(1.0f), vec3(0.1f));
#else
	m_rootTransform = Matrix::CreateScale(0.1f);
#endif
	UpdateSamplePoint(m_time);
	UpdateLocalTransforms();
	UpdateWorldTransforms();
#if SOFTWARE_DEFORM
	DeformBoneWeight();//note:シェーダーでデフォームする場合これを呼ばない
#endif
}

void FBXAsset::Render()
{
	//glEnable(GL_DEPTH_TEST);
	//glEnable(GL_LIGHT0);
	//glEnable(GL_LIGHTING);
	//RenderMesh();
	//glDisable(GL_DEPTH_TEST);
	//glDisable(GL_TEXTURE_2D);
	//glDisable(GL_LIGHT0);
	//glDisable(GL_LIGHTING);
	//glDisable(GL_COLOR_MATERIAL);
	//RenderBone();
}

uint32_t FBXAsset::GetMeshCount() const
{
	return static_cast<uint32_t>(m_meshes.size());
}

uint32_t FBXAsset::GetBoneCount(uint32_t mesh) const
{
	return static_cast<uint32_t>(m_meshes[mesh]->skin.clusters.size());
}

uint32_t FBXAsset::GetDeformation(uint32_t mesh, mat4* dst, size_t dstsize) const
{
	// todo:ここでidentity行列にしてvsのデフォームロジックを検証する
	auto boneCount = GetBoneCount(mesh);
	ASSERT(boneCount <= dstsize);
	auto& m = m_meshes[mesh];
	auto& si = m->skin;
	uint32_t i = 0;
	for (auto& ci : si.clusters)
	{
		if (i > dstsize)
		{
			TRACE("deform buffer overflow\n");
			ASSERT(0);
			break;
		}
		dst[i] = ComputeClusterDeformation(m, ci);
//		dst[i] = glm::transpose(ComputeClusterDeformation(m, ci));//DirectXではtranspose必要？
		i++;
	}
	return i;
}

uint32_t FBXAsset::GetMaterialCount(uint32_t mesh) const
{
	return static_cast <uint32_t>(m_meshes[mesh]->materials_.size());
}

FBXAsset::MaterialInfo* FBXAsset::GetMaterial(uint32_t mesh, uint32_t material) const
{
	return m_meshes[mesh]->materials_[material];
}

void FBXAsset::GetPrimitive(uint32_t mesh, uint32_t material, PrimitiveBatchPNCTAW* primitive)
{
#if 0
	auto m = m_meshes[mesh];
	auto& si = m->skin;
	for (auto i = 0u; i < m->ibuf_.size(); i += 3)
	{
		auto polyIdx = i / 3;
		if (polyIdx < m->MaterialIndex_.size() && m->MaterialIndex_[polyIdx] == material)
		{
			auto a = m->vbuf_[m->ibuf_[i + 0]];
			auto b = m->vbuf_[m->ibuf_[i + 1]];
			auto c = m->vbuf_[m->ibuf_[i + 2]];
		//	a.normal = b.normal = c.normal = MyMath::CalcTriangleNormal(a.pos, b.pos, c.pos);
		//	a.weight.Weight(0) = 1.0f;
		//	b.weight.Weight(1) = 1.0f;
		//	c.weight.Weight(2) = 1.0f;
		//	a.weight.Weights = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		//	b.weight.Weights = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		//	c.weight.Weights = Vector4(0.0f, 0.0f, 1.0f, 0.0f);
		//	a.tangent = Vector3(&a.weight.Weights.x);
		//	b.tangent = Vector3(&b.weight.Weights.x);
		//	c.tangent = Vector3(&c.weight.Weights.x);
			primitive->DrawTriangle(c, b, a);
		}
	}
#endif
}

uint32_t FBXAsset::GetWirePrimCount() const
{
	uint32_t sum = 0u;
	for (auto i = 0u; i < GetMeshCount(); i++)
	{
		sum += (uint32_t)m_meshes[i]->ibuf_.size();
	}
	return sum;
}

uint32_t FBXAsset::GetBonePrimCount() const
{
	uint32_t sum = 0u;
	for (auto i = 0u; i < GetMeshCount(); i++)
	{
		sum += (uint32_t)GetBoneCount(i) * 2;
	}
	return sum;
}

uint32_t FBXAsset::GetMeshPrimCount() const
{
	uint32_t sum = 0u;
	for (auto i = 0u; i < GetMeshCount(); i++)
	{
		sum += (uint32_t)m_meshes[i]->ibuf_.size();
	}
	return sum;
}

void FBXAsset::RenderMesh(std::vector<VertexPNCTAW>* primitive)
{
#if 0
	for (auto m : m_meshes)
	{
		for (auto i = 0; i < m->ibuf_.size(); i += 3)
		{
			auto& a = m->deform_[m->ibuf_[i + 0]];
			auto& b = m->deform_[m->ibuf_[i + 1]];
			auto& c = m->deform_[m->ibuf_[i + 2]];
			a.normal = b.normal = c.normal = MyMath::CalcTriangleNormal(a.pos, b.pos, c.pos);
			primitive->DrawTriangle(c, b, a);
//			primitive->DrawLine(a, b);
//			primitive->DrawLine(b, c);
//			primitive->DrawLine(c, a);
		}
//		glEnd();
//#endif
	}
//	glColor3f(1.0f, 1.0f, 1.0f);
#endif
}

void FBXAsset::RenderWire(std::vector<VertexPNCT>* primitive)
{
	for (auto m : m_meshes)
	{
		for (auto i = 0; i < m->ibuf_.size(); i += 3)
		{
			auto ia = m->ibuf_[i + 0];
			auto ib = m->ibuf_[i + 1];
			auto ic = m->ibuf_[i + 2];
			auto& a = m->deform_[ia];
			auto& b = m->deform_[ib];
			auto& c = m->deform_[ic];
			primitive->push_back(VertexPNCT(a.pos, a.normal, a.color, a.tex));
			primitive->push_back(VertexPNCT(b.pos, b.normal, b.color, b.tex));
			primitive->push_back(VertexPNCT(c.pos, c.normal, c.color, c.tex));
		}
	}
}

void FBXAsset::RenderBone(std::vector<VertexPC>* primitive)
{
#if 1
	/* each node */
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		if (!(itr->data.flags & ATTRIBUTE_FLAG_SKELETON))
		{
			continue;
		}
		auto posf = MyMath::GetTranslation(itr->data.worldTransform);
	}
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		if (!(itr->data.flags & ATTRIBUTE_FLAG_SKELETON))
		{
			continue;
		}
		if (itr->parent)
		{
			vec4 c(1.0f);
			auto posf = MyMath::GetTranslation(itr->data.worldTransform);
			auto pposf = MyMath::GetTranslation(itr->parent->data.worldTransform);
			primitive->push_back(VertexPC(posf, c));
			primitive->push_back(VertexPC(pposf, c));
		}
	}
#endif
}

void FBXAsset::GetWirePrim(uint32_t mesh, uint32_t material, std::vector<VertexPNCT>* primitive)
{
	auto m = m_meshes[mesh];
	auto& si = m->skin;
	for (auto i = 0u; i < m->ibuf_.size(); i += 3)
	{
		auto polyIdx = i / 3;
		if (polyIdx < m->MaterialIndex_.size() && m->MaterialIndex_[polyIdx] == material)
		{
			auto a = m->deform_[m->ibuf_[i + 0]];
			auto b = m->deform_[m->ibuf_[i + 1]];
			auto c = m->deform_[m->ibuf_[i + 2]];
//			a.normal = b.normal = c.normal = MyMath::CalcTriangleNormal(a.pos, b.pos, c.pos);
//			a.color = b.color = c.color = vec4(1.0f, 0.0f, 1.0f, 1.0f);

			primitive->push_back(a);
			primitive->push_back(b);
			primitive->push_back(c);
		}
	}
}

void FBXAsset::GetMeshPrim(uint32_t mesh, uint32_t material, std::vector<VertexPNCTAW>* primitive)
{
	auto m = m_meshes[mesh];
	auto& si = m->skin;
	for (auto i = 0u; i < m->ibuf_.size(); i += 3)
	{
		auto polyIdx = i / 3;
		if (polyIdx < m->MaterialIndex_.size() && m->MaterialIndex_[polyIdx] == material)
		{
			auto a = m->vbuf_[m->ibuf_[i + 0]];
			auto b = m->vbuf_[m->ibuf_[i + 1]];
			auto c = m->vbuf_[m->ibuf_[i + 2]];
			primitive->push_back(a);
			primitive->push_back(b);
			primitive->push_back(c);
		}
	}
}


inline static const char* typeNames[] =
{
	"eUnknown", "eNull", "eMarker", "eSkeleton", "eMesh", "eNurbs",
	"ePatch", "eCamera", "eCameraStereo", "eCameraSwitcher", "eLight",
	"eOpticalReference", "eOpticalMarker", "eNurbsCurve", "eTrimNurbsSurface",
	"eBoundary", "eNurbsSurface", "eShape", "eLODGroup", "eSubDiv",
	"eCachedEffect", "eLine"
};

inline static std::vector<fbxsdk::FbxNodeAttribute*> EnumNodeAttr(fbxsdk::FbxNode* node)
{
	std::vector<fbxsdk::FbxNodeAttribute*> result;
	if (!node)
	{
		result.push_back(nullptr);
		return result;
	}
	auto attrCount = node->GetNodeAttributeCount();
	for (auto i = 0; i < attrCount; i++)
	{
		result.push_back(node->GetNodeAttributeByIndex(i));
	}
	if (attrCount == 0)
	{
		result.push_back(nullptr);
	}
	return result;
}

inline static std::string EnumNodeAttrText(fbxsdk::FbxNode* node)
{
	std::string text;
	if (!node)
	{
		return text;
	}
	auto attrs = EnumNodeAttr(node);
	for (auto i = 0u; i < attrs.size(); i++)
	{
		auto& a = attrs[i];
		if (a)
		{
			text += typeNames[a->GetAttributeType()];
			if (i < attrs.size() - 1)
			{
				text += " ";
			}
		}
	}
	return text;
}

void FBXAsset::BuildNodeTree(NodeTree* tree, uint32_t NodeID, fbxsdk::FbxNode* node, NodeTree::Item* parent)
{
	if (!node)
	{
		return;
	}
	std::string text = EnumNodeAttrText(node);
	NodeInfo ni(node);
	ni.id = NodeID++;
	ni.type = text;
	ni.flags = GetNodeAttributeFlags(node);
	ni.scalingOffset = ToGlmVec3(node->GetScalingOffset(fbxsdk::FbxNode::eSourcePivot));
	ni.scalingPivot = ToGlmVec3(node->GetScalingPivot(fbxsdk::FbxNode::eSourcePivot));
	ni.rotationOffset = ToGlmVec3(node->GetRotationOffset(fbxsdk::FbxNode::eSourcePivot));
	ni.rotationPivot = ToGlmVec3(node->GetRotationPivot(fbxsdk::FbxNode::eSourcePivot));
	ni.preRotation = quat(glm::radians(ToGlmVec3(node->GetPreRotation(fbxsdk::FbxNode::eSourcePivot))));
	ni.postRotation = quat(glm::radians(ToGlmVec3(node->GetPostRotation(fbxsdk::FbxNode::eSourcePivot))));
	//ni.rotOrder = node->RotationOrder.Get();
	//PAUSE(ni.rotOrder == fbxsdk::eEulerXYZ);

	auto item = tree->Insert(ni, parent);
//	m_nodeMap[node->GetName()] = item;
	auto childCount = node->GetChildCount();
	for (auto i = 0; i < childCount; i++)
	{
		BuildNodeTree(tree, NodeID, node->GetChild(i), item);
	}
}

void FBXAsset::BuildNodeMap(NodeMap* NodeMap, NodeTree* tree)
{
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		(*NodeMap)[ni.name] = itr;
	}
}

void FBXAsset::SetRootTransform(const mat4& m)
{
	m_rootTransform = m;
}

void FBXAsset::UpdateLocalTransforms()
{
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		ni.localTransform = CalcLocalTransform(itr);
	}
}

void FBXAsset::UpdateWorldTransforms()
{
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		if (itr->parent)
		{
			auto& pni = itr->parent->data;
#if USE_GLM
			ni.worldTransform = pni.worldTransform * ni.localTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
#else
			ni.worldTransform =	ni.localTransform * pni.worldTransform;			// 親ノードのワールドトランスフォームにローカルを掛ける
#endif
		}
		else
		{
			// 親がいない場合、これはルートノードでありローカルトランスフォームがそのままワールドトランスフォーム
#if USE_GLM
			ni.worldTransform = m_rootTransform * ni.localTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
#else
			ni.worldTransform = ni.localTransform * m_rootTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
#endif
		}
	}
}

void FBXAsset::UpdateSamplePoint(float time)
{
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		auto lclScaling = dynamic_cast<MyMath::AnimCurve<vec3>*>(m_currentDopeSheet->curves[ni.lclScaling]);
		auto lclRotation = dynamic_cast<MyMath::AnimCurve<quat>*>(m_currentDopeSheet->curves[ni.lclRotation]);
		auto lclTranslation = dynamic_cast<MyMath::AnimCurve<vec3>*>(m_currentDopeSheet->curves[ni.lclTranslation]);
		m_samplePoint.answers[ni.lclScaling].xyz = lclScaling ? lclScaling->Evaluate(time) : vec3(1.0f);
		m_samplePoint.answers[ni.lclRotation].q = lclRotation ? lclRotation->Evaluate(time) : quat();
		m_samplePoint.answers[ni.lclTranslation].xyz = lclTranslation ? lclTranslation->Evaluate(time) : vec3(0.0f);
	}
	if (m_sampleAlpha > 0.0f)
	{
		m_samplePoint.Interp(m_prevSamplePoint, m_sampleAlpha);
	}
}

mat4 FBXAsset::CalcLocalTransform(NodeTree::Item* item)
{
	ASSERT(item);
	auto& ni = item->data;
	auto node = ni.node;
	ASSERT(node);

	// スケール、回転、位置のアニメーション値
	AnimCurveVec3* lclScaling = nullptr;
	AnimCurveQuat* lclRotation = nullptr;
	AnimCurveVec3* lclTranslation = nullptr;
	if (m_currentDopeSheet->curves.count(ni.lclScaling))
	{
		lclScaling = dynamic_cast<AnimCurveVec3*>(m_currentDopeSheet->curves[ni.lclScaling]);
	}
	if (m_currentDopeSheet->curves.count(ni.lclRotation))
	{
		lclRotation = dynamic_cast<AnimCurveQuat*>(m_currentDopeSheet->curves[ni.lclRotation]);
	}
	if (m_currentDopeSheet->curves.count(ni.lclTranslation))
	{
		lclTranslation = dynamic_cast<AnimCurveVec3*>(m_currentDopeSheet->curves[ni.lclTranslation]);
	}
	vec3 scaling(1.0f);
	quat rotation;
	vec3 translation(0.0f);

	// AnimCurveのEvaluteはここが実行される前に行われている
	if (m_samplePoint.answers.count(ni.lclScaling))
	{
		scaling = m_samplePoint.answers[ni.lclScaling].xyz;
	}
	if (m_samplePoint.answers.count(ni.lclRotation))
	{
		rotation = m_samplePoint.answers[ni.lclRotation].q;
	}
	if (m_samplePoint.answers.count(ni.lclTranslation))
	{
		translation = m_samplePoint.answers[ni.lclTranslation].xyz;
	}

	// 各行列の構築
	mat4 scalingOffsetMatrix = glm::translate(mat4(1.0f), ni.scalingOffset);
	mat4 scalingPivotMatrix = glm::translate(mat4(1.0f), ni.scalingPivot);
	mat4 scalingMatrix = glm::scale(mat4(1.0f), scaling);
	mat4 invScalingPivotMatrix = glm::translate(mat4(1.0f), -ni.scalingPivot);

	mat4 rotationOffsetMatrix = glm::translate(mat4(1.0f), ni.rotationOffset);
	mat4 rotationPivotMatrix = glm::translate(mat4(1.0f), ni.rotationPivot);
	mat4 preRotationMatrix = glm::toMat4(ni.preRotation);
	mat4 rotationMatrix = glm::toMat4(rotation);
	mat4 postRotationMatrix = glm::toMat4(ni.postRotation);

	mat4 invRotationPivotMatrix = glm::translate(mat4(1.0f), -ni.rotationPivot);

	mat4 translationMatrix = glm::translate(mat4(1.0f), translation);

	// トランスフォームの合成
	mat4 localTransform =
		translationMatrix *
		rotationOffsetMatrix *
		rotationPivotMatrix *
		preRotationMatrix *
		rotationMatrix *
		postRotationMatrix *
		invRotationPivotMatrix *
		scalingOffsetMatrix *
		scalingPivotMatrix *
		scalingMatrix *
		invScalingPivotMatrix;
	return	localTransform;
}

mat4 FBXAsset::CalcWorldTransform(NodeTree::Item* item/*, const fbxsdk::FbxTime& time*/)
{
	auto node = item->data.node;
	auto parent = item->parent;
	if (parent == nullptr)
	{
		// ルートノードの場合
		return CalcLocalTransform(item) * m_rootTransform;
	}
	else
	{
		// 親のグローバルトランスフォーム × 自ノードのローカルトランスフォーム
		return CalcLocalTransform(item) * CalcWorldTransform(parent);
	}
}

void FBXAsset::DeformBoneWeight()
{
	for (auto mesh : m_meshes)
	{
		auto& si = mesh->skin;
		// MeshInfoのクラスタ分の頂点変形行列を計算⇒パレットに置く
		std::vector<mat4>	clusterDeformations;
		for (auto& ci : si.clusters)
		{
			clusterDeformations.push_back(ComputeClusterDeformation(mesh, ci));
		}
		// TODO:バーテックスシェーダで実行する
		// 最終的な頂点座標を計算しVERTEXに変換
		for (auto i = 0u; i < mesh->pointsCount; i++)
		{
//			auto& inVertex = mesh->cp_[i];
			auto& inVertex = mesh->vbuf_[i];
			mesh->deform_[i].pos = vec3(0.0f);
			mesh->deform_[i].normal = vec3(0.0f);
			for (auto j = 0; j < BoneWeight::NUM_BONES_PER_VERTEX; j++)
			{
				auto w = inVertex.weight.Weights[j];
				if (w == 0.0f)
				{
					break;
				}
				auto b = inVertex.weight.IDs[j];
				mesh->deform_[i].pos += MyTransform(clusterDeformations[b], inVertex.pos) * w;
				mesh->deform_[i].normal += MyTransformNormal(clusterDeformations[b], inVertex.normal) * w;
			}
			mesh->deform_[i].normal = glm::normalize(mesh->deform_[i].normal);
			mesh->deform_[i].tex = inVertex.tex;
			mesh->deform_[i].color = inVertex.color;
		}
	}
}

mat4 FBXAsset::ComputeClusterDeformation(MeshInfo* mesh, const ClusterInfo& ci) const
{
	/*
	スキンデフォームの計算例 デフォームマトリクスを求める際には、以下の式が使われます:
	Mdeform=(MATmesh_bind)−1×MATbone_bind×MATbone_current

	MATmesh_bind​: GetTransformMatrix で取得したメッシュの初期トランスフォーム。
	MATbone_bind​: GetTransformLinkMatrix で取得したボーンの初期トランスフォーム。
	MATbone_current​: 現在のフレームのボーンのグローバルトランスフォーム。
	*/

	mat4 lReferenceGlobalInitPosition;
	mat4 lReferenceGlobalCurrentPosition;//mesh_bind
	mat4 lClusterGlobalInitPosition;//bone_bind
	mat4 lClusterGlobalCurrentPosition;

	mat4 lClusterRelativeInitPosition;
	mat4 lClusterRelativeCurrentPositionInverse;

	//GetTransformMatrix
	// 	メッシュのローカル座標系での初期トランスフォーム（バインドポーズ）を取得。
	// 	スキンメッシュの初期状態を基準とするトランスフォームを計算。
	//GetTransformLinkMatrix
	// 	ボーンのグローバル座標系での初期トランスフォーム（バインドポーズ）を取得。
	// 	ボーンの現在の姿勢と初期姿勢の相対変換を計算してデフォームを行う。
	//pCluster->GetTransformMatrix(lReferenceGlobalInitPosition);
	ci.GetTransformMatrix(&lReferenceGlobalInitPosition);
	lReferenceGlobalCurrentPosition = mat4(1.0f);
	// Multiply lReferenceGlobalInitPosition by Geometric Transformation
	lReferenceGlobalInitPosition *= mesh->GeometryOffset;

	// Get the link initial global position and the link current global position.
	//pCluster->GetTransformLinkMatrix(lClusterGlobalInitPosition);
	ci.GetTransformLinkMatrix(&lClusterGlobalInitPosition);
	if (m_nodeMap.find(ci.boneName) == m_nodeMap.end())
	{
		TRACE("ボーン<%s>が見つかりません\n", ci.boneName.c_str());
		return mat4(1.0f);
	}
	auto& bone = m_nodeMap.at(ci.boneName);
	lClusterGlobalCurrentPosition = bone->data.worldTransform;

	lClusterRelativeInitPosition = glm::inverse(lClusterGlobalInitPosition) * lReferenceGlobalInitPosition;
	lClusterRelativeCurrentPositionInverse = glm::inverse(lReferenceGlobalCurrentPosition) * lClusterGlobalCurrentPosition;
	return lClusterRelativeCurrentPositionInverse * lClusterRelativeInitPosition;
}


//DrawNode⇒DrawMesh⇒ComputeSkinDeformation⇒ComputeLinearDeformation⇒ComputeClusterDeformation



