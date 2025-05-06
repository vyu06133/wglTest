#include "pch.h"
#include "font.h"
#include "MyUtil.h"
#include "MyMath.h"
#include "binpacker.h"
#include "Vertex.h"

Font::Font()
{
}

Font::~Font()
{
	Term();
}



void Font::Term()
{
	glyphs_.clear();
	atlas_.DeleteTexture();
	if (solver_)
	{
		delete solver_;
		solver_ = nullptr;
	}
	if (face_)
	{
		FT_Done_Face(face_);
		face_ = nullptr;
	}
	if (library_)
	{
		FT_Done_FreeType(library_);
		library_ = nullptr;
	}
}

#if MAKE_FONT_ATLAS
// on the fly
bool Font::Init(const std::string& ttcpath, uint32_t w, uint32_t h)
{
	if (FT_Init_FreeType(&library_))
	{
		TRACE("FT_Init_FreeType failed\n");
		return false;
	}
	if (!library_)
	{
		TRACE("FT_Library nullptr\n");
		return false;
	}

	atlas_.TexImage(GL_RED, atlasMaxWidth, atlasMaxHeight, GL_RED, nullptr);

	solver_ = _NEW BinpackingSolver(&atlas_);
	if (!solver_)
	{
		TRACE("new BinpackingSolver failed\n");
		FT_Done_FreeType(library_);
		library_ = nullptr;
		return false;
	}

	atlas_.SetParameter(GL_CLAMP_TO_EDGE, GL_NEAREST);
	atlas_.ApplyParameter();
	FT_Error err = FT_Err_Ok;
	if (!library_)
	{
		TRACE("FT_Library nullptr\n");
		return false;
	}
	err = FT_New_Face(library_, ttcpath.c_str(), 0, &face_);
	if (err != FT_Err_Ok)
	{
		TRACE("FT_New_Face failed\n");
		return false;
	}
	if (!face_)
	{
		TRACE("FT_Face nullptr\n");
		return false;
	}
	ttcpath_ = ttcpath;
	size_ = h;
	err = FT_Set_Pixel_Sizes(face_, w, h);
	if (err != FT_Err_Ok)
	{
		TRACE("FT_Set_Pixel_Sizes failed\n");
		return false;
	}

	for (wchar_t i = 1; i <= 0x7f; i++)
	{
		GetCh(i);
	}
	return true;
}
#else
//
bool Font::Init(const std::string& atlas, const std::string& fontmap, int size)
{
	size_ = size;
	atlas_.TexImage(GL_RED, atlasMaxWidth, atlasMaxHeight, GL_RED, nullptr);
	atlas_.LoadFile(atlas.c_str());
	atlas_.SetParameter(GL_CLAMP_TO_EDGE, GL_NEAREST);
	atlas_.ApplyParameter();
	glyphs_ = MyUtil::DeserializeMap<wchar_t, Glyph>(fontmap);
	return true;
}
#endif

Font::Glyph Font::GetCh(const wchar_t ch)
{
	if (!library_)
	{
		if (glyphs_.count(ch) != 0)
		{
			return glyphs_[ch];
		}
		return Glyph();
	}
	ASSERT(library_);
	ASSERT(face_);
	FT_UInt idx = FT_Get_Char_Index(face_, ch);
	if (idx == 0)
	{
		TRACE("グリフ読み込み失敗　’%C'(%x)\n",ch,ch);
	}
	auto fterr = FT_Load_Glyph(face_, idx, FT_LOAD_DEFAULT);
	if (fterr != FT_Err_Ok)
	{
		TRACE("グリフ読み込み失敗%x\n",ch);
	}
	auto& glyph = face_->glyph;
	fterr = FT_Render_Glyph(glyph, FT_RENDER_MODE_NORMAL);
	const FT_Bitmap& bitmap = glyph->bitmap;
	if (idx && bitmap.width > 1 && bitmap.rows > 1 && bitmap.buffer)
	{
		auto modbmp = glyph->bitmap;
		modbmp.rows++;//test:マージン
		auto rect = solver_->insert(&modbmp, ch);
		atlas_.TexSubImage(rect, GL_RED, bitmap.buffer);

		Glyph value(rect, { glyph->bitmap_left, glyph->bitmap_top });
		value.advance_ = (float)(glyph->advance.x >> 6);
		glyphs_[ch] = value;
		//printf("'%C':%s(%s)\n", ch, rect.ToString().c_str(),glm::to_string(value.bearing_).c_str());
		used_++;
		TRACE("used_ = %d\n", used_);
		return value;
	}
	else
	{
		return Glyph();
	}
}


