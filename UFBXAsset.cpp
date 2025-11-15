#include "pch.h"
#include "MyUtil.h"
#include "UFBXAsset.h"

#define USE_GLM 1
#if USE_GLM
inline static vec3 MyTransform(const glm::mat4& m, const vec3& v)
{
	auto v4 = m * vec4(v.x, v.y, v.z, 1.0f);
	return vec3(v4.x, v4.y, v4.z);
}

inline static glm::mat3 ComputeNormalMatrix(const glm::mat4& m)
{
	// 逆行列の転置行列
	auto normalmatrix = glm::transpose(glm::inverse(glm::mat3(m)));
	return glm::mat3(normalmatrix);
}

inline static vec3 MyTransformNormal(const glm::mat4& m, const vec3& v)
{
	auto v3 = glm::normalize(ComputeNormalMatrix(m) * vec3(v.x, v.y, v.z));
	return vec3(v3.x, v3.y, v3.z);
}

inline static glm::mat4 ToGlmMat(const ufbx_matrix& ufbxMat)
{
	return glm::mat4(
		(float)ufbxMat.m00, (float)ufbxMat.m10, (float)ufbxMat.m20, (float)0.0f,
		(float)ufbxMat.m01, (float)ufbxMat.m11, (float)ufbxMat.m21, (float)0.0f,
		(float)ufbxMat.m02, (float)ufbxMat.m12, (float)ufbxMat.m22, (float)0.0f,
		(float)ufbxMat.m03, (float)ufbxMat.m13, (float)ufbxMat.m23, (float)1.0f
	);
}

#endif


#pragma warning(disable: 4244)
#define USE_ROOT_TRANSFORM 1
#define GEN_FLAT_NORMAL 1
#define MOTION_INTERP_RATE 30.0f
#define MAKE_OBB_PER_CLUSTER 0
#define MAKE_OBB_THRESHOLD 0.5f


inline static glm::vec2 ToGlmVec2(const ufbx_vec2& ufbxVec)
{
	return glm::vec2(static_cast<float>(ufbxVec.x), static_cast<float>(ufbxVec.y));
}

inline static glm::vec3 ToGlmVec3(const ufbx_vec3& ufbxVec)
{
	return glm::vec3(static_cast<float>(ufbxVec.x), static_cast<float>(ufbxVec.y), static_cast<float>(ufbxVec.z));
}

inline static ufbx_vec3 ToUfbxVec3(const glm::vec3& g)
{
	ufbx_vec3 result = { (ufbx_real)g.x, (ufbx_real)g.y, (ufbx_real)g.z };
	return result;
}

inline static glm::vec4 ToGlmVec4(const ufbx_vec4& ufbxVec)
{
	return glm::vec4(static_cast<float>(ufbxVec.x), static_cast<float>(ufbxVec.y), static_cast<float>(ufbxVec.z), static_cast<float>(ufbxVec.w));
}

inline static glm::vec4 ToGlmVec4(const ufbx_quat& ufbxQuat)
{
	return glm::vec4(static_cast<float>(ufbxQuat.x), static_cast<float>(ufbxQuat.y), static_cast<float>(ufbxQuat.z), static_cast<float>(ufbxQuat.w));
}

inline static glm::quat ToGlmQuat(const ufbx_vec3& ufbxVec)
{
	// XYZを度数法として、ラジアンにしてXYZオーダー回転からクォータニオンにする
	return glm::quat(glm::radians(ToGlmVec3(ufbxVec)));
}

inline static glm::quat ToGlmQuat(const ufbx_quat& ufbxQuat)
{
	// XYZを度数法として、ラジアンにしてXYZオーダー回転からクォータニオンにする
	return glm::quat((float)ufbxQuat.w, (float)ufbxQuat.x, (float)ufbxQuat.y, (float)ufbxQuat.z);
}

inline static uint32_t GetNodeAttributeFlags(const ufbx_node* node)
{
	uint32_t flags = 0;
	if (!node)
	{
		return flags;
	}
	if (node->camera)
	{
		return ATTRIBUTE_FLAG_CAMERA;
	}
	if (node->mesh)
	{
		return ATTRIBUTE_FLAG_MESH;
	}

	switch (node->attrib_type)
	{
	case UFBX_ELEMENT_NODE:                // < `ufbx_node`
		flags |= ATTRIBUTE_FLAG_NULL;
		break;
	case UFBX_ELEMENT_BONE:                // < `ufbx_bone`
		flags |= ATTRIBUTE_FLAG_SKELETON;
		break;
	case UFBX_ELEMENT_MESH:                // < `ufbx_mesh`
		flags |= ATTRIBUTE_FLAG_MESH;
		break;
	case UFBX_ELEMENT_CAMERA:              // < `ufbx_camera`
		flags |= ATTRIBUTE_FLAG_CAMERA;
		break;
	case UFBX_ELEMENT_LIGHT:               // < `ufbx_light`
		flags |= ATTRIBUTE_FLAG_LIGHT;
		break;
		// 必要に応じて他のタイプも追加できます
	default:
		// 未知のタイプや、特定のフラグを必要としないタイプの場合
		break;
	}
	return flags;
}




//------------------------------------------

UFBXAsset::UFBXAsset()
{
	m_rootTransform = mat4(1.0f);
	m_adjust =mat4(1.0f);
}

UFBXAsset::~UFBXAsset()
{
}

bool UFBXAsset::LoadAsset(const std::string& Filename, int anim)
{
	m_currentAssetPath = Filename;

	std::ifstream ifs(Filename, (std::ios::binary | std::ios::in));
	if (!ifs.is_open())
	{
		TRACE("エラー: ファイルを開けませんでした: %s\n", Filename.c_str());
		return false;
	}
	ifs.seekg(0, std::ios::end);
	auto fileLen = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	std::vector<uint8_t> data(fileLen, 0x00);
	if (ifs.bad())
	{
		TRACE("%s\n", Filename.c_str());
		DBGBREAK();
		return false;
	}
	ifs.read((char*)data.data(), fileLen);
	ifs.seekg(0, std::ios::beg);
	ifs.close();

	return LoadBinary(data, anim);
}

