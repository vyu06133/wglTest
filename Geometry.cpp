#include "pch.h"
#include "VAO.h"
#include "MyMath.h"
#include "MyUtil.h"
#include "Geometry.h"

inline vec3 CalcTriangleNormal(const vec3& a, const vec3& b, const vec3& c)
{
	auto n = glm::normalize(glm::cross(glm::normalize(b - a), glm::normalize(c - a)));

	//n.x += MyMath::mt.RandRange(0.0f, 0.3f);
	//n.y += MyMath::mt.RandRange(0.0f, 0.3f);
	//n.z += MyMath::mt.RandRange(0.0f, 0.3f);
	return n;
}

bool Geometry::RecalcVertexNormal(VertexPNCTVector* vertices, IndexVector* indices)
{
	auto size = vertices->size();
	if (indices)
	{
		ASSERT(vertices->size() == indices->size());
		for (auto i = 0; i < size; i += 3)
		{
			auto& v0 = (*vertices)[(*indices)[i + 0]];
			auto& v1 = (*vertices)[(*indices)[i + 1]];
			auto& v2 = (*vertices)[(*indices)[i + 2]];
			v0.normal = CalcTriangleNormal(v0.pos, v1.pos, v2.pos);
		}
	}
	else
	{
		for (auto i = 0; i < size; i += 3)
		{
			auto& v0 = (*vertices)[i + 0];
			auto& v1 = (*vertices)[i + 1];
			auto& v2 = (*vertices)[i + 2];
			v0.normal = CalcTriangleNormal(v0.pos, v1.pos, v2.pos);
		}
	}
	return true;
}

void Geometry::ReverseWinding(VertexPNCTVector* vertices, IndexVector* indices)
{
	if (indices)
	{
		ASSERT(((*indices).size() % 3) == 0);
		for (auto it = (*indices).begin(); it != (*indices).end(); it += 3)
		{
			std::swap(*it, *(it + 2));
		}
	}

	if (vertices)
	{
		for (auto& it : (*vertices))
		{
			it.tex.x = (1.f - it.tex.x);
		}
	}
}

void Geometry::InvertNormals(VertexPNCTVector* vertices)
{
	for (auto& it : (*vertices))
	{
		it.normal.x = -it.normal.x;
		it.normal.y = -it.normal.y;
		it.normal.z = -it.normal.z;
	}
}

void Geometry::DecompressArray(VertexPNCTVector* vertices, IndexVector* indices)
{
	VertexPNCTVector result;
	auto vertsize = vertices->size();
	result.reserve(vertsize);
	size_t size = indices->size();
	uint16_t idx = 0;
	for (auto i = 0; i < size; i++)
	{
		idx = indices->at(i);
		ASSERT(idx < vertsize);
		result.push_back(vertices->at(idx));
	}
	vertices->swap(result);
}

void Geometry::GenerateCheckerPlaneZX(VertexPNCTVector* vertices, IndexVector* indices, const vec3& width, const vec3& grid, const vec4& color0, const vec4& color1)
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
			auto n = CalcTriangleNormal(vec3(x1, 0.0f, z1), vec3(x2, 0.0f, z1), vec3(x1, 0.0f, z2));

			auto v11 = VertexPNCT(vec3(x1, 0.0f, z1), n, color[(i + j) & 1], vec2(u1, v1));
			auto v12 = VertexPNCT(vec3(x2, 0.0f, z1), n, color[(i + j) & 1], vec2(u2, v1));
			auto v21 = VertexPNCT(vec3(x1, 0.0f, z2), n, color[(i + j) & 1], vec2(u1, v2));
			auto v22 = VertexPNCT(vec3(x2, 0.0f, z2), n, color[(i + j) & 1], vec2(u2, v2));

			if (indices)
			{
				uint16_t vbase = static_cast<uint16_t>(vertices->size());
				ASSERT(vbase + 3 < USHRT_MAX);
				indices->push_back(vbase + 0);
				indices->push_back(vbase + 1);
				indices->push_back(vbase + 2);
				indices->push_back(vbase + 0);
				indices->push_back(vbase + 2);
				indices->push_back(vbase + 3);
				vertices->push_back(v11);
				vertices->push_back(v12);
				vertices->push_back(v22);
				vertices->push_back(v21);
			}
			else
			{
				vertices->push_back(v11);
				vertices->push_back(v12);
				vertices->push_back(v22);
				vertices->push_back(v11);
				vertices->push_back(v22);
				vertices->push_back(v21);
			}
		}
	}
}