void Font::RenderText(std::vector<VertexPCT>* verts, float x, float y, float scale, const wchar_t* format, ...)
{
	va_list arg;
	va_start(arg, format);
	static wchar_t text[4096] = {};
	vswprintf_s(text, _countof(text), format, arg);
	va_end(arg);
	auto len = lstrlenW(text);
	for (auto i = 0; i < len; i++)
	{
		auto ind = GetCh(text[i]);
//		TRACE("'%C':%s\n", text[i], ind.ToString().c_str());
		auto bearing = ind.bearing_;
		auto rect = ind.rect_;
		std::swap(rect.top, rect.bottom);

		float w = (float)rect.Width() * scale;
		float h = (float)rect.Height() * scale;

		auto inv = atlas_.InvSize();

		float xPos = x + bearing.x * scale;
		float yPos = y - bearing.y * scale + (float)size_;
		float u0 = rect.left * inv.x;
		float u1 = rect.right * inv.x;
		float v0 = (rect.top+0.0f) * inv.y;
		float v1 = (rect.bottom+0.0f) * inv.y;
		x += ind.advance_ * scale;
		verts->push_back(VertexPCT(vec3(xPos,      yPos + h, 0.0f), vec4(u0,0,1,v0),	u0, v0));
		verts->push_back(VertexPCT(vec3(xPos,      yPos,     0.0f), vec4(u0,0,1,v1),	u0, v1));
		verts->push_back(VertexPCT(vec3(xPos + w,  yPos,     0.0f), vec4(u1,0,1,v1),	u1, v1));
		verts->push_back(VertexPCT(vec3(xPos,      yPos + h, 0.0f), vec4(u0,1,0,v0),	u0, v0));
		verts->push_back(VertexPCT(vec3(xPos + w,  yPos,     0.0f), vec4(u1,1,0,v1),	u1, v1));
		verts->push_back(VertexPCT(vec3(xPos + w,  yPos + h, 0.0f), vec4(u1,1,0,v0),	u1, v0));
		
		//verts->push_back(VertexPCT(xpos, ypos-h + h,		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, rect.left * invw, rect.top * invh));
		//verts->push_back(VertexPCT(xpos, ypos-h,			0.0f, 0.0f, 0.0f, 1.0f, 1.0f, rect.left * invw, rect.bottom * invh));
		//verts->push_back(VertexPCT(xpos + w, ypos-h,		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, rect.right * invw, rect.bottom * invh));

		//verts->push_back(VertexPCT(xpos, ypos-h + h,		0.0f, 1.0f, 0.0f, 1.0f, 1.0f, rect.left * invw, rect.top * invh));
		//verts->push_back(VertexPCT(xpos + w, ypos-h,		0.0f, 0.0f, 1.0f, 0.0f, 1.0f, rect.right * invw, rect.bottom * invh));
		//verts->push_back(VertexPCT(xpos + w, ypos-h + h,	0.0f, 0.0f, 1.0f, 1.0f, 1.0f, rect.right * invw, rect.top * invh));
		const vec4 col(1, 0, 0, 1);
		//verts->push_back(VertexPCT(vec3(xpos, ypos, 0.0f),			vec4(0,0,0,1), vec2(rect.left * invw, rect.top * invh)));
		//verts->push_back(VertexPCT(vec3(xpos, ypos - h, 0.0f),		vec4(0,0,1,1), vec2(rect.left * invw, rect.bottom * invh)));
		//verts->push_back(VertexPCT(vec3(xpos + w, ypos - h, 0.0f),	vec4(1,0,0,1), vec2(rect.right * invw, rect.bottom * invh)));
		//verts->push_back(VertexPCT(vec3(xpos, ypos, 0.0f),			vec4(1,0,1,1), vec2(rect.left * invw, rect.top * invh)));
		//verts->push_back(VertexPCT(vec3(xpos + w, ypos - h, 0.0f),	vec4(0,1,0,1), vec2(rect.right * invw, rect.bottom * invh)));
		//verts->push_back(VertexPCT(vec3(xpos + w, ypos, 0.0f),		vec4(0,1,1,1), vec2(rect.right * invw, rect.top * invh)));
	}
}
#pragma region CACHE
// FONT_ATLAS作成の途中経過出力
TiXmlElement* TraverseBinpackingNode(BinpackingSolver::Node* node)
{
	if (!node)
	{
		return nullptr;
	}
	auto e = _NEW TiXmlElement("BinpackingNode");
	e->SetAttribute("id", node->GetId());
	std::stringstream ss;
	ss << node->GetRect().ToString();
	e->LinkEndChild(_NEW TiXmlText(ss.str().c_str()));
	auto c0 = node->GetChild0();
	if (c0)
	{
		auto c = TraverseBinpackingNode(c0);
		e->LinkEndChild(c);
	}
	auto c1 = node->GetChild1();
	if (c1)
	{
		auto c = TraverseBinpackingNode(c1);
		e->LinkEndChild(c);
	}

	return e;
}