bool UFBXAsset::LoadBinary(const std::vector<uint8_t>& data, int anim)
{
	TRACE("LoadBinary:anim=%d",anim);
	ufbx_load_opts opts = {};// ufbx_default_load_opts();
	// ここでオプションを必要に応じて変更
	opts.ignore_geometry = false;
	opts.ignore_animation = false;

	ufbx_error ufbxErr = {};
	m_scene = ufbx_load_memory((void*)data.data(), (size_t)data.size(), nullptr, &ufbxErr);
//	m_scene = ufbx_load_file(Filename.c_str(), &opts, &ufbxErr);
	if (!m_scene)
	{
//		TRACE("%s\n", Filename.c_str());
		DBGBREAK();
		return false;
	}

	auto stacks = m_scene->anim_stacks;
	TRACE("m_scene->anim_stacks.count = %zu", m_scene->anim_stacks.count );
	assert(stacks.count > anim);
	m_CurAnimStack = stacks[anim];
	assert(m_CurAnimStack);

	// アニメーションの開始時間と終了時間を取得
	auto setting = m_scene->settings;

	auto layers = m_CurAnimStack->layers;
	TRACE("layers.count = %zu",	layers.count);
	assert(layers.count > 0);
	m_CurAnimLayer = layers.data[0];
	assert(m_CurAnimLayer);
	m_dopeSheets.push_back(DopeSheet());
	m_CurSheet = &m_dopeSheets[m_dopeSheets.size() - 1];

	m_CurSheet->m_start = m_CurAnimStack->time_begin;
	m_CurSheet->m_stop = m_CurAnimStack->time_end;
	TRACE("start=%f end=%f\n", m_CurSheet->m_start, m_CurSheet->m_stop);
	
	// FPSモードに基づいてフレームレートを決定します
	switch (setting.time_mode)
	{
		default:												m_CurSheet->m_FPS = 30.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_DEFAULT:
		case ufbx_time_mode::UFBX_TIME_MODE_120_FPS:			m_CurSheet->m_FPS = 120.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_100_FPS:			m_CurSheet->m_FPS = 100.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_60_FPS:				m_CurSheet->m_FPS = 60.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_50_FPS:				m_CurSheet->m_FPS = 50.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_48_FPS:				m_CurSheet->m_FPS = 48.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_30_FPS_DROP:		m_CurSheet->m_FPS = 30.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_30_FPS:				m_CurSheet->m_FPS = 30.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_NTSC_DROP_FRAME:
		case ufbx_time_mode::UFBX_TIME_MODE_NTSC_FULL_FRAME:	m_CurSheet->m_FPS = 29.97f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_PAL:				m_CurSheet->m_FPS = 25.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_FILM_FULL_FRAME:	m_CurSheet->m_FPS = 24.0f; break;
		case ufbx_time_mode::UFBX_TIME_MODE_1000_FPS:
		case ufbx_time_mode::UFBX_TIME_MODE_CUSTOM:				m_CurSheet->m_FPS = (float)setting.frames_per_second; break;
	}
//	TRACE("setting.original_axis_up=%d\n", setting.original_axis_up);
	EnsureYUp(vec3(0.1f));

	auto root = m_scene->root_node;
	ASSERT(root);
	TRACE("BuildNodeTree{{{\n");
	BuildNodeTree(&m_nodeTree, 0, root, nullptr);
	TRACE("}}}\n");
	TRACE("m_nodeTree.GetCount()=%d\n", m_nodeTree.GetCount());

	ASSERT(m_scene->anim_stacks.count > 0);
	auto animStack = m_scene->anim_stacks.data[0];
	ASSERT(animStack);
	//m_start = setting->TimeSpanStart;
	//m_stop = setting->TimeSpanStop;

	// メッシュノードを取得
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		ParseMesh(itr);
	}
	if (m_meshes.size() == 0)
	{
		assert(0);
		TRACE("found no meshes\n");
		return false;
	}

	// 埋め込みテクスチャ解決
	for (auto mi : m_materials)
	{
		if (mi)
		{
			auto ufbxMaterial = mi->ufbxMaterial;
			if (ufbxMaterial)
			{
				// マテリアルから拡散色テクスチャを取得
				ufbx_texture* ufbxTexture = ufbxMaterial->pbr.base_color.texture;
				if (ufbxTexture && ufbxTexture->content.data && ufbxTexture->content.size > 0)
				{
					auto data = ufbxTexture->content.data;
					auto size = ufbxTexture->content.size;
					mi->texture = Texture2D::CreateFromMemory(data, size);
				}
			}
		}
	}

	return true;
}

int UFBXAsset::ParseMaterial(ufbx_material* material)
{
	MaterialInfo* mi = _NEW MaterialInfo;
	ASSERT(mi);
	mi->ufbxMaterial = material;

	const ufbx_prop* diffuse = ufbx_find_prop(&material->props, "DiffuseColor");
	if (diffuse && diffuse->type == UFBX_PROP_COLOR)
	{
		ufbx_vec3 c = diffuse->value_vec3;
	//	TRACE("Diffuse: %f %f %f\n", c.x, c.y, c.z);
		mi->diffuse = ToGlmVec3(c);
		mi->valid = true;
	}

	const ufbx_prop* ambient = ufbx_find_prop(&material->props, "AmbientColor");
	if (ambient && ambient->type == UFBX_PROP_COLOR)
	{
		ufbx_vec3 c = ambient->value_vec3;
	//	TRACE("Ambient: %f %f %f\n", c.x, c.y, c.z);
		mi->ambient = ToGlmVec3(c);
	}

	const ufbx_prop* emissive = ufbx_find_prop(&material->props, "EmissiveColor");
	if (emissive && emissive->type == UFBX_PROP_COLOR)
	{
		ufbx_vec3 c = emissive->value_vec3;
	//	TRACE("Emissive: %f %f %f\n", c.x, c.y, c.z);
		mi->emmisive = ToGlmVec3(c);
	}

	const ufbx_prop* specular = ufbx_find_prop(&material->props, "SpecularColor");
	if (specular && specular->type == UFBX_PROP_COLOR)
	{
		ufbx_vec3 c = specular->value_vec3;
	//	TRACE("Specular: %f %f %f\n", c.x, c.y, c.z);
		mi->specular = ToGlmVec3(c);
	}

	const ufbx_prop* shininess = ufbx_find_prop(&material->props, "Shininess");
	if (shininess && shininess->type == UFBX_PROP_NUMBER)
	{
		ufbx_real s = shininess->value_real;
	//	TRACE("Shininess: %f\n", s);
		mi->shininess = s;
	}

	auto texlist = material->textures;
	if (texlist.count > 0)
	{
		auto tex = texlist[0];
		std::string filename = tex.texture->filename.data;
		TRACE("filename: %s\n", filename.c_str());
		mi->textureFile = filename;
		mi->valid = true;
		//filename = MyUtil::StripPath(filename);
		//filename = MyUtil::RemoveFileSpec(m_currentAssetPath) + "\\" + filename;
		//mi->texture = Texture2D::CreateFromFile(filename.c_str());
	}

	if (mi->valid)
	{
		auto result = (int)m_materials.size();
		m_materials.push_back(mi);
		return result;
	}

	return -1;
}

