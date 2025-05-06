#pragma once
#include "framework.h"
#include "Vertex.h"
//#include "VAO.h"
//#include "Shader.h"
#include "Texture2D.h"
//#include "Graph.h"

#define MAKE_FONT_ATLAS	0
class BinpackingSolver;

class Font
{
public:
	struct Glyph
	{
		Glyph() {}
		Glyph(const Glyph& src) : rect_(src.rect_), bearing_(src.bearing_), advance_(src.advance_) {}
		Glyph(const Rect& r, const vec2& b, const float a = 0.0f) : rect_(r), bearing_(b), advance_(a) {}
		Glyph(const Rect& r) : rect_(r), bearing_(0.0f), advance_(0.0f) {}
		Rect rect_ = { 0,0,0,0 };
		vec2 bearing_ = { 0.0f, 0.0f };
		float advance_ = 0.0f;
		std::string ToString()
		{
			std::string result;
			std::stringstream ss;
			ss << "{rect=" << rect_.ToString() << ",bearing={" << bearing_.x << "," << bearing_.y << "},advance=" << advance_ << "}";
			ss >> result;
			return result;
		}
	};
	uint32_t used_ = 0;
	FT_Library library_ = nullptr;
	FT_Face face_ = nullptr;
	int size_ = 0;
	std::string ttcpath_;
	std::unordered_map<wchar_t, Glyph> glyphs_;
	Texture2D atlas_;
	static const int atlasMaxWidth = 4096;
	static const int atlasMaxHeight = 4096;
	BinpackingSolver* solver_ = nullptr;

	Font();
	~Font();
#if MAKE_FONT_ATLAS
	bool Init(const std::string& ttcpath, uint32_t w, uint32_t h);
#else
	bool Init(const std::string& atlas, const std::string& fontmap, int size);
#endif
	void Term();
	void SetColor3b(uint8_t r, uint8_t g, uint8_t b);
	void SetColor3f(float r, float g, float b);
	Glyph GetCh(const wchar_t ch);

	inline void BindTexture()
	{
		atlas_.BindTexture();
	}
	void RenderText(std::vector<VertexPCT>* verts, float x, float y, float scale, const wchar_t* format, ...);
	bool WriteCache();
private:
	uint32_t rgb_ = 0x00ffffff;//0xAARRGGBB
	inline uint32_t Buffer2BGRA(const uint8_t& a) const
	{
		return (a << 24) | (a << 0);// | (a << 8) << (a);
//		return rgb_ | (static_cast<uint32_t>(a) << 24);
	}
};
