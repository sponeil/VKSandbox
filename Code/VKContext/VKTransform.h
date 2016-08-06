// Transform.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __Transform_h__
#define __Transform_h__

#include "VKMatrix.h"

namespace VK {

/// Encapsulates an SRT (Scale-Rotate-Translate) transformation.
/// Keeping these pieces separate, instead of keeping them together in a
/// matrix, simplifies a number of things. The same Transform object can
/// be treated as a model or a camera (or both). Changing one element by
/// itself is cheaper than a matrix multiply, it won't cause the other
/// values to "drift", and it allows you to keep a different level of
/// precision for each element. For instance, you may want to use double-
/// precision for position and single-precision for scale and orientation.
/// Or you may want to use fixed-point integers for position.
/// @note The "relative" matrices are used to safely cast a double-precision
/// position down to single-precision for rendering. The farther away an
/// object is from the camera, the less we need to worry about precision loss,
/// so it is common to subtract model position from camera position, THEN
/// cast down to a float, and THEN build the model matrix for rendering.
template<class T>
class Transform
{
public:
	Vector3<T> pos;		///< Position can be float, double, int, etc.
	quat dir;			///< Should orientation should always be float (what about planet rotation)?
	float size;			///< Size should always be float

	Transform(const Vector3<T> &p = Vector3<T>(0, 0, 0), const quat &q = quat(), float s=1) : pos(p), dir(q), size(s) {}

	void scale(float f) { size *= f; }
	void translate(const Vector3<T> &v) { pos += v; }
	void rotate(const quat &q) { dir = q * dir; } // Note that quat multiplication is backwards for combining rotations

	/// Treat this object as a model matrix and transform something to get its world coordinates
	Vector3<T> vModel(const Vector3<T> &v) const { return dir.vTransform(v) * size + pos; }
	Vector3<T> nModel(const Vector3<T> &n) const { return dir.nTransform(n); }
	quat qModel(const quat &q) const { return q * dir; }
	Transform model(const Transform &t) const { return Transform(dir.vTransform(t.pos), qView(t.dir), size * t.size); }

	/// Treat this object as a view matrix and transform something to get its view-relative coordinates
	Vector3<T> vView(const Vector3<T> &v) const { return dir.unitInverse().vTransform(v / size - pos); }
	Vector3<T> nView(const Vector3<T> &n) const { return dir.vTransform(n); }
	quat qView(const quat &q) const { return dir.unitInverse() * q; }
	Transform<T> view(const Transform<T> &t) const { return Transform<T>(vView(t.pos), qView(t.dir), t.size); }

	Transform operator+(const Vector3<T> &v) const { return Transform(pos+v, dir, size); }
	Transform operator-(const Vector3<T> &v) const { return Transform(pos-v, dir, size); }
	Transform operator*(const quat &q) const { return Transform(pos, q*dir, size); }
	Transform operator*(float f) const { return Transform(pos, dir, size*f); }
	Transform<T> operator*(const Transform<T> &t) { return model(t); }

	void operator+=(const Vector3<T> &v) { pos += v; }
	void operator-=(const Vector3<T> &v) { pos -= v; }
	void operator*=(const quat &q) const { dir = q * dir; }
	void operator*=(float f) { size *= f; }
	void operator/=(float f) { size /= f; }

	Transform<T> inverse() const {
		Transform<T> ret;
		ret.size = 1/size;
		ret.dir = dir.unitInverse();
		ret.pos = ret.dir.vTransform(pos) * -ret.size;
		return ret;
	}

	vec3 getViewAxis() const		{ return dir.getViewAxis(); }
	vec3 getUpAxis() const			{ return dir.getUpAxis(); }
	vec3 getRightAxis() const		{ return dir.getRightAxis(); }

	mat4 modelMatrix() const {
		mat4 mat = dir;
		mat.v[3].x = (float)pos.x;
		mat.v[3].y = (float)pos.y;
		mat.v[3].z = (float)pos.z;
		if(size != 1) mat *= mat4::Scale(vec3(size, size, size));
		return mat;
	}
	mat4 viewMatrix() const { return inverse().modelMatrix(); }
	mat4 viewMatrix(const Vector3<T> &vAt) const { return viewMatrix(vAt, getUpAxis()); }
	mat4 viewMatrix(const Vector3<T> &vAt, const Vector3<T> &vUp) const {
		Vector3<T> vView = vAt - pos;
		Vector3<T> vRight = vView ^ vUp;
		Vector3<T> vTrueUp = vRight ^ vView;
		return Matrix4x4<T>::View(pos, vView.normalize(), vTrueUp.normalize(), vRight.normalize());
	}

	/// Gets the position relative to the camera's position (and casts down to single-precision for better performance)
	const vec3 getRelativePosition(const Vector3<T> &vRelative) const {
		return (vec3)(pos - vRelative);
	}
	/// Builds a view matrix with the camera at the origin (for extra precision close to the camera)
	mat4 relativeViewMatrix() const {
		return Transform<T>(Vector3<T>(0, 0, 0), dir, size).viewMatrix();
	}
	/// Builds a model matrix relative to the camera's position
	mat4 relativeModelMatrix(const Vector3<T> &vRelative) const {
		return Transform<T>(pos-vRelative, dir, size).modelMatrix();
	}

	// Performs linear interpolation to get smooth motion between two transforms
	Transform<T> lerp(const Transform<T> &srt, float t) {
		return Transform<T>(
			Math::Lerp(pos, srt.pos, t),
			quat::slerp(dir, srt.dir, t),
			Math::Lerp(size, srt.size, t)
		);
	}

	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz + sprintf(psz, "t[%f, ", size);
		pszEnd = dir.to_s(pszEnd, false);
		*pszEnd++ = ',';
		*pszEnd++ = ' ';
		pszEnd = pos.to_s(pszEnd, false);
		*pszEnd++ = ']';
		*pszEnd = 0;
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
	void from_s( const char *psz ) {
		std::sscanf(psz, "t[%f, q[%f, %f, %f, %f], v[%f, %f, %f]]", &size, &dir.x, &dir.y, &dir.z, &dir.w, &pos.x, &pos.y, &pos.z);
		dir = dir.normalize();
	}
};


} // namespace VK

#endif // __Transform_h__
