/*                                                                   */
/*  Copyright (c) 2009-2011  Hakuro Matsuda                          */
/*                           hakuroum@gmail.com                      */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Files located inModel/ folders are not under this license term. */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the MikuMikuPhone project team nor the names*/
/*   of its contributors may be used to endorse or promote products  */
/*   derived from this software without specific prior written       */
/*   permission.                                                     */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
//
// https://github.com/syoyo/MMDLoaderのMMDLoaderExampleを改変
//
#include "pch.h"
#include "MMD.h"






void mmd::MMDScene::AttachAnimation(VMD* anim)
{
	assert(model_);

	// Create hash map for bone <-> motion list
	using MotionMap = std::map<std::string, std::vector<Motion> >;
	MotionMap motionMap;

	for (int i = 0; i < anim->motions_.size(); i++)
	{
		VMDMotion& vmdMotion = anim->motions_[i];
		// bone_name[15] might not be null-terminated.
		// So append '\0' to reconstruct a string.
		char buf[16];
		memcpy(buf, vmdMotion.bone_name, 15);
		buf[15] = '\0';
		std::string boneName(buf);

		Motion motion;
		//curveのキーを記録しておく
		motion.frameNo = vmdMotion.frame_no;
		motion.pos = glm::make_vec3(vmdMotion.location);
		motion.rotation = glm::make_quat(vmdMotion.rotation);
		
		const auto& ptr = vmdMotion.interpolation;
		motion.interpX = glm::u8vec4(ptr[0], ptr[4], ptr[8], ptr[12]);
		motion.interpY = glm::u8vec4(ptr[1], ptr[5], ptr[9], ptr[13]);
		motion.interpZ = glm::u8vec4(ptr[2], ptr[6], ptr[10], ptr[14]);
		motion.interpR = glm::u8vec4(ptr[3], ptr[7], ptr[11], ptr[15]);
		motionMap[boneName].push_back(motion);
	}
	//originalでないならばMotion構造体自体不要

#if 1//original
	// Assign motion list to a bone.
	for (int i = 0; i < model_->bones_.size(); i++)
	{
		Bone& bone = model_->bones_[i];
		bone.motions.clear();
		MotionMap::iterator it = motionMap.find(bone.name);
		if (it != motionMap.end())
		{
			bone.motions = it->second;
		}
		else
		{
			printf("[MMD] Cannot find bone [ %s ] in PMD.\n", bone.name.c_str());
		}
	}
#endif

	anim_ = anim; // save for a reference.
}

void mmd::MMDScene::SetRootTransform(const mat4& m)
{
	root_ = m;
}

void mmd::MMDScene::UpdateBone(float frame, float step)
{
	//printf("UpdateBone()\n");
	if (model_->bones_.empty())
	{
		return;
	}

	assert(0);
}


#pragma region pmd_reader


namespace mmd
{
	struct sjis_table_rec_t
	{
		static const int MAX_BUF_LEN = 20;

		char unicode_name[MAX_BUF_LEN];
		char ascii_name[MAX_BUF_LEN];
		uint8_t sjis_name[MAX_BUF_LEN];
	};