void Geometry::GenarateBox(VertexPNCTVector* vertices, IndexVector* indices, const vec3& size)
{
	vertices->clear();
	if (indices)
	{
		indices->clear();
	}

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

		if (indices)
		{
			const uint16_t vbase = static_cast<uint16_t>((*vertices).size());
			ASSERT(vbase + 2 < USHRT_MAX);
			(*indices).push_back(vbase + 0);
			(*indices).push_back(vbase + 1);
			(*indices).push_back(vbase + 2);

			(*indices).push_back(vbase + 0);
			(*indices).push_back(vbase + 2);
			(*indices).push_back(vbase + 3);
			(*vertices).push_back(v0);
			(*vertices).push_back(v1);
			(*vertices).push_back(v2);
			(*vertices).push_back(v3);
		}
		else
		{
			(*vertices).push_back(v0);
			(*vertices).push_back(v1);
			(*vertices).push_back(v2);
			(*vertices).push_back(v0);
			(*vertices).push_back(v2);
			(*vertices).push_back(v3);
		}
	}
}

void Geometry::GenarateSphere(VertexPNCTVector* vertices, IndexVector* indices, float diameter, size_t tessellation)
{
	vertices->clear();
	ASSERT(tessellation >= 3);

	const size_t verticalSegments = tessellation;
	const size_t horizontalSegments = tessellation * 2;

	const float radius = diameter * 0.5f;

	// Create rings of vertices at progressively higher latitudes.
	for (size_t i = 0; i <= verticalSegments; i++)
	{
		const float v = 1 - float(i) / float(verticalSegments);

		const float latitude = (float(i) * 180.0f / float(verticalSegments)) - 90.0f;
		float dy = MyMath::Sin(latitude);
		float dxz = MyMath::Cos(latitude);

		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			const float u = float(j) / float(horizontalSegments);

			const float longitude = float(j) * 360.0f / float(horizontalSegments);
			float dx = MyMath::Sin(longitude);
			float dz = MyMath::Cos(longitude);

			dx *= dxz;
			dz *= dxz;

			const vec3 normal(dx, dy, dz);
			const vec2 tex(u, v);
			const vec4 col(dx, dy, dz, 1.0f);

			vertices->push_back(VertexPNCT(normal * radius, normal, col, tex));
		}
	}
	IndexVector ibuf;
	ibuf.reserve(verticalSegments * (verticalSegments+1) * 6);
	const size_t stride = horizontalSegments + 1;

	for (size_t i = 0; i < verticalSegments; i++)
	{
		for (size_t j = 0; j <= horizontalSegments; j++)
		{
			const size_t nextI = i + 1;
			const size_t nextJ = (j + 1) % stride;

			ibuf.push_back(uint16_t(i * stride + j));
			ibuf.push_back(uint16_t(nextI * stride + j));
			ibuf.push_back(uint16_t(i * stride + nextJ));

			ibuf.push_back(uint16_t(i * stride + nextJ));
			ibuf.push_back(uint16_t(nextI * stride + j));
			ibuf.push_back(uint16_t(nextI * stride + nextJ));
		}
	}
	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}

void Geometry::GenerateCapSub(VertexPNCTVector* vertices, IndexVector* indices, size_t tessellation, float height, float radius, bool isTop)
{
	for (size_t i = 0; i < tessellation - 2; i++)
	{
		size_t i1 = (i + 1) % tessellation;
		size_t i2 = (i + 2) % tessellation;

		if (isTop)
		{
			std::swap(i1, i2);
		}

		const size_t vbase = vertices->size();
		indices->push_back(uint16_t(vbase));
		indices->push_back(uint16_t(vbase + i1));
		indices->push_back(uint16_t(vbase + i2));
	}

	// Which end of the cylinder is this?
	vec3 normal(0.0f, 1.0f, 0.0f);
	vec2 texScale(-0.5f, -0.5f);

	if (!isTop)
	{
		normal = -normal;
		texScale.x = 0.5f;
	}

	// Create cap vertices.
	for (size_t i = 0; i < tessellation; i++)
	{
		const vec3 circleVector = MyMath::CircleVector(i, tessellation);

		const vec3 position = (circleVector * radius) + (normal * height);

		const vec2 tex(circleVector.y * texScale.x + 1.0f, circleVector.x * texScale.y + 1.0f);

		const vec4 col(normal.x, normal.y, normal.z, 1.0f);
		vertices->push_back(VertexPNCT(position, normal, col, tex));
	}
}

