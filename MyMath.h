#pragma once

#include <string>
#include <sstream>

#include "framework.h"

namespace MyMath
{
	// C++11からの共有体を使った
	// IEEE754のfloatサポート共有体
	union Binary32
	{
		float flt;
		uint32_t bin;
		Binary32() : bin(0u) {}
		Binary32(const uint32_t& u) : bin(u) {}
		Binary32(const float& f) : flt(f) {}
	};

	// 定数
	const float	_PAI = 355.0f / 113.0f;// (3.14159265358979323846264338f);
	const float	_E = (1.0e-6f);
	const Binary32 _NAN = Binary32(0x7fc00000u);
	const Binary32 _INF = Binary32(0x7f800000u);
	const Binary32 _NINF = Binary32(0xff800000u);


#pragma region degree&radian
	inline float RadToDeg(const float& x)
	{
		// ラジアン→度数法
		return x * (180.0f / _PAI);
	}
	inline float DegToRad(const float& x)
	{
		// 度数法→ラジアン
		return x * (_PAI / 180.0f);
	}
	inline float RadWrapPAI(const float& x)
	{
#if 1
		// ループで正規化する
		float d = x;
		// 180超なら360を減算
		while (d > _PAI)
		{
			d -= _PAI*2.0f;
		}
		// -180未満なら360を加算（-180はナシ）
		while (d < -_PAI)
		{
			d += _PAI*2.0f;
		}
		return d;
#else
		// ループせず正規化する
		
		// 360°を65536分割(16ビット)で、-180 ≦ rad ＜ 180 にする
		return static_cast<float>(static_cast<int16_t>(x * (65536.0f / (_PAI*2.0f)))) * ((_PAI*2.0f)/ 65536.0f);
#endif
	}
	inline float RadWrap2PAI(const float& x)
	{
#if 1
		// ループで正規化する
		float d = x;
		// 360以上なら360を減算
		while (d >= _PAI * 2.0f)
		{
			d -= _PAI * 2.0f;
		}
		// 0未満なら360を加算（0はアリ）
		while (d < 0.0f)
		{
			d += _PAI * 2.0f;
		}
		return d;
#else
		// ループせず正規化する

		// 360°を65536分割(16ビット)で、0 ≦ rad ＜ 360 にする
		return static_cast<float>(static_cast<uint16_t>(x * (65536.0f / (_PAI * 2.0f)))) * ((_PAI * 2.0f)/ 65536.0f);
#endif
	}
	inline float DegWrap180(const float& x)
	{
#if 1
		// ループで正規化する
		float d = x;
		// 180超なら360を減算
		while (d > 180.0f)
		{
			d -= 360.0f;
		}
		// -180未満なら360を加算（-180はナシ）
		while (d < -180.0f)
		{
			d += 360.0f;
		}
		return d;
#else
		// ループせず正規化する

		// 360°を65536分割(16ビット)で、-180 ≦ deg ＜ 180 にする
		return static_cast<float>(static_cast<int16_t>(x * (65536.0f / 360.0f))) * (360.0f / 65536.0f);
#endif
	}
	inline float DegWrap360(const float& x)
	{
#if 1
		// ループで正規化する
		float d = x;
		// 360以上なら360を減算
		while (d >= 360.0f)
		{
			d -= 360.0f;
		}
		// 0未満なら360を加算（0はアリ）
		while (d < 0.0f)
		{
			d += 360.0f;
		}
		return d;
#else
		// ループせず正規化する

		// 360°を65536分割(16ビット)で、0 ≦ deg ＜ 360 にする
		return static_cast<float>(static_cast<uint16_t>(x * (65536.0f / 360.0f))) * (360.0f / 65536.0f);
#endif
	}

#pragma endregion

#pragma region MinMaxAbs
	template<typename T> inline T Max(const T& s, const T& t)
	{
		return (s > t) ? s : t;
	}
	
	template<typename T> inline T Min(const T& s, const T& t)
	{
		return (s < t) ? s : t;
	}
	
	template<typename T> inline T Clamp(const T& value, const T& min, const T& max)
	{
		return Max(Min(value, max), min);
	}
	
	template<typename T> inline T Abs(const T& value)
	{
		return (value < 0) ? -value : value;
	}

	// Absのfloat特殊化
	template<> inline float Abs(const float& value)
	{
		// IEEE754では最上位ビットが符号なのでbinの＆演算で済む
		Binary32 temp(value);
		temp.bin &= 0x7fffffff;
		return temp.flt;
	}

	inline bool IsWithinTolerance(const float& value, const float& tolerance = _E)
	{
		return (Abs(value) <= tolerance);
	}

	inline bool IsNAN(const float& value)
	{
		return Binary32(value).bin == _NAN.bin;
		//{
		//	return true;
		//}

		// IEEE754ではNANなら値比較が出来なくなる
		return value != value;
	}

	inline bool IsINF(const float& value)
	{
		return Binary32(value).bin == _INF.bin;
	}

	inline bool IsNINF(const float& value)
	{
		return Binary32(value).bin == _NINF.bin;
	}
#pragma endregion

#pragma region BasicFuncs
	inline float Round(const float& x)
	{
		return static_cast<float>(static_cast<int32_t>((x < 0.0f) ? (x - 0.5f) : (x + 0.5f)));
	}

	inline float Ceil(const float& x)
	{
		Binary32 n(static_cast<float>(static_cast<int32_t>(x)));
		if (x < 0.0f)
		{
			n.bin |= 0x80000000;
			return n.flt;
		}
		else
			return ((x - n.flt) != 0.0f) ? (n.flt + 1.0f) : n.flt;
	}

	inline float Floor(const float& x)
	{
		Binary32 n(static_cast<float>(static_cast<int32_t>(x)));
		if (x < 0.0f)
			return ((x - n.flt) != 0.0f) ? (n.flt - 1.0f) : n.flt;
		else
			return n.flt;
	}

	inline float Sqrt(const float& value)
	{
		// 公式
		// X(n+1) = ( X(n) + X(1) / X(n) ) / 2	… n > 1
		if (value < 0.0f)	return _NAN.flt;
		float s(value), t;
		do {
			t = s;
			s = (t + value / t) * 0.5f;
		} while (s < t);
		return t;
	}

	inline float FastInvSqrt(const float& value)
	{
		Binary32 b;
		float x2, y;
		const float threehalfs = 1.5F;

		x2 = value * 0.5f;
		y = value;
		b.flt= y;	// evil floating point bit level hacking
		b.bin = 0x5f3759df - (b.bin >> 1);	// what the fuck?
		y = b.flt;
		//for (auto i = 0; i < 2; i++)
		{
			y = y * (threehalfs - (x2 * y * y));   // 1st iteration
		}
		
		return y;
	}

	inline float FastSqrt(const float& value)
	{
		return FastInvSqrt(value) * value;
	}

	inline float Cos(const float& rad)
	{
		// テイラー展開
		//              x²   x⁴   x⁶   x⁸
		// cos(x) = 1 - __ + __ - __ + __ ...
		//              2!   4!   6!   8!
		float x = RadWrapPAI(rad);
		float xx = x * x;
		float xxxx = xx * xx;
		return 1.0f - ((1.0f / 2.0f) * xx) + ((1.0f / 24.0f) * xxxx) - ((1.0f / 720.0f) * xxxx * xx) + ((1.0f / 40320.0f) * xxxx * xxxx) - ((1.0f / 3628800.0f) * xxxx * xxxx * xx);
	}
	
	inline float Sin(const float& rad)
	{
		// テイラー展開
		//              x³   x⁵   x⁷   x⁹
		// sin(x) = x - __ + __ - __ + __ ...
		//              3!   5!   7!   9!
		float x = RadWrapPAI(rad);
		float xx = x * x;
		float xxx = xx * x;
		float xxxx = xxx * x;
		return x - ((1.0f / 6.0f) * xxx) + ((1.0f / 120.0f) * xxxx * x) - ((1.0f / 5040.0f) * xxxx * xxx) + ((1.0f / 362880.0f) * xxx * xxx * xxx) - ((1.0f / 39916800.0f) * xxxx * xxxx * xxx);
	}
	
	inline float Tan(const float& rad)
	{
		// tan = sin / cos
		Binary32 c(Cos(rad));
		return ((c.bin & 0x7fffffff) == 0) ? F32_MAX : (Sin(rad) / c.flt);
	}

	/// <summary>
	/// アークタンジェント
	/// </summary>
	/// <param name="tann">正接</param>
	/// <returns>ラディアン</returns>
	inline float ArcTan(const float& tann)
	{
		bool neg(tann < 0.0f);
		float t(Abs(tann));
		bool inv(t > 1.0f);
		if (inv)
		{
			t = 1.0f / t;	// (π/4)以上はひっくり返して計算
		}
#if 1
		// マクローリン展開
		//               x³   x⁵   x⁷
		// atan(x) = x - __ + __ - __ ± …　[ -1<x<1 ]
		//               3    5    7
		// ルジャンドル多項式を利用して、定積分を求めるガウス求積という方法で近似式を一撃で解く https://fermiumbay13.hatenablog.com/entry/2019/03/31/162636
		float a = t * (12.0f * t * t + 45.0f) / (27.0f * t * t + 45.0f);
#else
		// ここでは45°の範囲でテーブルを持ち、シーケンシャルサーチというイニシエの手法で解く
		float a(0.0f);
		static const float interval = 45.0f / 15.0f;
		static const float tanTbl[16] =	// 0～45度のテーブル
		{
			0.00000000000000f,0.05240777928304f,0.10510423526568f,0.15838444032454f,0.21255656167002f,0.26794919243112f,0.32491969623291f,0.38386403503542f,
			0.44522868530854f,0.50952544949443f,0.57735026918963f,0.64940759319751f,0.72654252800536f,0.80978403319501f,0.90040404429784f,1.00000000000000f,
		};
		for (auto i = 0; i < numof(tanTbl) - 1; i++)	// 16個程度なのでシーケンシャルサーチでいいかな
		{
			if (tanTbl[i + 1] >= t)
			{
				a = RadToRad(interval * (static_cast<float>(i) + (t - tanTbl[i]) / (tanTbl[i + 1] - tanTbl[i])));	// 補間
				break;
			}
		}
#endif
		if (inv)	a = _PAI * 0.5f - a;
		if (neg)	a = -a;
		return a;
	}