bool Font::WriteCache()
{
	TiXmlDocument doc;

	// XML宣言を追加
	TiXmlDeclaration* decl = new TiXmlDeclaration("1.0", "UTF-8", "");
	doc.LinkEndChild(decl);

	// ルート要素を作成
	TiXmlElement* root = new TiXmlElement("root");
	doc.LinkEndChild(root);

//	<font size=s>name</font>
	TiXmlElement* f = new TiXmlElement("font");
	{
		f->SetAttribute("size", size_);
		f->LinkEndChild(new TiXmlText(MyUtil::PathA(ttcpath_).StripPath().string().c_str()));
		root->LinkEndChild(f);
	}
	if (solver_->root_)
	{
		auto nodes = TraverseBinpackingNode(solver_->root_);
		if (nodes)
		{
			root->LinkEndChild(nodes);
		}
	}

	// ファイルに保存
	if (!doc.SaveFile("output.xml"))
	{
		std::cerr << "ファイル保存失敗: " << doc.ErrorDesc() << std::endl;
		return false;
	}
	MyUtil::SerializeMap<wchar_t, Glyph>(glyphs_, "output.fontmap");
	atlas_.SaveFile("output.png");
	auto g=MyUtil::DeserializeMap<wchar_t, Glyph>("output.fontmap");
	exit(0);
	return true;
}
/*
JIS X 0208の第一水準漢字は、UnicodeのCJK統合漢字（CJK Unified Ideographs）ブロックに主に含まれています。
CJK統合漢字の範囲は「U+4E00 から U+9FFC」です。
*/
#pragma endregion

void Font::SetColor3b(uint8_t r, uint8_t g, uint8_t b)
{
	rgb_ = /*(a << 24) |*/ (((uint32_t)b) << 16) | (((uint32_t)g) << 8) | ((uint32_t)r);
}

void Font::SetColor3f(float r, float g, float b)
{
	SetColor3b(static_cast<uint8_t>(r * 255.0f), static_cast<uint8_t>(g * 255.0f), static_cast<uint8_t>(b * 255.0f));
}