void Geometry::GenarateCylinder(VertexPNCTVector* vertices, IndexVector* indices, float height, float diameter, size_t tessellation)
{
	IndexVector ibuf;
	vertices->clear();

	ASSERT(tessellation >= 3);

	height *= 0.5f;

	const vec3 topOffset(0.0f, height, 0.0f);

	const float radius = diameter * 0.5f;
	const size_t stride = tessellation + 1;

	for (size_t i = 0; i <= tessellation; i++)
	{
		const vec3 normal = MyMath::CircleVector(i, tessellation);

		const vec3 sideOffset = normal * radius;

		const vec2 tex(float(i) / float(tessellation), 0.0f);

		const vec4 col(normal.x, normal.y, normal.z, 1.0f);

		auto t = VertexPNCT(sideOffset + topOffset, normal, col, tex);
		auto b = VertexPNCT(sideOffset - topOffset, normal, col, tex + vec2(0.0f, 1.0f));
		vertices->push_back(t);
		vertices->push_back(b);

		ibuf.push_back(uint16_t(i * 2));
		ibuf.push_back(uint16_t((i * 2 + 2) % (stride * 2)));
		ibuf.push_back(uint16_t(i * 2 + 1));

		ibuf.push_back(uint16_t(i * 2 + 1));
		ibuf.push_back(uint16_t((i * 2 + 2) % (stride * 2)));
		ibuf.push_back(uint16_t((i * 2 + 3) % (stride * 2)));
	}

	GenerateCapSub(vertices, &ibuf, tessellation, height, radius, true);
	GenerateCapSub(vertices, &ibuf, tessellation, height, radius, false);

	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}

void Geometry::GenarateCone(VertexPNCTVector* vertices, IndexVector* indices, float diameter, float height, size_t tessellation)
{
	IndexVector ibuf;
	vertices->clear();

	ASSERT(tessellation >= 3);

	height *= 0.5f;

	const vec3 topOffset(0.0f, height, 0.0f);

	const float radius = diameter * 0.5f;
	const size_t stride = tessellation + 1;

	// Create a ring of triangles around the outside of the cone.
	for (size_t i = 0; i <= tessellation; i++)
	{
		const vec3 circlevec = MyMath::CircleVector(i, tessellation);

		const vec3 sideOffset = circlevec * radius;

		const vec2 tex(float(i) / float(tessellation), 0.0f);

		const vec4 col(circlevec.x, circlevec.y, circlevec.z, 1.0f);

		const vec3 pt = sideOffset - topOffset;

		vec3 normal = glm::normalize(glm::cross(MyMath::CircleTangent(i, tessellation), (topOffset - pt)));

		// Duplicate the top vertex for distinct normals
		auto c = VertexPNCT(topOffset, normal, col, vec2(0.0f));
		auto s = VertexPNCT(pt, normal, col, tex + vec2(0.0f, 1.0f));
		vertices->push_back(c);
		vertices->push_back(s);

		ibuf.push_back(uint16_t(i * 2));
		ibuf.push_back(uint16_t((i * 2 + 3) % (stride * 2)));
		ibuf.push_back(uint16_t((i * 2 + 1) % (stride * 2)));
	}

	// Create flat triangle fan caps to seal the bottom.
	GenerateCapSub(vertices, &ibuf, tessellation, height, radius, false);

	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}