	/// <summary>
	/// アークタンジェント2
	/// </summary>
	/// <param name="y"></param>
	/// <param name="x"></param>
	/// <returns>ラジアン</returns>
	inline float ArcTan2(const float& y, const float& x)
	{
		if (x > 0.0f)
		{
			return ArcTan(y / x);
		}
		else
		{
			if (IsWithinTolerance(x))
			{
				return (y > 0.0f) ? _PAI * 0.5f : -_PAI * 0.5f;
			}
			if (y > 0.0f)
			{
				return ArcTan(y / x) + _PAI;
			}
			else
			{
				return ArcTan(y / x) - _PAI;
			}
		}
	}

	/// <summary>
	/// アークコサイン
	/// </summary>
	/// <param name="cosn"></param>
	/// <returns>ラジアン</returns>
	inline float ArcCos(const float& cosn)
	{
		float x = Abs(cosn);
		float result = -0.0187293f;
		result = result * x;
		result = result + 0.0742610f;
		result = result * x;
		result = result - 0.2121144f;
		result = result * x;
		result = result + 1.5707288f;
		result = result * Sqrt(1.0f - x);
		if (cosn < 0)
		{
			result = result - 2.0f * result;
			return result + _PAI;
		}
		else
		{
			return result;
		}
	}

	inline float ArcSin(const float& sinn)
	{
		return ArcTan2(sinn, Sqrt(1.0f - sinn * sinn));
	}

	inline float Pow(const float& x, const float& n)
	{
		if (n < 0.0f)	return _NAN.flt;
		float	X(x);
		float	v = 1.0f;
		float	n_int = Floor(n);
		float	n_flt = n - n_int;

		// 整数部
		int32_t in = static_cast<int32_t>(n_int);
		while (in > 0)
		{
			v *= X;
			in -= 1;
		}

		// 小数部…0.5ずつ処理（ ∵ n^0.5 == sqrt(n) ） 
		const static uint32_t fbit = 12;	// ←精度
		const static uint32_t fbias = (1 << fbit);
		const static uint32_t fmask = fbias - 1;
		uint32_t fn = static_cast<uint32_t>(n_flt * static_cast<float>(fbias));
		if (fn)
		{
			do {
				X = Sqrt(X);
				fn = (fn & (fmask - 1)) << 1;
				if ((fn & fbias) != 0)
					v *= X;
			} while (fn);
		}
		return v;
	}
#pragma endregion

	inline mat4 OrthonormalizeMat(const mat4& mat)
	{
		// 元の行列の列ベクトルを取得（glm は列優先レイアウト）
		vec3 xAxis(mat[0].xyz); // X軸
		vec3 yAxis(mat[1].xyz); // Y軸
		vec3 zAxis(mat[2].xyz); // Z軸

		// X軸を正規化
		xAxis=glm::normalize(xAxis);

		// Y軸をX軸に直交させ、正規化
		yAxis = yAxis - xAxis * glm::dot(xAxis, yAxis);
		yAxis = glm::normalize(yAxis);

		// Z軸を X と Y の外積から再計算し、正規化
		zAxis = glm::cross(xAxis, yAxis);
		zAxis = glm::normalize(zAxis);

		// 新しい行列を構築
		mat3 result(mat);

		// 回転部分を更新（列ベクトルとして設定）
		result[0] = xAxis;
		result[1] = yAxis;
		result[2] = zAxis;

		// 平行移動（第4列）とスケーリングはそのまま
		return result;
	}

	inline vec3 CircleVector(size_t Numerator, size_t denominator)
	{
		float angle = float(Numerator) * 360.0f / float(denominator);
		return vec3(Sin(angle), 0.0f, Cos(angle));
	};

	inline vec3 CircleTangent(size_t i, size_t tessellation)
	{
		const float angle = (float(i) * 360.0f / float(tessellation)) + 90.0f;
		return vec3(Cos(angle), 0.0f, Sin(angle));
	}

#if 0
#pragma region VectorUtil
	inline float LengthSquare(const vec3& a, const vec3& b)
	{
		vec3 t(b - a);
		return t.x * t.x + t.y * t.y + t.z * t.z;
	}

	/// <summary>
	/// srcをgoalに漸近させる
	/// </summary>
	/// <param name="src">元になる角度</param>
	/// <param name="goal">目的の角度値</param>
	/// <param name="addVal">増分値</param>
	/// <returns></returns>
	inline vec3 ConvergenceRotate(const vec3& src, const vec3& goal, const vec3& addVal = vec3(5.0f))
	{
		vec3 result(src);
		vec3 d(goal - src);

		d.z = RadWrapPAI(d.z);
		if (Abs(d.z) < addVal.z)
		{
			result.z = goal.z;
		}
		else if (d.z < 0.0f)
		{
			result.z -= addVal.z;
		}
		else /*if (d.z > 0.0f)*/
		{
			result.z += addVal.z;
		}

		d.y = RadWrapPAI(d.y);
		if (Abs(d.y) < addVal.y)
		{
			result.y = goal.y;
		}
		else if (d.y < 0.0f)
		{
			result.y -= addVal.y;
		}
		else /*if (d.y > 0.0f)*/
		{
			result.y += addVal.y;
		}

		d.x = RadWrapPAI(d.x);
		if (Abs(d.x) < addVal.x)
		{
			result.x = goal.x;
		}
		else if (d.x < 0.0f)
		{
			result.x -= addVal.x;
		}
		else /*if (d.x > 0.0f)*/
		{
			result.x += addVal.x;
		}

		return result;
	}

	/// <summary>
	/// src値をgoalに漸近させる
	/// </summary>
	/// <param name="src">値</param>
	/// <param name="goal">ゴール</param>
	/// <param name="addVal">増分値</param>
	/// <returns></returns>
	inline float Convergence(const float& src, const float& goal, const float& addVal = 5.0f)
	{
		float result(src);
		float d(goal - src);

		if (Abs(d) < addVal)
		{
			result = goal;
		}
		else if (d < 0.0f)
		{
			result -= addVal;
		}
		else /*if (d > 0.0f)*/
		{
			result += addVal;
		}

		return result;
	}

	/// <summary>
	/// from→toの角度計算
	/// 
	/// 基準面はZX平面
	///	高さは + Y軸
	/// 回転軸は右手系
	/// </summary>
	/// <param name="from"></param>
	/// <param name="to"></param>
	/// <returns>vec3(ピッチ角,ヘディング角,0)</returns>
	inline vec3 LookAtRotation(const vec3& from, const vec3& to)
	{
		vec3 d(to - from);
		return vec3(
			-(ArcTan2(d.y, Sqrt(d.z * d.z + d.x * d.x))),
			(ArcTan2(d.x, d.z)),
			0.0f);
	}

#pragma endregion
#endif


	//特定の方向からオイラー角を求める
	//https://qiita.com/kit2cuz/items/55be3f432783fc979b16
#if 0

	// 軸は@N、Y軸は@up
	// @upと@Nの外積からx軸のベクトルをつくる
	vector L = cross(@up, @N)

	RzRxRy = { L.x,  L.y,  L.z,
				up.x, up.y, up.z,
				N.x,  N.y,  N.z }
	となります。このベクトルで構成した行列とZXY回転をした行列が同じ結果になるための角度x, y, zは何かを考えます。

	L.x = cos(z) * cos(y) + sin(z) * sin(x) * sin(y)
	L.y = sin(z) * cos(x)
	L.z = -cos(z)sin(y) + sin(z) * sin(x) * cos(y)

	up.x = -sin(z) * cos(y) + cos(z) * sin(x) * sin(y)
	up.y = cos(z) * cos(x)
	up.z = sin(z) * sin(y) + cos(z) * sin(x) * cos(y)

	N.x = cos(x) * sin(y)
	N.y = -sin(x)
	N.z = cos(x) * cos(y)

	角度x
	N.yが - sin(x)に当たるので
	N.y = -sin(x)
	という等式が成り立つ
	x = -asin(N.y)
	となります。

	角度z
	zはL.yをup.yで割ることで求めることができます。
	L.y / up.y = sin(z) * cos(x) / cos(z) * cos(x) = sin(z) / cos(z) = tan(z)
	つまり
	z = atan(L.y, up.y)
	となります。

