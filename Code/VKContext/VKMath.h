// VKMath.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKMath_h__
#define __VKMath_h__

#define _USE_MATH_DEFINES
#include <math.h>

namespace VK {

const float LOGHALF = -0.693147f;			// log(0.5)
const float LOGHALFI = -1.442695f;			// Inverse of log(0.5)
const float DELTA = 1e-6f;					// Small number for comparing floating point numbers
const double PI_DOUBLE = 3.141592653589793;
const double PI_FLOAT = (float)PI_DOUBLE;

/// A namespace for encapsulating common math routines.
/// Aside from allowing me to define new "standard" math routines, like cubic
/// and quintic, it also allows me to override existing math routines like sin
/// and cos. For now the single-precision versions of the trig functions are
/// used because they're an order of magnitude faster than the double-
/// precision versions.
namespace Math {
	template <class T> inline void Swap(T &a, T &b)		{ T t = a; a = b; b = t; }
	template <class T> inline T Abs(T a)				{ return (a < 0 ? -a : a); }
	template <class T> inline T Min(T a, T b)			{ return (a < b ? a : b); }
	template <class T> inline T Max(T a, T b)			{ return (a > b ? a : b); }
	template <class T> inline T Avg(T a, T b)			{ return (a + b) / (T)2; }
	template <class T> inline T Clamp(T x, T min, T max){ return (x < min ? min : (x > max ? max : x)); }
	template <class T> inline T Lerp(T a, T b, float t){ return (a + (b - a) * t); }
	template <class T> inline T Cubic(T a)				{ return a*a*((T)3-(T)2*a); }
	template <class T> inline T Quintic(T a)			{ return a*a*a*(a*(a*(T)6-(T)15)+(T)10); }
	template <class T> inline T Sign(T a)				{ return a < 0 ? (T)-1 : (T)1; }
	template <class T> inline T Square(T a)				{ return a * a; }
	template <class T> inline T SquareWithSign(T a)		{ return a < 0 ? -(a * a) : (a * a); }
	template <class T> inline T Step(T a, T x)			{ return (T)(x >= a); }
	template <class T> inline T Boxstep(T a, T b, T x)	{ return Clamp((x-a)/(b-a), 0, 1); }
	template <class T> inline T Pulse(T a, T b, T x)	{ return (T)((x >= a) - (x >= b)); }
	template <class T> inline bool IsPo2(T n)			{
		for(int bits = 0; bits == 0 && n > 0; n >>= 1)
			bits += n & 1;
		return n == 0;
	}

	// Implements linear interpolation between two points
	// As t goes from 0 to 1, the return value goes from p0 to p1 following a straight line
	// Transitions are usually sharp and very noticeable
	template <class T> inline float LinearInterpolation(const T &p0, const T &p1, float t) {
		return (p0 + (p1 - p0) * t);
	}

	// Implements cubic interpolation between two points
	// As t goes from 0 to 1, the return value goes from p0 to p1 following a fixed curve
	// Some transitions look better than linear, but some look worse
	template <class T> inline float CubicInterpolation(const T &p0, const T &p1, float t) {
		return (p0 + (p1 - p0) * Cubic(t));
	}

	// Implements Camull-Rom spline interpolation between 2 points (requires the 2 surrounding points).
	// As t goes from 0 to 1, the return value goes from p1 to p2 following a dynamic curve based on the 2 surrounding points
	// It is very smooth with no noticeable transitions
	template <class T> inline float CatmullRom(const T &p0, const T &p1, const T &p2, const T &p3, float t) {
		float t2 = t*t;
		float t3 = t2*t;
		const float K = 0.5f;
		return (float)(p1 + (-K*p0 + K*p2)*t + (2*K*p0 + (K-3)*p1 + (3-2*K)*p2 - K*p3)*t2 + (-K*p0 + (2-K)*p1 + (K-2)*p2 + K*p3)*t3);
		//return 0.5f * ((2 * p1) + (-p0 + p2)*t + (2*p0 - 5*p1 + 4*p2 - p3)*t2 + (-p0 + 3*p1 - 3*p2 + p3)*t3);
	}

	inline float ToRadians(float fDegrees)			{ return fDegrees * 0.01745329f; }
	inline float ToDegrees(float fRadians)			{ return fRadians * 57.295779f; }
	inline float Sin(float a)						{ return sinf(a); }
	inline float Cos(float a)						{ return cosf(a); }
	inline float Tan(float a)						{ return tanf(a); }
	inline float Asin(float a)						{ return asinf(a); }
	inline float Acos(float a)						{ return acosf(a); }
	inline float Atan(float a)						{ return atanf(a); }
	inline float Atan2(float y, float x)			{ return atan2f(y, x); }
	inline float Sqrt(float a)						{ return sqrtf(a); }

	inline int Floor(float a)						{ return ((int)a - (a < 0 && a != (int)a)); }
	inline int Ceiling(float a)						{ return ((int)a + (a > 0 && a != (int)a)); }
	inline float SqrtWithSign(float a)				{ return a < 0 ? -sqrtf(-a) : sqrtf(a); }
	inline float Gamma(float a, float g)			{ return powf(a, 1/g); }
	inline float Bias(float a, float b)				{ return powf(a, logf(b) * LOGHALFI); }
	inline float Expose(float l, float k)			{ return (1 - expf(-l * k)); }

	inline float Gain(float a, float b) {
		if(a <= DELTA)
			return 0;
		if(a >= 1-DELTA)
			return 1;

		register float p = (logf(1 - b) * LOGHALFI);
		if(a < 0.5)
			return powf(2 * a, p) * 0.5f;
		else
			return 1 - powf(2 * (1 - a), p) * 0.5f;
	}

	inline float Smoothstep(float a, float b, float x) {
		if(x <= a)
			return 0;
		if(x >= b)
			return 1;
		return Cubic((x - a) / (b - a));
	}

	inline float Mod(float a, float b) {
		a -= ((int)(a / b)) * b;
		if(a < 0)
			a += b;
		return a;
	}

	inline void Normalize(float *f, int n) {
		int i;
		float fMagnitude = 0;
		for(i=0; i<n; ++i)
			fMagnitude += f[i]*f[i];
		fMagnitude = 1 / sqrtf(fMagnitude);
		for(i=0; i<n; ++i)
			f[i] *= fMagnitude;
	}
};

} // namespace VK

#endif // __VKMath_h__