void Geometry::GenarateTorus(VertexPNCTVector* vertices, IndexVector* indices, float diameter, float thickness, size_t tessellation)
{
	IndexVector ibuf;
	vertices->clear();

	ASSERT(tessellation >= 3);

	const size_t stride = tessellation + 1;

	// First we loop around the main ring of the torus.
	for (size_t i = 0; i <= tessellation; i++)
	{
		const float u = float(i) / float(tessellation);

	//	const float outerAngle = float(i) * XM_2PI / float(tessellation) - XM_PIDIV2;
		const float outerAngle = float(i) * 360.0f / float(tessellation) - 90.0f;

		// Create a transform matrix that will align geometry to
		// slice perpendicularly though the current ring position.
	//	const XMMATRIX transform = XMMatrixTranslation(diameter / 2, 0, 0) * XMMatrixRotationY(outerAngle);
		const mat4 transform = MyMath::CreateRotateYMat4(outerAngle) * MyMath::CreateTranslateMat4(vec3(diameter * 0.5f, 0.0f, 0.0f));

		// Now we loop along the other axis, around the side of the tube.
		for (size_t j = 0; j <= tessellation; j++)
		{
			const float v = 1 - float(j) / float(tessellation);

	//		const float innerAngle = float(j) * XM_2PI / float(tessellation) + XM_PI;
			const float innerAngle = float(j) * 360.0f / float(tessellation) + 180.0f;
	//		XMScalarSinCos(&dy, &dx, innerAngle);
			float dx = MyMath::Cos(innerAngle);
			float dy = MyMath::Sin(innerAngle);

	//		//auto tovec4 = [](const vec3& v3, float w) { return vec4(v3.x, v3.y, v3.z, w); };
	//		//auto tovec3 = [](const vec4& v4) { return vec3(v4.x, v4.y, v4.z); };
			// Create a vertex.
			vec3 normal(dx, dy, 0.0f);
	//		XMVECTOR position = XMVectorScale(normal, thickness / 2);
			vec3 position(normal * thickness * 0.5f);
			const vec2 tex(u, v);

			
			
	//		position = XMVector3Transform(position, transform);
			position = (transform * vec4(position, 1.0f)).xyz;
	//		normal = XMVector3TransformNormal(normal, transform);
			normal = (transform * vec4(normal, 0.0f)).xyz;
			vec4 col = vec4(normal, 1.0f);

			vertices->push_back(VertexPNCT(position, normal, col, tex));

			// And create indices for two triangles.
			const size_t nextI = (i + 1) % stride;
			const size_t nextJ = (j + 1) % stride;

			ibuf.push_back(uint16_t(i * stride + j));
			ibuf.push_back(uint16_t(i * stride + nextJ));
			ibuf.push_back(uint16_t(nextI * stride + j));

			ibuf.push_back(uint16_t(i * stride + nextJ));
			ibuf.push_back(uint16_t(nextI * stride + nextJ));
			ibuf.push_back(uint16_t(nextI * stride + j));
		}
	}

	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}

struct DiamondSquare
{
	Geometry::VertexPNCTVector vbuf_;
	Geometry::IndexVector ibuf_;
	MyMath::MersenneTwister mt_;
	float lengthX_;
	float lengthY_;
	int32_t tileX_;
	int32_t tileY_;
	MyUtil::Aarray2d<float> heightmap_;
	void __stdcall GenerateSub(int32_t x, int32_t y, int32_t size, float tl, float tr, float bl, float br, int32_t maxSize)
	{
//		TRACE("GenerateSub(%d,%d,%d,%f,%f,%f,%f,%d)\n", x, y, size, tl, tr, bl, br, maxSize);
		float center = (tl + tr + bl + br) * 0.25f;
		if (size <= 1)
		{
			if (heightmap_.IsValidIndex(x, y))
				heightmap_.Set(x, y, center);
		}
		else
		{
			center += mt_.RandRangef(-0.5f, 0.5f) * static_cast<float>(size) / static_cast<float>(maxSize);
			center = MyMath::Clamp(center, 0.0f, 1.0f);
			float t = (tl + tr) * 0.5f;
			float b = (bl + br) * 0.5f;
			float l = (tl + bl) * 0.5f;
			float r = (tr + br) * 0.5f;

			size = (size + 1) / 2;

			GenerateSub(x + 0, y + 0, size, tl, t, l, center, maxSize);
			GenerateSub(x + size, y + 0, size, t, tr, center, r, maxSize);
			GenerateSub(x + 0, y + size, size, l, center, bl, b, maxSize);
			GenerateSub(x + size, y + size, size, center, r, b, br, maxSize);
		}
	}