	角度y
	yも同じ要領でNxをNzで割ることで求まります。
	Nx / Nz = cos(x) * sin(y) / cos(x) * cos(y) = sin(y) / cos(y) = tan(y)
	y = atan(Nx, Nz)
#endif

#if 0
		/ 球面線形補間関数
		// out   : 補間ベクトル（出力）
		// start : 開始ベクトル
		// end : 終了ベクトル
		// t : 補間値（0～1）
		D3DXVECTOR3 * SphereLinear(D3DXVECTOR3 * out, D3DXVECTOR3 * start, D3DXVECTOR3 * end, float t) {

		D3DXVECTOR3 s, e;
		D3DXVec3Normalize(&s, start);
		D3DXVec3Normalize(&e, end);


		// 2ベクトル間の角度（鋭角側）
		float angle = acos(D3DXVec3Dot(&s, &e));

		// sinθ
		float SinTh = sin(angle);

		// 補間係数
		float Ps = sin(angle * (1 - t));
		float Pe = sin(angle * t);

		*out = (Ps * s + Pe * e) / SinTh;

		// 一応正規化して球面線形補間に
		D3DXVec3Normalize(out, out);

		return out;
	}

	// 球面線形補間による補間姿勢算出関数
	// out : 補間姿勢（出力）
	// start : 開始姿勢
	// end : 目標姿勢
	// t : 補間係数（0～1）
	D3DXMATRIX* CalcInterPause(D3DXMATRIX* out, D3DXMATRIX* start, D3DXMATRIX* end, float t) {

		// 各姿勢ベクトル抽出
		D3DXVECTOR3 Sy, Sz;
		D3DXVECTOR3 Ey, Ez;

		memcpy(&Sy, start->m[1], sizeof(float) * 3);
		memcpy(&Sz, start->m[2], sizeof(float) * 3);
		memcpy(&Ey, end->m[1], sizeof(float) * 3);
		memcpy(&Ez, end->m[2], sizeof(float) * 3);

		// 中間ベクトル算出
		D3DXVECTOR3 IY, IZ;
		SphereLinear(&IY, &Sy, &Ey, t);
		SphereLinear(&IZ, &Sz, &Ez, t);

		// 中間ベクトルから姿勢ベクトルを再算出
		D3DXVECTOR3 IX;
		D3DXVec3Cross(&IX, &IY, &IZ);
		D3DXVec3Cross(&IY, &IZ, &IX);
		D3DXVec3Normalize(&IX, &IX);
		D3DXVec3Normalize(&IY, &IY);
		D3DXVec3Normalize(&IZ, &IZ);

		memset(out, 0, sizeof(D3DXMATRIX));
		memcpy(out->m[0], &IX, sizeof(float) * 3);
		memcpy(out->m[1], &IY, sizeof(float) * 3);
		memcpy(out->m[2], &IZ, sizeof(float) * 3);
		out->_44 = 1.0f;

		return out;
	}
#endif


#pragma region Interp

	template<typename T = float>
	inline T Interp(const T& s, const T& e, const float Alpha)
	{
		return s + (e - s) * Alpha;
	}
	template<>
	inline quat Interp<quat>(const quat& s, const quat& e, const float Alpha)
	{
		return glm::mix(s, e, Alpha);
	}

	template<typename T = float>
	inline T EaseIn(const T& A, const T& B, float Alpha, float Exp)
	{
		float const PowAlpha = Pow(Alpha, Exp);
		return Interp<T>(A, B, PowAlpha);
	}

	template<typename T = float>
	inline T EaseOut(const T& A, const T& B, float Alpha, float Exp)
	{
		float const PowAlpha = 1.0f - Pow(1.0f - Alpha, Exp);
		return Interp<T>(A, B, PowAlpha);
	}

	template<typename T = float>
	inline T EaseInOut(const T& A, const T& B, float Alpha, float Exp)
	{
		float easedAlpha;
		if (Alpha < 0.5f)
		{
			easedAlpha = EaseIn(0.0f, 1.0f, Alpha * 2.0f, Exp) * 0.5f;
		}
		else
		{
			easedAlpha = EaseOut(0.0f, 1.0f, Alpha * 2.0f - 1.0f, Exp) * 0.5f + 0.5f;
		}
		return Interp<T>(A, B, easedAlpha);
	}

#if 0
	/// <summary>
	/// quatの球面線形補間
	/// </summary>
	/// <param name="s"></param>
	/// <param name="e"></param>
	/// <param name="Alpha"></param>
	/// <returns>quat</returns>
	inline Quaternion SlerpQuat(const Quaternion& s, const Quaternion& e, const float Alpha)
	{
		float cosTheta = s.Dot(e);

		// Perform a linear interpolation when cosTheta is close to 1 to avoid side effect of sin(angle) becoming a zero denominator
		if (cosTheta > 1.0f - _E)
		{
			// Linear interpolation
			return (1.0f - Alpha) * s + Alpha * e;
		}
		else
		{
			// Essential Mathematics, page 467
			float angle = ArcCos(cosTheta);
			return (Sin((1.0f - Alpha) * angle) * s + Sin(Alpha * angle) * e) / Sin(angle);
		}
	}
#endif
	/// <summary>
	/// vec3の球面線形補間
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="t"></param>
	/// <returns></returns>
#if 0
	inline Vector3 SlerpVec(const Vector3& start, const Vector3& end, const float& t)
	{
		Vector3 s(start);
		Vector3 e(end);
		s.Normalize();
		e.Normalize();

		// 2ベクトル間の角度（鋭角側）
		float theta(ArcCos(s.Dot(e)));
		
		// sinθ
		float SinTh(Sin(theta));

		// 補間係数
		float Ps = Sin(theta * (1.0f - t));
		float Pe = Sin(theta * t);

		if (IsWithinTolerance(theta))
		{
			ASSERT(0);
			return e;
		}
		Vector3 result = (s * Ps + e * Pe) / SinTh;
		result.Normalize();

		return result;
	}

	/// <summary>
	/// Matrixの球面線形補間
	/// </summary>
	/// <param name="start"></param>
	/// <param name="end"></param>
	/// <param name="t"></param>
	/// <returns></returns>
	inline Matrix SlerpMat(const Matrix& start, const Matrix& end, float t)
	{
		// 各姿勢ベクトル抽出
		Vector3 ys(start.Up());
		Vector3 zs(start.Backward());
		Vector3 ye(end.Up());
		Vector3 ze(end.Backward());

		// 各姿勢ベクトル補間
		Vector3 ynew(SlerpVec(ys, ye, t));
		Vector3 znew(SlerpVec(zs, ze, t));

		// 姿勢ベクトルから行列を再算出
		Vector3 xnew(ynew.Cross(znew));
		ynew = znew.Cross(xnew);
		xnew.Normalize();
		ynew.Normalize();
		znew.Normalize();
		
		// 移動成分は線形補間
		Vector3 wnew = Lerp(start.Translation(), end.Translation(), t);
		Vector4 x4(xnew.x, xnew.y, xnew.z, 0.0f);
		Vector4 y4(ynew.x, ynew.y, ynew.z, 0.0f);
		Vector4 z4(znew.x, znew.y, znew.z, 0.0f);
		Vector4 w4(wnew.x, wnew.y, wnew.z, 1.0f);
		return Matrix(x4, y4, z4, w4);
	}
#endif

#pragma endregion

#pragma region MatrixUtil

/// <summary>
/// 回転順
/// </summary>
enum ROTATIONORDER
{
	eRotOrderXYZ,
	eRotOrderXZY,
	eRotOrderYZX,
	eRotOrderYXZ,
	eRotOrderZXY,
	eRotOrderZYX,
};

inline mat4 CreateRotationEuler(const vec3& euler, ROTATIONORDER rotOrder = eRotOrderXYZ)
{
//	Matrix m(Matrix::Identity);
	vec3 r0, r1, r2;
	float cx = Cos(euler.x);
	float sx = Sin(euler.x);
	float cy = Cos(euler.y);
	float sy = Sin(euler.y);
	float cz = Cos(euler.z);
	float sz = Sin(euler.z);
	if (rotOrder == eRotOrderXYZ)
	{
		r0 = vec3(
			cy * cz,
			cy * sz,
			-sy);
		r1 = vec3(
			sx * sy * cz - cx * sz,
			sx * sy * sz + cx * cz,
			sx * cy);
		r2 = vec3(
			cx * sy * cz + sx * sz,
			cx * sy * sz - sx * cz,
			cx * cy);
	}
	else if (rotOrder == eRotOrderXZY)
	{
		r0 = vec3(
			cy * cz,
			sz,
			-sy * cz);
		r1 = vec3(
			sx * sy - cx * cy * sz,
			cx * cz,
			cx * sy * sz + sx * cy);
		r2 = vec3(
			sx * cy * sz + cx * sy,
			-sx * cz,
			cx * cy - sx * sy * sz);
	}
	else if (rotOrder == eRotOrderYXZ)
	{
		r0 = vec3(
			cy * cz - sx * sy * sz,
			cy * sz + sx * sy * cz,
			-cx * sy);
		r1 = vec3(
			-cx * sz,
			cx * cz,
			sx);
		r2 = vec3(
			sx * cy * sz + sy * cz,
			sy * sz - sx * cy * cz,
			cx * cy);
	}
	else if (rotOrder == eRotOrderYZX)
	{
		r0 = vec3(
			cy * cz,
			cx * cy * sz + sx * sy,
			sx * cy * sz - cx * sy);
		r1 = vec3(
			-sz,
			cx * cz,
			sx * cz);
		r2 = vec3(
			sy * cz,
			cx * sy * sz - sx * cy,
			sx * sy * sz + cx * cy);
	}
	else if (rotOrder == eRotOrderZXY)
	{
		r0 = vec3(
			sx * sy * sz + cy * cz,
			cx * sz,
			sx * cy * sz - sy * cz);
		r1 = vec3(
			sx * sy * cz - cy * sz,
			cx * cz,
			sy * sz + sx * cy * cz);
		r2 = vec3(
			cx * sy,
			-sx,
			cx * cy);
	}
	else if (rotOrder == eRotOrderZYX)
	{
		r0 = vec3(
			cy * cz,
			cx * sz + sx * sy * cz,
			sx * sz - cx * sy * cz);
		r1 = vec3(
			-cy * sz,
			cx * cz - sx * sy * sz,
			cx * sy * sz + sx * cz);
		r2 = vec3(
			sy,
			-sx * cy,
			cx * cy);
	}
	else
	{
		// rotOrder undefined
		ASSERT(0);
	}

	return mat4(
		r0.x, r0.y, r0.z, 0.0f,
		r1.x, r1.y, r1.z, 0.0f,
		r2.x, r2.y, r2.z, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f);
}

#if 0

