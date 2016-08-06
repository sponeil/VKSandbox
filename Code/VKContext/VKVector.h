// VKVector.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKVector_h__
#define __VKVector_h__

#include "VKMath.h"

#pragma warning(disable : 4127)
#pragma warning(disable : 4146)

namespace VK {

// Forward declarations (for converting between different size vectors)
template <class T> class Vector1;
template <class T> class Vector2;
template <class T> class Vector3;
template <class T> class Vector4;

#pragma pack(1)
/// Encapsulates a 1D vector.
/// It is used when assigning 1D attributes in VK::Vertex.
template <class T> class Vector1 {
public:
	T x;

	/// @name Constructors
	//@{
	Vector1() {}
	Vector1(T a) { this->x = a; }
	Vector1(T *p) { x = p[0]; }
	Vector1(const Vector1<T> &v) { x = v.x; }
	//@}

	/// Use to determine the number of channels in this vector (for when it's used as a template argument)
	int getChannels() { return 1; }

	/// @name Casting and unary operators
	//@{
	operator Vector1<float>() const		{ return Vector1<float>((float)x); }
	operator Vector1<double>() const	{ return Vector1<double>((double)x); }
	T &operator[](unsigned int n)			{ if(n > 0) VKLogException("Vector1[%u] - Invalid index", n); return (&x)[n]; }
	const T &operator[](unsigned int n) const { if(n > 0) VKLogException("Vector1[%u] - Invalid index", n); return (&x)[n]; }
	Vector1<T> operator-() const		{ return Vector1<T>(-x); }
	//@}

	/// @name Arithmetic operators (vector with scalar)
	//@{
	Vector1<T> operator+(const T t) const	{ return Vector1<T>(x+t); }
	Vector1<T> operator-(const T t) const	{ return Vector1<T>(x-t); }
	Vector1<T> operator*(const T t) const	{ return Vector1<T>(x*t); }
	Vector1<T> operator/(const T t) const	{ return Vector1<T>(x/t); }
	void operator+=(const T t)				{ x += t; }
	void operator-=(const T t)				{ x -= t; }
	void operator*=(const T t)				{ x *= t; }
	void operator/=(const T t)				{ x /= t; }
	//@}

	/// @name Arithmetic operators (vector with vector)
	//@{
	Vector1<T> operator+(const Vector1<T> &v) const	{ return Vector1<T>(x+v.x); }
	Vector1<T> operator-(const Vector1<T> &v) const	{ return Vector1<T>(x-v.x); }
	Vector1<T> operator*(const Vector1<T> &v) const	{ return Vector1<T>(x*v.x); }
	Vector1<T> operator/(const Vector1<T> &v) const	{ return Vector1<T>(x/v.x); }
	void operator+=(const Vector1<T> &v)			{ x += v.x; }
	void operator-=(const Vector1<T> &v)			{ x -= v.x; }
	void operator*=(const Vector1<T> &v)			{ x *= v.x; }
	void operator/=(const Vector1<T> &v)			{ x /= v.x; }
	//@}

	/// @name Common vector operations
	//@{
	T mag2() const									{ return x*x; }
	T mag() const									{ return x; }
	T dist2(const Vector1<T> &v) const				{ return (*this - v).mag2(); }
	T dist(const Vector1<T> &v) const				{ return (*this - v).mag(); }
	T dot(const Vector1<T> &v) const				{ return x*v.x; }
	Vector1<T> midpoint(const Vector1<T> &v) const	{ return Vector1<T>((*this - v) / (T)2 + v); }
	Vector1<T> average(const Vector1<T> &v) const	{ return Vector1<T>((*this + v) / (T)2); }
	Vector1<T> normalize() const					{ return *this / mag(); }
	//@}

	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz + sprintf(psz, "v[%f]", (float)this->x);
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
	void from_s(const char *psz) {
		float a;
		std::sscanf(psz, "v[%f]", &a);
		x = (T)a;
		dir = dir.normalize();
	}

	/// @name Swizzle operators
	//@{
	Vector2<T> xx() const;
	Vector3<T> xxx() const;
	Vector4<T> xxxx() const;
	//@}
};

/// Encapsulates a 2D vector.
/// It is used when assigning 2D attributes in VK::Vertex,
/// but can also be used for 2D transformations.
template <class T> class Vector2 : public Vector1<T> {
public:
	T y;

	/// @name Constructors
	//@{
	Vector2() : Vector1<T>() {}
	Vector2(T a) : Vector1<T>(a) {}
	Vector2(T a, T b) : Vector1<T>(a) { this->y = b;}
	Vector2(T *p) { x = p[0]; y = p[1]; }
	Vector2(const Vector1<T> &v, T b) : Vector1<T>(v) { y = b; }
	Vector2(const Vector2<T> &v) : Vector1<T>(v) { y = v.y; }
	//@}

	/// Use to determine the number of channels in this vector (for when it's used as a template argument)
	int getChannels() { return 2; }

	/// @name Casting and unary operators
	//@{
	operator Vector2<float>() const		{ return Vector2<float>((float)this->x, (float)this->y); }
	operator Vector2<double>() const	{ return Vector2<double>((double)this->x, (double)this->y); }
	T &operator[](unsigned int n)			{ if(n > 1) VKLogException("Vector2[%u] - Invalid index", n); return (&this->x)[n]; }
	const T &operator[](unsigned int n) const { if(n > 1) VKLogException("Vector2[%u] - Invalid index", n); return (&this->x)[n]; }
	Vector2<T> operator-() const		{ return Vector2<T>(-this->x, -this->y); }
	//@}

	/// @name Arithmetic operators (vector with scalar)
	//@{
	Vector2<T> operator+(const T t) const	{ return Vector2<T>(this->x+t, this->y+t); }
	Vector2<T> operator-(const T t) const	{ return Vector2<T>(this->x-t, this->y-t); }
	Vector2<T> operator*(const T t) const	{ return Vector2<T>(this->x*t, this->y*t); }
	Vector2<T> operator/(const T t) const	{ return Vector2<T>(this->x/t, this->y/t); }
	void operator+=(const T t)				{ this->x += t; this->y += t; }
	void operator-=(const T t)				{ this->x -= t; this->y -= t; }
	void operator*=(const T t)				{ this->x *= t; this->y *= t; }
	void operator/=(const T t)				{ this->x /= t; this->y /= t; }
	//@}

	/// @name Arithmetic operators (vector with vector)
	//@{
	Vector2<T> operator+(const Vector2<T> &v) const	{ return Vector2<T>(this->x+v.x, this->y+v.y); }
	Vector2<T> operator-(const Vector2<T> &v) const	{ return Vector2<T>(this->x-v.x, this->y-v.y); }
	Vector2<T> operator*(const Vector2<T> &v) const	{ return Vector2<T>(this->x*v.x, this->y*v.y); }
	Vector2<T> operator/(const Vector2<T> &v) const	{ return Vector2<T>(this->x/v.x, this->y/v.y); }
	void operator+=(const Vector2<T> &v)			{ this->x += v.x; this->y += v.y; }
	void operator-=(const Vector2<T> &v)			{ this->x -= v.x; this->y -= v.y; }
	void operator*=(const Vector2<T> &v)			{ this->x *= v.x; this->y *= v.y; }
	void operator/=(const Vector2<T> &v)			{ this->x /= v.x; this->y /= v.y; }
	//@}

	/// @name Common vector operations
	//@{
	T mag2() const									{ return this->x*this->x + this->y*this->y; }
	T mag() const									{ return (T)sqrt(mag2()); }
	T dist2(const Vector2<T> &v) const				{ return (*this - v).mag2(); }
	T dist(const Vector2<T> &v) const				{ return (*this - v).mag(); }
	T dot(const Vector2<T> &v) const				{ return this->x*v.x + this->y*v.y; }
	Vector2<T> midpoint(const Vector2<T> &v) const	{ return Vector2<T>((*this - v) / (T)2 + v); }
	Vector2<T> average(const Vector2<T> &v) const	{ return Vector2<T>((*this + v) / (T)2); }
	Vector2<T> normalize() const					{ return *this / mag(); }
	//@}

	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz + sprintf(psz, "v[%f, %f]", (float)this->x, (float)this->y);
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
	void from_s(const char *psz) {
		float a, b;
		std::sscanf(psz, "v[%f, %f]", &a, &b);
		x = (T)a; y = (T)b;
		dir = dir.normalize();
	}

	/// @name Operator overloads for common vector operations
	//@{
	/// Pointer dereference for magnitude squared?
	T operator*() const								{ return mag2(); }
	/// One's complement for magnitude?
	T operator~() const								{ return mag(); }
	/// Bit-wise or for dot?
	T operator|(const Vector2<T> &v) const			{ return dot(v); }
	/// Bit-wise and for dist?
	T operator&(const Vector2<T> &v) const			{ return dist(v); }
	//@}

	/// @name Swizzle operators
	//@{
	Vector2<T> xy() const { return Vector2<T>(this->x, this->y); }
	Vector2<T> yx() const { return Vector2<T>(this->y, this->x); }
	Vector2<T> yy() const { return Vector2<T>(this->y, this->y); }
	Vector3<T> xxy() const;
	Vector3<T> xyx() const;
	Vector3<T> xyy() const;
	Vector3<T> yxx() const;
	Vector3<T> yxy() const;
	Vector3<T> yyx() const;
	Vector3<T> yyy() const;
	Vector4<T> xxxy() const;
	Vector4<T> xxyx() const;
	Vector4<T> xxyy() const;
	Vector4<T> xyxx() const;
	Vector4<T> xyxy() const;
	Vector4<T> xyyx() const;
	Vector4<T> xyyy() const;
	Vector4<T> yxxx() const;
	Vector4<T> yxxy() const;
	Vector4<T> yxyx() const;
	Vector4<T> yxyy() const;
	Vector4<T> yyxx() const;
	Vector4<T> yyxy() const;
	Vector4<T> yyyx() const;
	Vector4<T> yyyy() const;
	//@}
};

/// Encapsulates a 3D vector.
/// It is used when assigning 3D attributes in VK::Vertex,
/// but can also be used for 3D transformations.
template <class T> class Vector3 : public Vector2<T> {
public:
	T z;
    
	/// @name Constructors
	//@{
	Vector3() : Vector2<T>() {}
	Vector3(T a) : Vector2<T>(a) {}
	Vector3(T a, T b) : Vector2<T>(a, b) {}
	Vector3(T a, T b, T c) : Vector2<T>(a, b) { this->z = c; }
	Vector3(T *p) { x = p[0]; y = p[1]; z = p[2]; }
	Vector3(const Vector1<T> &v, T b, T c) : Vector2<T>(v, b) { this->z = c; }
	Vector3(const Vector2<T> &v, T c) : Vector2<T>(v) { this->z = c; }
	Vector3(const Vector3<T> &v) : Vector2<T>(v) { this->z = v.z; }
	//@}

	/// Use to determine the number of channels in this vector (for when it's used as a template argument)
	int getChannels() { return 3; }

	/// @name Casting and unary operators
	//@{
	operator Vector3<float>() const			{ return Vector3<float>((float)this->x, (float)this->y, (float)this->z); }
	operator Vector3<double>() const		{ return Vector3<double>((double)this->x, (double)this->y, (double)this->z); }
	T &operator[](unsigned int n)			{ if(n > 2) VKLogException("Vector3[%u] - Invalid index", n); return (&this->x)[n]; }
	const T &operator[](unsigned int n) const { if(n > 2) VKLogException("Vector3[%u] - Invalid index", n); return (&this->x)[n]; }
	Vector3<T> operator-() const			{ return Vector3<T>(-this->x, -this->y, -this->z); }
	//@}

	/// @name Arithmetic operators (vector with scalar)
	//@{
	Vector3<T> operator+(const T t) const	{ return Vector3<T>(this->x+t, this->y+t, this->z+t); }
	Vector3<T> operator-(const T t) const	{ return Vector3<T>(this->x-t, this->y-t, this->z-t); }
	Vector3<T> operator*(const T t) const	{ return Vector3<T>(this->x*t, this->y*t, this->z*t); }
	Vector3<T> operator/(const T t) const	{ return Vector3<T>(this->x/t, this->y/t, this->z/t); }
	void operator+=(const T t)				{ this->x += t; this->y += t; this->z += t; }
	void operator-=(const T t)				{ this->x -= t; this->y -= t; this->z -= t; }
	void operator*=(const T t)				{ this->x *= t; this->y *= t; this->z *= t; }
	void operator/=(const T t)				{ this->x /= t; this->y /= t; this->z /= t; }
	//@}

	/// @name Arithmetic operators (vector with vector)
	//@{
	Vector3<T> operator+(const Vector3<T> &v) const	{ return Vector3<T>(this->x+v.x, this->y+v.y, this->z+v.z); }
	Vector3<T> operator-(const Vector3<T> &v) const	{ return Vector3<T>(this->x-v.x, this->y-v.y, this->z-v.z); }
	Vector3<T> operator*(const Vector3<T> &v) const	{ return Vector3<T>(this->x*v.x, this->y*v.y, this->z*v.z); }
	Vector3<T> operator/(const Vector3<T> &v) const	{ return Vector3<T>(this->x/v.x, this->y/v.y, this->z/v.z); }
	void operator+=(const Vector3<T> &v)			{ this->x += v.x; this->y += v.y; this->z += v.z; }
	void operator-=(const Vector3<T> &v)			{ this->x -= v.x; this->y -= v.y; this->z -= v.z; }
	void operator*=(const Vector3<T> &v)			{ this->x *= v.x; this->y *= v.y; this->z *= v.z; }
	void operator/=(const Vector3<T> &v)			{ this->x /= v.x; this->y /= v.y; this->z /= v.z; }
	//@}

	/// @name Common vector operations
	//@{
	T mag2() const									{ return this->x*this->x + this->y*this->y + this->z*this->z; }
	T mag() const									{ return (T)sqrt(mag2()); }
	T dist2(const Vector3<T> &v) const				{ return (*this - v).mag2(); }
	T dist(const Vector3<T> &v) const				{ return (*this - v).mag(); }
	T dot(const Vector3<T> &v) const				{ return this->x*v.x + this->y*v.y + this->z*v.z; }
	Vector3<T> cross(const Vector3<T> &v) const		{ return Vector3<T>(this->y*v.z - this->z*v.y, this->z*v.x - this->x*v.z, this->x*v.y - this->y*v.x); }
	Vector3<T> midpoint(const Vector3<T> &v) const	{ return Vector3<T>((*this - v) / (T)2 + v); }
	Vector3<T> average(const Vector3<T> &v) const	{ return Vector3<T>((*this + v) / (T)2); }
	Vector3<T> normalize() const					{ return *this / mag(); }
	//@}

	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz + sprintf(psz, "v[%f, %f, %f]", (float)this->x, (float)this->y, (float)this->z);
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
	void from_s(const char *psz) {
		float a, b, c;
		std::sscanf(psz, "v[%f, %f, %f]", &a, &b, &c);
		x = (T)a; y = (T)b; z = (T)c;
		dir = dir.normalize();
	}

	/// @name Operator overloads for common vector operations
	//@{
	/// Pointer dereference for magnitude squared?
	T operator*() const								{ return mag2(); }
	/// One's complement for magnitude?
	T operator~() const								{ return mag(); }
	/// Bit-wise or for dot?
	T operator|(const Vector3<T> &v) const			{ return dot(v); }
	/// Bit-wise and for dist?
	T operator&(const Vector3<T> &v) const			{ return dist(v); }
	/// Bit-wise xor for cross?
	Vector3<T> operator^(const Vector3<T> &v) const	{ return cross(v); }
	//@}

	/// @name Swizzle operators
	//@{
	Vector2<T> xz() const { return Vector2<T>(this->x, this->z); }
	Vector2<T> yz() const { return Vector2<T>(this->y, this->z); }
	Vector2<T> zx() const { return Vector2<T>(this->z, this->x); }
	Vector2<T> zy() const { return Vector2<T>(this->z, this->y); }
	Vector2<T> zz() const { return Vector2<T>(this->z, this->z); }
	Vector3<T> xxz() const { return Vector3<T>(this->x, this->x, this->z); }
	Vector3<T> xyz() const { return Vector3<T>(this->x, this->y, this->z); }
	Vector3<T> xzx() const { return Vector3<T>(this->x, this->z, this->x); }
	Vector3<T> xzy() const { return Vector3<T>(this->x, this->z, this->y); }
	Vector3<T> xzz() const { return Vector3<T>(this->x, this->z, this->z); }
	Vector3<T> yxz() const { return Vector3<T>(this->y, this->x, this->z); }
	Vector3<T> yyz() const { return Vector3<T>(this->y, this->y, this->z); }
	Vector3<T> yzx() const { return Vector3<T>(this->y, this->z, this->x); }
	Vector3<T> yzy() const { return Vector3<T>(this->y, this->z, this->y); }
	Vector3<T> yzz() const { return Vector3<T>(this->y, this->z, this->z); }
	Vector3<T> zxx() const { return Vector3<T>(this->z, this->x, this->x); }
	Vector3<T> zxy() const { return Vector3<T>(this->z, this->x, this->y); }
	Vector3<T> zxz() const { return Vector3<T>(this->z, this->x, this->z); }
	Vector3<T> zyx() const { return Vector3<T>(this->z, this->y, this->x); }
	Vector3<T> zyy() const { return Vector3<T>(this->z, this->y, this->y); }
	Vector3<T> zyz() const { return Vector3<T>(this->z, this->y, this->z); }
	Vector3<T> zzx() const { return Vector3<T>(this->z, this->z, this->x); }
	Vector3<T> zzy() const { return Vector3<T>(this->z, this->z, this->y); }
	Vector3<T> zzz() const { return Vector3<T>(this->z, this->z, this->z); }
	Vector4<T> xxxz() const;
	Vector4<T> xxyz() const;
	Vector4<T> xxzx() const;
	Vector4<T> xxzy() const;
	Vector4<T> xxzz() const;
	Vector4<T> xyxz() const;
	Vector4<T> xyyz() const;
	Vector4<T> xyzx() const;
	Vector4<T> xyzy() const;
	Vector4<T> xyzz() const;
	Vector4<T> xzxx() const;
	Vector4<T> xzxy() const;
	Vector4<T> xzxz() const;
	Vector4<T> xzyx() const;
	Vector4<T> xzyy() const;
	Vector4<T> xzyz() const;
	Vector4<T> xzzx() const;
	Vector4<T> xzzy() const;
	Vector4<T> xzzz() const;
	Vector4<T> yxxz() const;
	Vector4<T> yxyz() const;
	Vector4<T> yxzx() const;
	Vector4<T> yxzy() const;
	Vector4<T> yxzz() const;
	Vector4<T> yyxz() const;
	Vector4<T> yyyz() const;
	Vector4<T> yyzx() const;
	Vector4<T> yyzy() const;
	Vector4<T> yyzz() const;
	Vector4<T> yzxx() const;
	Vector4<T> yzxy() const;
	Vector4<T> yzxz() const;
	Vector4<T> yzyx() const;
	Vector4<T> yzyy() const;
	Vector4<T> yzyz() const;
	Vector4<T> yzzx() const;
	Vector4<T> yzzy() const;
	Vector4<T> yzzz() const;
	Vector4<T> zxxx() const;
	Vector4<T> zxxy() const;
	Vector4<T> zxxz() const;
	Vector4<T> zxyx() const;
	Vector4<T> zxyy() const;
	Vector4<T> zxyz() const;
	Vector4<T> zxzx() const;
	Vector4<T> zxzy() const;
	Vector4<T> zxzz() const;
	Vector4<T> zyxx() const;
	Vector4<T> zyxy() const;
	Vector4<T> zyxz() const;
	Vector4<T> zyyx() const;
	Vector4<T> zyyy() const;
	Vector4<T> zyyz() const;
	Vector4<T> zyzx() const;
	Vector4<T> zyzy() const;
	Vector4<T> zyzz() const;
	Vector4<T> zzxx() const;
	Vector4<T> zzxy() const;
	Vector4<T> zzxz() const;
	Vector4<T> zzyx() const;
	Vector4<T> zzyy() const;
	Vector4<T> zzyz() const;
	Vector4<T> zzzx() const;
	Vector4<T> zzzy() const;
	Vector4<T> zzzz() const;
	//@}
};

/// Encapsulates a 4D vector.
/// It is used when assigning 4D attributes in VK::Vertex,
/// but can also be used for 4D transformations.
template <class T> class Vector4 : public Vector3<T> {
public:
	T w;

	/// @name Constructors
	//@{
	Vector4() : Vector3<T>() {}
	Vector4(T a) : Vector3<T>(a) {}
	Vector4(T a, T b) : Vector3<T>(a, b) {}
	Vector4(T a, T b, T c) : Vector3<T>(a, b, c) {}
	Vector4(T a, T b, T c, T d) : Vector3<T>(a, b, c) { this->w = d; }
	Vector4(T *p) { x = p[0]; y = p[1]; z = p[2]; w = p[3]; }
	Vector4(const Vector1<T> &v, T b, T c, T d) : Vector3<T>(v, b, c) { this->w = d; }
	Vector4(const Vector2<T> &v, T c, T d) : Vector3<T>(v, c) { this->w = d; }
	Vector4(const Vector3<T> &v, T d) : Vector3<T>(v) { this->w = d; }
	Vector4(const Vector4<T> &v) : Vector3<T>(v) { this->w = v.w; }
	//@}

	/// Use to determine the number of channels in this vector (for when it's used as a template argument)
	int getChannels() { return 4; }

	/// @name Casting and unary operators
	//@{
	operator Vector4<float>() const		{ return Vector4<float>((float)this->x, (float)this->y, (float)this->z, (float)this->w); }
	operator Vector4<double>() const	{ return Vector4<double>((double)this->x, (double)this->y, (double)this->z, (double)this->w); }
	T &operator[](unsigned int n)			{ if(n > 3) VKLogException("Vector4[%u] - Invalid index", n); return (&this->x)[n]; }
	const T &operator[](unsigned int n) const { if(n > 3) VKLogException("Vector4[%u] - Invalid index", n); return (&this->x)[n]; }
	Vector4<T> operator-() const		{ return Vector4<T>(-this->x, -this->y, -this->z, -this->w); }
	//@}

	/// @name Arithmetic operators (vector with scalar)
	//@{
	Vector4<T> operator+(const T t) const	{ return Vector4<T>(this->x+t, this->y+t, this->z+t, this->w+t); }
	Vector4<T> operator-(const T t) const	{ return Vector4<T>(this->x-t, this->y-t, this->z-t, this->w-t); }
	Vector4<T> operator*(const T t) const	{ return Vector4<T>(this->x*t, this->y*t, this->z*t, this->w*t); }
	Vector4<T> operator/(const T t) const	{ return Vector4<T>(this->x/t, this->y/t, this->z/t, this->w/t); }
	void operator+=(const T t)				{ this->x += t; this->y += t; this->z += t; this->w += t; }
	void operator-=(const T t)				{ this->x -= t; this->y -= t; this->z -= t; this->w -= t; }
	void operator*=(const T t)				{ this->x *= t; this->y *= t; this->z *= t; this->w *= t; }
	void operator/=(const T t)				{ this->x /= t; this->y /= t; this->z /= t; this->w /= t; }
	//@}

	/// @name Arithmetic operators (vector with vector)
	//@{
	Vector4<T> operator+(const Vector4<T> &v) const	{ return Vector4<T>(this->x+v.x, this->y+v.y, this->z+v.z, this->w+v.w); }
	Vector4<T> operator-(const Vector4<T> &v) const	{ return Vector4<T>(this->x-v.x, this->y-v.y, this->z-v.z, this->w-v.w); }
	Vector4<T> operator*(const Vector4<T> &v) const	{ return Vector4<T>(this->x*v.x, this->y*v.y, this->z*v.z, this->w*v.w); }
	Vector4<T> operator/(const Vector4<T> &v) const	{ return Vector4<T>(this->x/v.x, this->y/v.y, this->z/v.z, this->w/v.w); }
	void operator+=(const Vector4<T> &v)			{ this->x += v.x; this->y += v.y; this->z += v.z; this->w += v.w; }
	void operator-=(const Vector4<T> &v)			{ this->x -= v.x; this->y -= v.y; this->z -= v.z; this->w -= v.w; }
	void operator*=(const Vector4<T> &v)			{ this->x *= v.x; this->y *= v.y; this->z *= v.z; this->w *= v.w; }
	void operator/=(const Vector4<T> &v)			{ this->x /= v.x; this->y /= v.y; this->z /= v.z; this->w /= v.w; }
	//@}

	/// @name Common vector operations
	//@{
	T mag2() const									{ return this->x*this->x + this->y*this->y + this->z*this->z + this->w*this->w; }
	T mag() const									{ return (T)sqrt(mag2()); }
	T dist2(const Vector4<T> &v) const				{ return (*this - v).mag2(); }
	T dist(const Vector4<T> &v) const				{ return (*this - v).mag(); }
	T dot(const Vector4<T> &v) const				{ return this->x*v.x + this->y*v.y + this->z*v.z + this->w*v.w; }
	Vector4<T> midpoint(const Vector4<T> &v) const	{ return Vector4<T>((*this - v) / (T)2 + v); }
	Vector4<T> average(const Vector4<T> &v) const	{ return Vector4<T>((*this + v) / (T)2); }
	Vector4<T> normalize() const					{ return *this / mag(); }
	//@}

	/// @name Operator overloads for common vector operations
	//@{
	/// Pointer dereference for magnitude squared?
	T operator*() const								{ return mag2(); }
	/// One's complement for magnitude?
	T operator~() const								{ return mag(); }
	/// Bit-wise or for dot?
	T operator|(const Vector4<T> &v) const			{ return dot(v); }
	/// Bit-wise and for dist?
	T operator&(const Vector4<T> &v) const			{ return dist(v); }
	//@}

	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz + sprintf(psz, "v[%f, %f, %f, %f]", (float)this->x, (float)this->y, (float)this->z, (float)this->w);
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
	void from_s(const char *psz) {
		float a, b, c, d;
		std::sscanf(psz, "v[%f, %f, %f, %f]", &a, &b, &c, &d);
		x = (T)a; y = (T)b; z = (T)c; w = (T)d;
		dir = dir.normalize();
	}

	/// @name Swizzle operators
	//@{
	Vector2<T> xw() const { return Vector2<T>(this->x, this->w); }
	Vector2<T> yw() const { return Vector2<T>(this->y, this->w); }
	Vector2<T> zw() const { return Vector2<T>(this->z, this->w); }
	Vector2<T> wx() const { return Vector2<T>(this->w, this->x); }
	Vector2<T> wy() const { return Vector2<T>(this->w, this->y); }
	Vector2<T> wz() const { return Vector2<T>(this->w, this->z); }
	Vector2<T> ww() const { return Vector2<T>(this->w, this->w); }
	Vector3<T> xxw() const { return Vector3<T>(this->x, this->x, this->w); }
	Vector3<T> xyw() const { return Vector3<T>(this->x, this->y, this->w); }
	Vector3<T> xzw() const { return Vector3<T>(this->x, this->z, this->w); }
	Vector3<T> xwx() const { return Vector3<T>(this->x, this->w, this->x); }
	Vector3<T> xwy() const { return Vector3<T>(this->x, this->w, this->y); }
	Vector3<T> xwz() const { return Vector3<T>(this->x, this->w, this->z); }
	Vector3<T> xww() const { return Vector3<T>(this->x, this->w, this->w); }
	Vector3<T> yxw() const { return Vector3<T>(this->y, this->x, this->w); }
	Vector3<T> yyw() const { return Vector3<T>(this->y, this->y, this->w); }
	Vector3<T> yzw() const { return Vector3<T>(this->y, this->z, this->w); }
	Vector3<T> ywx() const { return Vector3<T>(this->y, this->w, this->x); }
	Vector3<T> ywy() const { return Vector3<T>(this->y, this->w, this->y); }
	Vector3<T> ywz() const { return Vector3<T>(this->y, this->w, this->z); }
	Vector3<T> yww() const { return Vector3<T>(this->y, this->w, this->w); }
	Vector3<T> zxw() const { return Vector3<T>(this->z, this->x, this->w); }
	Vector3<T> zyw() const { return Vector3<T>(this->z, this->y, this->w); }
	Vector3<T> zzw() const { return Vector3<T>(this->z, this->z, this->w); }
	Vector3<T> zwx() const { return Vector3<T>(this->z, this->w, this->x); }
	Vector3<T> zwy() const { return Vector3<T>(this->z, this->w, this->y); }
	Vector3<T> zwz() const { return Vector3<T>(this->z, this->w, this->z); }
	Vector3<T> zww() const { return Vector3<T>(this->z, this->w, this->w); }
	Vector3<T> wxx() const { return Vector3<T>(this->w, this->x, this->x); }
	Vector3<T> wxy() const { return Vector3<T>(this->w, this->x, this->y); }
	Vector3<T> wxz() const { return Vector3<T>(this->w, this->x, this->z); }
	Vector3<T> wxw() const { return Vector3<T>(this->w, this->x, this->w); }
	Vector3<T> wyx() const { return Vector3<T>(this->w, this->y, this->x); }
	Vector3<T> wyy() const { return Vector3<T>(this->w, this->y, this->y); }
	Vector3<T> wyz() const { return Vector3<T>(this->w, this->y, this->z); }
	Vector3<T> wyw() const { return Vector3<T>(this->w, this->y, this->w); }
	Vector3<T> wzx() const { return Vector3<T>(this->w, this->z, this->x); }
	Vector3<T> wzy() const { return Vector3<T>(this->w, this->z, this->y); }
	Vector3<T> wzz() const { return Vector3<T>(this->w, this->z, this->z); }
	Vector3<T> wzw() const { return Vector3<T>(this->w, this->z, this->w); }
	Vector3<T> wwx() const { return Vector3<T>(this->w, this->w, this->x); }
	Vector3<T> wwy() const { return Vector3<T>(this->w, this->w, this->y); }
	Vector3<T> wwz() const { return Vector3<T>(this->w, this->w, this->z); }
	Vector3<T> www() const { return Vector3<T>(this->w, this->w, this->w); }
	Vector4<T> xxxw() const { return Vector4<T>(this->x, this->x, this->x, this->w); }
	Vector4<T> xxyw() const { return Vector4<T>(this->x, this->x, this->y, this->w); }
	Vector4<T> xxzw() const { return Vector4<T>(this->x, this->x, this->z, this->w); }
	Vector4<T> xxwx() const { return Vector4<T>(this->x, this->x, this->w, this->x); }
	Vector4<T> xxwy() const { return Vector4<T>(this->x, this->x, this->w, this->y); }
	Vector4<T> xxwz() const { return Vector4<T>(this->x, this->x, this->w, this->z); }
	Vector4<T> xxww() const { return Vector4<T>(this->x, this->x, this->w, this->w); }
	Vector4<T> xyxw() const { return Vector4<T>(this->x, this->y, this->x, this->w); }
	Vector4<T> xyyw() const { return Vector4<T>(this->x, this->y, this->y, this->w); }
	Vector4<T> xyzw() const { return Vector4<T>(this->x, this->y, this->z, this->w); }
	Vector4<T> xywx() const { return Vector4<T>(this->x, this->y, this->w, this->x); }
	Vector4<T> xywy() const { return Vector4<T>(this->x, this->y, this->w, this->y); }
	Vector4<T> xywz() const { return Vector4<T>(this->x, this->y, this->w, this->z); }
	Vector4<T> xyww() const { return Vector4<T>(this->x, this->y, this->w, this->w); }
	Vector4<T> xzxw() const { return Vector4<T>(this->x, this->z, this->x, this->w); }
	Vector4<T> xzyw() const { return Vector4<T>(this->x, this->z, this->y, this->w); }
	Vector4<T> xzzw() const { return Vector4<T>(this->x, this->z, this->z, this->w); }
	Vector4<T> xzwx() const { return Vector4<T>(this->x, this->z, this->w, this->x); }
	Vector4<T> xzwy() const { return Vector4<T>(this->x, this->z, this->w, this->y); }
	Vector4<T> xzwz() const { return Vector4<T>(this->x, this->z, this->w, this->z); }
	Vector4<T> xzww() const { return Vector4<T>(this->x, this->z, this->w, this->w); }
	Vector4<T> xwxx() const { return Vector4<T>(this->x, this->w, this->x, this->x); }
	Vector4<T> xwxy() const { return Vector4<T>(this->x, this->w, this->x, this->y); }
	Vector4<T> xwxz() const { return Vector4<T>(this->x, this->w, this->x, this->z); }
	Vector4<T> xwxw() const { return Vector4<T>(this->x, this->w, this->x, this->w); }
	Vector4<T> xwyx() const { return Vector4<T>(this->x, this->w, this->y, this->x); }
	Vector4<T> xwyy() const { return Vector4<T>(this->x, this->w, this->y, this->y); }
	Vector4<T> xwyz() const { return Vector4<T>(this->x, this->w, this->y, this->z); }
	Vector4<T> xwyw() const { return Vector4<T>(this->x, this->w, this->y, this->w); }
	Vector4<T> xwzx() const { return Vector4<T>(this->x, this->w, this->z, this->x); }
	Vector4<T> xwzy() const { return Vector4<T>(this->x, this->w, this->z, this->y); }
	Vector4<T> xwzz() const { return Vector4<T>(this->x, this->w, this->z, this->z); }
	Vector4<T> xwzw() const { return Vector4<T>(this->x, this->w, this->z, this->w); }
	Vector4<T> xwwx() const { return Vector4<T>(this->x, this->w, this->w, this->x); }
	Vector4<T> xwwy() const { return Vector4<T>(this->x, this->w, this->w, this->y); }
	Vector4<T> xwwz() const { return Vector4<T>(this->x, this->w, this->w, this->z); }
	Vector4<T> xwww() const { return Vector4<T>(this->x, this->w, this->w, this->w); }
	Vector4<T> yxxw() const { return Vector4<T>(this->y, this->x, this->x, this->w); }
	Vector4<T> yxyw() const { return Vector4<T>(this->y, this->x, this->y, this->w); }
	Vector4<T> yxzw() const { return Vector4<T>(this->y, this->x, this->z, this->w); }
	Vector4<T> yxwx() const { return Vector4<T>(this->y, this->x, this->w, this->x); }
	Vector4<T> yxwy() const { return Vector4<T>(this->y, this->x, this->w, this->y); }
	Vector4<T> yxwz() const { return Vector4<T>(this->y, this->x, this->w, this->z); }
	Vector4<T> yxww() const { return Vector4<T>(this->y, this->x, this->w, this->w); }
	Vector4<T> yyxw() const { return Vector4<T>(this->y, this->y, this->x, this->w); }
	Vector4<T> yyyw() const { return Vector4<T>(this->y, this->y, this->y, this->w); }
	Vector4<T> yyzw() const { return Vector4<T>(this->y, this->y, this->z, this->w); }
	Vector4<T> yywx() const { return Vector4<T>(this->y, this->y, this->w, this->x); }
	Vector4<T> yywy() const { return Vector4<T>(this->y, this->y, this->w, this->y); }
	Vector4<T> yywz() const { return Vector4<T>(this->y, this->y, this->w, this->z); }
	Vector4<T> yyww() const { return Vector4<T>(this->y, this->y, this->w, this->w); }
	Vector4<T> yzxw() const { return Vector4<T>(this->y, this->z, this->x, this->w); }
	Vector4<T> yzyw() const { return Vector4<T>(this->y, this->z, this->y, this->w); }
	Vector4<T> yzzw() const { return Vector4<T>(this->y, this->z, this->z, this->w); }
	Vector4<T> yzwx() const { return Vector4<T>(this->y, this->z, this->w, this->x); }
	Vector4<T> yzwy() const { return Vector4<T>(this->y, this->z, this->w, this->y); }
	Vector4<T> yzwz() const { return Vector4<T>(this->y, this->z, this->w, this->z); }
	Vector4<T> yzww() const { return Vector4<T>(this->y, this->z, this->w, this->w); }
	Vector4<T> ywxx() const { return Vector4<T>(this->y, this->w, this->x, this->x); }
	Vector4<T> ywxy() const { return Vector4<T>(this->y, this->w, this->x, this->y); }
	Vector4<T> ywxz() const { return Vector4<T>(this->y, this->w, this->x, this->z); }
	Vector4<T> ywxw() const { return Vector4<T>(this->y, this->w, this->x, this->w); }
	Vector4<T> ywyx() const { return Vector4<T>(this->y, this->w, this->y, this->x); }
	Vector4<T> ywyy() const { return Vector4<T>(this->y, this->w, this->y, this->y); }
	Vector4<T> ywyz() const { return Vector4<T>(this->y, this->w, this->y, this->z); }
	Vector4<T> ywyw() const { return Vector4<T>(this->y, this->w, this->y, this->w); }
	Vector4<T> ywzx() const { return Vector4<T>(this->y, this->w, this->z, this->x); }
	Vector4<T> ywzy() const { return Vector4<T>(this->y, this->w, this->z, this->y); }
	Vector4<T> ywzz() const { return Vector4<T>(this->y, this->w, this->z, this->z); }
	Vector4<T> ywzw() const { return Vector4<T>(this->y, this->w, this->z, this->w); }
	Vector4<T> ywwx() const { return Vector4<T>(this->y, this->w, this->w, this->x); }
	Vector4<T> ywwy() const { return Vector4<T>(this->y, this->w, this->w, this->y); }
	Vector4<T> ywwz() const { return Vector4<T>(this->y, this->w, this->w, this->z); }
	Vector4<T> ywww() const { return Vector4<T>(this->y, this->w, this->w, this->w); }
	Vector4<T> zxxw() const { return Vector4<T>(this->z, this->x, this->x, this->w); }
	Vector4<T> zxyw() const { return Vector4<T>(this->z, this->x, this->y, this->w); }
	Vector4<T> zxzw() const { return Vector4<T>(this->z, this->x, this->z, this->w); }
	Vector4<T> zxwx() const { return Vector4<T>(this->z, this->x, this->w, this->x); }
	Vector4<T> zxwy() const { return Vector4<T>(this->z, this->x, this->w, this->y); }
	Vector4<T> zxwz() const { return Vector4<T>(this->z, this->x, this->w, this->z); }
	Vector4<T> zxww() const { return Vector4<T>(this->z, this->x, this->w, this->w); }
	Vector4<T> zyxw() const { return Vector4<T>(this->z, this->y, this->x, this->w); }
	Vector4<T> zyyw() const { return Vector4<T>(this->z, this->y, this->y, this->w); }
	Vector4<T> zyzw() const { return Vector4<T>(this->z, this->y, this->z, this->w); }
	Vector4<T> zywx() const { return Vector4<T>(this->z, this->y, this->w, this->x); }
	Vector4<T> zywy() const { return Vector4<T>(this->z, this->y, this->w, this->y); }
	Vector4<T> zywz() const { return Vector4<T>(this->z, this->y, this->w, this->z); }
	Vector4<T> zyww() const { return Vector4<T>(this->z, this->y, this->w, this->w); }
	Vector4<T> zzxw() const { return Vector4<T>(this->z, this->z, this->x, this->w); }
	Vector4<T> zzyw() const { return Vector4<T>(this->z, this->z, this->y, this->w); }
	Vector4<T> zzzw() const { return Vector4<T>(this->z, this->z, this->z, this->w); }
	Vector4<T> zzwx() const { return Vector4<T>(this->z, this->z, this->w, this->x); }
	Vector4<T> zzwy() const { return Vector4<T>(this->z, this->z, this->w, this->y); }
	Vector4<T> zzwz() const { return Vector4<T>(this->z, this->z, this->w, this->z); }
	Vector4<T> zzww() const { return Vector4<T>(this->z, this->z, this->w, this->w); }
	Vector4<T> zwxx() const { return Vector4<T>(this->z, this->w, this->x, this->x); }
	Vector4<T> zwxy() const { return Vector4<T>(this->z, this->w, this->x, this->y); }
	Vector4<T> zwxz() const { return Vector4<T>(this->z, this->w, this->x, this->z); }
	Vector4<T> zwxw() const { return Vector4<T>(this->z, this->w, this->x, this->w); }
	Vector4<T> zwyx() const { return Vector4<T>(this->z, this->w, this->y, this->x); }
	Vector4<T> zwyy() const { return Vector4<T>(this->z, this->w, this->y, this->y); }
	Vector4<T> zwyz() const { return Vector4<T>(this->z, this->w, this->y, this->z); }
	Vector4<T> zwyw() const { return Vector4<T>(this->z, this->w, this->y, this->w); }
	Vector4<T> zwzx() const { return Vector4<T>(this->z, this->w, this->z, this->x); }
	Vector4<T> zwzy() const { return Vector4<T>(this->z, this->w, this->z, this->y); }
	Vector4<T> zwzz() const { return Vector4<T>(this->z, this->w, this->z, this->z); }
	Vector4<T> zwzw() const { return Vector4<T>(this->z, this->w, this->z, this->w); }
	Vector4<T> zwwx() const { return Vector4<T>(this->z, this->w, this->w, this->x); }
	Vector4<T> zwwy() const { return Vector4<T>(this->z, this->w, this->w, this->y); }
	Vector4<T> zwwz() const { return Vector4<T>(this->z, this->w, this->w, this->z); }
	Vector4<T> zwww() const { return Vector4<T>(this->z, this->w, this->w, this->w); }
	Vector4<T> wxxx() const { return Vector4<T>(this->w, this->x, this->x, this->x); }
	Vector4<T> wxxy() const { return Vector4<T>(this->w, this->x, this->x, this->y); }
	Vector4<T> wxxz() const { return Vector4<T>(this->w, this->x, this->x, this->z); }
	Vector4<T> wxxw() const { return Vector4<T>(this->w, this->x, this->x, this->w); }
	Vector4<T> wxyx() const { return Vector4<T>(this->w, this->x, this->y, this->x); }
	Vector4<T> wxyy() const { return Vector4<T>(this->w, this->x, this->y, this->y); }
	Vector4<T> wxyz() const { return Vector4<T>(this->w, this->x, this->y, this->z); }
	Vector4<T> wxyw() const { return Vector4<T>(this->w, this->x, this->y, this->w); }
	Vector4<T> wxzx() const { return Vector4<T>(this->w, this->x, this->z, this->x); }
	Vector4<T> wxzy() const { return Vector4<T>(this->w, this->x, this->z, this->y); }
	Vector4<T> wxzz() const { return Vector4<T>(this->w, this->x, this->z, this->z); }
	Vector4<T> wxzw() const { return Vector4<T>(this->w, this->x, this->z, this->w); }
	Vector4<T> wxwx() const { return Vector4<T>(this->w, this->x, this->w, this->x); }
	Vector4<T> wxwy() const { return Vector4<T>(this->w, this->x, this->w, this->y); }
	Vector4<T> wxwz() const { return Vector4<T>(this->w, this->x, this->w, this->z); }
	Vector4<T> wxww() const { return Vector4<T>(this->w, this->x, this->w, this->w); }
	Vector4<T> wyxx() const { return Vector4<T>(this->w, this->y, this->x, this->x); }
	Vector4<T> wyxy() const { return Vector4<T>(this->w, this->y, this->x, this->y); }
	Vector4<T> wyxz() const { return Vector4<T>(this->w, this->y, this->x, this->z); }
	Vector4<T> wyxw() const { return Vector4<T>(this->w, this->y, this->x, this->w); }
	Vector4<T> wyyx() const { return Vector4<T>(this->w, this->y, this->y, this->x); }
	Vector4<T> wyyy() const { return Vector4<T>(this->w, this->y, this->y, this->y); }
	Vector4<T> wyyz() const { return Vector4<T>(this->w, this->y, this->y, this->z); }
	Vector4<T> wyyw() const { return Vector4<T>(this->w, this->y, this->y, this->w); }
	Vector4<T> wyzx() const { return Vector4<T>(this->w, this->y, this->z, this->x); }
	Vector4<T> wyzy() const { return Vector4<T>(this->w, this->y, this->z, this->y); }
	Vector4<T> wyzz() const { return Vector4<T>(this->w, this->y, this->z, this->z); }
	Vector4<T> wyzw() const { return Vector4<T>(this->w, this->y, this->z, this->w); }
	Vector4<T> wywx() const { return Vector4<T>(this->w, this->y, this->w, this->x); }
	Vector4<T> wywy() const { return Vector4<T>(this->w, this->y, this->w, this->y); }
	Vector4<T> wywz() const { return Vector4<T>(this->w, this->y, this->w, this->z); }
	Vector4<T> wyww() const { return Vector4<T>(this->w, this->y, this->w, this->w); }
	Vector4<T> wzxx() const { return Vector4<T>(this->w, this->z, this->x, this->x); }
	Vector4<T> wzxy() const { return Vector4<T>(this->w, this->z, this->x, this->y); }
	Vector4<T> wzxz() const { return Vector4<T>(this->w, this->z, this->x, this->z); }
	Vector4<T> wzxw() const { return Vector4<T>(this->w, this->z, this->x, this->w); }
	Vector4<T> wzyx() const { return Vector4<T>(this->w, this->z, this->y, this->x); }
	Vector4<T> wzyy() const { return Vector4<T>(this->w, this->z, this->y, this->y); }
	Vector4<T> wzyz() const { return Vector4<T>(this->w, this->z, this->y, this->z); }
	Vector4<T> wzyw() const { return Vector4<T>(this->w, this->z, this->y, this->w); }
	Vector4<T> wzzx() const { return Vector4<T>(this->w, this->z, this->z, this->x); }
	Vector4<T> wzzy() const { return Vector4<T>(this->w, this->z, this->z, this->y); }
	Vector4<T> wzzz() const { return Vector4<T>(this->w, this->z, this->z, this->z); }
	Vector4<T> wzzw() const { return Vector4<T>(this->w, this->z, this->z, this->w); }
	Vector4<T> wzwx() const { return Vector4<T>(this->w, this->z, this->w, this->x); }
	Vector4<T> wzwy() const { return Vector4<T>(this->w, this->z, this->w, this->y); }
	Vector4<T> wzwz() const { return Vector4<T>(this->w, this->z, this->w, this->z); }
	Vector4<T> wzww() const { return Vector4<T>(this->w, this->z, this->w, this->w); }
	Vector4<T> wwxx() const { return Vector4<T>(this->w, this->w, this->x, this->x); }
	Vector4<T> wwxy() const { return Vector4<T>(this->w, this->w, this->x, this->y); }
	Vector4<T> wwxz() const { return Vector4<T>(this->w, this->w, this->x, this->z); }
	Vector4<T> wwxw() const { return Vector4<T>(this->w, this->w, this->x, this->w); }
	Vector4<T> wwyx() const { return Vector4<T>(this->w, this->w, this->y, this->x); }
	Vector4<T> wwyy() const { return Vector4<T>(this->w, this->w, this->y, this->y); }
	Vector4<T> wwyz() const { return Vector4<T>(this->w, this->w, this->y, this->z); }
	Vector4<T> wwyw() const { return Vector4<T>(this->w, this->w, this->y, this->w); }
	Vector4<T> wwzx() const { return Vector4<T>(this->w, this->w, this->z, this->x); }
	Vector4<T> wwzy() const { return Vector4<T>(this->w, this->w, this->z, this->y); }
	Vector4<T> wwzz() const { return Vector4<T>(this->w, this->w, this->z, this->z); }
	Vector4<T> wwzw() const { return Vector4<T>(this->w, this->w, this->z, this->w); }
	Vector4<T> wwwx() const { return Vector4<T>(this->w, this->w, this->w, this->x); }
	Vector4<T> wwwy() const { return Vector4<T>(this->w, this->w, this->w, this->y); }
	Vector4<T> wwwz() const { return Vector4<T>(this->w, this->w, this->w, this->z); }
	Vector4<T> wwww() const { return Vector4<T>(this->w, this->w, this->w, this->w); }
	//@}
};
#pragma pack()

// Casting from a smaller vector to a larger one requires a forward reference
template<class T> inline Vector2<T> Vector1<T>::xx() const { return Vector2<T>(this->x, this->x); }
template<class T> inline Vector3<T> Vector1<T>::xxx() const { return Vector3<T>(this->x, this->x, this->x); }
template<class T> inline Vector4<T> Vector1<T>::xxxx() const { return Vector4<T>(this->x, this->x, this->x, this->x); }

template<class T> inline Vector3<T> Vector2<T>::xxy() const { return Vector3<T>(this->x, this->x, this->y); }
template<class T> inline Vector3<T> Vector2<T>::xyx() const { return Vector3<T>(this->x, this->y, this->x); }
template<class T> inline Vector3<T> Vector2<T>::xyy() const { return Vector3<T>(this->x, this->y, this->y); }
template<class T> inline Vector3<T> Vector2<T>::yxx() const { return Vector3<T>(this->y, this->x, this->x); }
template<class T> inline Vector3<T> Vector2<T>::yxy() const { return Vector3<T>(this->y, this->x, this->y); }
template<class T> inline Vector3<T> Vector2<T>::yyx() const { return Vector3<T>(this->y, this->y, this->x); }
template<class T> inline Vector3<T> Vector2<T>::yyy() const { return Vector3<T>(this->y, this->y, this->y); }
template<class T> inline Vector4<T> Vector2<T>::xxxy() const { return Vector4<T>(this->x, this->x, this->x, this->y); }
template<class T> inline Vector4<T> Vector2<T>::xxyx() const { return Vector4<T>(this->x, this->x, this->y, this->x); }
template<class T> inline Vector4<T> Vector2<T>::xxyy() const { return Vector4<T>(this->x, this->x, this->y, this->y); }
template<class T> inline Vector4<T> Vector2<T>::xyxx() const { return Vector4<T>(this->x, this->y, this->x, this->x); }
template<class T> inline Vector4<T> Vector2<T>::xyxy() const { return Vector4<T>(this->x, this->y, this->x, this->y); }
template<class T> inline Vector4<T> Vector2<T>::xyyx() const { return Vector4<T>(this->x, this->y, this->y, this->x); }
template<class T> inline Vector4<T> Vector2<T>::xyyy() const { return Vector4<T>(this->x, this->y, this->y, this->y); }
template<class T> inline Vector4<T> Vector2<T>::yxxx() const { return Vector4<T>(this->y, this->x, this->x, this->x); }
template<class T> inline Vector4<T> Vector2<T>::yxxy() const { return Vector4<T>(this->y, this->x, this->x, this->y); }
template<class T> inline Vector4<T> Vector2<T>::yxyx() const { return Vector4<T>(this->y, this->x, this->y, this->x); }
template<class T> inline Vector4<T> Vector2<T>::yxyy() const { return Vector4<T>(this->y, this->x, this->y, this->y); }
template<class T> inline Vector4<T> Vector2<T>::yyxx() const { return Vector4<T>(this->y, this->y, this->x, this->x); }
template<class T> inline Vector4<T> Vector2<T>::yyxy() const { return Vector4<T>(this->y, this->y, this->x, this->y); }
template<class T> inline Vector4<T> Vector2<T>::yyyx() const { return Vector4<T>(this->y, this->y, this->y, this->x); }
template<class T> inline Vector4<T> Vector2<T>::yyyy() const { return Vector4<T>(this->y, this->y, this->y, this->y); }
template<class T> inline Vector4<T> Vector3<T>::xxxz() const { return Vector4<T>(this->x, this->x, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xxyz() const { return Vector4<T>(this->x, this->x, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xxzx() const { return Vector4<T>(this->x, this->x, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::xxzy() const { return Vector4<T>(this->x, this->x, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::xxzz() const { return Vector4<T>(this->x, this->x, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xyxz() const { return Vector4<T>(this->x, this->y, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xyyz() const { return Vector4<T>(this->x, this->y, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xyzx() const { return Vector4<T>(this->x, this->y, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::xyzy() const { return Vector4<T>(this->x, this->y, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::xyzz() const { return Vector4<T>(this->x, this->y, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xzxx() const { return Vector4<T>(this->x, this->z, this->x, this->x); }
template<class T> inline Vector4<T> Vector3<T>::xzxy() const { return Vector4<T>(this->x, this->z, this->x, this->y); }
template<class T> inline Vector4<T> Vector3<T>::xzxz() const { return Vector4<T>(this->x, this->z, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xzyx() const { return Vector4<T>(this->x, this->z, this->y, this->x); }
template<class T> inline Vector4<T> Vector3<T>::xzyy() const { return Vector4<T>(this->x, this->z, this->y, this->y); }
template<class T> inline Vector4<T> Vector3<T>::xzyz() const { return Vector4<T>(this->x, this->z, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::xzzx() const { return Vector4<T>(this->x, this->z, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::xzzy() const { return Vector4<T>(this->x, this->z, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::xzzz() const { return Vector4<T>(this->x, this->z, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yxxz() const { return Vector4<T>(this->y, this->x, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yxyz() const { return Vector4<T>(this->y, this->x, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yxzx() const { return Vector4<T>(this->y, this->x, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::yxzy() const { return Vector4<T>(this->y, this->x, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::yxzz() const { return Vector4<T>(this->y, this->x, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yyxz() const { return Vector4<T>(this->y, this->y, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yyyz() const { return Vector4<T>(this->y, this->y, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yyzx() const { return Vector4<T>(this->y, this->y, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::yyzy() const { return Vector4<T>(this->y, this->y, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::yyzz() const { return Vector4<T>(this->y, this->y, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yzxx() const { return Vector4<T>(this->y, this->z, this->x, this->x); }
template<class T> inline Vector4<T> Vector3<T>::yzxy() const { return Vector4<T>(this->y, this->z, this->x, this->y); }
template<class T> inline Vector4<T> Vector3<T>::yzxz() const { return Vector4<T>(this->y, this->z, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yzyx() const { return Vector4<T>(this->y, this->z, this->y, this->x); }
template<class T> inline Vector4<T> Vector3<T>::yzyy() const { return Vector4<T>(this->y, this->z, this->y, this->y); }
template<class T> inline Vector4<T> Vector3<T>::yzyz() const { return Vector4<T>(this->y, this->z, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::yzzx() const { return Vector4<T>(this->y, this->z, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::yzzy() const { return Vector4<T>(this->y, this->z, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::yzzz() const { return Vector4<T>(this->y, this->z, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zxxx() const { return Vector4<T>(this->z, this->x, this->x, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zxxy() const { return Vector4<T>(this->z, this->x, this->x, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zxxz() const { return Vector4<T>(this->z, this->x, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zxyx() const { return Vector4<T>(this->z, this->x, this->y, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zxyy() const { return Vector4<T>(this->z, this->x, this->y, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zxyz() const { return Vector4<T>(this->z, this->x, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zxzx() const { return Vector4<T>(this->z, this->x, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zxzy() const { return Vector4<T>(this->z, this->x, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zxzz() const { return Vector4<T>(this->z, this->x, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zyxx() const { return Vector4<T>(this->z, this->y, this->x, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zyxy() const { return Vector4<T>(this->z, this->y, this->x, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zyxz() const { return Vector4<T>(this->z, this->y, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zyyx() const { return Vector4<T>(this->z, this->y, this->y, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zyyy() const { return Vector4<T>(this->z, this->y, this->y, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zyyz() const { return Vector4<T>(this->z, this->y, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zyzx() const { return Vector4<T>(this->z, this->y, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zyzy() const { return Vector4<T>(this->z, this->y, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zyzz() const { return Vector4<T>(this->z, this->y, this->z, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zzxx() const { return Vector4<T>(this->z, this->z, this->x, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zzxy() const { return Vector4<T>(this->z, this->z, this->x, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zzxz() const { return Vector4<T>(this->z, this->z, this->x, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zzyx() const { return Vector4<T>(this->z, this->z, this->y, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zzyy() const { return Vector4<T>(this->z, this->z, this->y, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zzyz() const { return Vector4<T>(this->z, this->z, this->y, this->z); }
template<class T> inline Vector4<T> Vector3<T>::zzzx() const { return Vector4<T>(this->z, this->z, this->z, this->x); }
template<class T> inline Vector4<T> Vector3<T>::zzzy() const { return Vector4<T>(this->z, this->z, this->z, this->y); }
template<class T> inline Vector4<T> Vector3<T>::zzzz() const { return Vector4<T>(this->z, this->z, this->z, this->z); }


// VKSL-style type names
typedef Vector1<float> vec1;
typedef Vector2<float> vec2;
typedef Vector3<float> vec3;
typedef Vector4<float> vec4;
typedef Vector1<double> dvec1;
typedef Vector2<double> dvec2;
typedef Vector3<double> dvec3;
typedef Vector4<double> dvec4;
typedef Vector1<int32_t> ivec1;
typedef Vector2<int32_t> ivec2;
typedef Vector3<int32_t> ivec3;
typedef Vector4<int32_t> ivec4;
typedef Vector1<uint32_t> uvec1;
typedef Vector2<uint32_t> uvec2;
typedef Vector3<uint32_t> uvec3;
typedef Vector4<uint32_t> uvec4;
typedef Vector1<int16_t> svec1;
typedef Vector2<int16_t> svec2;
typedef Vector3<int16_t> svec3;
typedef Vector4<int16_t> svec4;
typedef Vector1<uint16_t> usvec1;
typedef Vector2<uint16_t> usvec2;
typedef Vector3<uint16_t> usvec3;
typedef Vector4<uint16_t> usvec4;
typedef Vector1<int8_t> bvec1;
typedef Vector2<int8_t> bvec2;
typedef Vector3<int8_t> bvec3;
typedef Vector4<int8_t> bvec4;
typedef Vector1<uint8_t> ubvec1;
typedef Vector2<uint8_t> ubvec2;
typedef Vector3<uint8_t> ubvec3;
typedef Vector4<uint8_t> ubvec4;

} // namespace VK

#endif // __VKVector_h__