bool UFBXAsset::ParseMesh(NodeTree::Item* item)
{
	ASSERT(item && item->data.node);
	auto node = item->data.node;
	if (item->data.flags & ATTRIBUTE_FLAG_MESH)
	{
		//auto found = m_nodeMap.find(node->name);
		auto m = _NEW MeshInfo;
		ASSERT(m);
		m->node = node;
		m->name = node->name.data;
		m->mesh = node->mesh;
		ASSERT(m->mesh);

		//マテリアル
		if (!ParseMaterialGroup(m))
		{
			return false;
		}
//		if (!ParseMesh_BoneWeight(m))
//		{
//			return false;
//		}
		
		m_meshes.push_back(m);
		return true;
	}
	return false;
}

void UFBXAsset::GetDeformed(std::vector<VertexPNCTAW>* pnctaw, uint32_t meshIndex, uint32_t material)
{
	pnctaw->clear();
	auto& m = m_meshes[meshIndex];
	for (auto i = 0; i < m->ibuf_.size(); i += 3)
	{
		auto& a = m->deform_[m->ibuf_[i + 0]];
		auto& b = m->deform_[m->ibuf_[i + 1]];
		auto& c = m->deform_[m->ibuf_[i + 2]];
		auto n = MyMath::CalcTriangleNormal(a.pos, b.pos, c.pos);
#if 1//TRIANGLE
		pnctaw->push_back(VertexPNCTAW(a.pos, n, a.color, a.texcoord, a.tangent));
		pnctaw->push_back(VertexPNCTAW(b.pos, n, b.color, b.texcoord, b.tangent));
		pnctaw->push_back(VertexPNCTAW(c.pos, n, c.color, c.texcoord, c.tangent));
#else//LINES
		pnctaw->push_back(VertexPNCTAW(a.pos, n, a.color, a.tex, a.tangent, BoneWeight()));
		pnctaw->push_back(VertexPNCTAW(b.pos, n, b.color, b.tex, b.tangent, BoneWeight()));
		pnctaw->push_back(VertexPNCTAW(b.pos, n, b.color, b.tex, b.tangent, BoneWeight()));
		pnctaw->push_back(VertexPNCTAW(c.pos, n, c.color, c.tex, c.tangent, BoneWeight()));
		pnctaw->push_back(VertexPNCTAW(c.pos, n, c.color, c.tex, c.tangent, BoneWeight()));
		pnctaw->push_back(VertexPNCTAW(a.pos, n, a.color, a.tex, a.tangent, BoneWeight()));
#endif
	}
	return;
}

static VertexPNCTAW get_skinned_vertex(ufbx_mesh* mesh, ufbx_skin_deformer* skin, size_t index)
{
	VertexPNCTAW v;
	v.pos = ToGlmVec3(mesh->vertex_position[index]);
	v.normal = ToGlmVec3(mesh->vertex_normal.exists ? mesh->vertex_normal[index] : ufbx_zero_vec3);
	v.color = ToGlmVec4(mesh->vertex_color.exists ? mesh->vertex_color[index] : ufbx_zero_vec4);
	v.texcoord = ToGlmVec2(mesh->vertex_uv.exists ? mesh->vertex_uv[index] : ufbx_zero_vec2);
	v.tangent = ToGlmVec3(mesh->vertex_tangent.exists ? mesh->vertex_tangent[index] : ufbx_zero_vec3);
	v.texcoord.y = 1.0f - v.texcoord.y;
	// NOTE: This calculation below is the same for each `vertex`, we could
	// precalculate these up to `mesh->num_vertices`, and just load the results.
	uint32_t vertex = mesh->vertex_indices[index];

	ufbx_skin_vertex skin_vertex = skin->vertices[vertex];
	auto num_weights = skin_vertex.num_weights;
	if (num_weights > _countof(v.bone))
	{
		num_weights = _countof(v.bone);
	}

	float total_weight = 0.0f;
	for (auto i = 0u; i < num_weights; i++)
	{
		ufbx_skin_weight skin_weight = skin->weights[skin_vertex.weight_begin + i];
		v.bone[i] = skin_weight.cluster_index;
		v.weight[i] = (float)skin_weight.weight;
		total_weight += (float)skin_weight.weight;
	}

	// FBX does not guarantee that skin weights are normalized, and we may even
	// be dropping some, so we must renormalize them.
	for (auto i = 0u; i < num_weights; i++)
	{
		v.weight[i] /= total_weight;
	}

	return v;
}

struct Bone
{
	std::string boneName;
	uint32_t node_index;
	mat4 geometry_to_bone;
};

struct Mesh
{
	std::vector<Bone> bones;
	std::vector<VertexPNCTAW> vertices;
	std::vector<uint16_t> index;
};