	inline mat3 CreateRotateXMat3(const float& rad)
	{
		mat3	m;
		float c = Cos(rad);
		float s = Sin(rad);
		m[0] = vec3(1.0f, 0.0f, 0.0f);
		m[1] = vec3(0.0f, c, s);
		m[2] = vec3(0.0f, -s, c);
		return m;
	}

	inline mat4 CreateRotateXMat4(const float& rad)
	{
		float c = Cos(rad);
		float s = Sin(rad);
		return mat4(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, c, s, 0.0f),
			vec4(0.0f, -s, c, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	inline mat3 CreateRotateYMat3(const float& rad)
	{
		mat3	m;
		float c = Cos(rad);
		float s = Sin(rad);
		m[0] = vec3(c, 0.0f, -s);
		m[1] = vec3(0.0f, 1.0f, 0.0f);
		m[2] = vec3(s, 0.0f, c);
		return m;
	}
	inline mat4 CreateRotateYMat4(const float& rad)
	{
		float c = Cos(rad);
		float s = Sin(rad);
		return mat4(
			vec4(c, 0.0f, -s, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(s, 0.0f, c, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	inline mat3 CreateRotateZMat3(const float& rad)
	{
		mat3	m;
		float c = Cos(rad);
		float s = Sin(rad);
		m[0] = vec3(c, s, 0.0f);
		m[1] = vec3(-s, c, 0.0f);
		m[2] = vec3(0.0f, 0.0f, 1.0f);
		return m;
	}

	inline mat4 CreateRotateZMat4(const float& rad)
	{
		mat3 m3(CreateRotateZMat3(rad));
		float c = Cos(rad);
		float s = Sin(rad);
		return mat4(
			vec4(c, s, 0.0f, 0.0f),
			vec4(-s, c, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	inline mat3 CreateRotateAxisMat3(const float& rad, const vec3& axis)
	{
		mat3	m;
		float c = Cos(rad);
		float s = Sin(rad);
		float t = 1.0f - c;
		vec3	a(glm::normalize(axis));
		m[0].x = t * a.x * a.x + c;
		m[0].y = t * a.x * a.y + s * a.z;
		m[0].z = t * a.z * a.x - s * a.y;
		m[1].x = t * a.x * a.y - s * a.z;
		m[1].y = t * a.y * a.y + c;
		m[1].z = t * a.z * a.y + s * a.x;
		m[2].x = t * a.x * a.z + s * a.y;
		m[2].y = t * a.y * a.z - s * a.x;
		m[2].z = t * a.z * a.z + c;
		return m;
	}

	inline mat4 CreateRotateAxisMat4(const float& rad, const vec3& axis)
	{
		mat3 m3(CreateRotateAxisMat3(rad,axis));
		return mat4(
			vec4(m3[0].x, m3[0].y, m3[0].z, 0.0f),
			vec4(m3[1].x, m3[1].y, m3[1].z, 0.0f),
			vec4(m3[2].x, m3[2].y, m3[2].z, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}


	inline mat4 CreateScaleMat4(const vec3& scale)
	{
		mat4 m(1.0f);
		m[0].x = scale.x;
		m[1].y = scale.y;
		m[2].z = scale.z;
		return m;
	}

	inline mat4 CreateRotateMat4(const quat& q)
	{
		return glm::toMat4(q);
	}

	inline mat4 CreateTranslateMat4(const vec3& translate)
	{
		mat4 m(1.0f);
		m[3].x = translate.x;
		m[3].y = translate.y;
		m[3].z = translate.z;
		return m;
	}

	inline mat3 CreateRotateMat3(const vec3& euler, ROTATIONORDER rotOrder = eRotOrderXYZ)
	{
		mat3 m(1.0f);
		float cx = Cos(euler.x);
		float sx = Sin(euler.x);
		float cy = Cos(euler.y);
		float sy = Sin(euler.y);
		float cz = Cos(euler.z);
		float sz = Sin(euler.z);
		if (rotOrder == eRotOrderXYZ)
		{
			m[0] = vec3(
				cy * cz,
				cy * sz,
				-sy);
			m[1] = vec3(
				sx * sy * cz - cx * sz,
				sx * sy * sz + cx * cz,
				sx * cy);
			m[2] = vec3(
				cx * sy * cz + sx * sz,
				cx * sy * sz - sx * cz,
				cx * cy);
		}
		else if (rotOrder == eRotOrderXZY)
		{
			m[0] = vec3(
				cy * cz,
				sz,
				-sy * cz);
			m[1] = vec3(
				sx * sy - cx * cy * sz,
				cx * cz,
				cx * sy * sz + sx * cy);
			m[2] = vec3(
				sx * cy * sz + cx * sy,
				-sx * cz,
				cx * cy - sx * sy * sz);
		}
		else if (rotOrder == eRotOrderYXZ)
		{
			m[0] = vec3(
				cy * cz - sx * sy * sz,
				cy * sz + sx * sy * cz,
				-cx * sy);
			m[1] = vec3(
				-cx * sz,
				cx * cz,
				sx);
			m[2] = vec3(
				sx * cy * sz + sy * cz,
				sy * sz - sx * cy * cz,
				cx * cy);
		}
		else if (rotOrder == eRotOrderYZX)
		{
			m[0] = vec3(
				cy * cz,
				cx * cy * sz + sx * sy,
				sx * cy * sz - cx * sy);
			m[1] = vec3(
				-sz,
				cx * cz,
				sx * cz);
			m[2] = vec3(
				sy * cz,
				cx * sy * sz - sx * cy,
				sx * sy * sz + cx * cy);
		}
		else if (rotOrder == eRotOrderZXY)
		{
			m[0] = vec3(
				sx * sy * sz + cy * cz,
				cx * sz,
				sx * cy * sz - sy * cz);
			m[1] = vec3(
				sx * sy * cz - cy * sz,
				cx * cz,
				sy * sz + sx * cy * cz);
			m[2] = vec3(
				cx * sy,
				-sx,
				cx * cy);
		}
		else if (rotOrder == eRotOrderZYX)
		{
			m[0] = vec3(
				cy * cz,
				cx * sz + sx * sy * cz,
				sx * sz - cx * sy * cz);
			m[1] = vec3(
				-cy * sz,
				cx * cz - sx * sy * sz,
				cx * sy * sz + sx * cz);
			m[2] = vec3(
				sy,
				-sx * cy,
				cx * cy);
		}
		else
		{
			// rotOrder undefined
			ASSERT(0);
		}

		return m;
	}
	inline mat4 CreateRotateMat4(const vec3& euler, ROTATIONORDER rotOrder = eRotOrderXYZ)
	{
		mat3 m3(CreateRotateMat3(euler, rotOrder));
		return mat4(
			vec4(m3[0].x, m3[0].y, m3[0].z, 0.0f),
			vec4(m3[1].x, m3[1].y, m3[1].z, 0.0f),
			vec4(m3[2].x, m3[2].y, m3[2].z, 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}
	inline quat CreateRotateQuat(const vec3& euler, ROTATIONORDER rotOrder = eRotOrderXYZ)
	{
		return glm::toQuat(CreateRotateMat3(euler, rotOrder));
	}

#endif

#if 1
#pragma region Curve
	template<typename T = vec3> struct KeyFrame
	{
		float time;
		T value;
		KeyFrame(float t, const T& v) : time(t), value(v) {}
		KeyFrame() : time(0.0f), value(0.0f) {}
	};

	template<typename T = vec3>	struct Curve
	{
		std::vector<KeyFrame<T>> keys;
		void AddKeyFrame(float time, const T& value)
		{
			keys.push_back(KeyFrame<T>(time, value));
		}
		void AddKeyFrame(const KeyFrame<T>& k)
		{
			keys.push_back(k);

		}
		bool FindKeyFrame(float time, KeyFrame<T>* s, KeyFrame<T>* e)
		{
			auto size = keys.size();
			if (size == 0)
			{
				s->time = 0.0f;
				s->value = T(0.0f);
				return false;
			}
			if (size == 1)
			{
				*s = keys[0];
				return false;
			}
			else//if (size >= 2)
			{
				for (auto i = 0u; i < size - 1; i++)
				{
					if (keys[i].time <= time && time < keys[i + 1].time)
					{
						*s = keys[i];
						*e = keys[i + 1];
						return true;
					}
				}
				return false;
			}
		}
		T Evalute(float time)
		{
			KeyFrame<T> s, e;
			if (FindKeyFrame(time, &s, &e))
			{
				float alpha = (time - s.time) / (e.time - s.time);
				return Interp<T>(s.value, e.value, alpha);
			}
			else
			{
				return s.value;
			}
		}
	};
#pragma endregion


	/// <summary>
	/// Gram-Schmidtの直交化法
	/// </summary>
	/// <param name="matrix"></param>
	/// <returns></returns>
	inline mat4 Orthogonalize(const mat4& matrix)
	{
		mat4 result = matrix;

		// 各列ベクトルを正規化
		for (int i = 0; i < 3; ++i)
		{
			auto v = glm::normalize(result[i]);
			result[i] = v;

			// 後続のベクトルから、現在のベクトル成分を差し引く
			for (int j = i + 1; j < 3; ++j)
			{
				auto projection = glm::dot(result[j], v) * v;
				result[j] -= projection;
			}
		}

		return result;
	}

	/// <summary>
	/// 合成済行列をSRTに分解
	/// </summary>
	/// <param name="mat"></param>
	/// <param name="scale">vec3のポインターだがnullptrの場合scaleは取得しない</param>
	/// <param name="rotate"></param>
	/// <param name="translate"></param>
	inline void Decompose(const mat4& mat, vec3* scale = nullptr, mat3* rotate = nullptr, vec3* translate = nullptr)
	{
		if (translate)
		{
			*translate = mat[3];
		}

		vec3 r = mat[0].xyz;
		vec3 u = mat[1].xyz;
		vec3 b = mat[2].xyz;
		vec3 s(
			glm::length(r),
			glm::length(u),
			glm::length(b));
		if (scale)
		{
			*scale = s;
		}

		if (rotate)
		{
			if (IsWithinTolerance(s.x) || IsWithinTolerance(s.y) || IsWithinTolerance(s.z) )
			{
				// 回転行列の各軸成分を正規化出来ない
				*rotate = mat3(r, u, b);
			}
			else
			{
				r = r * (1.0f / s.x);
				u = u * (1.0f / s.y);
				b = b * (1.0f / s.z);
				*rotate = mat3(r, u, b);
			}
		}
	}

	inline vec3 GetTranslation(const mat4& mat)
	{
		return mat[3].xyz;
	}
	
	inline vec3 GetScaling(const mat4& mat)
	{
		vec3 r = mat[0].xyz;
		vec3 u = mat[1].xyz;
		vec3 b = mat[2].xyz;
		return vec3(
			glm::length(r),
			glm::length(u),
			glm::length(b));
	}

	inline mat3 GetRotation(const mat4& mat, bool withNormalize = true)
	{
		vec3 r = mat[0].xyz;
		vec3 u = mat[1].xyz;
		vec3 b = mat[2].xyz;
		vec3 s(
			glm::length(r),
			glm::length(u),
			glm::length(b));
		if (withNormalize)
		{
			if (IsWithinTolerance(s.x) || IsWithinTolerance(s.y) || IsWithinTolerance(s.z))
			{
				// 回転行列の各軸成分を正規化出来ない
			}
			else
			{
				r = r * (1.0f / s.x);
				u = u * (1.0f / s.y);
				b = b * (1.0f / s.z);
			}
		}
		else
		{
		}
		return mat3(r, u, b);
	}
	
	inline mat4 CreateScaling(const vec3& scale)
	{
		return mat4(
			scale.x, 0.0f, 0.0f, 0.0f,
			0.0f, scale.y, 0.0f, 0.0f,
			0.0f, 0.0f, scale.z, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f);

	}

	inline void SetScaling(mat4* m, const vec3& scale)
	{
		vec3 r = glm::normalize((*m)[0]);
		vec3 u = glm::normalize((*m)[1]);
		vec3 b = glm::normalize((*m)[2]);
		vec3 t = (*m)[3];
		r *= scale.x;
		u *= scale.y;
		b *= scale.z;
		*m = mat4(
			r.x, r.y, r.z, 0.0f,
			u.x, u.y, u.z, 0.0f,
			b.x, b.y, b.z, 0.0f,
			t.x, t.y, t.z, 1.0f);
	}
	
	inline mat4 CreateRotation(const mat3& rotate)
	{
		return mat4(
			vec4(rotate[0], 0.0f),
			vec4(rotate[1], 0.0f),
			vec4(rotate[2], 0.0f),
			vec4(0.0f, 0.0f, 0.0f, 1.0f));
	}

	inline void SetRotation(mat4* m, const mat3& rotate)
	{
		vec3 t = (*m)[3];
		*m = mat4(
			vec4(rotate[0], 0.0f),
			vec4(rotate[1], 0.0f),
			vec4(rotate[2], 0.0f),
			vec4(t, 1.0f));
	}
	
	inline mat4 CreateTranslation(const vec3& translate)
	{
		return mat4(
			vec4(1.0f, 0.0f, 0.0f, 0.0f),
			vec4(0.0f, 1.0f, 0.0f, 0.0f),
			vec4(0.0f, 0.0f, 1.0f, 0.0f),
			vec4(translate, 1.0f));
	}

	inline void SetTranslation(mat4* m, const vec3& translate)
	{
		(*m)[3] = vec4(translate, 1.0f);
	}

	/// <summary>
	/// 3軸からオイラー角を求める
	/// ※2軸を引数で与え、もう1軸はそれぞれに垂直な軸である
	/// </summary>
	/// <param name="axisX"></param>
	/// <param name="axisY"></param>
	/// <param name="rotOrder">回転順</param>
	/// <returns>XYZの角度(rad)</returns>
	inline vec3 AxisToEuler(const vec3& axisX, const vec3& axisY, const ROTATIONORDER& rotOrder)
	{
		vec3 X = glm::normalize(axisX);
		vec3 Y = glm::normalize(axisY - glm::dot(X,axisY) * X);
		vec3 Z = glm::normalize(glm::cross(X, Y));
		vec3 euler(0.0f);

		if (rotOrder == eRotOrderZXY)
		{
			euler.z = ArcTan2(X.y, Y.y);
			euler.x = ArcSin(-Z.y);
			euler.y = ArcTan2(Z.x, Z.z);
			if (IsWithinTolerance(Cos(euler.x)))
			{
				euler.z = 0.0f;
				euler.y = (ArcTan2(-X.z, X.x));
			}
		}
		else if (rotOrder == eRotOrderXYZ)
		{
			euler.x = (ArcTan2(Y.z, Z.z));
			euler.y = (ArcSin(-X.z));
			euler.z = (ArcTan2(X.y, X.x));
			if (IsWithinTolerance(Cos((euler.y))))
			{
				euler.x = 0.0f;
				euler.z = (ArcTan2(-Y.x, Y.y));
			}
		}
		else if (rotOrder == eRotOrderXZY)
		{
			euler.x = (ArcTan2(-Z.y, Y.y));
			euler.z = (ArcSin(X.y));
			euler.y = (ArcTan2(-X.z, X.x));
			if (IsWithinTolerance(Cos((euler.z))))
			{
				euler.x = 0.0f;
				euler.y = (ArcTan2(Z.x, Z.z));
			}
		}
		else if (rotOrder == eRotOrderYXZ)
		{
			euler.y = (ArcTan2(-X.z, Z.z));
			euler.x = (ArcSin(Y.z));
			euler.z = (ArcTan2(-Y.x, Y.y));
			if (IsWithinTolerance(Cos((euler.x))))
			{
				euler.y = 0.0f;
				euler.z = (ArcTan2(X.y, X.x));
			}
		}
		else if (rotOrder == eRotOrderYZX)
		{
			euler.y = (ArcTan2(Z.x, X.x));
			euler.z = (ArcSin(-Y.x));
			euler.x = (ArcTan2(Y.z, Y.y));
			if (IsWithinTolerance(Cos((euler.z))))
			{
				euler.x = 0.0;
				euler.y = (ArcTan2(-Z.y, Z.z));
			}
		}
		else if (rotOrder == eRotOrderZYX)
		{
			euler.z = (ArcTan2(-Y.x, X.x));
			euler.y = (ArcSin(Z.x));
			euler.x = (ArcTan2(-Z.y, Z.z));
			if (IsWithinTolerance(Cos((euler.y))))
			{
				euler.z = 0.0f;
				euler.x = (ArcTan2(Y.z, Y.y));
			}
		}
		else
		{
			// rotOrder undefined
			ASSERT(0);
		}
		return euler;
	}

#pragma endregion
#endif
#pragma region Transform


	inline vec3 CalcTriangleNormal(const vec3& a, const vec3& b, const vec3& c)
	{
		auto ab = glm::normalize(b - a);
		auto ac = glm::normalize(c - a);
		return glm::normalize(glm::cross(ab, ac));
	};

#pragma endregion

#pragma region AnimCurve
	struct AnimCurveBase
	{
		AnimCurveBase() = default;
		virtual ~AnimCurveBase() = default;
		std::string name;
	};

#if 0
	struct AnimFloatCurve : public AnimCurveBase
	{
		typedef float T;
		typedef std::pair<float, T> KeyFrame;
		AnimFloatCurve() = default;
		~AnimFloatCurve() = default;
		T defaultValue;
		void SetDefaultValue(const T& v)
		{
			defaultValue = v;
		}
		std::vector<KeyFrame> keys;
		void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					keys[i].second = value;
					return;
				}
			}
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time) const
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return defaultValue;
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					return Interp(key1.second, key2.second, t);
				}
			}
			return defaultValue;
		}
	};
	struct AnimVec2Curve : public AnimCurveBase
	{
		typedef vec2 T;
		typedef std::pair<float, T> KeyFrame;
		AnimVec2Curve() = default;
		~AnimVec2Curve() = default;
		T defaultValue;
		void SetDefaultValue(const T& v)
		{
			defaultValue = v;
		}
		std::vector<KeyFrame> keys;
		void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					keys[i].second = value;
					return;
				}
			}
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time) const
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return defaultValue;
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					return Interp(key1.second, key2.second, t);
				}
			}
			return defaultValue;
		}
	};
	struct AnimVec3Curve : public AnimCurveBase
	{
		typedef vec3 T;
		typedef std::pair<float, T> KeyFrame;
		AnimVec3Curve() = default;
		~AnimVec3Curve() = default;
		T defaultValue;
		void SetDefaultValue(const T& v)
		{
			defaultValue = v;
		}
		std::vector<KeyFrame> keys;
		void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					keys[i].second = value;
					return;
				}
			}
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time) const
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return defaultValue;
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					//return key1.second;
					return Interp(key1.second, key2.second, t);
				}
			}
			return defaultValue;
		}
	};
	struct AnimVec4Curve : public AnimCurveBase
	{
		typedef vec4 T;
		typedef std::pair<float, T> KeyFrame;
		AnimVec4Curve() = default;
		~AnimVec4Curve() = default;
		T defaultValue;
		void SetDefaultValue(const T& v)
		{
			defaultValue = v;
		}
		std::vector<KeyFrame> keys;
		void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					keys[i].second = value;
					return;
				}
			}
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time) const
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return defaultValue;
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					return Interp(key1.second, key2.second, t);
				}
			}
			return defaultValue;
		}
	};
	struct AnimQuatCurve : public AnimCurveBase
	{
		typedef quat T;
		typedef std::pair<float, T> KeyFrame;
		AnimQuatCurve() = default;
		AnimQuatCurve(const AnimVec3Curve& src)
		{
			name = src.name;
			SetDefaultValue(quat(src.defaultValue));
			for (const auto& itr : src.keys)
			{
				auto& v = itr.second;
				vec3 rot = glm::radians(v);

				keys.push_back(KeyFrame(itr.first, quat(rot)));//todo:XYZの順番でよいのか
			}
		}
		~AnimQuatCurve() = default;
		T defaultValue;
		void SetDefaultValue(const T& v)
		{
			defaultValue = v;
		}
		std::vector<KeyFrame> keys;
		void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					keys[i].second = value;
					return;
				}
			}
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time) const
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return defaultValue;
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					return glm::slerp(key1.second, key2.second, t);
				}
			}
			return defaultValue;
		}
	};