	// http://akemiwhy.deviantart.com/art/mmd-reference-japanese-bone-names-430962605
	// http://ash.jp/code/unitbl21.htm
	sjis_table_rec_t sjis_table[] =
	{
		{"グルーブ", "groove", {0x83, 0x4F, 0x83, 0x8B, 0x81, 0x5B, 0x83, 0x75, 0x00}},
		{"センター", "center", {0x83, 0x5A, 0x83, 0x93, 0x83, 0x5E, 0x81, 0x5B, 0x00}},
		{"上半身", "upper_body", {0x8F, 0xE3, 0x94, 0xBC, 0x90, 0x67, 0x00}},
		{"下半身", "lower_body", {0x89, 0xBA, 0x94, 0xBC, 0x90, 0x67, 0x00}},
		{"両目", "eyes", {0x97, 0xBC, 0x96, 0xDA, 0x00}},
		{"全ての親", "mother", {0x91, 0x53, 0x82, 0xC4, 0x82, 0xCC, 0x90, 0x65, 0x00}},
		{"右つま先ＩＫ", "toe_IK_R", {0x89, 0x45, 0x82, 0xC2, 0x82, 0xDC, 0x90, 0xE6, 0x82, 0x68, 0x82, 0x6A, 0x00}},
		{"右ひざ", "knee_R", {0x89, 0x45, 0x82, 0xD0, 0x82, 0xB4, 0x00}},
		{"右ひじ", "elbow_R", {0x89, 0x45, 0x82, 0xD0, 0x82, 0xB6, 0x00}},
		{"右中指１", "middle1_R", {0x89, 0x45, 0x92, 0x86, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"右中指２", "middle2_R", {0x89, 0x45, 0x92, 0x86, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"右中指３", "middle3_R", {0x89, 0x45, 0x92, 0x86, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"右人指１", "fore1_R", {0x89, 0x45, 0x90, 0x6C, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"右人指２", "fore2_R", {0x89, 0x45, 0x90, 0x6C, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"右人指３", "fore3_R", {0x89, 0x45, 0x90, 0x6C, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"右小指１", "little1_R", {0x89, 0x45, 0x8F, 0xAC, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"右小指２", "little2_R", {0x89, 0x45, 0x8F, 0xAC, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"右小指３", "little3_R", {0x89, 0x45, 0x8F, 0xAC, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"右手首", "wrist_R", {0x89, 0x45, 0x8E, 0xE8, 0x8E, 0xF1, 0x00}},
		{"右目", "eye_R", {0x89, 0x45, 0x96, 0xDA, 0x00}},
		{"右肩", "shoulder_R", {0x89, 0x45, 0x8C, 0xA8, 0x00}},
		{"右腕", "arm_R", {0x89, 0x45, 0x98, 0x72, 0x00}},
		{"右薬指１", "third1_R", {0x89, 0x45, 0x96, 0xF2, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"右薬指２", "third2_R", {0x89, 0x45, 0x96, 0xF2, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"右薬指３", "third3_R", {0x89, 0x45, 0x96, 0xF2, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"右袖", "sleeve_R", {0x89, 0x45, 0x91, 0xb3, 0x00}},
		{"右袖先", "cuff_R", {0x89, 0x45, 0x91, 0xb3, 0x90, 0xe6, 0x00}},
		{"右親指１", "thumb1_R", {0x89, 0x45, 0x90, 0x65, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"右親指２", "thumb2_R", {0x89, 0x45, 0x90, 0x65, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"右足", "leg_R", {0x89, 0x45, 0x91, 0xAB, 0x00}},
		{"右足首", "ankle_R", {0x89, 0x45, 0x91, 0xAB, 0x8E, 0xF1, 0x00}},
		{"右足ＩＫ", "leg_IK_R", {0x89, 0x45, 0x91, 0xAB, 0x82, 0x68, 0x82, 0x6A, 0x00}},
		{"左つま先ＩＫ", "toe_IK_L", {0x8D, 0xB6, 0x82, 0xC2, 0x82, 0xDC, 0x90, 0xE6, 0x82, 0x68, 0x82, 0x6A, 0x00}},
		{"左ひざ", "knee_L", {0x8D, 0xB6, 0x82, 0xD0, 0x82, 0xB4, 0x00}},
		{"左ひじ", "elbow_L", {0x8D, 0xB6, 0x82, 0xD0, 0x82, 0xB6, 0x00}},
		{"左中指１", "middle1_L", {0x8D, 0xB6, 0x92, 0x86, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"左中指２", "middle2_L", {0x8D, 0xB6, 0x92, 0x86, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"左中指３", "middle3_L", {0x8D, 0xB6, 0x92, 0x86, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"左人指１", "fore1_L", {0x8D, 0xB6, 0x90, 0x6C, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"左人指２", "fore2_L", {0x8D, 0xB6, 0x90, 0x6C, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"左人指３", "fore3_L", {0x8D, 0xB6, 0x90, 0x6C, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"左小指１", "little1_L", {0x8D, 0xB6, 0x8F, 0xAC, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"左小指２", "little2_L", {0x8D, 0xB6, 0x8F, 0xAC, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"左小指３", "little3_L", {0x8D, 0xB6, 0x8F, 0xAC, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"左手首", "wrist_L", {0x8D, 0xB6, 0x8E, 0xE8, 0x8E, 0xF1, 0x00}},
		{"左目", "eye_L", {0x8D, 0xB6, 0x96, 0xDA, 0x00}},
		{"左肩", "shoulder_L", {0x8D, 0xB6, 0x8C, 0xA8, 0x00}},
		{"左腕", "arm_L", {0x8D, 0xB6, 0x98, 0x72, 0x00}},
		{"左薬指１", "third1_L", {0x8D, 0xB6, 0x96, 0xF2, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"左薬指２", "third2_L", {0x8D, 0xB6, 0x96, 0xF2, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"左薬指３", "third3_L", {0x8D, 0xB6, 0x96, 0xF2, 0x8E, 0x77, 0x82, 0x52, 0x00}},
		{"左袖", "sleeve_L", {0x8D, 0xB6, 0x91, 0xb3, 0x00}},
		{"左袖先", "cuff_L", {0x8D, 0xB6, 0x91, 0xb3, 0x90, 0xe6, 0x00}},
		{"左親指１", "thumb1_L", {0x8D, 0xB6, 0x90, 0x65, 0x8E, 0x77, 0x82, 0x50, 0x00}},
		{"左親指２", "thumb2_L", {0x8D, 0xB6, 0x90, 0x65, 0x8E, 0x77, 0x82, 0x51, 0x00}},
		{"左足", "leg_L", {0x8D, 0xB6, 0x91, 0xAB, 0x00}},
		{"左足首", "ankle_L", {0x8D, 0xB6, 0x91, 0xAB, 0x8E, 0xF1, 0x00}},
		{"左足ＩＫ", "leg_IK_L", {0x8D, 0xB6, 0x91, 0xAB, 0x82, 0x68, 0x82, 0x6A, 0x00}},
		{"頭", "head", {0x93, 0xAA, 0x00}},
		{"首", "neck", {0x8E, 0xF1, 0x00}},
	};

	using sjis_map_t = std::map<std::string, sjis_table_rec_t*>;
	sjis_map_t sjis_map;
	std::set<std::string> unvisited_bones;
} // namespace

//
//
//



bool mmd::PMDReader::InitSjisTbl()
{
	size_t n = sizeof(sjis_table) / sizeof(sjis_table_rec_t);
	for (int i = 0; i < n; i++)
	{
		// DBG
		// wchar_t* unicode_name = (wchar_t*)sjis_table[i].unicode_name;
		// char* ascii_name = sjis_table[i].ascii_name;
		// wchar_t* sjis_name = (wchar_t*)sjis_table[i].sjis_name;
		// printf("unicode_name: %ls, ascii_name: %s, sjis_name: %ls\n", unicode_name, ascii_name, sjis_name);
		sjis_map.insert(sjis_map_t::value_type((char*)sjis_table[i].sjis_name, &sjis_table[i]));
	}
	return true;
}


//static bool DumpBone(PMD* model)
//{
//	auto boneCnt = model->bones_.size();
//	for (auto i = 0u; i < boneCnt; i++)
//	{
//		auto& bone = model->bones_[i];
//		auto name = bone.name.c_str();
//		auto ascii_name = bone.ascii_name.c_str();
//		TRACE("[%zu/%zu] parent=%u name='%s' ascii='%s'\n", i, boneCnt, bone.parentIndex, name, ascii_name);
//	}
//
//	return true;
//}

mmd::PMD* mmd::PMDReader::LoadFromFile(const std::string& filename)
{
	std::ifstream is(filename, std::ios::binary);
	if (!is)
	{
		TRACE("Can't read PMD file [ %s ]\n", filename.c_str());
		return NULL;
	}
	auto pmd = LoadFromStream(is);
	ASSERT(pmd);
	ASSERT(is.good());
	is.close();
	return pmd;
}

mmd::PMD* mmd::PMDReader::LoadFromStream(std::istream& is)
{
	PMD* model = new PMD();
	ASSERT(model);

	// file header
	{
		const char kMagic[] = "Pmd";
		const float kVersion = 1.0f; // 0x3F800000

		char magic[3];
		is.read((char*)(magic), 3);
		assert(magic[0] == kMagic[0]);
		assert(magic[1] == kMagic[1]);
		assert(magic[2] == kMagic[2]);

		float version = 0.0f;
		is.read((char*)(&version), 4);
		assert(version == kVersion);

		model->version_ = version;
	}

	// name&comment
	{
		unsigned char name[20];
		unsigned char comment[256];
		is.read((char*)name, 20);
		is.read((char*)comment, 256);

		model->name_ = std::string((char*)(name));
		model->comment_ = std::string((char*)(comment));

		TRACE("[PMDReader] name = %s\n", model->name_.c_str());
		TRACE("[PMDReader] comment = %s\n", model->comment_.c_str());
	}

	// Vertices
	{
		int numVertices;
		is.read((char*)(&numVertices), 4);
		TRACE("[PMD] Num vertices: %d\n", numVertices);
		assert(sizeof(PMDVertex) == 38);
		model->vertices_.resize(numVertices);
		is.read((char*)(&(model->vertices_[0])), sizeof(PMDVertex) * numVertices);
		//for (const auto& v : model->vertices_)
		//{
		//	TRACE("%f %f %f	%f %f\n", v.pos[0], v.pos[1], v.pos[2], v.uv[0], v.uv[1]);
		//}
	}

	// Indices
	{
		int numIndices;
		is.read((char*)(&numIndices), 4);
		TRACE("[PMD] Num indices: %d\n", numIndices);
		model->indices_.resize(numIndices);
		is.read((char*)(&(model->indices_[0])), sizeof(unsigned short) * numIndices);

		// validate
		for (int i = 0; i < numIndices; i++)
		{
			assert(model->indices_[i] < model->vertices_.size());
		}
	}

	// Materials
	{
		int numMaterials;
		is.read((char*)(&numMaterials), 4);
		TRACE("[PMD] Num materials: %d\n", numMaterials);
		assert(sizeof(PMDMaterial) == 70);
		model->materials_.resize(numMaterials);
		is.read((char*)(&(model->materials_[0])), sizeof(PMDMaterial) * numMaterials);

		// validate
		size_t sumVertexCount = 0;
		for (int i = 0; i < numMaterials; i++)
		{
			assert((model->materials_[i].vertex_count % 3) == 0);
			sumVertexCount += model->materials_[i].vertex_count;

			TRACE("mat[%d] texname = %s\n", i, model->materials_[i].texture_filename);
		}
		assert(sumVertexCount == model->indices_.size());
	}

	//ASSERT(ParseBone(model, ifs));
	{
		unvisited_bones.clear();
		for (sjis_map_t::iterator p = sjis_map.begin(); p != sjis_map.end(); p++)
		{
			unvisited_bones.insert((*p).second->ascii_name);
		}

		// cp932 encoding of 'leg'(hi-za) in Japanese
		const unsigned char kLegName[4 + 1] = { 0x82, 0xd0, 0x82, 0xb4, 0x00 }; // +1 for \0

		unsigned short numBones;
		is.read((char*)(&numBones), sizeof(uint16_t));
		TRACE("[PMD] Num bones: %d\n", numBones);
		assert(sizeof(PMDBone) == 39);
		std::vector<PMDBone> pmdBones(numBones);
		is.read((char*)(&(pmdBones[0])),
			sizeof(PMDBone) * numBones);

		model->bones_.clear();

		// Bone with name containing 'leg' in Japanese need special treatment
		// when computing IK.
		for (int i = 0; i < numBones; i++)
		{
			Bone bone;

			bone.parentIndex = pmdBones[i].parent_bone_index;
			bone.tailIndex = pmdBones[i].tail_bone_index;
			bone.type = pmdBones[i].bone_type;
			bone.parentIndexIK = pmdBones[i].ik_parent_bone_index;

			if (pmdBones[i].tail_bone_index == (unsigned short)(-1))
			{
				// skip
				TRACE("[PMD] Bone [%d] is the tail. Skipping.\n", i);
				continue;
			}

			bone.pos[0] = pmdBones[i].bone_pos[0];
			bone.pos[1] = pmdBones[i].bone_pos[1];
			bone.pos[2] = pmdBones[i].bone_pos[2];
			bone.pos[3] = 1.0f;

			bone.bindPose = glm::mat4(1.0f);
			bone.bindPose[3].x = -pmdBones[i].bone_pos[0];
			bone.bindPose[3].y = -pmdBones[i].bone_pos[1];
			bone.bindPose[3].z = -pmdBones[i].bone_pos[2];

			char buf[21];
			memcpy(buf, pmdBones[i].bone_name, 20);
			buf[20] = '\0'; // add '\0' for safety
			TRACE("[%d]bone_name=[%s]\n", i, buf);
			bone.name = std::string(buf);
			if (bone.name.find((char*)kLegName) != std::string::npos)
			{
				TRACE("[PMD] Bone [%d] is leg\n", i);
				bone.isLeg = true;
			}
			else
			{
				bone.isLeg = false;
			}
			sjis_map_t::iterator p = sjis_map.find(buf);
			if (p != sjis_map.end())
			{
				bone.ascii_name = (*p).second->ascii_name;
				std::set<std::string>::iterator q = unvisited_bones.find((*p).second->ascii_name);
				if (q != unvisited_bones.end())
				{
					unvisited_bones.erase(q);
				}
			}

			model->bones_.push_back(bone);
		}

		for (std::set<std::string>::iterator r = unvisited_bones.begin(); r != unvisited_bones.end(); r++)
		{
			TRACE("[PMD] Cannot find bone [ %s ] in PMD.\n",r->c_str());
		}
	}

	//ASSERT(ParseIK(model, is));
	{
		unsigned short numIKs;
		is.read((char*)(&numIKs), sizeof(unsigned short));
		TRACE("[PMD] Num IKs: %d\n", numIKs);
		assert(sizeof(PMDIK) == 11);
		std::vector<IK> iks(numIKs);
		for (int i = 0; i < numIKs; i++) {
			PMDIK pmdIK;
			is.read((char*)(&pmdIK), sizeof(PMDIK));

			iks[i].boneIndex = pmdIK.bone_index;
			iks[i].targetBoneIndex = pmdIK.target_bone_index;
			iks[i].chainLength = pmdIK.chain_length;
			iks[i].iterations = pmdIK.iterations;
			iks[i].weight = pmdIK.weight;

			iks[i].childBoneIndices.resize(iks[i].chainLength);
			is.read((char*)(&(iks[i].childBoneIndices[0])),
				sizeof(unsigned short) * iks[i].chainLength);
		}

		model->iks_ = iks;
	}

	//ASSERT(ParseMorph(model, is));
	{
		uint16_t numMorphs;
		is.read((char*)(&numMorphs), sizeof(uint16_t));
		TRACE("[PMD] Num Morphs: %d\n", numMorphs);
		static_assert(sizeof(PMDMorphVertex) == 16);

		model->morphs_.resize(numMorphs);
		std::vector<Morph> morphs(numMorphs);
		for (int i = 0; i < numMorphs; i++)
		{
			PMDMorph pmdMorph = {};
			is.read((char*)(&pmdMorph), sizeof(PMDMorph));
			std::vector<PMDMorphVertex> vertices(pmdMorph.vertex_count);
			TRACE("pmdMorph[%d/%d].name='%s' .vertex_vount=%d\n",i, numMorphs, pmdMorph.name, pmdMorph.vertex_count);
			
			auto& morph = model->morphs_[i];
			morph.name = pmdMorph.name;
			morph.type = pmdMorph.type;
			morph.vertexCount = pmdMorph.vertex_count;
			morph.vertices.resize(pmdMorph.vertex_count);
			for (int j = 0; j < (int)pmdMorph.vertex_count; j++)
			{
				is.read((char*)&morph.vertices[j], sizeof(PMDMorphVertex));
			}
		}
	}

//	is.close();

	TRACE("[PMD] Load OK\n");

	return model;
}
#pragma endregion

#pragma region vmd_amimation

//
// MMD IK computation is based on MikuMikuDroid:
// http://svn.sourceforge.jp/svnroot/mikumikudroid/trunk/MikuMikuDroid
//
// IK ccd solver:
// http://www.tmps.org/index.php?CCD-IK%20and%20Particle-IK
//

//using namespace mmd;


// Recursive.
void mmd::MMDScene::UpdateBoneMatrix(Bone& bone)
{
	if (bone.updated == false)
	{
		if (bone.parentIndex == 0xffffu)
		{
			bone.matrix = bone.matrixTemp * root_;
		}
		else
		{
			Bone& parent = model_->bones_[bone.parentIndex];
			UpdateBoneMatrix(parent);
			bone.matrix = parent.matrix * bone.matrixTemp;
		}
		bone.updated = true;
	}
}

// Recurvesly update bone matrix.
void mmd::MMDScene::UpdateBoneMatrix(Bone* bone, PMD* model)
{
	if (bone->updated == false)
	{
		if (bone->parentIndex != 0xffffu)
		{
			Bone* p = &(model->bones_.at(bone->parentIndex));
			UpdateBoneMatrix(p, model);
			bone->matrix = p->matrix * bone->matrixTemp;
		}
		else
		{
			bone->matrix = bone->matrixTemp;
		}
		bone->updated = true;
	}
}

void mmd::MMDScene::GetCurrentBoneMatrix(mat4& mat, Bone& bone, PMD* model)
{
	UpdateBoneMatrix(&bone, model);
	mat = bone.matrix;
}

void mmd::MMDScene::GetCurrentBonePosition(vec3& v, Bone& bone, PMD* model)
{
	UpdateBoneMatrix(&bone, model);
	v = bone.matrix[3].xyz;
}

float mmd::MMDScene::BezierEval(const glm::u8vec4& ip, float t)
{
	// MMDのベジェ制御点 (P1, P2) の値を取得
	// P0=(0, 0), P3=(1, 1) は固定
	float xa = (float)ip[0] / 256.0f; // P1.x
	float xb = (float)ip[2] / 256.0f; // P2.x
	float ya = (float)ip[1] / 256.0f; // P1.y
	float yb = (float)ip[3] / 256.0f; // P2.y

	float min_t = 0.0f;
	float max_t = 1.0f;

	// ct は曲線のパラメータ (0.0 から 1.0)
	float ct = t; // 初期推定値として t (求めたいx座標) を使うのは妥当

	const int MAX_ITERATIONS = 20; // 最大反復回数を設定 (MMDでは20〜50回程度で十分)
	const float EPSILON = 1.0e-06f; // 収束判定のしきい値

	// 最初に x 座標が 0.0 や 1.0 に近い場合のチェック
	if (t <= 0.0f) return 0.0f;
	if (t >= 1.0f) return 1.0f;

	// 二分探索/反復処理で、曲線の x 座標が t になるようなパラメータ ct を探す
	for (int i = 0; i < MAX_ITERATIONS; i++)
	{
		// デ・カステルジョのアルゴリズムで、現在の ct における曲線の x 座標 (x3) を計算
		// x軸の制御点: (0, 0), (xa, ya), (xb, yb), (1, 1)

		// 1次補間
		float x11 = 0.0f + (xa - 0.0f) * ct;       // P0-P1 間
		float x12 = xa + (xb - xa) * ct;           // P1-P2 間
		float x13 = xb + (1.0f - xb) * ct;         // P2-P3 間

		// 2次補間
		float x21 = x11 + (x12 - x11) * ct;
		float x22 = x12 + (x13 - x12) * ct;

		// 3次補間: 最終的な曲線上での x 座標
		float x3 = x21 + (x22 - x21) * ct;

		// 1. 収束判定
		if (fabs(x3 - t) < EPSILON)
		{
			// x 座標が目標値 t に十分近ければ、その ct を使って y 座標を計算し返す

			// 同様に y 座標 (y3) を計算
			float y11 = 0.0f + (ya - 0.0f) * ct;
			float y12 = ya + (yb - ya) * ct;
			float y13 = yb + (1.0f - yb) * ct;

			float y21 = y11 + (y12 - y11) * ct;
			float y22 = y12 + (y13 - y12) * ct;

			float y3 = y21 + (y22 - y21) * ct;

			return y3;
		}
		// 2. 二分探索の範囲を更新
		else if (x3 < t)
		{
			// 現在の x3 は目標値 t より小さいので、パラメータ ct は min_t と max_t の間の後半にある
			min_t = ct;
		}
		else
		{
			// 現在の x3 は目標値 t より大きいので、パラメータ ct は min_t と max_t の間の前半にある
			max_t = ct;
		}

		// 次の試行のための ct を更新 (二分探索)
		ct = min_t * 0.5f + max_t * 0.5f;
	}

	// MAX_ITERATIONS 回反復しても収束しなかった場合、最後に計算した y 座標を返すか、
	// エラー処理として t を返すなど、アプリケーションの仕様に合わせる
	// ここでは、最も近いはずの y3 の計算を最終的に行う

	// 最後に計算した ct を使って y3 を再度計算し返す
	float y11 = 0.0f + (ya - 0.0f) * ct;
	float y12 = ya + (yb - ya) * ct;
	float y13 = yb + (1.0f - yb) * ct;

	float y21 = y11 + (y12 - y11) * ct;
	float y22 = y12 + (y13 - y12) * ct;

	float y3 = y21 + (y22 - y22) * ct;

	return y3;
}

void mmd::MMDScene::ClearUpdateFlags(int rootIndex, int boneIndex, mmd::PMD* model)
{
	mmd::Bone* bone = &model->bones_.at(boneIndex);
	while (rootIndex != boneIndex)
	{
		bone->updated = false;
		if (bone->parentIndex != 0xffffu)
		{
			bone = &model->bones_.at(bone->parentIndex);
		}
		else
		{
			return;
		}
	}
	mmd::Bone& root = model->bones_.at(rootIndex);
	root.updated = false;
}

void mmd::MMDScene::ClearUpdateFlags(std::vector<mmd::Bone>& bones)
{
	for (int i = 0; i < bones.size(); i++)
	{
		bones[i].updated = false;
	}
}

#if 0
void mmd::MMDScene::IKSolve(IK* ik, float errToleranceSq)
{
	//
	// Solve IK with CCD algorithm.
	//
	ASSERT(ik);
	Bone& effector = model_->bones_.at(ik->boneIndex);
	Bone& target = model_->bones_.at(ik->targetBoneIndex);

	vec3 localTargetPos = vec3(0.0f);
	vec3 localEffectorPos = vec3(0.0f);

	vec3 effectorPos;
	GetCurrentBonePosition(effectorPos, effector, model_);

	for (int i = 0; i < ik->iterations; i++)
	{
		for (int j = 0; j < ik->chainLength; j++)
		{
			Bone& bone = model_->bones_.at(ik->childBoneIndices[j]);

			ClearUpdateFlags(ik->childBoneIndices[j], ik->targetBoneIndex, model_);

			vec3 targetPos;
			GetCurrentBonePosition(targetPos, target, model_);

			if (bone.isLeg)
			{
				if (i == 0)
				{
					Bone& base = model_->bones_.at(ik->childBoneIndices[ik->chainLength - 1]);
					GetCurrentBonePosition(localTargetPos, bone, model_);
					GetCurrentBonePosition(localEffectorPos, base, model_);

					vec3 effectorVec = effectorPos - localEffectorPos;
					vec3 boneVec = localTargetPos - localEffectorPos;
					vec3 targetVec = targetPos - localTargetPos;
					float el = glm::length(effectorVec);
					float bl = glm::length(boneVec);
					float tl = glm::length(targetVec);
					float c = (el * el - bl * bl - tl * tl) / (2.0f * bl * tl);
					if (c < -1.0f)	c = -1.0f;
					if (c > 1.0f)	c = 1.0f;
					float angle = MyMath::ArcCos(c);

					vec3 axis = vec3(-1.0f, 0.0f, 0.0f);

					quat qa = glm::angleAxis(angle, axis);
					bone.rotation = glm::normalize(bone.rotation * qa);

					// Preserve translation
					mat4 m;
					m = glm::toMat4(bone.rotation);
					bone.matrixTemp[0] = m[0];
					bone.matrixTemp[1] = m[1];
					bone.matrixTemp[2] = m[2];
				}
			}
			else
			{
				vec3 d = effectorPos - targetPos;
				float diffSq = glm::length(d);
				if (diffSq < errToleranceSq)
				{
					// converged.
					ClearUpdateFlags(model_->bones_);
					return;
				}

				// world -> local
				mat4 invM;
				GetCurrentBoneMatrix(invM, bone, model_);
				invM = glm::inverse(invM);

				localEffectorPos = glm::vec3(invM * glm::vec4(effectorPos, 1.0f));
				localTargetPos = glm::vec3(invM * glm::vec4(targetPos, 1.0f));

				// basis -> effector
				vec3 basis2Effector = localEffectorPos;
				basis2Effector = glm::normalize(basis2Effector);

				// basis -> target
				vec3 basis2Target = localTargetPos;
				basis2Target = glm::normalize(basis2Target);

				// Calculate shortest rotation angle.
				float rotationDotProduct = glm::dot(basis2Effector, basis2Target);

				if (rotationDotProduct < -1.0f)	rotationDotProduct = -1.0f;
				if (rotationDotProduct > 1.0f)	rotationDotProduct = 1.0f;
				
				float rotationAngle = (float)acos(rotationDotProduct);
				rotationAngle *= ik->weight;
				// if (rotationAngle > 1.0e-5f) {
				{
					vec3 rotationAxis;
					rotationAxis = glm::cross(basis2Target, basis2Effector);
					rotationAxis = glm::normalize(rotationAxis);

					quat q0 = glm::angleAxis(rotationAngle, rotationAxis);
					bone.rotation = glm::normalize(bone.rotation * q0);

					// Preserve translation
					mat4 m;
					m = glm::toMat4(bone.rotation);
					bone.matrixTemp[0] = m[0];
					bone.matrixTemp[1] = m[1];
					bone.matrixTemp[2] = m[2];
				}
			}
		}
	}

	ClearUpdateFlags(model_->bones_);
}
#else
void mmd::MMDScene::IKSolve(IK* ik, float errToleranceSq)
{
	//
	// Solve IK with CCD algorithm.
	//
	ASSERT(ik);
	Bone& effector = model_->bones_.at(ik->boneIndex);
	Bone& target = model_->bones_.at(ik->targetBoneIndex);

	vec3 localTargetPos = vec3(0.0f);
	vec3 localEffectorPos = vec3(0.0f);

	vec3 effectorPos;
	GetCurrentBonePosition(effectorPos, effector, model_);

	for (int i = 0; i < ik->iterations; i++)
	{
		for (int j = 0; j < ik->chainLength; j++)
		{
			Bone& bone = model_->bones_.at(ik->childBoneIndices[j]);

			ClearUpdateFlags(ik->childBoneIndices[j], ik->targetBoneIndex, model_);

			vec3 targetPos;
			GetCurrentBonePosition(targetPos, target, model_);

			if (bone.isLeg)
			{
				if (i == 0)
				{
					Bone& base = model_->bones_.at(ik->childBoneIndices[ik->chainLength - 1]);
					GetCurrentBonePosition(localTargetPos, bone, model_);
					GetCurrentBonePosition(localEffectorPos, base, model_);

					vec3 effectorVec = effectorPos - localEffectorPos;
					vec3 boneVec = localTargetPos - localEffectorPos;
					vec3 targetVec = targetPos - localTargetPos;
					float el = glm::length(effectorVec);
					float bl = glm::length(boneVec);
					float tl = glm::length(targetVec);
					float c = (el * el - bl * bl - tl * tl) / (2.0f * bl * tl);
					if (c < -1.0f)	c = -1.0f;
					if (c > 1.0f)	c = 1.0f;
					float angle = MyMath::ArcCos(c);

					vec3 axis = vec3(-1.0f, 0.0f, 0.0f);

					quat qa = glm::angleAxis(angle, axis);
					bone.rotation = glm::normalize(bone.rotation * qa);

					// Preserve translation
					glm::mat3 m = glm::toMat3(bone.rotation);
					bone.matrixTemp[0] = vec4(m[0], 0.0f);
					bone.matrixTemp[1] = vec4(m[1], 0.0f);
					bone.matrixTemp[2] = vec4(m[2], 0.0f);
				}
			}
			else
			{
				vec3 d = effectorPos - targetPos;
				float diffSq = glm::length(d);
				if (diffSq < errToleranceSq)
				{
					// converged.
					ClearUpdateFlags(model_->bones_);
					return;
				}

				// world -> local
				mat4 invM;
				GetCurrentBoneMatrix(invM, bone, model_);
				invM = glm::inverse(invM);

				localEffectorPos = glm::vec3(invM * glm::vec4(effectorPos, 1.0f));
				localTargetPos = glm::vec3(invM * glm::vec4(targetPos, 1.0f));

				// basis -> effector
				vec3 basis2Effector = localEffectorPos;
				basis2Effector = glm::normalize(basis2Effector);

				// basis -> target
				vec3 basis2Target = localTargetPos;
				basis2Target = glm::normalize(basis2Target);

				// Calculate shortest rotation angle.
				float rotationDotProduct = glm::dot(basis2Effector, basis2Target);

				if (rotationDotProduct < -1.0f)	rotationDotProduct = -1.0f;
				if (rotationDotProduct > 1.0f)	rotationDotProduct = 1.0f;

				float rotationAngle = MyMath::ArcCos(rotationDotProduct);
				rotationAngle *= ik->weight;
				if (rotationAngle > 1.0e-5f)
				{
					vec3 rotationAxis;
					rotationAxis = glm::cross(basis2Target, basis2Effector);
					rotationAxis = glm::normalize(rotationAxis);

					quat q0 = glm::angleAxis(rotationAngle, rotationAxis);
					bone.rotation = glm::normalize(bone.rotation * q0);

					// Preserve translation
					glm::mat3 m = glm::toMat4(bone.rotation);
					bone.matrixTemp[0] = vec4(m[0], 0.0f);
					bone.matrixTemp[1] = vec4(m[1], 0.0f);
					bone.matrixTemp[2] = vec4(m[2], 0.0f);
				}
			}
		}
	}

	ClearUpdateFlags(model_->bones_);
}
#endif

#pragma endregion

#pragma region vmd_reader



mmd::VMD* mmd::VMDReader::LoadFromFile(const std::string& filename)
{
	std::ifstream is(filename, std::ios::binary);
	if (!is)
	{
		TRACE("Can't read PMD file [ %s ]\n", filename.c_str());
		return NULL;
	}
	auto vmd = LoadFromStream(is);
	ASSERT(vmd);
	ASSERT(is.good());
	is.close();
	return vmd;
}

mmd::VMD* mmd::VMDReader::LoadFromStream(std::istream& is)
{
	VMD* anim = new VMD();

	// file header
	{
		const char kMagic[] = "Vocaloid Motion Data 0002";

		char header[30];
		is.read((char*)(header), 30);
		int ret = strncmp(kMagic, header, strlen(kMagic));
		assert(ret == 0);

		char name[20];
		is.read((char*)(name), 20);

		anim->name_ = std::string(name);

		TRACE("[VMDReader] name = %s\n", anim->name_.c_str());
	}

	// Motion
	{
		int numMotions;
		is.read((char*)(&numMotions), 4);
		TRACE("[VMD] Num motions: %d\n", numMotions);
		assert(numMotions);
		if (numMotions)
		{
			static_assert(sizeof(VMDMotion) == 111);
			anim->motions_.resize(numMotions);
			is.read((char*)(&(anim->motions_[0])), sizeof(VMDMotion) * numMotions);

			// Motion data may not be ordererd in frame number. Sort it here.
			std::sort(anim->motions_.begin(), anim->motions_.end(),
				[&](const VMDMotion& m1, const VMDMotion& m2)
				{
					return m1.frame_no < m2.frame_no;
				}
			);

			anim->start_ = FLT_MAX;
			anim->end_ = -FLT_MAX;
			for (auto& m : anim->motions_)
			{
				anim->start_ = std::min(anim->start_, (float)m.frame_no);
				anim->end_ = std::max(anim->end_, (float)m.frame_no);
			}
			TRACE("start=%f\n", anim->start_);
			TRACE("end=%f\n", anim->end_);
		}

#if USE_ANIMCURVE
		struct MotionVector
		{
			MotionVector() 	{}
			std::vector<VMDMotion> mMotVec;
			void PushBack(const VMDMotion& mot) { mMotVec.push_back(mot); }
			void PushBack(VMDMotion& mot) { mMotVec.push_back(mot); }
			auto begin() { return mMotVec.begin(); }
			auto end() { return mMotVec.end(); }
			auto size() { return mMotVec.size(); }
			VMDMotion& operator[](size_t i) { return mMotVec[i]; }
		};

		// MotionVectorマップに溜め込む
		std::map<std::string, MotionVector> motMap;
		for (auto& m : anim->motions_)
		{
			std::string key = m.bone_name;
			auto it = sjis_map.find(m.bone_name);
			if (it != sjis_map.end())
			{
				auto& found = sjis_map[m.bone_name];
				//				TRACE("[%s]→sjis=[%s]ascii=[%s]uni=[%s]\n", m.bone_name, found->sjis_name, found->ascii_name,found->unicode_name);
				key = (char*)found->ascii_name;
			}
			if (motMap.count(key) == 0)
			{
				motMap[key] = MotionVector();
			}
			auto& mv = motMap[key];
			TRACE("%s(%llu) %d\n", key.c_str(), mv.size(), m.frame_no);
			mv.PushBack(m);
		}

		// 溜めたMotionVectorからBezierを考慮してサンプリング
		for(auto& mm : motMap)
		{
			auto& key = mm.first;
			
			auto& mv = motMap[key];
			assert(mv.size());
			// add or find rot curve
			if (anim->QuatCurves_.count(key) == 0)
			{
				// NewObject
				anim->QuatCurves_[key] = AnimCurveQuat(key.c_str(), glm::make_quat(mv[0].rotation));
			}
			if (anim->Vec3Curves_.count(key) == 0)
			{
				// NewObject
				anim->Vec3Curves_[key] = AnimCurveVec3(key.c_str(), glm::make_vec3(mv[0].location));
			}
			auto& rotCurve = anim->QuatCurves_[key];
			auto& posCurve = anim->Vec3Curves_[key];
			for (auto& m : mv)
			{
				rotCurve.Insert((float)m.frame_no, glm::make_quat(m.rotation));
				posCurve.Insert((float)m.frame_no, glm::make_vec3(m.location));
			}
		}
#endif
		TRACE("\n");
	}

	// Morph
	{
		int numMorphs;
		is.read((char*)(&numMorphs), 4);
		TRACE("[VMD] Num morphs: %d\n", numMorphs);
		static_assert(sizeof(VMDMorph) == 23);
		if (numMorphs)
		{
			anim->morphs_.resize(numMorphs);
			is.read((char*)(&(anim->morphs_[0])), sizeof(VMDMorph) * numMorphs);

			// Morph data may not be ordererd in frame number. Sort it here.
			std::sort(anim->morphs_.begin(), anim->morphs_.end(),
				[&](const VMDMorph& m1, const VMDMorph& m2)
				{
					return m1.frame_no < m2.frame_no;
				}
			);
			for (auto i = 0; i < numMorphs; i++)
			{
				auto& morph = anim->morphs_[i];
				auto& morph_name = morph.morph_name;
				if (anim->MorphCurves_.count(morph_name) == 0)
				{
					anim->MorphCurves_[morph_name] = AnimCurveFloat(morph_name, morph.weight);
				}
				auto& mc = anim->MorphCurves_[morph_name];
				//if (morph.frame_no > 0)_CrtDbgBreak();
				mc.Insert((float)morph.frame_no, morph.weight);
			}
#if 0
			for (auto& itr : anim->morphs_)
			{
				auto& mc = anim->MorphCurves_[itr.morph_name];
				TRACE_QUIET("\"%s\":(%zd)", itr.morph_name,mc.keys.size());
				for (auto k : mc.keys)
				{
					TRACE_QUIET("\t%u(%.2f)", (uint32_t)k.first, k.second);
				}
				TRACE_QUIET("\n");
			}
#endif
		}
	}

//	is.close();

	//DumpBone(anim);
//	{
//		auto motCnt = anim->motions_.size();
//		for (auto i = 0u; i < motCnt; i++)
//		{
//			auto& mot = anim->motions_[i];
//			TRACE("[%zu/%zu] bone_name='%s' %u\n", i, motCnt, mot.bone_name, mot.frame_no);
//		}
//	}

	TRACE("[VMD] Load OK\n");

	return anim;
}

#pragma endregion

//namespace mmd
//{

	mmd::MMDScene::MMDScene()	{}
	mmd::MMDScene::~MMDScene()	{}

	//MMDScene::MMDScene() : model_(NULL), anim_(NULL),
	//	static_min(),
	//	static_max(),
	//	static_dim(),
	//	static_center(),
	//	dynamic_min(),
	//	dynamic_max(),
	//	dynamic_dim()
	//{}

	//MMDScene::~MMDScene()  {}

	bool mmd::MMDScene::LoadPMD(const char* pmdmodel, const char* texture_dir)
	{
		PMDReader pmdreader;
		auto model = pmdreader.LoadFromFile(pmdmodel);
		//auto model = pmdreader.LoadPMXFromFile(pmdmodel);
		assert(model);
		SetModel(model);
		model->textures_.clear();
		model->textures_.resize(model->materials_.size());
		for (auto matIdx = 0u; matIdx < model->materials_.size(); matIdx++)
		{
			auto& m = model->materials_[matIdx];
			std::string filename(m.texture_filename);
			TRACE("texture=%s\n", filename.c_str());
			if (filename.length())
			{
				std::replace(filename.begin(), filename.end(), '*', '\0');
				TRACE("texture=%s\n", filename.c_str());
				if (filename.length())
				{
					filename = std::string(texture_dir) + filename;	// filenameを補正、修正
					auto tex2d = Texture2D::CreateFromFile(filename.c_str());
					//assert(tex2d);
					model->textures_[matIdx] = tex2d;
				}
			}
		}
		return true;
	}

	bool mmd::MMDScene::LoadPMDStream(std::istream& is)
	{
		PMDReader pmdreader;
		auto model = pmdreader.LoadFromStream(is);
		assert(model);
		SetModel(model);
		return model;
	}

	bool mmd::MMDScene::LoadVMD(const char* vmdmodel)
	{
		VMDReader vmdreader;
		auto anim = vmdreader.LoadFromFile(vmdmodel);
		assert(anim);
		AttachAnimation(anim);
		return false;
	}

	bool mmd::MMDScene::LoadVMDStream(std::istream& is)
	{
		VMDReader vmdreader;
		auto anim = vmdreader.LoadFromStream(is);
		assert(anim);
		AttachAnimation(anim);
		return false;
	}

	float* mmd::MMDScene::Prepare()
	{
		assert(model_);
		morphBuffer.resize(model_->GetVerticesCount());
		deformBuffer.resize(model_->GetVerticesCount());
		CalcBbox();
		return renderVertices;
	}

	void mmd::MMDScene::SetBoneMatrix(int idx, Bone& bone, float frame)
	{
#if USE_ANIMCURVE
		// Bezierパラメータを使わず、AnimCurve<>で線形補間にする
		bone.matrixTemp = mat4(1.0f);
		quat motionRot(1.0f, 0.0f, 0.0f, 0.0f);
		vec3 motionPos(0.0f);
		
		if (anim_->QuatCurves_.count(bone.ascii_name) > 0)
		{
			motionRot = anim_->QuatCurves_[bone.ascii_name].Evaluate(frame);
		}
		if (anim_->Vec3Curves_.count(bone.ascii_name) > 0)
		{
			motionPos = anim_->Vec3Curves_[bone.ascii_name].Evaluate(frame);
		}

		bone.rotation = motionRot;
		bone.matrixTemp = glm::toMat4(motionRot);

		if (bone.parentIndex == 0xffffu)
		{
			bone.matrixTemp[3] = vec4(bone.pos.xyz + motionPos, 1.0f);
		}
		else
		{
			const Bone& parent = model_->bones_[bone.parentIndex];

			bone.matrixTemp[3] = vec4(bone.pos.xyz - parent.pos.xyz + motionPos, 1.0f);
		}
#else
		bone.matrixTemp = mat4(1.0f);

		if (bone.motions.empty())
		{
			bone.rotation = quat(1.0f, 0.0f, 0.0f, 0.0f);

			if (bone.parentIndex == 0xffffu)
			{
				bone.matrixTemp[3] = vec4(bone.pos.xyz, 1.0f);
			}
			else
			{
				const Bone& parent = model_->bones_[bone.parentIndex];

				bone.matrixTemp[3] = vec4(bone.pos.xyz - parent.pos.xyz, 1.0f);
			}
		}
		else
		{
			quat motionRot;
			vec3 motionPos;
			InterpolateMotion(motionRot, motionPos, bone.motions, frame);

			bone.rotation = motionRot;
			bone.matrixTemp = glm::toMat4(bone.rotation);

			if (bone.parentIndex == 0xffffu)
			{
				bone.matrixTemp[3] = vec4(bone.pos.xyz + motionPos, 1.0f);
			}
			else
			{
				const Bone& parent = model_->bones_[bone.parentIndex];

				bone.matrixTemp[3] = vec4((bone.pos.xyz - parent.pos.xyz) + motionPos, 1.0f);
			}
		}
#endif
	};

	void mmd::MMDScene::Update(float delta)
	{
		if(!model_||!anim_)return;
		m_currentFrame += delta * m_FPS;
		printf("\r%f", m_currentFrame);
		if (anim_ && m_currentFrame > anim_->end_)
		{
			m_currentFrame = anim_->start_;
		}
		if(deformBuffer.size() != morphBuffer.size())return;
		if(deformBuffer.size() != model_->vertices_.size())return;
		
		//morph
		for (auto& morphCurve : anim_->MorphCurves_)
		{
			morphCurve.second.Evaluate(m_currentFrame);
		}
		UpdateMorph();

		// skeletal
		for (int i = 0; i < model_->bones_.size(); i++)
		{
			Bone& b = model_->bones_[i];
			if (b.parentIndex != 0xFFFF)
			{
				assert(b.parentIndex < i);
			}

			SetBoneMatrix(i, b, m_currentFrame);
			b.updated = false;
		}

		//UpdateIK
#if 1
		{
			for (int i = 0; i < model_->iks_.size(); i++)
			{
				IK& ik = model_->iks_[i];
				IKSolve(&ik, 0.01f);
			}
		}
#endif

		// Clear update flag
		for (int i = 0; i < model_->bones_.size(); i++)
		{
			model_->bones_[i].updated = false;
		}

		for (int i = 0; i < model_->bones_.size(); i++)
		{
			Bone& b = model_->bones_[i];
			UpdateBoneMatrix(b);
		}

		VertexTransform();
	}

	void mmd::MMDScene::UpdateMorph()
	{
		assert(deformBuffer.size() == morphBuffer.size());
		for (int i = 0; i < model_->vertices_.size(); i++)
		{
			morphBuffer[i] = glm::make_vec3(model_->vertices_[i].pos);
		}
		int no = 0;
		for (auto& morphCurve : anim_->MorphCurves_)
		{
			auto weight = morphCurve.second.GetCurrentValue();

			{
					if (GetAsyncKeyState(VK_RSHIFT) < 0)
					{
						weight = 1.0f;
						 
						morphCurve.second.SetCurrentValue(weight);
						TRACE("%s %f\n", morphCurve.first.c_str(), weight);
					}
			}
			if (weight != 0.0f)
			{
				for (auto& m : model_->morphs_)
				{
					if (morphCurve.first == m.name)
					{
						for (const auto& mv : m.vertices)
						{
							morphBuffer[mv.vertex_index] += glm::make_vec3(mv.pos) * weight;
						}
					}
				}
			}
		}
	}

	uint32_t mmd::MMDScene::GetDeformMatrixCount() const
	{
		if (!model_)
		{
			ASSERT(0);
			return 0;
		}
		return (uint32_t)model_->bones_.size();
	}

	void mmd::MMDScene::GetDeformMatrix(mat4* dst, size_t dstCount) const
	{
		if (!model_||!dst||dstCount< GetDeformMatrixCount())
		{
			ASSERT(0);
			return;
		}
		for (auto i = 0u; i < dstCount; i++)
		{
			dst[i] = model_->bones_[i].matrix * model_->bones_[i].bindPose;
		}
	}

	mmd::MMDScene::MotionSegment mmd::MMDScene::FindMotionSegment(float frame, std::vector<Motion>& motions)
	{
		MotionSegment ms;
		ms.m0 = 0;
		ms.m1 = (int)motions.size() - 1;

		if ((int)frame >= motions[ms.m1].frameNo)
		{
			ms.m0 = ms.m1;
			ms.m1 = -1;
			return ms;
		}

		// バイナリサーチ
		while (true)
		{
			int middle = (ms.m0 + ms.m1) / 2;

			if (middle == ms.m0)
			{
				return ms;
			}

			if (motions[middle].frameNo == (int)frame)
			{
				ms.m0 = middle;
				ms.m1 = -1;
				return ms;
			}
			else if (motions[middle].frameNo > (int)frame)
			{
				ms.m1 = middle;
			}
			else
			{
				ms.m0 = middle;
			}
		}
	}

	void mmd::MMDScene::InterpolateMotion(quat& rotation, vec3& position, std::vector<Motion>& motions, float frame)
	{
		MotionSegment ms = FindMotionSegment(frame, motions);
		if (ms.m1 == -1)
		{
			position = motions[ms.m0].pos;
			rotation = motions[ms.m0].rotation;
		}
		else
		{
			float diff = (float)(motions[ms.m1].frameNo - motions[ms.m0].frameNo);
			float a0 = frame - motions[ms.m0].frameNo;
			float ratio = a0 / diff; // [0, 1]
#if USE_BEZIER_INTERP
			// Use interpolation parameter
			float tx = BezierEval(motions[ms.m0].interpX, ratio);
			float ty = BezierEval(motions[ms.m0].interpY, ratio);
			float tz = BezierEval(motions[ms.m0].interpZ, ratio);
			float tr = BezierEval(motions[ms.m0].interpR, ratio);
			position.x = glm::mix(motions[ms.m0].pos.x, motions[ms.m1].pos.x, tx);
			position.y = glm::mix(motions[ms.m0].pos.y, motions[ms.m1].pos.y, ty);
			position.z = glm::mix(motions[ms.m0].pos.z, motions[ms.m1].pos.z, tz);
			rotation = glm::slerp(motions[ms.m0].rotation, motions[ms.m1].rotation, tr);
#else //NOT USE_BEZIER…ならばAnimCurveを使えば良い
			position = glm::mix(motions[ms.m0].pos, motions[ms.m1].pos, ratio);
			rotation = glm::slerp(motions[ms.m0].rotation, motions[ms.m1].rotation, ratio);
#endif
		}
	}

	float mmd::MMDScene::GetFrameTime() const
	{
		return (float)m_currentFrame;
	}

	void mmd::MMDScene::VertexTransform()
	{
//return;
		for (auto i = 0u; i < model_->vertices_.size(); i++)
		{
			PMDVertex& pv = model_->vertices_[i];
			auto b0(pv.bone[0]);
			auto b1(pv.bone[1]);

			vec3 p0 = morphBuffer[i];
			vec3 p1 = morphBuffer[i];

			mat4& m0 = model_->bones_[b0].matrix;
			mat4& m1 = model_->bones_[b1].matrix;

#if 0
			// ボーン行列は絶対座標で定義されています。
			// （モデルの）相対座標にある頂点を、そのボーン行列に渡します。
			//p0 -= model_->bones_[b0].pos.xyz();
			//p1 -= model_->bones_[b1].pos.xyz();
			vec3 v0(glm::vec3(m0 * glm::vec4(p0 - model_->bones_[b0].pos.xyz(), 1.0f)));
			vec3 v1(glm::vec3(m1 * glm::vec4(p1 - model_->bones_[b1].pos.xyz(), 1.0f)));
#else
			mat4& bind0 = model_->bones_[b0].bindPose;
			mat4& bind1 = model_->bones_[b1].bindPose;
			vec3 v0(glm::vec3(m0 * bind0 * glm::vec4(morphBuffer[i], 1.0f)));
			vec3 v1(glm::vec3(m1 * bind1 * glm::vec4(morphBuffer[i], 1.0f)));
#endif

			float w(pv.weight / 100.0f);

			vec3 v = w * v0 + (1.0f - w) * v1;

			deformBuffer[i] = v;
		}

#if 0
		// calculate dynamic scene bbox
		dynamic_min.x = FLT_MAX;
		dynamic_min.y = FLT_MAX;
		dynamic_min.z = FLT_MAX;
		dynamic_max.x = -FLT_MAX;
		dynamic_max.y = -FLT_MAX;
		dynamic_max.z = -FLT_MAX;
		for (int j = 0; j < model_->vertices_.size(); j++)
		{
			const auto& p = deformBuffer[j];
			dynamic_min.x = std::min(dynamic_min.x, p.x);
			dynamic_min.y = std::min(dynamic_min.y, p.y);
			dynamic_min.z = std::min(dynamic_min.z, p.z);
			dynamic_max.x = std::max(dynamic_max.x, p.x);
			dynamic_max.y = std::max(dynamic_max.y, p.y);
			dynamic_max.z = std::max(dynamic_max.z, p.z);
		}
		dynamic_dim = dynamic_max - dynamic_min;
#endif
	}

	uint32_t mmd::MMDScene::GetMaterialCount() const
	{
		return (uint32_t)model_->materials_.size();
	}

	mmd::PMDMaterial& mmd::MMDScene::GetMaterial(uint32_t materialIdx) const
	{
		return model_->materials_[materialIdx];
	}

	Texture2D* mmd::MMDScene::GetTexture(uint32_t materialIdx) const
	{
		if (materialIdx < model_->textures_.size())
		{
			return model_->textures_[materialIdx];
		}
		return nullptr;
	}

	void mmd::MMDScene::SetTexture(uint32_t materialIdx, Texture2D* tex)
	{
		if (materialIdx < GetMaterialCount())
		{
			if (materialIdx + 1 > model_->textures_.size())
			{
				model_->textures_.resize(materialIdx + 1);
			}
			model_->textures_[materialIdx] = tex;
		}
	}

	void mmd::MMDScene::DrawMesh(uint32_t materialIdx, std::vector<VertexPNT>* vert)
	{
		//glDisable(GL_LIGHTING);
		//glDisable(GL_DEPTH_TEST);
		//glCullFace(GL_BACK);
		//glEnable(GL_CULL_FACE);

		if (materialIdx >= GetMaterialCount())
		{
			ASSERT(0);
			return;
		}
		uint32_t vertexStart = 0u;
		for (auto m = 0u; m < GetMaterialCount(); m++)
		{
			const auto& mat = GetMaterial(m);
			if (m == materialIdx)
			{
				vert->clear();
				vert->reserve(mat.vertex_count);
				auto face_count = mat.vertex_count / 3;
				for (auto i = 0u; i < face_count; i++)
				{
					auto idx0 = model_->indices_[vertexStart + 3 * i + 0];
					auto idx1 = model_->indices_[vertexStart + 3 * i + 2];
					auto idx2 = model_->indices_[vertexStart + 3 * i + 1];
					auto& p0 = deformBuffer[idx0];
					auto& p1 = deformBuffer[idx1];
					auto& p2 = deformBuffer[idx2];
					auto n0 = MyMath::CalcTriangleNormal(p0, p1, p2);
					auto n1 = MyMath::CalcTriangleNormal(p1, p2, p0);
					auto n2 = MyMath::CalcTriangleNormal(p2, p0, p1);
					auto& uv0 = model_->vertices_[idx0].uv;
					auto& uv1 = model_->vertices_[idx1].uv;
					auto& uv2 = model_->vertices_[idx2].uv;
					VertexPNT v0(p0.x, p0.y, -p0.z, n0.x, n0.y, n0.z, uv0[0], uv0[1]);
					VertexPNT v1(p1.x, p1.y, -p1.z, n1.x, n1.y, n1.z, uv1[0], uv1[1]);
					VertexPNT v2(p2.x, p2.y, -p2.z, n2.x, n2.y, n2.z, uv2[0], uv2[1]);
					vert->push_back(v0);
					vert->push_back(v1);
					vert->push_back(v2);
				}
				return;
			}
			vertexStart += mat.vertex_count;
		}
	}
	
	inline static void AddWireCube(std::vector<VertexPC>* vert, const float size, const mat4& mat, const vec4& color)
	{
//		vert->clear();

		const float tsize = size * 0.5f;

		// Define the 8 corner points of the cube.
		// The order is important for defining the edges later.
		const vec4 p[8] =
		{
			// Bottom Face (z = -1.0f)
			{ -1.0f, -1.0f, -1.0f,	1.0f }, // p0
			{  1.0f, -1.0f, -1.0f,	1.0f }, // p1
			{  1.0f,  1.0f, -1.0f,	1.0f }, // p2
			{ -1.0f,  1.0f, -1.0f,	1.0f }, // p3

			// Top Face (z = 1.0f)
			{ -1.0f, -1.0f,  1.0f,	1.0f }, // p4
			{  1.0f, -1.0f,  1.0f,	1.0f }, // p5
			{  1.0f,  1.0f,  1.0f,	1.0f }, // p6
			{ -1.0f,  1.0f,  1.0f,	1.0f }  // p7
		};

		// p0 -> p1
		vert->push_back(VertexPC((mat * p[0]).xyz, color));
		vert->push_back(VertexPC((mat * p[1]).xyz, color));
		// p1 -> p2
		vert->push_back(VertexPC((mat * p[1]).xyz, color));
		vert->push_back(VertexPC((mat * p[2]).xyz, color));
		// p2 -> p3
		vert->push_back(VertexPC((mat * p[2]).xyz, color));
		vert->push_back(VertexPC((mat * p[3]).xyz, color));
		// p3 -> p0
		vert->push_back(VertexPC((mat * p[3]).xyz, color));
		vert->push_back(VertexPC((mat * p[0]).xyz, color));
		// p4 -> p5
		vert->push_back(VertexPC((mat * p[4]).xyz, color));
		vert->push_back(VertexPC((mat * p[5]).xyz, color));
		// p5 -> p6
		vert->push_back(VertexPC((mat * p[5]).xyz, color));
		vert->push_back(VertexPC((mat * p[6]).xyz, color));
		// p6 -> p7
		vert->push_back(VertexPC((mat * p[6]).xyz, color));
		vert->push_back(VertexPC((mat * p[7]).xyz, color));
		// p7 -> p4
		vert->push_back(VertexPC((mat * p[7]).xyz, color));
		vert->push_back(VertexPC((mat * p[4]).xyz, color));
		// p0 -> p4
		vert->push_back(VertexPC((mat * p[0]).xyz, color));
		vert->push_back(VertexPC((mat * p[4]).xyz, color));
		// p1 -> p5
		vert->push_back(VertexPC((mat * p[1]).xyz, color));
		vert->push_back(VertexPC((mat * p[5]).xyz, color));
		// p2 -> p6
		vert->push_back(VertexPC((mat * p[2]).xyz, color));
		vert->push_back(VertexPC((mat * p[6]).xyz, color));
		// p3 -> p7
		vert->push_back(VertexPC((mat * p[3]).xyz, color));
		vert->push_back(VertexPC((mat * p[7]).xyz, color));
	}
	
	void mmd::MMDScene::DrawBone(std::vector<VertexPC>* pc)
	{
		//glDisable(GL_LIGHTING);
		//glDisable(GL_DEPTH_TEST);
		//glLineWidth(3.0);
		vec3 v;
		vec4 c;

		for (int i = 0; i < model_->bones_.size(); i++)
		{
			Bone& b = model_->bones_[i];

			// read from bone matrix elements corresponding with translation
			if (b.parentIndex == 0xFFFF)
			{
				//glBegin(GL_POINTS);
				if (b.isLeg)
				{
					c = vec4(1.0f, 1.0f, 1.0f, 1.0f);//glColor3f(1.0f, 1.0f, 1.0f);
				}
				else
				{
					c = vec4(1.0f, 0.0f, 0.0f, 1.0f);//glColor3f(1.0f, 0.0f, 0.0f);
				}
				v = b.matrix[3].xyz;
				pc->push_back(VertexPC(vec3(v.x, v.y, -v.z), c));//glVertex3f(v.x, v.y, -v.z);
				//glEnd();
			}
			else
			{
				Bone& p = model_->bones_[b.parentIndex];

				//glBegin(GL_LINES);
				c = vec4(0.0f, 0.0f, 1.0f, 1.0f);//glColor3f(0.0f, 0.0f, 1.0f);
				v = p.matrix[3].xyz;
				pc->push_back(VertexPC(vec3(v.x, v.y, -v.z), c));//glVertex3f(v.x, v.y, -v.z);
				if (b.isLeg)
				{
					c = vec4(1.0f, 1.0f, 1.0f, 1.0f);//glColor3f(1.0f, 1.0f, 1.0f);
				}
				else
				{
					c = vec4(0.0f, 1.0f, 0.0f, 1.0f);//glColor3f(0.0f, 1.0f, 0.0f);
				}
				v = b.matrix[3].xyz;
				pc->push_back(VertexPC(vec3(v.x, v.y, -v.z), c));//glVertex3f(v.x, v.y, -v.z);
				//glEnd();
			}
		}

		//glLineWidth(1.0);
		//glEnable(GL_DEPTH_TEST);
		//glEnable(GL_LIGHTING);
	}

	void mmd::MMDScene::DrawBoneBbox(std::vector<VertexPC>* pc)
	{
		//glDisable(GL_LIGHTING);
		//glDisable(GL_DEPTH_TEST);
		const vec4 collist[7] =
		{
			{1.0f, 0.0f, 0.0f, 0.25f},
			{0.0f, 1.0f, 0.0f, 0.25f},
			{0.0f, 0.0f, 1.0f, 0.25f},
			{1.0f, 1.0f, 0.0f, 0.25f},
			{1.0f, 0.0f, 1.0f, 0.25f},
			{0.0f, 1.0f, 1.0f, 0.25f},
			{1.0f, 1.0f, 1.0f, 0.25f},
		};
		pc->clear();

		for (int i = 0; i < model_->bones_.size(); i++)
		{
			Bone& b = model_->bones_[i];
			mat4 M;
			M = glm::scale(mat4(1.0f), vec3(1.0f, 1.0f, -1.0f));	//	glMultMatrixf(glm::value_ptr(M));
			M *= b.matrix;											//	glMultMatrixf(glm::value_ptr(M));
			M *= glm::translate(b.min);								//	glMultMatrixf(glm::value_ptr(M));
			M *= glm::scale(b.dim);									//	glMultMatrixf(glm::value_ptr(M));
			M *= glm::translate(vec3(0.5f));						//	glMultMatrixf(glm::value_ptr(M));
			AddWireCube(pc, 1.0f, M, collist[i%7]);
			//			glColor3f(collist[cidx][0], collist[cidx][1], collist[cidx][2]);
			/*glutWireCube(1);*/
		}

		//glEnable(GL_DEPTH_TEST);
		//glEnable(GL_LIGHTING);
	}
	void mmd::MMDScene::IdentifyChainBones(std::string seed_name, std::set<std::string>* exception_list)
	{
		Bone* seed_bone = NULL;
		for (int i = 0; i < model_->bones_.size(); i++) {
			if (model_->bones_[i].ascii_name == seed_name) {
				seed_bone = &model_->bones_[i];
				break;
			}
		}
		std::set<Bone*> flood_fill_bones;
		flood_fill_bones.insert(seed_bone);
		int n = 0;
		bool change = true;
		while (change) {
			change = false;
			//    TRACE("model->bones_.size()=%zu\n", model->bones_.size());
			for (int j = 0; j < model_->bones_.size(); j++)
			{
				//        TRACE("j=%d \n", j);
				if ((model_->bones_[j].ascii_name.empty() ||
					(exception_list && exception_list->find(model_->bones_[j].ascii_name) != exception_list->end())) &&
					flood_fill_bones.find(&model_->bones_[model_->bones_[j].parentIndex]) != flood_fill_bones.end())
				{
					std::stringstream ss;
					ss << seed_name << n;
					model_->bones_[j].ascii_name = ss.str();
					flood_fill_bones.insert(&model_->bones_[j]);
					change = true;
					n++;
				}
			}
		}
		flood_fill_bones.erase(seed_bone);
		for (int k = 0; k < model_->bones_.size(); k++)
		{
			if (model_->bones_[k].isChain || model_->bones_[k].isPinnedChain)
			{
				continue;
			}

			// heuristic to identify chain bones:
			// if current bone is connected to seed bone directly/indirectly, current bone is a chain bone
			model_->bones_[k].isChain = (flood_fill_bones.find(&model_->bones_[k]) != flood_fill_bones.end());

			// heuristic to identify pinned chain bones:
			// if parent bone's tail index is illegal, parent bone must be a diverging bone
			// if parent bone is a diverging bone, current bone must be excluded from physics simulation
			// if parent bone is seed bone, current bone must be excluded from physics simulation
			model_->bones_[k].isPinnedChain =
				(model_->bones_[k].isChain && (!model_->bones_[model_->bones_[k].parentIndex].tailIndex ||
					&model_->bones_[model_->bones_[k].parentIndex] == seed_bone));

		}
	}

	void mmd::MMDScene::CalcBbox()
	{
		//note: 正直シーンのボックスは不要だが、ボーンのボックスはここで求められてる
		// calculate bone bbox
		for (int i = 0; i < model_->bones_.size(); i++)
		{
			auto& b = model_->bones_[i];
			b.min = vec3(FLT_MAX);
			b.max = vec3(-FLT_MAX);
		}
		for (int j = 0; j < model_->vertices_.size(); j++)
		{
			PMDVertex& pv = model_->vertices_[j];
			vec3 p0 = glm::make_vec3(pv.pos);
			unsigned short b0 = pv.bone[0];
			model_->bones_[b0].min = glm::min(model_->bones_[b0].min, p0);
			model_->bones_[b0].max = glm::max(model_->bones_[b0].max, p0);
		}

		for (int k = 0; k < model_->bones_.size(); k++)
		{
			model_->bones_[k].dim = model_->bones_[k].max - model_->bones_[k].min;

			vec3 axis = model_->bones_[k].pos.xyz();

			model_->bones_[k].max -= axis;
			model_->bones_[k].min -= axis;
		}
	}
	