	void Generate(float lengthX, float lengthY, int32_t tileX, int32_t tileY, float heightScale, uint32_t seed = 0)
	{
		lengthX_ = lengthX;
		lengthY_ = lengthY;
		tileX_ = tileX;
		tileY_ = tileY;
		heightmap_.Init(tileX + 1, tileY + 1, 0.0f);
		mt_.Init(seed ? seed : timeGetTime());
		GenerateSub(0, 0, tileX + 1, mt_.randf(), mt_.randf(), mt_.randf(), mt_.randf(), tileX + 1);
		int32_t quadNum = tileX * tileY;
		for (int32_t q = 0; q < quadNum; ++q)
		{
			int32_t x = (q % tileX);
			int32_t y = (q / tileY);

			vec3 scl(lengthX / static_cast<float>(tileX), heightScale, lengthY / static_cast<float>(tileY));

			auto CalcVertex = [&](const int32_t s, const int32_t t)
				{
					float u = static_cast<float>(s) - static_cast<float>(tileX) * 0.5f;
					float v = static_cast<float>(t) - static_cast<float>(tileY) * 0.5f;

					return VertexPNCT(
						vec3(u * scl.x, heightmap_(s, t) * scl.y, v * scl.z),
						vec3(0.0f, 1.0f, 0.0f),
						vec4(1.0f, 1.0f, heightmap_(s, t), 1.0f),
						vec2(static_cast<float>(s) / static_cast<float>(tileX_), static_cast<float>(t) / static_cast<float>(tileY_)));
				};

			uint32_t vbase = static_cast<uint32_t>(vbuf_.size());
			vbuf_.push_back(CalcVertex(x + 0, y + 0));
			vbuf_.push_back(CalcVertex(x + 0, y + 1));
			vbuf_.push_back(CalcVertex(x + 1, y + 0));
			vbuf_.push_back(CalcVertex(x + 1, y + 1));

			ibuf_.push_back(vbase + 0);
			ibuf_.push_back(vbase + 2);
			ibuf_.push_back(vbase + 1);

			ibuf_.push_back(vbase + 1);
			ibuf_.push_back(vbase + 2);
			ibuf_.push_back(vbase + 3);
		}
	}
};

void Geometry::GenerateDiamondSquare(VertexPNCTVector* vertices, IndexVector* indices, float lengthX, float lengthY, int32_t tileX, int32_t tileY, float heightScale, uint32_t seed)
{
	DiamondSquare ds;
	ds.Generate(lengthX, lengthY, tileX, tileY, heightScale, timeGetTime());
	if (indices)
	{
		indices->swap(ds.ibuf_);
	}
	else
	{
		vertices->swap(ds.vbuf_);
		DecompressArray(vertices, &ds.ibuf_);
	}
}

void Geometry::GenerateTextQuads(VertexPNCTVector* vertices, const vec3& pos, const vec3& dx, const vec3& dy, const std::vector <Rect>& texcoords, int32_t atlasWidth, int32_t atlasHeight)
{
	vertices->clear();
	auto count = texcoords.size();
	ASSERT(count);
	vec3 x = glm::normalize(dx);
	vec3 y = glm::normalize(dy);
	vec3 p = pos;
	vec4 c(0,1,1,1);
	vec3 n(0.0f, 0.0f, 1.0f);
	vec2 s(1.0f / (float)atlasWidth, 1.0f / (float)atlasHeight);
	for (auto i = 0; i < count; i++)
	{
		auto& r = texcoords[i];
		auto v0 = VertexPNCT(p, n, c, vec2((float)r.left, (float)r.top)*s);
		auto v1 = VertexPNCT(p+x, n, c, vec2((float)r.left, (float)r.top) * s);
		auto v2 = VertexPNCT(p+x+y, n, c, vec2((float)r.right, (float)r.bottom) * s);
		auto v3 = VertexPNCT(p+y, n, c, vec2((float)r.left, (float)r.bottom) * s);
		vertices->push_back(v0);
		vertices->push_back(v1);
		vertices->push_back(v2);
		vertices->push_back(v3);
	}
}

#if 1	//xtk
#include "xtk_teapot.h"
void Geometry::GenerateXtkTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size, size_t tessellation)
{
	vertices->clear();
	IndexVector ibuf;

	ASSERT(tessellation >= 1);

	XtkTeapot_Generate(vertices, &ibuf, size * 2.0f, tessellation);
	
	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}
#endif

#if 1	// static_mesh
#include "static_teapot_mesh.inc"

