// VKQuaternion.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKQuaternion_h__
#define __VKQuaternion_h__

#include "VKVector.h"

namespace VK {

/// Encapsulates a quaternion for managing rotations/orientations.
/// Many people seem to think quaternions are too unintuitive to use on a
/// regular basis, but IMHO they are more, intuitive, simple, and efficient
/// than any other scheme for managing rotations and orientations.
/// Imagine you're tracking objects on a spherical planet. The "up" vector
/// changes every time an object moves, and so do directions like "north"
/// and "east". You can't rotate around (0,1,0) to turn a model unless it is
/// sitting exactly on the north pole. If it is sitting on the north pole,
/// directions like "north" and "east" do not even exist.
/// Here are some simple quaternion use cases:<PRE>
/// VK::quat q(vAxis, fAngle); // Initialize a quaternion with an initial axis+angle
/// VK::quat qYaw(q.getUpAxis(), fYaw); // This quaternion would rotate it around its up axis (yaw)
/// VK::quat qPitch(q.getRightAxis(), fPitch); // This quaternion would rotate it around its right axis (pitch)
/// VK::quat qRoll(q.getViewAxis(), fRoll); // This quaternion would rotate it around its view axis (roll)
/// VK::quat qPlanetTurn(vPos.normalize(), fTurn); // This quaternion would turn an object standing on a planet
/// </PRE>
/// Once you have a quaternion, it is dead simple to use. It is also dead
/// simple to convert to a rotation matrix, which can then be used to build
/// a model or view matrix. The hardest part is often getting started. If
/// you don't have an axis+angle to initialize a quaternion with, you can use
/// view and up vectors to build an orientation matrix and then cast it to a
/// quaternion. It takes a little time to get used to, but once you do,
/// things that used to be hard become easy.
template <class T> class Quaternion : public Vector4<T> {
public:
	/// @name Constructors
	//@{
	Quaternion() : Vector4<T>(0, 0, 0, 1) {}
	Quaternion(T a, T b, T c, T d)	: Vector4<T>(a, b, c, d) {}
	Quaternion(const Vector3<T> &vAxis, T fAngle) { setAxisAngle(vAxis, fAngle); }
	Quaternion(const Vector4<T> &v) { *this = v; }
	Quaternion(const Quaternion<T> &q) { *this = q; }
	//@}

	/// @name Casting and unary operators
	//@{
	operator Quaternion<float>() const		{ return Quaternion<float>((float)this->x, (float)this->y, (float)this->z, (float)this->w); }
	operator Quaternion<double>() const		{ return Quaternion<double>((double)this->x, (double)this->y, (double)this->z, (double)this->w); }
	T &operator[](const int n)				{ return Vector4<T>::operator[](n); }
	T operator[](const int n) const			{ return Vector4<T>::operator[](n); }
	Quaternion<T> operator-() const			{ return Quaternion<T>(-this->x, -this->y, -this->z, -this->w); }
	//@}

	/// @name Assignment operators
	//@{
	void operator=(const Vector4<T> &v)		{ this->x = v.x, this->y = v.y, this->z = v.z, this->w = v.w; }
	void operator=(const Quaternion<T> &q)	{ this->x = q.x; this->y = q.y; this->z = q.z; this->w = q.w; }
	//@}

	/// @name Arithmetic operators (quaternion and quaternion)
	//@{
	Quaternion<T> operator*(T f) const { return Vector4<T>::operator*(f); }
	void operator*=(T f) { Vector4<T>::operator*=(f); }
	Quaternion<T> operator/(T f) const { return Vector4<T>::operator/(f); }
	void operator/=(T f) { Vector4<T>::operator/=(f); }
	Quaternion<T> operator*(const Quaternion &q) const {
		return Quaternion<T>(// 16 muls, 12 adds
			this->w*q.x + this->x*q.w + this->y*q.z - this->z*q.y,
			this->w*q.y + this->y*q.w + this->z*q.x - this->x*q.z,
			this->w*q.z + this->z*q.w + this->x*q.y - this->y*q.x,
			this->w*q.w - this->x*q.x - this->y*q.y - this->z*q.z
		);
	}
	void operator*=(const Quaternion &q)	{ *this = *this * q; }
	Quaternion<T> normalize() const			{ return *this / mag(); }
	//@}

	/// @name Advanced quaternion methods
	//@{
	/// Return the conjugate of this quaternion
	Quaternion<T> conjugate() const			{ return Quaternion<T>(-this->x, -this->y, -this->z, this->w); }
	/// Return the inverse of a normalized (unit length) quaternion
	Quaternion<T> unitInverse() const		{ return conjugate(); }
	/// Return the inverse of any size quaternion
	Quaternion<T> inverse() const			{ return conjugate() / Vector4<T>::mag2(); }
	/// Transform (rotate) a vector by this quaternion
	Vector3<T> vTransform(const Vector3<T> &v) const { return *this * Quaternion<T>(v.x, v.y, v.z, 0) * unitInverse(); }
	/// Transform (rotate) a normal by this quaternion
	Vector3<T> nTransform(const Vector3<T> &n) const { return unitInverse() * Quaternion<T>(n.x, n.y, n.z, 0) * *this; }
	//@}

	/// Creates a quaternion that equates to the "default" orientation rotated around vAxis by fAngle
	void setAxisAngle(const Vector3<T> &vAxis, T fAngle) {
		// 4 muls, 2 trig function calls
		T f = fAngle * (T)0.5;
		Vector3<T>::operator=(vAxis * sin(f));
		this->w = cos(f);
	}
	/// Returns the axis and angle around which the "default" orientation would need to be rotated to equal this quaternion
	void getAxisAngle(Vector3<T> &vAxis, T &fAngle) const {
		// 4 muls, 1 div, 2 trig function calls
		fAngle = acos(this->w);
		vAxis = *this / sin(fAngle);
		fAngle *= (T)2.0;
	}

	/// Returns this quaternion's right axis, or (1,0,0) transformed by this quaternion
	Vector3<T> getRightAxis() const {
		// 6 muls, 7 adds
		T /*x2 = x + x, */y2 = this->y + this->y, z2 = this->z + this->z;
		return Vector3<T>((T)1 - (this->y*y2 + this->z*z2), this->x*y2 + this->w*z2, this->x*z2 - this->w*y2);
	}
	/// Returns this quaternion's up axis, or (0,1,0) transformed by this quaternion
	Vector3<T> getUpAxis() const {
		// 6 muls, 7 adds
		T x2 = this->x + this->x, y2 = this->y + this->y, z2 = this->z + this->z;
		return Vector3<T>(this->x*y2 - this->w*z2, (T)1 - (this->x*x2 + this->z*z2), this->y*z2 + this->w*x2);
	}
	/// Returns this quaternion's view axis, or (0,0,-1) transformed by this quaternion
	Vector3<T> getViewAxis() const {
		// 6 muls, 7 adds
		T x2 = this->x + this->x, y2 = this->y + this->y, z2 = this->z + this->z;
		return -Vector3<T>(this->x*z2 + this->w*y2, this->y*z2 - this->w*x2, (T)1 - (this->x*x2 + this->y*y2));
	}
    
	/// Builds a string representation of this quaternion
	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz + sprintf(psz, "q[%f, %f, %f, %f]", this->x, this->y, this->z, this->w);
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
	void from_s(const char *psz) {
		std::sscanf(psz, "q[%f, %f, %f, %f]", &this->x, &this->y, &this->z, &this->w);
		dir = dir.normalize();
	}

	/// Performs a sperhical LERP (linear interpolation) between two quaternions
	static Quaternion<T> slerp(const Quaternion<T> &q1, const Quaternion<T> &q2, T t) {
		// Calculate the cosine of the angle between the two
		T fScale0, fScale1;
		T dCos = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

		// If the angle is significant, use the spherical interpolation
		if((1.0f - abs(dCos)) > DELTA) {
			T dTemp = acos(abs(dCos));
			T dSin = sin(dTemp);
			fScale0 = (T)(sin((1.0f - t) * dTemp) / dSin);
			fScale1 = (T)(sin(t * dTemp) / dSin);
		} else { // Else use the cheaper linear interpolation
			fScale0 = (T)1.0 - t;
			fScale1 = t;
		}
		if(dCos < 0.0)
			fScale1 = -fScale1;

		// Return the interpolated result
		return (q1 * fScale0) + (q2 * fScale1);
	}
};

// VKSL-style type names
typedef Quaternion<float> quat;
typedef Quaternion<float> fquat;
typedef Quaternion<double> dquat;

} // namespace VK

#endif // __VKQuaternion_h__