static Mesh process_skinned_mesh(ufbx_mesh* mesh, ufbx_skin_deformer* skin, uint32_t target_material_index)
{
	Mesh result;

	// Triangulate the mesh, using `get_skinned_vertex()` to fetch each index.
	size_t num_tri_indices = mesh->max_face_triangles * 3;
	std::vector<uint32_t> tri_indices(num_tri_indices);
	for (auto face_index = 0u; face_index < mesh->num_faces; face_index++)
	{
		// このフェイスのマテリアルスロット
		uint32_t mat_index = mesh->face_material[face_index];

		// フィルタリング
		if (mat_index != target_material_index)
		{
			continue;
		}
		// 対象フェイス取得
		ufbx_face& face = mesh->faces[face_index];

		uint32_t num_tris = ufbx_triangulate_face(tri_indices.data(), tri_indices.size(), mesh, face);
		for (size_t i = 0; i < num_tris * 3; i++)
		{
			uint32_t index = tri_indices[i];
			result.index.push_back((uint16_t)index);
			result.vertices.push_back(get_skinned_vertex(mesh, skin, index));
		}
	}
//	ASSERT(result.vertices.size() == mesh->num_triangles * 3);

	if (skin)
	{
		TRACE("mesh='%s'\n", mesh->name.data);
		// Create bone descriptions
		for (ufbx_skin_cluster* cluster : skin->clusters)
		{
			auto bone = Bone{
				cluster->bone_node->name.data,
				cluster->bone_node->typed_id,
				ToGlmMat(cluster->geometry_to_bone),
			};
			TRACE("bone=%u(%s)	geometry_to_bone=%s\n", bone.node_index, cluster->bone_node->name.data, glm::to_string(bone.geometry_to_bone).c_str());
			result.bones.push_back(bone);
		}
	}

	return result;
}

bool UFBXAsset::ParseMesh_BoneWeight(MeshInfo* meshinfo)
{
	DBGBREAK();
	ASSERT(meshinfo);
	auto node = meshinfo->node;
	auto mesh = meshinfo->mesh;
	
	ufbx_mesh* ufbxmesh = node->mesh;
	ASSERT(ufbxmesh);
	
	meshinfo->GeometricMatrix = ToGlmMat(node->geometry_to_node);
	if(ufbxmesh->skin_deformers.count > 0)
	{
		ufbx_skin_deformer* ufbxskin = ufbxmesh->skin_deformers[0];

		for (auto matIndex = 0u; matIndex < ufbxmesh->materials.count; matIndex++)
		{
			auto mat = ParseMaterial(ufbxmesh->materials[matIndex]);
//			if (mat >= 0)
//			{
//				meshinfo->matSlots_.push_back(mat);
//			}
			Mesh result = process_skinned_mesh(ufbxmesh, ufbxskin, matIndex);

			meshinfo->vbuf_.clear();
			meshinfo->ibuf_.clear();
//			meshinfo->mbuf_.clear();
			meshinfo->pointsCount = (uint32_t)result.vertices.size();
			for (auto i = 0u; i < result.vertices.size(); i++)
			{
				auto& v = result.vertices[i];
				meshinfo->vbuf_.push_back(v);
				meshinfo->ibuf_.push_back(i);
//				meshinfo->mbuf_.push_back(matIndex);
			}
			auto si = &meshinfo->skin;
			for (auto& bone : result.bones)
			{
				ClusterInfo ci;
				ci.boneName = bone.boneName;
				ci.SetTransformLinkMatrix(bone.geometry_to_bone);
				si->clusters.push_back(ci);
			}
		}
	}

	return true;
}

template <typename T>
inline static size_t AddUnique(std::vector<T>& vec, const T& value)
{
#if 0
	// std::findで要素を検索
	for (auto it = vec.begin(); it != vec.end(); ++it)
	{
		if (*it == value)
		{
			// 重複している場合、その要素のインデックスを返す
			return std::distance(vec.begin(), it);
		}
	}
#endif

	// 重複していない場合、要素を追加し、追加された要素のインデックスを返す
	vec.push_back(value);
	return vec.size() - 1;
}

bool UFBXAsset::ParseMaterialGroup(MeshInfo* meshinfo)
{
	ASSERT(meshinfo);
	auto node = meshinfo->node;
	auto mesh = meshinfo->mesh;
	
	ufbx_mesh* ufbxmesh = node->mesh;
	ASSERT(ufbxmesh);
	
	meshinfo->GeometricMatrix = ToGlmMat(node->geometry_to_node);
	if(ufbxmesh->skin_deformers.count > 0)
	{
		ufbx_skin_deformer* ufbxskin = ufbxmesh->skin_deformers[0];

		for (auto matIndex = 0u; matIndex < ufbxmesh->materials.count; matIndex++)
		{
			auto mat = ParseMaterial(ufbxmesh->materials[matIndex]);
			//?重複はOK?
			ASSERT(mat >= 0);
			Mesh result = process_skinned_mesh(ufbxmesh, ufbxskin, matIndex);
			Geometry geom;
			geom.material = mat;
			geom.vbuf.clear();
			geom.ibuf.clear();
			geom.pointsCount = (uint32_t)result.vertices.size();
			for (auto i = 0u; i < result.vertices.size(); i++)
			{
//				auto index = AddUnique(geom.vbuf, result.vertices[i]);//Trianglesに渡せる形式を維持する為、マージはさせない
				auto index = geom.vbuf.size();
				geom.vbuf.push_back( result.vertices[i]);//Trianglesに渡せる形式を維持する為、マージはさせない
				geom.ibuf.push_back((uint16_t)index);
			}
			geom.pointsCount = (uint32_t)geom.vbuf.size();
			auto si = &meshinfo->skin;
			for (auto& bone : result.bones)
			{
				ClusterInfo ci;
				ci.boneName = bone.boneName;
				ci.SetTransformLinkMatrix(bone.geometry_to_bone);
				si->clusters.push_back(ci);
			}
			meshinfo->MaterialGroup.push_back(geom);
		}
	}

	return true;
}


void UFBXAsset::DeleteAsset()
{
	if (m_scene)
	{
		ufbx_free_scene(m_scene);
		m_scene = nullptr;
	}
	//for (auto& m : m_meshes)
	//{
	//	if (m)
	//	{
	//		delete m;
	//		m = nullptr;
	//	}
	//}
	m_meshes.clear();
	m_nodeTree.Clear();
}