#endif
	template<typename T>
	struct AnimCurve : public AnimCurveBase
	{
		typedef std::pair<float, T> KeyFrame;
		AnimCurve() = default;
		virtual ~AnimCurve()
		{
			keys.clear();
		};
		T defaultValue;
		virtual void SetDefaultValue(const T& v)
		{
			defaultValue = v;
		}
		std::vector<KeyFrame> keys;
		virtual void Insert(float time, const T& value)
		{
			auto keyCount = keys.size();
			for (auto i = 0u; i < keyCount; i++)
			{
				if (keys[i].first == time)
				{
					//時刻が同じなので値上書き
					keys[i].second = value;
					return;
				}
			}
			//追加
			keys.push_back(KeyFrame(time, value));
		}
		T Evaluate(const float& time) const
		{
			auto keyCount = keys.size();
			if (keyCount == 0)
			{
				return defaultValue;
			}
			for (auto i = 0u; i < keyCount - 1; i++)
			{
				const auto& key1 = keys[i];
				const auto& key2 = keys[i + 1];
				PAUSE(key1.first != key2.first);
				if (key1.first <= time && time <= key2.first)
				{
					float t = (time - key1.first) / (key2.first - key1.first);
					return Interp(key1.second, key2.second, t);
				}
			}
			return defaultValue;
		}
	};

	using AnimVec3Curve = AnimCurve<vec3>;
	using AnimQuatCurve = AnimCurve<quat>;
	inline bool Vec3CurveToQuatCurve(AnimCurve<quat>* dst, const AnimCurve<vec3>* src)
	{
		const float RAD = _PAI / 180.0f;
		dst->name = src->name;
		dst->SetDefaultValue(quat(src->defaultValue * RAD));
		for (const auto& itr : src->keys)
		{
			auto& v = itr.second;
			dst->Insert(itr.first, quat(v * RAD));
		}
		return true;
	}

