#pragma once
#include "framework.h"
#include "MyMath.h"


//https://edom18.hateblo.jp/entry/2018/06/27/132235

/// <summary>
/// OBB	
/// </summary>
class OBB
{
public:
	vec3 obbCenter_;
	vec3 obbHalfSize_;
	mat3 obbAxes_;

	std::vector<vec3> edges_;
	vec3 origin_;
	inline std::string ToString() const
	{
		std::stringstream ss;
		ss << "OBB{ center{" << glm::to_string(obbCenter_) << "}, ";
		ss << "half{" << glm::to_string(obbHalfSize_) << "}, ";
		ss << "axes{ " << glm::to_string(obbAxes_) << "} }";
		return ss.str();
	}

	inline void GenarateLines(std::vector<VertexPC>* vertices, const vec4& color)
	{
		vertices->clear();
		if (edges_.size() == 0)
		{
			return;
		}
		/*
				if (!lines)
				{
					//TRIANGLES
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

					// Create each face in turn.
					for (int i = 0; i < FaceCount; i++)
					{
						auto normal = faceNormals[i];

						// Get two vectors perpendicular both to the face normal and to each other.
						const auto basis = (i >= 4) ? vec3(0.0f, 0.0f, 1.0f) : vec3(0.0f, 1.0f, 0.0f);

						const auto side1 = glm::cross(normal, basis);
						const auto side2 = glm::cross(normal, side1);
						const vec3 tsize(obbHalfSize_);
						vec4 col(MyMath::Abs(normal.x), MyMath::Abs(normal.y), MyMath::Abs(normal.z), 1.0f);

						// Four vertices per face.
						auto v0 = VertexPNCT((normal - side1 - side2) * tsize, normal, col, vec2());

						auto v1 = VertexPNCT((normal - side1 + side2) * tsize, normal, col, vec2());

						auto v2 = VertexPNCT((normal + side1 + side2) * tsize, normal, col, vec2());

						auto v3 = VertexPNCT((normal + side1 - side2) * tsize, normal, col, vec2());

						(*vertices).push_back(v0);
						(*vertices).push_back(v1);
						(*vertices).push_back(v2);
						(*vertices).push_back(v0);
						(*vertices).push_back(v2);
						(*vertices).push_back(v3);
					}
				}
				else
				{
		*/
		//LINES
		auto ftr = VertexPC(obbCenter_ + (edges_[0] * 0.5f) + (edges_[1] * 0.5f) + (edges_[2] * 0.5f), color);
		auto fbr = VertexPC(ftr.pos - edges_[2], color);
		auto ftl = VertexPC(ftr.pos - edges_[0], color);
		auto fbl = VertexPC(ftl.pos - edges_[2], color);

		auto btr = VertexPC(ftr.pos - edges_[1], color);
		auto bbr = VertexPC(btr.pos - edges_[2], color);
		auto btl = VertexPC(btr.pos - edges_[0], color);
		auto bbl = VertexPC(btl.pos - edges_[2], color);


		(*vertices).push_back(ftr);	(*vertices).push_back(fbr);
		(*vertices).push_back(ftr);	(*vertices).push_back(ftl);
		(*vertices).push_back(ftl); (*vertices).push_back(fbl);
		(*vertices).push_back(fbl); (*vertices).push_back(fbr);

		(*vertices).push_back(btr); (*vertices).push_back(bbr);
		(*vertices).push_back(btr); (*vertices).push_back(btl);
		(*vertices).push_back(btl); (*vertices).push_back(bbl);
		(*vertices).push_back(bbl); (*vertices).push_back(bbr);

		(*vertices).push_back(ftr); (*vertices).push_back(btr);
		(*vertices).push_back(ftl); (*vertices).push_back(btl);
		(*vertices).push_back(fbr); (*vertices).push_back(bbr);
		(*vertices).push_back(fbl); (*vertices).push_back(bbl);

		auto vc = VertexPC(obbCenter_, vec4(1.0f));
		auto v0 = VertexPC(obbCenter_ + obbAxes_[0], vec4(0, 0, 1, 1));
		auto v1 = VertexPC(obbCenter_ + obbAxes_[1], vec4(1, 0, 0, 1));
		auto v2 = VertexPC(obbCenter_ + obbAxes_[2], vec4(0, 1, 0, 1));
		(*vertices).push_back(vc); (*vertices).push_back(v0);
		(*vertices).push_back(vc); (*vertices).push_back(v1);
		(*vertices).push_back(vc); (*vertices).push_back(v2);
	}

	inline void Calc(const std::vector<vec3>& verts)
	{
		std::vector<vec3> points;
		for (const auto& v : verts)
		{
			points.push_back(v);
		}
		ComputeEdges(points);
	}