void UFBXAsset::Update(float delta)
{
	m_time += delta;
	//TRACE("m_time=%f 1/m_FPS=%f delta=%f\n", m_time, 1.0f / m_FPS, delta);
	if (m_CurSheet && m_time > m_CurSheet->m_stop)
	{
		m_time -= (m_CurSheet->m_stop - m_CurSheet->m_start);
		//m_time = m_CurSheet->m_start;
	}
	UpdateLocalTransforms();
	UpdateWorldTransforms();
#if SOFTWARE_DEFORM
	DeformBoneWeight();//note:シェーダーでデフォームする場合これを呼ばない
#endif
}

int UFBXAsset::GetDeformMatrixCount(uint32_t mesh) const
{
	return GetBoneCount(mesh);
}

void UFBXAsset::GetDeformMatrix(uint32_t mesh, mat4* dst, size_t dstCount) const
{
	auto &m = m_meshes[mesh];
	GetDeformMatrix(m, dst, dstCount);
}

void UFBXAsset::GetDeformMatrix(const MeshInfo* mesh, mat4* dst, size_t dstCount) const
{
	assert(mesh);
	auto& si = mesh->skin;
	uint32_t i = 0;
	for (auto& ci : si.clusters)
	{
		if (i >= dstCount)
		{
			return;
		}
		dst[i++] = ComputeClusterDeformation(mesh, ci);
	}
}


void UFBXAsset::Render()
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

uint32_t UFBXAsset::GetMeshCount() const
{
	return static_cast<uint32_t>(m_meshes.size());
}

uint32_t UFBXAsset::GetBoneCount(uint32_t mesh) const
{
	auto m = m_meshes[mesh];
	return static_cast<uint32_t>(m->skin.clusters.size());
}

uint32_t UFBXAsset::GetMaterialGroupCount(uint32_t mesh) const
{
	return (uint32_t)m_meshes[mesh]->MaterialGroup.size();
//MaterialGroup処理に移行	return static_cast <uint32_t>(m_meshes[mesh]->matSlots_.size());
}


UFBXAsset::MaterialInfo* UFBXAsset::GetMaterialInfo(uint32_t info) const
{
	return m_materials[info];
}

UFBXAsset::Geometry* UFBXAsset::GetMaterialGeometry(uint32_t mesh, uint32_t group) const
{
	return &m_meshes[mesh]->MaterialGroup[group];
}

void UFBXAsset::GetVBuf(uint32_t mesh, uint32_t material, std::vector<VertexPNCTAW>* vbuf) const
{
	auto& m = m_meshes[mesh];
	//TODO: m->mbuf_でフィルタリング
	vbuf->clear();
	vbuf->reserve(m->vbuf_.size());
	for (const auto& i : m->ibuf_)
	{
		vbuf->push_back(m->vbuf_[i]);
	}
}

UFBXAsset::MeshInfo* UFBXAsset::GetMeshInfo(uint32_t mesh) const
{
	return m_meshes[mesh];
}

uint32_t UFBXAsset::GetBonePrimCount() const
{
	uint32_t sum = 0u;
	for (auto i = 0u; i < GetMeshCount(); i++)
	{
		sum += (uint32_t)GetBoneCount(i) * 2;
	}
	return sum;
}

uint32_t UFBXAsset::GetMeshPrimCount() const
{
	uint32_t sum = 0u;
	for (auto i = 0u; i < GetMeshCount(); i++)
	{
		sum += (uint32_t)m_meshes[i]->ibuf_.size();
	}
	return sum;
}

void UFBXAsset::RenderMesh(std::vector<VertexPNCTAW>* primitive)
{
}

void UFBXAsset::RenderWire(std::vector<VertexPNCT>* primitive)
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
			primitive->push_back(VertexPNCT(a.pos, a.normal, a.color, a.texcoord));
			primitive->push_back(VertexPNCT(b.pos, b.normal, b.color, b.texcoord));
			primitive->push_back(VertexPNCT(c.pos, c.normal, c.color, c.texcoord));
		}
	}
}

void UFBXAsset::GetSkeleton(std::vector<VertexPC> *primitive)
{
	/* each node */
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		if (!(itr->data.flags & ATTRIBUTE_FLAG_SKELETON))
		{
			continue;
		}
		auto posf = itr->data.worldTransform[3];
	}
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		if (!(itr->data.flags & ATTRIBUTE_FLAG_SKELETON))
		{
			continue;
		}
		if (itr->parent)
		{
			vec4 c(1.0f, 0.0f, 0.0f, 1.0f );
			auto posf = itr->data.worldTransform[3];
			auto pposf = itr->parent->data.worldTransform[3];
			primitive->push_back(VertexPC(posf, c));
			primitive->push_back(VertexPC(pposf, c));
		}
	}
}


inline static const char* typeNames[] =
{
	"ROOT",
	"GEOMETRY",
	"SHAPE",
	"MATERIAL",
	"MESH",
	"TEXTURE",
	"LIMB_NODE",
	"NULL_NODE",
	"CAMERA",
	"LIGHT",
	"NODE_ATTRIBUTE",
	"CLUSTER",
	"SKIN",
	"BLEND_SHAPE",
	"BLEND_SHAPE_CHANNEL",
	"ANIMATION_STACK",
	"ANIMATION_LAYER",
	"ANIMATION_CURVE",
	"ANIMATION_CURVE_NODE",
	"POSE"
};

inline static std::vector<uint32_t> EnumNodeType(const ufbx_node* node)
{
	std::vector<uint32_t> result;
	if (!node)
	{
		result.push_back(0);
		return result;
	}
	result.push_back((uint32_t)node->element.type);
	return result;
}

inline static std::string EnumNodeTypeText(const ufbx_node* node)
{
	std::string text;
	if (!node)
	{
		return text;
	}
	auto types = EnumNodeType(node);
	for (auto i = 0u; i < types.size(); i++)
	{
		text += typeNames[(int)types[i]];
		if (i < types.size() - 1)
		{
			text += " ";
		}
	}
	return text;
}

UFBXAsset::AnimCurveVec3* UFBXAsset::ParseProperty(const ufbx_node* node, const char* propName, const vec3& defaultValue)
{
	if (!node || !propName) return nullptr;
	ASSERT(m_scene);

	ASSERT(m_scene->anim_stacks.count > 0);

	auto animStack = m_scene->anim_stacks.data[0];
	ASSERT(animStack);
	if (!animStack) return nullptr;

	if (animStack->layers.count < 1) return nullptr;
	auto animLayer = animStack->layers.data[0];
	ASSERT(animLayer);
	if (!animLayer) return nullptr;

	return 0;
}
std::string ChainedName(const ufbx_node* n)
{
	std::string s(n ? n->name.data : "NULL");
	if (n->parent)s = ChainedName(n->parent) + "/" + s;
	return s;
}