#pragma endregion
#if 1
	struct Ray
	{
		vec3 pos;
		vec3 dir;

		Ray() noexcept : pos(0.0f, 0.0f, 0.0f), dir(0.0f, 0.0f, 1.0f) {}
		Ray(const vec3& Pos, const vec3& Dir) noexcept : pos(Pos), dir(Dir) {}
	};

	struct LineSegment
	{
		vec3 start;
		vec3 end;

		LineSegment() noexcept : start(0.0f), end(0.0f) {}
		LineSegment(const vec3& S, const vec3& E) noexcept : start(S), end(E) {}
	};

	//平面の法線ベクトルをn=(a,b,c)とし、平面上の1点をP=(x0,y0,z0)P=(x0​,y0​,z0​)
	//d=−(ax0​+by0​+cz0​)として、4次元ベクトルv=(a,b,c,d)v=(a,b,c,d)
	class Plane : public vec4
	{
	public:
		inline Plane() noexcept {}
		inline Plane(const vec4& V) noexcept : vec4(V) {}

		/// <summary>
		/// それぞれの成分を受け取るコンストラクタ
		/// </summary>
		/// <param name="X"></param>
		/// <param name="Y"></param>
		/// <param name="Z"></param>
		/// <param name="W"></param>
		inline Plane(const float& X, const float& Y, const float& Z, const float& W) : vec4(X, Y, Z, W) {}

		/// <summary>
		/// 法線とWを受け取るコンストラクタ
		/// </summary>
		/// <param name="Normal">法線</param>
		/// <param name="W"></param>
		inline Plane(const vec3& Normal, const float& W) : vec4(Normal.x, Normal.y, Normal.z, W) {}

		/// <summary>
		/// 点と法線を受け取るコンストラクタ
		/// </summary>
		/// <param name="Base">点</param>
		/// <param name="Normal">法線</param>
		inline Plane(const vec3& Base, const vec3& Normal)
		{
			x = Normal.x;
			y = Normal.y;
			z = Normal.z;
			w = -(Base.x * Normal.x + Base.y * Normal.y + Base.z * Normal.z);
		}

		/// <summary>
		/// 平面上の３点を受け取るコンストラクタ
		/// </summary>
		/// <param name="A"></param>
		/// <param name="B"></param>
		/// <param name="C"></param>
		inline Plane(const vec3& A, const vec3& B, const vec3& C)
		{
			auto BA = glm::normalize(A - B);
			auto CA = glm::normalize(A - C);
			auto N = glm::normalize(glm::cross(BA, CA));
			w = -(N.x * A.x + N.y * A.y + N.z * A.z);
		}

		inline vec3 GetNormal() const
		{
			return vec3(x, y, z);
		}

		inline vec3 GetBase() const
		{
			return vec3(x * w, y * w, z * w);
		}
	};

	inline bool RayPlaneIntersection(const Ray& ray, const MyMath::Plane& plane, vec3* result)
	{
		const auto planeNormal = plane.GetNormal();
		const auto planeOrigin = plane.GetBase();

		const float denom = glm::dot(ray.dir, planeNormal);
		if (MyMath::IsWithinTolerance(denom))
		{
			return false;
		}
		const float distance = glm::dot(planeOrigin - ray.pos, planeNormal) / denom;
		*result = ray.pos + ray.dir * distance;
		return true;
	}

	inline static float calcDistPointAndPlane(const vec3& pt, const Plane& plane)
	{
		auto n = plane.GetNormal();
		auto b = plane.GetBase();
		return Abs(glm::dot(n, pt - b));
	}

	inline static float calcDistPointAndPlane(const vec3& pt, const vec3& v0, const vec3& v1, const vec3& v2)
	{
		vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v1));
		vec3 g = (v0 + v1 + v2) / 3.0f;
		return Abs(glm::dot(n, pt - g));
	}


	inline static bool detectPointAndPolygon(const vec3& pt, const vec3& v0, const vec3& v1, const vec3& v2)
	{
		vec3 n = glm::normalize(glm::cross(v1 - v0, v2 - v1));

		vec3 n0 = glm::normalize(glm::cross(v1 - v0, pt - v1));
		vec3 n1 = glm::normalize(glm::cross(v2 - v1, pt - v2));
		vec3 n2 = glm::normalize(glm::cross(v0 - v2, pt - v0));

		if ((1.0f - glm::dot(n, n0)) > _E) return false;
		if ((1.0f - glm::dot(n, n1)) > _E) return false;
		if ((1.0f - glm::dot(n, n2)) > _E) return false;

		return true;
	}

	/*
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

// 直線と平面の交差を計算する関数
bool intersection(const vec3& linePoint, const vec3& lineDirection,
				  const vec4& plane, vec3& intersectionPoint) {
	// 直線の方向ベクトルが平行であるかをチェック
	float denom = dot(plane.xyz(), lineDirection);
	if (abs(denom) < 0.0001f) {
		// 直線と平面が平行なので交差しない
		return false;
	}

	// 直線の方向ベクトルと平面の法線ベクトルの内積を計算
	float t = -(dot(plane.xyz(), linePoint) + plane.w) / denom;

	// 直線上の点を計算
	intersectionPoint = linePoint + t * lineDirection;

	return true;
}

int main() {
	// 直線のパラメータ
	vec3 linePoint(0.0f, 0.0f, 0.0f);
	vec3 lineDirection(1.0f, 1.0f, 1.0f);

	// 平面のパラメータ (ax + by + cz + d = 0)
	vec4 plane(1.0f, 2.0f, 3.0f, -6.0f);

	// 交点を格納する変数
	vec3 intersectionPoint;

	// 交差計算
	if (intersection(linePoint, lineDirection, plane, intersectionPoint)) {
		std::cout << "Intersection point: (" << intersectionPoint.x << ", "
				  << intersectionPoint.y << ", " << intersectionPoint.z << ")" << std::endl;
	} else {
		std::cout << "The line and the plane are parallel, so they do not intersect." << std::endl;
	}

	return 0;
}

*/
#endif