void Geometry::GenerateStaticTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size)
{
	vertices->clear();
	IndexVector ibuf;
	auto scale = size * 0.5f;

	auto faces = _countof(teapot_mesh_indices);
	for (auto i = 0; i < faces; i++)
	{
		uint16_t vbase = static_cast<uint16_t>(vertices->size());
		ibuf.push_back(vbase + 0);
		ibuf.push_back(vbase + 1);
		ibuf.push_back(vbase + 2);
		auto& tri = teapot_mesh_indices[i];
		for (auto j = 0; j < 3; j++)
		{
			auto p0 = VertexPNCT(teapot_mesh_positions[tri[0]]*scale, teapot_mesh_normals[tri[1]], vec4(teapot_mesh_normals[tri[1]],1.0f), teapot_mesh_texcords[tri[2]]);
			auto p1 = VertexPNCT(teapot_mesh_positions[tri[3]]*scale, teapot_mesh_normals[tri[4]], vec4(teapot_mesh_normals[tri[1]],1.0f), teapot_mesh_texcords[tri[5]]);
			auto p2 = VertexPNCT(teapot_mesh_positions[tri[6]]*scale, teapot_mesh_normals[tri[7]], vec4(teapot_mesh_normals[tri[1]],1.0f), teapot_mesh_texcords[tri[8]]);
			vertices->push_back(p2);
			vertices->push_back(p1);
			vertices->push_back(p0);
		}
	}
	
	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}
#endif

#if 1	//FREEGLUT