bool UFBXAsset::ParseAnimCurve(AnimCurveVec3& dst, const ufbx_anim_prop* animProp, const ufbx_vec3& defValue)
{
	if (!animProp)
	{
		dst.SetDefaultValue(ToGlmVec3(defValue));
		return false;
	}
	dst.Init();
	ASSERT(animProp && animProp->anim_value);
	auto animValue = animProp->anim_value;
	dst.SetDefaultValue(ToGlmVec3(animProp->anim_value->default_value));
	if (animValue)
	{
		const ufbx_anim_curve* xCurve = animValue->curves[0]; // X軸
		const ufbx_anim_curve* yCurve = animValue->curves[1]; // Y軸
		const ufbx_anim_curve* zCurve = animValue->curves[2]; // Z軸
		AnimCurveFloat x; x.SetDefaultValue(defValue.x);
		AnimCurveFloat y; y.SetDefaultValue(defValue.y);
		AnimCurveFloat z; z.SetDefaultValue(defValue.z);
		// xチャネルとyチャネルでキーフレーム数が異なる場合のためそれぞれ単独カーブに展開する
		if (xCurve)
		{
			auto KeyCount = xCurve->keyframes.count;
			for (auto k = 0; k < KeyCount; ++k)
			{
				float time = (float)xCurve->keyframes.data[k].time;
				x.Insert(time, (float)xCurve->keyframes.data[k].value);
			}
		}
		if (yCurve)
		{
			auto KeyCount = yCurve->keyframes.count;
			for (auto k = 0; k < KeyCount; ++k)
			{
				float time = (float)yCurve->keyframes.data[k].time;
				y.Insert(time, (float)yCurve->keyframes.data[k].value);
			}
		}
		if (zCurve)
		{
			auto KeyCount = zCurve->keyframes.count;
			for (auto k = 0; k < KeyCount; ++k)
			{
				float time = (float)zCurve->keyframes.data[k].time;
				z.Insert(time, (float)zCurve->keyframes.data[k].value);
			}
		}
		// xカーブ、yカーブ、zカーブを統合してdstカーブにする
		if(z.keys.size())
		{
			auto KeyCount = z.keys.size();
			for (auto k = 0; k < KeyCount; ++k)
			{
				float time = z.keys[k].first;
				auto value=vec3(x.Evaluate(time), y.Evaluate(time), (float)z.keys[k].second);
				dst.Insert(time, value);
			}
		}
		return true;
	}
	return false;
}

#if 1
static inline ufbx_string make_ufbx_string(const char* data, size_t length)
{
	const char* empty = "\0";
	ufbx_string str = { length > 0 ? data : empty, length};
	return str;
}

static inline bool operator<(ufbx_string a, ufbx_string b)
{
	size_t len = std::min(a.length, b.length);
	int cmp = memcmp(a.data, b.data, len);
	if (cmp != 0) return cmp < 0;
	return a.length < b.length;
}

static inline bool operator==(ufbx_string a, ufbx_string b)
{
	return a.length == b.length && !memcmp(a.data, b.data, a.length);
}

// ufbxi_macro_lower_bound_eq の関数版 (ufbx_anim_prop 固定)
static void ufbxi_lower_bound_eq_anim_prop(
		size_t linear_size,
		size_t* result_ptr,
		const ufbx_anim_prop* data,
		size_t dataCount,
		size_t begin,
		size_t size,
		const ufbx_element* target_element,
		ufbx_string target_prop_str
)
{
	const ufbx_anim_prop* mi_data = data;
	size_t mi_lo = begin;
	size_t mi_hi = size;
	size_t mi_linear_size = /*ufbxi_clamp_linear_threshold*/(linear_size);

	ASSERT(mi_linear_size > 1);

	// Binary search
	while (mi_hi - mi_lo > mi_linear_size)
	{
		size_t mi_mid = mi_lo + (mi_hi - mi_lo) / 2;
		const ufbx_anim_prop* a;
		if(mi_mid>=size)
		{
			mi_lo = mi_mid + 1;
			continue;
		}
		a = &mi_data[mi_mid];

		if (a->element != target_element ?
		    a->element < target_element :
		    a->prop_name< target_prop_str)
		{
			mi_lo = mi_mid + 1;
		}
		else
		{
			mi_hi = mi_mid + 1;
		}
	}
	// Linear scan
	for (; mi_lo < mi_hi; mi_lo++)
	{
		const ufbx_anim_prop* a;
		if(mi_lo>=size)continue;
		a = &mi_data[mi_lo];
		if (a->element == target_element &&
		    a->prop_name== target_prop_str)
		{
			*result_ptr = mi_lo;
			break;
		}
	}
}

static ufbx_anim_prop* findAnimProp(
		const ufbx_anim_layer* layer,
		const ufbx_element* element,
		const char* prop
)
{
	ufbx_assert(layer);
	ufbx_assert(element);
	if (!layer || !element)
	{
		ASSERT(0);
		return NULL;
	}

	ufbx_string prop_str = make_ufbx_string(prop, strlen(prop));

	size_t index = layer->anim_props.count;
	ufbxi_lower_bound_eq_anim_prop(
			16, &index,
			layer->anim_props.data,
			layer->anim_props.count,
			0, layer->anim_props.count,
			element, prop_str
	);

	if (index == SIZE_MAX) return NULL;
	if (index >= layer->anim_props.count)
	{
		ASSERT(0);
		return NULL;
	}
	return &layer->anim_props.data[index];
}