#pragma region MersenneTwister
	struct MersenneTwister
	{
		/* Period parameters */
		static const int32_t MT_N_ = 624;
		static const int32_t MT_M_ = 397;
		static const uint32_t MATRIX_A_ = 0x9908b0dful;
		static const uint32_t UPPER_MASK_ = 0x80000000ul;
		static const uint32_t LOWER_MASK_ = 0x7ffffffful;

		uint32_t mt_[MT_N_] = { 0 }; /* the array for the state vector  */
		int32_t mti_ = MT_N_ + 1; /* mti_==MT_N_+1 means mt_[MT_N_] is not initialized */

		MersenneTwister() {}
		MersenneTwister(uint32_t seed) { Init(seed); }
		MersenneTwister(const MersenneTwister& rhs)
		{
			memcpy_s(mt_, sizeof(mt_), rhs.mt_, sizeof(mt_));
			mti_ = rhs.mti_;
		}
		
		void Save(std::vector<uint32_t>* info, int32_t* index) const
		{
			ASSERT(info);
			ASSERT(index);
			info->clear();
			info->reserve(numof(mt_));
			for (auto i : mt_)
			{
				info->push_back(i);
			}
			*index = mti_;
		}
		
		bool Restore(const std::vector<uint32_t> info, int32_t index)
		{
			if (info.size() != MT_N_)
			{
				return false;
			}
			for (int32_t i = 0; i < MT_N_; i++)
			{
				mt_[i] = info[i];
			}
			mti_ = index;
			return true;
		}

		/* initializes mt_[MT_N_] with a seed */
		void Init(uint32_t seed)
		{
			mt_[0] = seed & 0xfffffffful;
			for (mti_ = 1; mti_ < MT_N_; mti_++)
			{
				mt_[mti_] = (1812433253ul * (mt_[mti_ - 1] ^ (mt_[mti_ - 1] >> 30)) + mti_);
				mt_[mti_] &= 0xfffffffful;
			}
		}

		void InitByArray(uint32_t init_key[], int key_length)
		{
			int32_t i, j, k;
			Init(19650218ul);
			i = 1; j = 0;
			k = (MT_N_ > key_length ? MT_N_ : key_length);
			for (; k; k--) {
				mt_[i] = (mt_[i] ^ ((mt_[i - 1] ^ (mt_[i - 1] >> 30)) * 1664525ul))
					+ init_key[j] + j; /* non linear */
				mt_[i] &= 0xfffffffful; /* for WORDSIZE > 32 machines */
				i++; j++;
				if (i >= MT_N_) { mt_[0] = mt_[MT_N_ - 1]; i = 1; }
				if (j >= key_length) j = 0;
			}
			for (k = MT_N_ - 1; k; k--) {
				mt_[i] = (mt_[i] ^ ((mt_[i - 1] ^ (mt_[i - 1] >> 30)) * 1566083941ul)) - i; /* non linear */
				mt_[i] &= 0xfffffffful; /* for WORDSIZE > 32 machines */
				i++;
				if (i >= MT_N_) { mt_[0] = mt_[MT_N_ - 1]; i = 1; }
			}

			mt_[0] = 0x80000000ul; /* MSB is 1; assuring non-zero initial array */
		}

		/// <summary>
		/// 0～0xffffffffのuint32_t乱数
		/// </summary>
		/// <returns>値</returns>
		uint32_t randu(void)
		{
			uint32_t y;
			static uint32_t mag01[2] = { 0x0ul, MATRIX_A_ };

			if (mti_ >= MT_N_)
			{ /* generate N words at one time */
				int32_t kk;

				if (mti_ == MT_N_ + 1)   /* if init_genrand() has not been called, */
				{
					Init(5489ul); /* a default initial seed is used */
				}

				for (kk = 0; kk < MT_N_ - MT_M_; kk++)
				{
					y = (mt_[kk] & UPPER_MASK_) | (mt_[kk + 1] & LOWER_MASK_);
					mt_[kk] = mt_[kk + MT_M_] ^ (y >> 1) ^ mag01[y & 0x1u];
				}
				for (; kk < MT_N_ - 1; kk++)
				{
					y = (mt_[kk] & UPPER_MASK_) | (mt_[kk + 1] & LOWER_MASK_);
					mt_[kk] = mt_[kk + (MT_M_ - MT_N_)] ^ (y >> 1) ^ mag01[y & 0x1u];
				}
				y = (mt_[MT_N_ - 1] & UPPER_MASK_) | (mt_[0] & LOWER_MASK_);
				mt_[MT_N_ - 1] = mt_[MT_M_ - 1] ^ (y >> 1) ^ mag01[y & 0x1u];

				mti_ = 0;
			}

			y = mt_[mti_++];

			/* Tempering */
			y ^= (y >> 11);
			y ^= (y << 7) & 0x9d2c5680ul;
			y ^= (y << 15) & 0xefc60000ul;
			y ^= (y >> 18);

			return y;
		}

		/// <summary>
		/// 0～0x7fffffffのint32_t乱数
		/// </summary>
		/// <param name=""></param>
		/// <returns>値</returns>
		int32_t randi(void)
		{
			uint32_t t = randu() & 0x7fffffffu;
			return *reinterpret_cast<int32_t*>(&t);
		}

		/// <summary>
		/// 0～255のBYTE乱数
		/// </summary>
		/// <param name=""></param>
		/// <returns>値</returns>
		uint8_t randb(void)
		{
			return static_cast<uint32_t>(randu() & 0x00ffu);
		}

		/// <summary>
		/// 0.0f～1.0fのfloat乱数
		/// </summary>
		/// <returns>値</returns>
		float randf(void)
		{
			Binary32 t;
#if _DEBUG
			int retry = 0;
#endif
			do {
				t.flt = static_cast<float>(randu() & 0x00ffffffu) / 16777215.0f;	// 24ビット整数の乱数→浮動小数点数へ
#if _DEBUG
				if (++retry > 10)
				{
					_CrtDbgBreak();
					return 0.0f;
				}
#endif
			} while (IsNAN(t.flt) || IsINF(t.flt) || IsNINF(t.flt));
			return t.flt;
		}

		/// <summary>
		/// 0.0～1.0のdouble乱数
		/// </summary>
		/// <returns>値</returns>
		double randd(void)
		{
			uint32_t a = randu() >> 5;
			uint32_t b = randu() >> 6;
			return (static_cast<double>(a) * 67108864.0 + static_cast<double>(b)) * (1.0 / 9007199254740992.0);
		}

		/// <summary>
		/// low～highのint32_t乱数
		/// </summary>
		/// <param name="low">最小</param>
		/// <param name="high">最大</param>
		/// <returns>値</returns>
		int32_t RandRangei(int32_t low, int32_t high)
		{
			auto range = Abs(high - low);
			ASSERT(range != 0);
			return Clamp((randi() % range) + Min(low, high), low, high);
		}
		/// <summary>
		/// low～highのfloat乱数
		/// </summary>
		/// <param name="low">最小</param>
		/// <param name="high">最大</param>
		/// <returns>値</returns>
		float RandRangef(float low, float high)
		{
			return Clamp(randf() * Abs(high - low) + Min(low, high), low, high);
		}
		
		inline vec3 RandRangeVec(const vec3& low, const vec3& high)
		{
			return vec3(
				RandRangef(low.x, high.x),
				RandRangef(low.y, high.y),
				RandRangef(low.z, high.z));
		}
	};

	inline static MersenneTwister mt;


#pragma endregion