	inline void ComputeEdges(const std::vector<vec3>& points)
	{
		auto cov = ComputeCovariance(points);
		TRACE("cov=%s\n", glm::to_string(cov).c_str());
		mat3 eigenVectors;
		vec3 eigenValues;
		JacobiEigenSolver(cov, &eigenValues, &eigenVectors);
		TRACE("eigenValues=%s\n", glm::to_string(eigenValues).c_str());
		TRACE("eigenVectors=%s\n", glm::to_string(eigenVectors).c_str());

		// 全頂点に対して内積を取り、最小値・最大値を計算する
		vec3 v1(eigenVectors[0][0], eigenVectors[1][0], eigenVectors[2][0]);
		vec3 v2(eigenVectors[0][1], eigenVectors[1][1], eigenVectors[2][1]);
		vec3 v3(eigenVectors[0][2], eigenVectors[1][2], eigenVectors[2][2]);
		float min1 = FLT_MAX;
		float min2 = FLT_MAX;
		float min3 = FLT_MAX;
		float max1 = -FLT_MIN;
		float max2 = -FLT_MIN;
		float max3 = -FLT_MIN;

		for (const auto& p : points)
		{
			float dot1 = glm::dot(v1, p);
			if (dot1 > max1)
			{
				max1 = dot1;
			}
			if (dot1 < min1)
			{
				min1 = dot1;
			}

			float dot2 = glm::dot(v2, p);
			if (dot2 > max2)
			{
				max2 = dot2;
			}
			if (dot2 < min2)
			{
				min2 = dot2;
			}

			float dot3 = glm::dot(v3, p);
			if (dot3 > max3)
			{
				max3 = dot3;
			}
			if (dot3 < min3)
			{
				min3 = dot3;
			}
		}

		float len1 = max1 - min1;
		float len2 = max2 - min2;
		float len3 = max3 - min3;
		obbHalfSize_ = vec3(len1, len2, len3);

		vec3 edge1 = (v1 * len1);
		vec3 edge2 = (v2 * len2);
		vec3 edge3 = (v3 * len3);

		edges_.clear();
		edges_.push_back(edge1);
		edges_.push_back(edge2);
		edges_.push_back(edge3);
		obbAxes_[0] = glm::normalize(v1);
		obbAxes_[1] = glm::normalize(v2);
		obbAxes_[2] = glm::normalize(v3);

		vec3 center1 = (v1 * (max1 + min1)) * 0.5f;
		vec3 center2 = (v2 * (max2 + min2)) * 0.5f;
		vec3 center3 = (v3 * (max3 + min3)) * 0.5f;

		obbCenter_ = (center1 + center2 + center3);
	}

	/// <summary>
	/// 点群の重心を計算する
	/// </summary>
	/// <returns>重心</returns>
	inline vec3 ComputeCentroid(const std::vector<vec3>& points)
	{
		vec3 centroid(0.0f);
		for (const auto& p : points)
		{
			centroid += p;
		}
		centroid /= static_cast<float>(points.size());
		return centroid;
	}

	/// <summary>
	/// 共分散行列を計算する
	/// </summary>
	/// <returns>共分散行列</returns>
	inline mat3 ComputeCovariance(const std::vector<vec3>& points)
	{
		vec3 c = ComputeCentroid(points);

		float c11 = 0.0f; float c22 = 0.0f; float c33 = 0.0f;
		float c12 = 0.0f; float c13 = 0.0f; float c23 = 0.0f;

		for (const auto& p : points)
		{
			c11 += (p.x - c.x) * (p.x - c.x);
			c22 += (p.y - c.y) * (p.y - c.y);
			c33 += (p.z - c.z) * (p.z - c.z);

			c12 += (p.x - c.x) * (p.y - c.y);
			c13 += (p.x - c.x) * (p.z - c.z);
			c23 += (p.y - c.y) * (p.z - c.z);
		}

		c11 /= static_cast<float>(points.size());
		c22 /= static_cast<float>(points.size());
		c33 /= static_cast<float>(points.size());
		c12 /= static_cast<float>(points.size());
		c13 /= static_cast<float>(points.size());
		c23 /= static_cast<float>(points.size());

		mat3 covariance =
		{
			{ c11, c12, c13 },
			{ c12, c22, c23 },
			{ c13, c23, c33 },
		};

		return covariance;
	}


	/// <summary>
	/// ヤコビ法による対称行列の固有値と固有ベクトルの計算
	/// </summary>
	/// <param name="A">対称行列</param>
	/// <param name="eigenvalues">固有値</param>
	/// <param name="eigenvectors">固有ベクトル</param>
	inline void JacobiEigenSolver(const mat3& A, vec3* eigenvalues, mat3* eigenvectors)
	{
		const int maxIterations = 50;
		const float epsilon = MyMath::_E;

		*eigenvectors = mat3(1.0f);
		mat3 D(A);
		for (int iter = 0; iter < maxIterations; ++iter)
		{
			// 行列の対角線上ではない要素の中で、絶対値が最も大きい要素を探す
			float maxOffDiag = 0.0f;
			int p = 0, q = 0;
			for (int i = 0; i < 3; ++i)
			{
				for (int j = i + 1; j < 3; ++j)
				{
					if (MyMath::Abs(D[i][j]) > maxOffDiag)
					{
						maxOffDiag = MyMath::Abs(D[i][j]);
						p = i;
						q = j;
					}
				}
			}

			// 最大の非対角成分が小さい場合、行列はすでに対角化されている
			if (maxOffDiag <= epsilon)
			{
				break;
			}

			// ヤコビ回転行列（行列の対角成分以外の要素を0にするために使う）を計算する
			float rad = 0.5f * MyMath::ArcTan2(2.0f * D[p][q], D[q][q] - D[p][p]);
			float deg = MyMath::RadToDeg(rad);
			float c = MyMath::Cos(deg);
			float s = MyMath::Sin(deg);
			mat3 J = mat3(1.0f);
			J[p][p] = c;
			J[q][q] = c;
			J[p][q] = s;
			J[q][p] = -s;

			// 対角行列 D とその固有ベクトルを更新する
			D = glm::transpose(J) * D * J;
			*eigenvectors *= J;
		}

		// 行列 D の対角成分から固有値を抽出する
		*eigenvalues = vec3(D[0][0], D[1][1], D[2][2]);
	}
};