static ufbx_anim_prop *ufbx_find_anim_prop_len_linear(
		const ufbx_anim_layer *layer,
		const ufbx_element *element,
		const char *prop
)
{
	ufbx_assert(layer);
	ufbx_assert(element);
	if (!layer || !element) return nullptr;

	ufbx_string prop_str = make_ufbx_string(prop, strlen(prop));

	auto count= layer->anim_props.count;
	if (count == SIZE_MAX)
	{
		return nullptr;
	}
	//TRACE("layer->name=%s layer->anim_props.count=%zu(%zx)\n", layer->name.data, layer->anim_props.count, layer->anim_props.count);

	for (size_t i = 0; i < count; i++)
	{
		auto a = &layer->anim_props.data[i];

		if (a->element == element && a->prop_name == prop_str)
		{
			return a;
		}
	}

	return nullptr;
}

static ufbx_anim_prop* getAnimProp(const ufbx_anim_layer* layer, const ufbx_node* node, const char* str)
{
//return (ufbx_anim_prop*)nullptr;//debug
	assert(layer);
	ufbx_string_view view(str, strlen(str));
	ufbx_anim_prop* animProp = ufbx_find_anim_prop_len_linear(
			layer,
			&node->element,
			str);
	return animProp;
};

#endif

void UFBXAsset::BuildNodeTree(NodeTree* tree, uint32_t NodeID, const ufbx_node* node, NodeTree::Item* parent)
{
	if (!node)
	{
		return;
	}

	std::string text = EnumNodeTypeText(node);
	NodeInfo ni(node);
	if (node == m_scene->root_node)
	{
		ni.name = "ROOT";
	}
	ni.id = NodeID;
	ni.type = text;
	std::string cn = ChainedName(node);
	TRACE("(%d)%s:%s:rotOrder=%d\n", node->node_depth, cn.c_str(), ni.type.c_str(),node->rotation_order);

	const ufbx_props* props = &node->props;

	// 変換プロパティを取得します。//実際には、これらはたとえばアニメーション カーブから手動で評価できます。int64_t
	auto rotation_order = ufbx_find_int(props, "RotationOrder", UFBX_ROTATION_ORDER_XYZ);
	ASSERT(rotation_order == UFBX_ROTATION_ORDER_XYZ);
	ufbx_vec3 lcl_scaling = node->local_transform.scale;
	ufbx_vec3 lcl_rotation = ToUfbxVec3(glm::eulerAngles(ToGlmQuat(node->local_transform.rotation)));//quatからeulerに変換
	ufbx_vec3 lcl_translation = node->local_transform.translation;
	auto rotation_pivot = ufbx_find_vec3(props, "RotationPivot", ufbx_zero_vec3);
	auto scaling_pivot = ufbx_find_vec3(props, "ScalingPivot", ufbx_zero_vec3);
	auto rotation_offset = ufbx_find_vec3(props, "RotationOffset", ufbx_zero_vec3);
	auto scaling_offset = ufbx_find_vec3(props, "ScalingOffset", ufbx_zero_vec3);
	
	AnimCurveVec3 lclSclCurve;
	ParseAnimCurve(lclSclCurve, getAnimProp(m_CurAnimLayer, node, "Lcl Scaling"), lcl_scaling);
#if USE_DOPESHEET
	ni.lclSclCurveLabel = DopeSheet::MakeLabel(node, "Lcl Scaling");
	m_CurSheet->curveMap[ni.lclSclCurveLabel] = lclSclCurve;
#else
	ni.lclSclCurve = lclSclCurve;
#endif

	AnimCurveVec3 tmp;
	ParseAnimCurve(tmp, getAnimProp(m_CurAnimLayer, node, "Lcl Rotation"), lcl_rotation);
	AnimCurveQuat lclRotCurve;
	MyMath::Vec3CurveToQuatCurve(&lclRotCurve, &tmp);
#if USE_DOPESHEET
	ni.lclRotCurveLabel = DopeSheet::MakeLabel(node, "Lcl Rotation");
	m_CurSheet->curveMap[ni.lclRotCurveLabel] = lclRotCurve;
#else
	ni.lclRotCurve = lclRotCurve;
#endif

	AnimCurveVec3 lclTraCurve;
	ParseAnimCurve(lclTraCurve, getAnimProp(m_CurAnimLayer, node, "Lcl Translation"), lcl_translation);
#if USE_DOPESHEET
	ni.lclTraCurveLabel = DopeSheet::MakeLabel(node, "Lcl Translation");
	m_CurSheet->curveMap[ni.lclTraCurveLabel] = lclTraCurve;
#else
	ni.lclTraCurve = lclTraCurve;
#endif

	ni.scalingPivot = ToGlmVec3(scaling_pivot);
	ni.scalingOffset = ToGlmVec3(scaling_offset);
	ni.rotationPivot = ToGlmVec3(rotation_pivot);
	ni.rotationOffset = ToGlmVec3(rotation_offset);
	ni.preRotation = ToGlmQuat(ufbx_find_vec3(props, "PreRotation", ufbx_zero_vec3));
	ni.postRotation = ToGlmQuat(ufbx_find_vec3(props, "PostRotation ", ufbx_zero_vec3));

	ni.flags = GetNodeAttributeFlags(node);
	//TRACE("%x\n", ni.flags);

	auto item = tree->Insert(ni, parent);
	m_nodeMap[node->name.data] = item;

	for (auto child : node->children)
	{
		BuildNodeTree(tree, NodeID++, child, item);
	}
}

void UFBXAsset::SetRootTransform(const mat4& m)
{
	m_rootTransform = m;
}

void UFBXAsset::EnsureYUp(const vec3& scale)
{
			ASSERT(m_scene);
	if (m_scene->settings.original_axis_up == UFBX_COORDINATE_AXIS_POSITIVE_Z)
	{
		// Z-up to Y-up：X軸を中心に-90度回転させる変換行列を作成
		m_adjust = MyMath::CreateRotationEuler(vec3(-MyMath::_PAI * 0.5f, 0.0f, 0.0f)) *
		           MyMath::CreateScaling(vec3(0.1f));
	}
	else if (m_scene->settings.original_axis_up == UFBX_COORDINATE_AXIS_POSITIVE_X)
	{
		// X-up to Y-up：Z軸を中心に-90度回転させる変換行列を作成
		m_adjust = MyMath::CreateRotationEuler(vec3(0.0f, 0.0f, -MyMath::_PAI * 0.5f)) *
		           MyMath::CreateScaling(vec3(0.1f));
	}
	else if	(m_scene->settings.original_axis_up == UFBX_COORDINATE_AXIS_POSITIVE_Y)
	{
		m_adjust = MyMath::CreateScaling(vec3(0.1f));
	}
}