#if 0	// util
	void DeprojectScreenToWorld(const Vector2& ScreenPos, const Rectangle& ViewRect, const Matrix& InvViewMatrix, const Matrix& InvProjectionMatrix, Vector4& out_WorldOrigin, Vector4& out_WorldDirection)
	{
		int32 PixelX = FMath::TruncToInt(ScreenPos.X);
		int32 PixelY = FMath::TruncToInt(ScreenPos.Y);

		// Get the eye position and direction of the mouse cursor in two stages (inverse transform projection, then inverse transform view).
		// This avoids the numerical instability that occurs when a view matrix with large translation is composed with a projection matrix

		// Get the pixel coordinates into 0..1 normalized coordinates within the constrained view rectangle
		const float NormalizedX = (PixelX - ViewRect.Min.X) / ((float)ViewRect.Width());
		const float NormalizedY = (PixelY - ViewRect.Min.Y) / ((float)ViewRect.Height());

		// Get the pixel coordinates into -1..1 projection space
		const float ScreenSpaceX = (NormalizedX - 0.5f) * 2.0f;
		const float ScreenSpaceY = ((1.0f - NormalizedY) - 0.5f) * 2.0f;

		// The start of the ray trace is defined to be at mousex,mousey,1 in projection space (z=1 is near, z=0 is far - this gives us better precision)
		// To get the direction of the ray trace we need to use any z between the near and the far plane, so let's use (mousex, mousey, 0.01)
		const FVector4 RayStartProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 1.0f, 1.0f);
		const FVector4 RayEndProjectionSpace = FVector4(ScreenSpaceX, ScreenSpaceY, 0.01f, 1.0f);

		// Projection (changing the W coordinate) is not handled by the FMatrix transforms that work with vectors, so multiplications
		// by the projection matrix should use homogeneous coordinates (i.e. FPlane).
		const FVector4 HGRayStartViewSpace = InvProjectionMatrix.TransformFVector4(RayStartProjectionSpace);
		const FVector4 HGRayEndViewSpace = InvProjectionMatrix.TransformFVector4(RayEndProjectionSpace);
		FVector RayStartViewSpace(HGRayStartViewSpace.X, HGRayStartViewSpace.Y, HGRayStartViewSpace.Z);
		FVector RayEndViewSpace(HGRayEndViewSpace.X, HGRayEndViewSpace.Y, HGRayEndViewSpace.Z);
		// divide vectors by W to undo any projection and get the 3-space coordinate
		if (HGRayStartViewSpace.W != 0.0f)
		{
			RayStartViewSpace /= HGRayStartViewSpace.W;
		}
		if (HGRayEndViewSpace.W != 0.0f)
		{
			RayEndViewSpace /= HGRayEndViewSpace.W;
		}
		FVector RayDirViewSpace = RayEndViewSpace - RayStartViewSpace;
		RayDirViewSpace = RayDirViewSpace.GetSafeNormal();

		// The view transform does not have projection, so we can use the standard functions that deal with vectors and normals (normals
		// are vectors that do not use the translational part of a rotation/translation)
		const FVector RayStartWorldSpace = InvViewMatrix.TransformPosition(RayStartViewSpace);
		const FVector RayDirWorldSpace = InvViewMatrix.TransformVector(RayDirViewSpace);

		// Finally, store the results in the hitcheck inputs.  The start position is the eye, and the end position
		// is the eye plus a long distance in the direction the mouse is pointing.
		out_WorldOrigin = RayStartWorldSpace;
		out_WorldDirection = RayDirWorldSpace.GetSafeNormal();
	}

	// -------------------------------------------------------------
	/// <summary>
	/// ポリゴン上に点が含まれるかを判定
	/// </summary>
	bool detectPointIsEnclosedByPolygon(const FVector& p, const FVector& v0, const FVector& v1, const FVector& v2)
	{
		FVector n = (FVector::CrossProduct(v1 - v0, v2 - v1)).GetSafeNormal();

		FVector n0 = (FVector::CrossProduct(v1 - v0, p - v1)).GetSafeNormal();
		FVector n1 = (FVector::CrossProduct(v2 - v1, p - v2)).GetSafeNormal();
		FVector n2 = (FVector::CrossProduct(v0 - v2, p - v0)).GetSafeNormal();

		if ((1.0f - FVector::DotProduct(n, n0)) > 0.001f) return false;
		if ((1.0f - FVector::DotProduct(n, n1)) > 0.001f) return false;
		if ((1.0f - FVector::DotProduct(n, n2)) > 0.001f) return false;

		return true;
	}

	// -------------------------------------------------------------
	/// <summary>
	/// 3次元座標上の線分と平面の交点座標を求める
	/// </summary>
	FVector calcIntersectionLineSegmentAndPlane(const FVector& a, const FVector& b, const FVector& v0, const FVector& v1, const FVector& v2)
	{

		float distAP = calcDistancePointAndPlane(a, v0, v1, v2);
		float distBP = calcDistancePointAndPlane(b, v0, v1, v2);

		float t = distAP / (distAP + distBP);

		return (b - a) * t + a;
	}

	// -------------------------------------------------------------
	/// <summary>
	/// ある点から平面までの距離
	/// </summary>
	float calcDistancePointAndPlane(const FVector& p, const FVector& v0, const FVector& v1, const FVector& v2)
	{
		FVector n = (FVector::CrossProduct(v1 - v0, v2 - v1)).GetSafeNormal();
		FVector g = (v0 + v1 + v2) / 3.0f;
		return FMath::Abs(FVector::DotProduct(n, p - g));
	}

	// -------------------------------------------------------------
	/// <summary>
	/// 3次元座標上の線分と平面が交差してるかを判定
	/// </summary>
	bool detectCollisionLineSegmentAndPlane(const FVector& a, const FVector& b, const FVector& v0, const FVector& v1, const FVector& v2)
	{

		FVector n = (FVector::CrossProduct(v1 - v0, v2 - v1)).GetSafeNormal();
		FVector g = (v0 + v1 + v2) / 3.0f;

		if (FVector::DotProduct((a - g), n) * FVector::DotProduct((b - g), n) <= 0)
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	inline vec3 RayPlaneIntersection(const vec3& RayOrigin, const vec3& RayDirection, const Plane& Plane)
	{
		const vec3 PlaneNormal = vec3(Plane.X, Plane.Y, Plane.Z);
		const vec3 PlaneOrigin = PlaneNormal * Plane.W;

		const float Distance = vec3::DotProduct((PlaneOrigin - RayOrigin), PlaneNormal) / FVector::DotProduct(RayDirection, PlaneNormal);
		return RayOrigin + RayDirection * Distance;
	}
#endif


	class Rect : public RECT
	{
	public:
		Rect()
		{
			left = 0;
			top = 0;
			right = 0;
			bottom = 0;
		}

		Rect(long l, long t, long r, long b)
		{
			left = l;
			top = t;
			right = r;
			bottom = b;
		}

		~Rect() {}

//		inline static Rect XYWH(long x, long y, long w, long h)
//		{
//			return Rect(x, y, w - 1, h - 1);
//		}

		inline std::string ToString() const
		{
			std::string result;
			std::stringstream ss;
			ss << "{" << left << "," << top << "," << right << "," << bottom << "}";
			ss >> result;
			return result;
		}

		inline long Width() const
		{
			return Abs(right - left) + 1;
		}

		inline long Height() const
		{
			return Abs(bottom - top) + 1;
		}

		inline vec2 InvScale() const
		{
			if (Width() && Height())
			{
				return vec2(1.0f / (float)Width(), 1.0f / (float)Height());
			}
			return vec2(0.0f);
		}

		inline vec2 LeftTop() const
		{
			return vec2((float)left, (float)top);
		}

		inline vec2 RightBottom() const
		{
			return vec2((float)right, (float)bottom);
		}
		inline vec2 RightTop() const
		{
			return vec2((float)right, (float)top);
		}

		inline vec2 LeftBottom() const
		{
			return vec2((float)left, (float)bottom);
		}
	};

	class XYWH : public Rect
	{
	public:
		XYWH(long x, long y, uint32_t w, uint32_t h) : Rect(x, y, x + w - 1, y + h - 1)
		{}
	};
}
using MyMath::Rect;
#if 0
template< class Func >
class MatrixHelper
{
public:
	MatrixHelper(Func func)
		:_func(func)
	{
		// スタックのデフォルト基底階層に単位行列をロードしておく
		_matrix_stack.push(glm::mat4(1.0));
	}
	// matrix support
	void LoadIdentity(void) { LoadMatrix(glm::mat4(1.0)); }
	void LoadMatrix(const glm::mat4& mat)
	{
		_matrix_stack.top() = mat;
		// モデルマトリックス送信
		//glUniformMatrix4fv(_uniform_modelmatrix_id, 1, 0,
		//	glm::value_ptr(_matrix_stack.top()));
		_func(_matrix_stack.top());
		return;
	}
	void MultMatrix(const glm::mat4& mat)
	{
		_matrix_stack.top() *= mat;
		// モデルマトリックス送信
		//glUniformMatrix4fv(_uniform_modelmatrix_id, 1, 0,
		//	glm::value_ptr(_matrix_stack.top()));
		_func(_matrix_stack.top());
		return;
	}
	void PushMatrix(void)
	{
		_matrix_stack.push(_matrix_stack.top());
		return;
	}
	void PopMatrix(void)
	{
		_matrix_stack.pop();
		// モデルマトリックス送信
		//glUniformMatrix4fv(_uniform_modelmatrix_id, 1, 0,
		//	glm::value_ptr(_matrix_stack.top()));
		_func(_matrix_stack.top());
	}
	const glm::mat4& GetTopMatrix(void)
	{
		return _matrix_stack.top();
	}
public:
	std::stack< glm::mat4 > _matrix_stack;
	Func _func;
};
#endif

