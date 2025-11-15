#pragma once
#include "framework.h"
#include "MyMath.h"
#include "MyUtil.h"
#include "Vertex.h"

struct Geometry
{
	struct OBB
	{
		vec3 center_;
		mat3 rotate_;
		vec3 extent_;
	};
	using VertexVector = std::vector<VertexP>;
	using VertexPNCTVector = std::vector<VertexPNCT>;
	using IndexVector = std::vector<uint16_t>;

	static bool RecalcVertexNormal(VertexPNCTVector* vertices, IndexVector* indices);
	static void GenerateCheckerPlaneZX(VertexPNCTVector* vertices, IndexVector* indices, const vec3& width, const vec3& grid, const vec4& color0, const vec4& color1);
	static void ReverseWinding(VertexPNCTVector* vertices, IndexVector* indices);
	static void InvertNormals(VertexPNCTVector* vertices);
	static void DecompressArray(VertexPNCTVector* vertices, IndexVector* indices);

	static void GenarateBox(VertexPNCTVector* vertices, IndexVector* indices, const vec3& size);
	static void GenarateSphere(VertexPNCTVector* vertices, IndexVector* indices, float diameter, size_t tessellation);
	
	static void GenerateCapSub(VertexPNCTVector* vertices, IndexVector* indices, size_t tessellation, float height, float radius, bool isTop);
	static void GenarateCylinder(VertexPNCTVector* vertices, IndexVector* indices, float height, float diameter, size_t tessellation);
	static void GenarateCone(VertexPNCTVector* vertices, IndexVector* indices, float diameter, float height, size_t tessellation);
	static void GenarateTorus(VertexPNCTVector* vertices, IndexVector* indices, float diameter, float thickness, size_t tessellation);
	static void GenerateStaticTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size);
	static void GenerateXtkTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size, size_t tessellation);
	static void GenerateWireTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size);
	static void GenerateSolidTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size);

	static void GenerateDiamondSquare(VertexPNCTVector* vertices, IndexVector* indices, float lengthX, float lengthY, int32_t tileX, int32_t tileY, float heightScale, uint32_t seed = 0);
	static void GenerateTextQuads(VertexPNCTVector* vertices, const vec3& pos, const vec3& dx, const vec3& dy, const std::vector <Rect>& texcoords, int32_t atlasWidth, int32_t atlasHeight);
	static mat4 CalcOBB(VertexPNCTVector* vertices);
};