void UFBXAsset::UpdateLocalTransforms()
{
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		ni.localTransform = CalcLocalTransform(itr);
	}
}

void UFBXAsset::UpdateWorldTransforms()
{
	for (auto itr = m_nodeTree.Begin(); itr; itr = itr->next)
	{
		auto& ni = itr->data;
		if (itr->parent)
		{
			auto& pni = itr->parent->data;
			ni.worldTransform = pni.worldTransform * ni.localTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
		}
		else
		{
			// 親がいない場合、これはルートノードでありローカルトランスフォームがそのままワールドトランスフォーム
			ni.worldTransform = m_rootTransform * ni.localTransform;// 親ノードのワールドトランスフォームにローカルを掛ける
		}
	}
}

mat4 UFBXAsset::CalcLocalTransform(NodeTree::Item* item)
{
	ASSERT(item);
	auto& ni = item->data;
	auto node = ni.node;
	ASSERT(node);

	vec3 scaling = ToGlmVec3(node->local_transform.scale);
	quat rotation = ToGlmQuat(node->local_transform.rotation);
	vec3 translation = ToGlmVec3(node->local_transform.translation);

#if USE_DOPESHEET
	//variantがAnimCurveXXXなのか確認せねば
	if (m_CurSheet && m_CurSheet->curveMap.count(ni.lclSclCurveLabel))
	{
		auto& curve = m_CurSheet->curveMap[ni.lclSclCurveLabel];
		if (std::holds_alternative<AnimCurveVec3>(curve))
		{
			scaling = std::get<AnimCurveVec3>(curve).Evaluate(m_time);
		}
	}
	if (m_CurSheet && m_CurSheet->curveMap.count(ni.lclRotCurveLabel))
	{
		auto& curve = m_CurSheet->curveMap[ni.lclRotCurveLabel];
		if (std::holds_alternative<AnimCurveQuat>(curve))
		{
			rotation = std::get<AnimCurveQuat>(curve).Evaluate(m_time);
		}
	}
	if (m_CurSheet && m_CurSheet->curveMap.count(ni.lclTraCurveLabel))
	{
		auto& curve = m_CurSheet->curveMap[ni.lclTraCurveLabel];
		if (std::holds_alternative<AnimCurveVec3>(curve))
		{
			translation = std::get<AnimCurveVec3>(curve).Evaluate(m_time);
		}
	}
#else
	scaling = ni.lclSclCurve.Evaluate(m_time);
	rotation = ni.lclRotCurve.Evaluate(m_time);
	translation = ni.lclTraCurve.Evaluate(m_time);
#endif

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

mat4 UFBXAsset::CalcWorldTransform(NodeTree::Item* item/*, const fbxsdk::FbxTime& time*/)
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

void UFBXAsset::DeformBoneWeight()
{
	for (auto mesh : m_meshes)
	{
		if (mesh->vbuf_.size())
		{
			mesh->deform_.resize(mesh->vbuf_.size());
		}
	}
	for (auto mesh : m_meshes)
	{
		auto& si = mesh->skin;
		// MeshInfoのクラスタ分の頂点変形行列を計算⇒パレットに置く
		std::vector<mat4>	clusterDeformations(96,mat4(1.0f));
		GetDeformMatrix(mesh, clusterDeformations.data(), clusterDeformations.size());
		for(auto& geom : mesh->MaterialGroup)
		{
			geom.deform.resize(geom.vbuf.size());
			// 最終的に頂点座標を計算しVERTEXに変換
			for (auto i = 0u; i < geom.pointsCount; i++)
			{
				auto& inVertex = geom.vbuf[i];
#if 1
				mat4 skinMat =
					inVertex.weight[0] * clusterDeformations[inVertex.bone[0]] +
					inVertex.weight[1] * clusterDeformations[inVertex.bone[1]] +
					inVertex.weight[2] * clusterDeformations[inVertex.bone[2]] +
					inVertex.weight[3] * clusterDeformations[inVertex.bone[3]];
				mat3 normalMatrix = transpose(inverse(mat3(skinMat))); // 正確な法線変換用
				geom.deform[i].pos = (skinMat * vec4(inVertex.pos, 1.0f)).xyz();
				geom.deform[i].normal = glm::normalize(normalMatrix * inVertex.normal);
				geom.deform[i].tangent = glm::normalize(normalMatrix * inVertex.tangent);
				geom.deform[i].texcoord = inVertex.texcoord;
#else
				//ここの処理をシェーダにまわす
				geom.deform[i].pos = vec3(0.0f);
				geom.deform[i].normal = vec3(0.0f);
				geom.deform[i].tangent = vec3(0.0f);
				vec3 deformed(0.0f);
				for (auto j = 0; j < 4/*BoneWeight::NUM_BONES_PER_VERTEX*/; j++)
				{
					auto w = inVertex.weight[j];
					if (w == 0.0f)
					{
						continue;
					}
					auto b = inVertex.bone[j];
					geom.deform[i].pos += MyTransform(clusterDeformations[b], inVertex.pos) * w;
					geom.deform[i].normal += MyTransformNormal(clusterDeformations[b], inVertex.normal) * w;
					geom.deform[i].tangent += MyTransformNormal(clusterDeformations[b], inVertex.tangent) * w;
				}
				geom.deform[i].normal = glm::normalize(geom.deform_[i].normal);
				geom.deform[i].tangent = glm::normalize(geom.deform_[i].tangent);
				geom.deform[i].texcoord = inVertex.texcoord;
				geom.deform[i].color = inVertex.color;
#endif
			}
		}
	}
}

mat4 UFBXAsset::ComputeClusterDeformation(const MeshInfo* mesh, const ClusterInfo& ci) const
{
	if (m_nodeMap.find(ci.boneName) == m_nodeMap.end())
	{
		TRACE("ボーン<%s>が見つかりません\n", ci.boneName.c_str());
		return mat4(1.0f);
	}
	auto& bone = m_nodeMap.at(ci.boneName);
	assert(bone);

	return bone->data.worldTransform * ci.GetTransformLinkMatrix() * mesh->GeometricMatrix;
}