static void fghDrawGeometrySolid11(GLfloat* vertices, GLfloat* normals, GLfloat* textcs, GLsizei numVertices,
	GLushort* vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
	int i;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glNormalPointer(GL_FLOAT, 0, normals);

	if (textcs)
	{
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glTexCoordPointer(2, GL_FLOAT, 0, textcs);
	}

	if (!vertIdxs)
		glDrawArrays(GL_TRIANGLES, 0, numVertices);
	else
		if (numParts > 1)
			for (i = 0; i < numParts; i++)
				glDrawElements(GL_TRIANGLE_STRIP, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs + i * numVertIdxsPerPart);
		else
			glDrawElements(GL_TRIANGLES, numVertIdxsPerPart, GL_UNSIGNED_SHORT, vertIdxs);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	if (textcs)
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

static void fghDrawGeometryWire11(GLfloat* vertices, GLfloat* normals,
	GLushort* vertIdxs, GLsizei numParts, GLsizei numVertPerPart)
{
	int i;

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glVertexPointer(3, GL_FLOAT, 0, vertices);
	glNormalPointer(GL_FLOAT, 0, normals);


	if (!vertIdxs)
		/* Draw per face (TODO: could use glMultiDrawArrays if available) */
		for (i = 0; i < numParts; i++)
			glDrawArrays(GL_LINE_STRIP, i * numVertPerPart, numVertPerPart);
	else
		for (i = 0; i < numParts; i++)
			glDrawElements(GL_LINE_STRIP, numVertPerPart, GL_UNSIGNED_SHORT, vertIdxs + i * numVertPerPart);

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
}


void fghDrawGeometryWire(GLfloat* vertices, GLfloat* normals, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertPerPart)
{
	fghDrawGeometryWire11(vertices, normals, vertIdxs, numParts, numVertPerPart);
}


void fghDrawGeometrySolid(GLfloat* vertices, GLfloat* normals, GLfloat* textcs, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
{
		fghDrawGeometrySolid11(vertices, normals, textcs, numVertices, vertIdxs, numParts, numVertIdxsPerPart);
}





//#include "fg_teapot.inc"

void glutWireTeapot(std::vector<VertexPNCT>* vbuf, std::vector<uint16_t>* ibuf, float size, std::function<void(GLfloat* vertices, GLfloat* normals, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertPerPart)>wout);
void glutSolidTeapot(std::vector<VertexPNCT>* vbuf, std::vector<uint16_t>* ibuf, float size, std::function<void(GLfloat* vertices, GLfloat* normals, GLfloat* textcs, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)>sout);
void glutWireTeacup(double size);
void glutSolidTeacup(double size);
void glutWireTeaspoon(double size);
void glutSolidTeaspoon(double size);



void Geometry::GenerateWireTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size)
{
	vertices->clear();
	IndexVector ibuf;
//	using WireOutputFunc = std::function<void(GLfloat* vertices, GLfloat* normals, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertPerPart)>;
//	using SolidOutputFunc = std::function<void(GLfloat* vertices, GLfloat* normals, GLfloat* textcs, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)>;

	glutWireTeapot(vertices, &ibuf, size, [&](GLfloat* v, GLfloat* n, GLsizei numVertices, GLushort* idx, GLsizei numParts, GLsizei numVertPerPart)
		{
			for (auto i = 0; i < numVertices; i++)
			{
				vertices->push_back(VertexPNCT(vec3(v[i * 3 + 0]), vec3(n[i * 3 + 0]), vec4(), vec2()));
				vertices->push_back(VertexPNCT(vec3(v[i * 3 + 1]), vec3(n[i * 3 + 1]), vec4(), vec2()));
				vertices->push_back(VertexPNCT(vec3(v[i * 3 + 2]), vec3(n[i * 3 + 2]), vec4(), vec2()));
			}
		}
	);

	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}

void Geometry::GenerateSolidTeapot(VertexPNCTVector* vertices, IndexVector* indices, float size)
{
	vertices->clear();
	IndexVector ibuf;

	glutSolidTeapot(vertices, &ibuf, size, [&](GLfloat* vertices, GLfloat* normals, GLfloat* textcs, GLsizei numVertices, GLushort* vertIdxs, GLsizei numParts, GLsizei numVertIdxsPerPart)
		{

		}
	);

	if (indices)
	{
		indices->swap(ibuf);
	}
	else
	{
		DecompressArray(vertices, &ibuf);
	}
}

#endif

#if 1	//ChatGPT:OBB　計算　C++ glm
mat4 Geometry::CalcOBB(VertexPNCTVector* vertices)
{
	if (vertices->size() < 1)
	{
		return mat4(0.0f);
	}

	// 中心を計算
	vec3 center(0.0f);

	for (const auto& v : *vertices)
	{
		center += v.pos;
	}
	center /= static_cast<float>(vertices->size());

	// 共分散行列を計算
	mat3 covariance(0.0f);
	vec3 diff;
	for (const auto& v : *vertices)
	{
		diff = v.pos - center;
		covariance += glm::outerProduct(diff, diff);
	}
	covariance /= static_cast<float>(vertices->size());

	// 共分散行列を対角化
	quat orientation;
	vec3 scale;
	vec3 translation;
	vec3 skew;
	vec4 perspective;
	glm::decompose(mat4(covariance), scale, orientation, translation, skew, perspective);

	mat3 rotation = mat3(orientation);

	// OBBのサイズを計算
	vec3 extents = vec3(MyMath::Sqrt(scale.x), MyMath::Sqrt(scale.y), MyMath::Sqrt(scale.z));
	//mat4 result = MyMath::CreateTranslateMat4(center) * mat4(rotation) * MyMath::CreateScaleMat4(extents);
	
	mat4 result = glm::scale(mat4(1.0f), extents);
	result = mat4(rotation) * result;
	result = glm::translate(mat4(1.0f), center) * result;

	return result;
}


// バウンディングカプセルを計算する関数
void computeBoundingCapsule(const std::vector<vec3>& points) {
	// 平均点を計算
	glm::vec3 mean(0.0f);
	for (const vec3& point : points) {
		mean += glm::vec3(point.x, point.y, point.z);
	}
	mean /= static_cast<float>(points.size());

	// 最遠点を探す
	float maxDistanceSq = 0.0f;
	glm::vec3 furthestPoint;
	for (const vec3& point : points) {
		glm::vec3 pointVec(point.x, point.y, point.z);
		float distanceSq = glm::distance2(mean, pointVec);
		if (distanceSq > maxDistanceSq) {
			maxDistanceSq = distanceSq;
			furthestPoint = pointVec;
		}
	}

	// 最遠点と中心から半径を計算
	float radius = sqrt(maxDistanceSq);
	glm::vec3 centerToFarthest = furthestPoint - mean;
	glm::vec3 direction = glm::normalize(centerToFarthest);

	// 結果を出力
	std::cout << "Bounding Capsule Center: (" << mean.x << ", " << mean.y << ", " << mean.z << ")\n";
	std::cout << "Bounding Capsule Radius: " << radius << "\n";
	std::cout << "Bounding Capsule Direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")\n";
}
#endif
#if 0	//ChatGPT:OBB　計算　C++ glm
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// Structure to represent a 3D point
struct Point3D {
	float x;
	float y;
	float z;

	Point3D(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}
};

// Function to calculate the oriented bounding box (OBB)
glm::mat4 calculate_obb(const std::vector<Point3D>& points) {
	// Create a matrix to hold the points
	glm::mat3 covariance(0.0f);
	glm::vec3 center(0.0f);
	for (const Point3D& p : points) {
		glm::vec3 point(p.x, p.y, p.z);
		center += point;
	}
	center /= static_cast<float>(points.size());

	for (const Point3D& p : points) {
		glm::vec3 point(p.x, p.y, p.z);
		glm::vec3 deviation = point - center;
		covariance += glm::outerProduct(deviation, deviation);
	}
	covariance /= static_cast<float>(points.size());

	// Compute the eigenvectors of the covariance matrix
	glm::mat3 rotation = glm::mat3(glm::eulerAngles(glm::quat(glm::inverse(glm::mat3(covariance)))));

	// Compute the extents along the eigenvectors
	glm::vec3 minExtents = glm::vec3(FLT_MAX);
	glm::vec3 maxExtents = glm::vec3(-FLT_MAX);
	for (const Point3D& p : points) {
		glm::vec3 point(p.x, p.y, p.z);
		glm::vec3 transformed = rotation * (point - center);
		minExtents = glm::min(minExtents, transformed);
		maxExtents = glm::max(maxExtents, transformed);
	}

	// Compute the transformation matrix for the OBB
	glm::mat4 modelMatrix(1.0f);
	modelMatrix = glm::translate(modelMatrix, center);
	modelMatrix *= glm::mat4(rotation);
	modelMatrix = glm::scale(modelMatrix, maxExtents - minExtents);

	return modelMatrix;
}

int main() {
	std::vector<Point3D> points = { Point3D(1.0f, 2.0f, 3.0f), Point3D(3.0f, 5.0f, 7.0f), Point3D(7.0f, 8.0f, 2.0f), Point3D(2.0f, 6.0f, 9.0f) };
	glm::mat4 obbTransform = calculate_obb(points);

	std::cout << "OBB Transform Matrix:" << std::endl;
	for (int i = 0; i < 4; ++i) {
		for (int j = 0; j < 4; ++j) {
			std::cout << obbTransform[j][i] << " ";
		}
		std::cout << std::endl;
	}

	return 0;
}
#endif
#if 0
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// 点のデータ構造
struct Point {
	float x, y, z;
};

// 点群からOBBを計算する関数
void computeOBB(const std::vector<Point>& points) {
	// 平均点を計算
	glm::vec3 mean(0.0f);
	for (const Point& point : points) {
		mean += glm::vec3(point.x, point.y, point.z);
	}
	mean /= static_cast<float>(points.size());

	// 共分散行列を計算
	glm::mat3 covariance(0.0f);
	for (const Point& point : points) {
		glm::vec3 diff = glm::vec3(point.x, point.y, point.z) - mean;
		covariance += glm::outerProduct(diff, diff);
	}
	covariance /= static_cast<float>(points.size());

	// 共分散行列を対角化
	glm::mat3 rotation;
	glm::vec3 scale;
	glm::vec3 skew;
	glm::vec4 perspective;
	glm::decompose(glm::mat4(covariance), scale, rotation, skew, perspective);

	// OBBのサイズを計算
	glm::vec3 size(sqrt(scale.x), sqrt(scale.y), sqrt(scale.z));

	// 結果を出力
	std::cout << "OBB Center: (" << mean.x << ", " << mean.y << ", " << mean.z << ")\n";
	std::cout << "OBB Size: (" << size.x << ", " << size.y << ", " << size.z << ")\n";
	std::cout << "OBB Rotation Matrix:\n";
	std::cout << rotation[0][0] << " " << rotation[0][1] << " " << rotation[0][2] << "\n";
	std::cout << rotation[1][0] << " " << rotation[1][1] << " " << rotation[1][2] << "\n";
	std::cout << rotation[2][0] << " " << rotation[2][1] << " " << rotation[2][2] << "\n";
}

int main() {
	// テスト用の点群を作成
	std::vector<Point> points = {
		{1.0f, 2.0f, 3.0f},
		{4.0f, 5.0f, 6.0f},
		{7.0f, 8.0f, 9.0f}
		// 他の点を追加
	};

	// OBBを計算
	computeOBB(points);

	return 0;
}
#endif

