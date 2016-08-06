// Matrix.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __Matrix_h__
#define __Matrix_h__

#include "VKQuaternion.h"

namespace VK {

template<class T> class Matrix4x4; // Forward declaration

/// Encapsulates a 3x3 rotation matrix.
/// This class uses column-major order, as used by OpenVK.
/// Matrix values will generally be accessed in this fashion:<PRE>
/// | v[0].x v[1].x v[2].x |   | v[0][0] v[1][0] v[2][0] |
/// | v[0].y v[1].y v[2].y | = | v[0][1] v[1][1] v[2][1] |
/// | v[0].z v[1].z v[2].z |   | v[0][2] v[1][2] v[2][2] |
/// </PRE>
template<class T> class Matrix3x3 {
public:
	Vector3<T> v[3];

	/// @name Constructors
	//@{
	Matrix3x3() {}
	Matrix3x3(const T *p) { *this = p; }
	Matrix3x3(const Quaternion<T> &q) { *this = q; }
	Matrix3x3(const Vector3<T> &v0, const Vector3<T> &v1, const Vector3<T> &v2) { v[0] = v0; v[1] = v1; v[2] = v2; }
	Matrix3x3(const Matrix4x4<T> &m); // Needs forward declaration
	//@}

	/// @name Init functions
	//@{
	static Matrix3x3<T> Zero() { ///< Builds and returns a matrix filled with 0's.
		Vector3<T> v(0, 0, 0);
		return Matrix3x3<T>(v, v, v);
	}
	static Matrix3x3<T> Identity() { ///< Builds and returns an identity matrix
		return Matrix3x3<T>(
			Vector3<T>(1, 0, 0),
			Vector3<T>(0, 1, 0),
			Vector3<T>(0, 0, 1)
		);
	}
	//@}

	/// @name Casting and unary operators
	//@{
	operator Matrix3x3<float>() const	{ return Matrix3x3<float>(v[0], v[1], v[2]); }
	operator Matrix3x3<double>() const	{ return Matrix3x3<double>(v[0], v[1], v[2]); }
	operator T*()						{ return &v[0].x; }
	operator const T*() const			{ return &v[0].x; }
	Vector3<T> &operator[](unsigned int n)	{ if(n > 2) VKLogException("Matrix3x3[%u] - Invalid index", n); return v[n]; }
	const Vector3<T> &operator[](unsigned int n) const { if(n > 2) VKLogException("Matrix3x3[%u] - Invalid index", n); return v[n]; }
	//@}

	/// @name Assignment operators
	//@{
	const Matrix3x3<T> &operator=(const T *p) {
		v[0] = Vector3<T>(p[0], p[1], p[2]);
		v[1] = Vector3<T>(p[3], p[4], p[5]);
		v[2] = Vector3<T>(p[6], p[7], p[8]);
		return *this;
	}
	const Matrix3x3<T> &operator=(const Matrix4x4<T> &m);
	const Matrix3x3<T> &operator=(const Quaternion<T> &q) {
		// 9 muls, 15 adds
		T x2 = q.x + q.x, y2 = q.y + q.y, z2 = q.z + q.z;
		T xx = q.x * x2, xy = q.x * y2, xz = q.x * z2;
		T yy = q.y * y2, yz = q.y * z2, zz = q.z * z2;
		T wx = q.w * x2, wy = q.w * y2, wz = q.w * z2;

		v[0].x = 1-(yy+zz);	v[1].x = xy-wz;		v[2].x = xz+wy;
		v[0].y = xy+wz;		v[1].y = 1-(xx+zz);	v[2].y = yz-wx;
		v[0].z = xz-wy;		v[1].z = yz+wx;		v[2].z = 1-(xx+yy);
		return *this;
	}
	//@}

	/// @name Arithmetic operators
	//@{
	Matrix3x3<T> operator*(T f) const { return Matrix3x3<T>(v[0] * f, v[1] * f, v[2] * f); }
	Matrix3x3<T> operator*(const Matrix3x3<T> &m) const {
		// 27 muls, 18 adds
		Matrix3x3<T> t = transpose();
		return Matrix3x3<T>(
			Vector3<T>(t.v[0] | m.v[0], t.v[1] | m.v[0], t.v[2] | m.v[0]),
			Vector3<T>(t.v[0] | m.v[1], t.v[1] | m.v[1], t.v[2] | m.v[1]),
			Vector3<T>(t.v[0] | m.v[2], t.v[1] | m.v[2], t.v[2] | m.v[2])
		);
	}
	Matrix3x3<T> operator*(const Quaternion<T> &q) const { return operator*(Matrix3x3<T>(q)); }
	void operator*=(T f) { v[0] *= f; v[1] *= f; v[2] *= f; }
	void operator*=(const Matrix3x3<T> &m) { *this = operator*(m); }
	void operator*=(const Quaternion<T> &q) { *this = operator*(q); }
	//@}

	/// Returns the transpose of the current matrix
	Matrix3x3<T> transpose() const {
		return Matrix3x3<T>(
			Vector3<T>(v[0].x, v[1].x, v[2].x),
			Vector3<T>(v[0].y, v[1].y, v[2].y),
			Vector3<T>(v[0].z, v[1].z, v[2].z)
		);
	}

	/// Transforms a vector by the current rotation matrix
	Vector3<T> vTransform(const Vector3<T> &vec) const {
		// 9 muls, 6 adds
		return Vector3<T>(
			v[0].x*vec.x + v[1].x*vec.y + v[2].x*vec.z,
			v[0].y*vec.x + v[1].y*vec.y + v[2].y*vec.z,
			v[0].z*vec.x + v[1].z*vec.y + v[2].z*vec.z
		);
	}

	/// Transforms a normal by the current rotation matrix
	Vector3<T> nTransform(const Vector3<T> &normal) const {
		// 9 muls, 6 adds
		return Vector3<T>(
			v[0].x*normal.x + v[0].y*normal.y + v[0].z*normal.z,
			v[1].x*normal.x + v[1].y*normal.y + v[1].z*normal.z,
			v[2].x*normal.x + v[2].y*normal.y + v[2].z*normal.z
		);
	}

	/// Converts the current rotation matrix to a quaternion
	Quaternion<T> to_q() const;
};

/// Encapsulates a 4x4 transformation matrix.
/// This class uses column-major order, as used by OpenVK.
/// Matrix values will generally be accessed in this fashion:<PRE>
/// | v[0].x v[1].x v[2].x v[3].x |   | v[0][0] v[1][0] v[2][0] v[3][0] |
/// | v[0].y v[1].y v[2].y v[3].y | = | v[0][1] v[1][1] v[2][1] v[3][1] |
/// | v[0].z v[1].z v[2].z v[3].z |   | v[0][2] v[1][2] v[2][2] v[3][2] |
/// | v[0].w v[1].w v[2].w v[3].w |   | v[0][3] v[1][3] v[2][3] v[3][3] |
/// </PRE>
template<class T> class Matrix4x4 {
public:
	Vector4<T> v[4];

	/// @name Constructors
	//@{
	Matrix4x4() {}
	Matrix4x4(const T *p) { *this = p; }
	Matrix4x4(const Quaternion<T> &q) { *this = q; }
	Matrix4x4(const Matrix3x3<T> &m) { *this = m; }
	Matrix4x4(const Vector4<T> &v0, const Vector4<T> &v1, const Vector4<T> &v2, const Vector4<T> &v3) { v[0] = v0; v[1] = v1; v[2] = v2; v[3] = v3; }
	//@}

	/// @name Init functions
	//@{
	static Matrix4x4<T> Zero() { ///< Builds and returns a matrix filled with 0's.
		Vector4<T> v(0, 0, 0, 0);
		return Matrix4x4<T>(v, v, v, v);
	}

	static Matrix4x4<T> Identity() { ///< Builds and returns an identity matrix.
		return Matrix4x4<T>(
			Vector4<T>(1, 0, 0, 0),
			Vector4<T>(0, 1, 0, 0),
			Vector4<T>(0, 0, 1, 0),
			Vector4<T>(0, 0, 0, 1)
		);
	}

	/// Builds and returns a scale matrix.
	/// @param[in] s A vector containing the x, y, and z scaling factors
	static Matrix4x4<T> Scale(const Vector3<T> &s) {
		return Matrix4x4<T>(
			Vector4<T>(s.x, 0, 0, 0),
			Vector4<T>(0, s.y, 0, 0),
			Vector4<T>(0, 0, s.z, 0),
			Vector4<T>(0, 0, 0, 1)
		);
	}

	/// Builds and returns a translate matrix.
	/// @param[in] t A vector containing the x, y, and z amounts to translate
	static Matrix4x4<T> Translate(const Vector3<T> &t) {
		return Matrix4x4<T>(
			Vector4<T>(1, 0, 0, 0),
			Vector4<T>(0, 1, 0, 0),
			Vector4<T>(0, 0, 1, 0),
			Vector4<T>(t.x, t.y, t.z, 1)
		);
	}

	/// Builds and returns a scale and translate matrix.
	/// @param[in] s A vector containing the x, y, and z scaling factors
	/// @param[in] t A vector containing the x, y, and z amounts to translate
	static Matrix4x4<T> ScaleTranslate(const Vector3<T> &s, const Vector3<T> &t) {
		return Matrix4x4<T>(
			Vector4<T>(s.x, 0, 0, 0),
			Vector4<T>(0, s.y, 0, 0),
			Vector4<T>(0, 0, s.z, 0),
			Vector4<T>(t.x, t.y, t.z, 1)
		);
	}

	/// Builds and returns a view matrix.
	/// Assumes vView, vRight, and vUp are already normalized and perpendicular.
	/// @param[in] vEye The eye (or camera) position for this view
	/// @param[in] vView The view direction vector for this view
	/// @param[in] vUp The up direction vector for this view
	/// @param[in] vRight The right direction vector for this view
	static Matrix4x4<T> View(const Vector3<T> &vEye, const Vector3<T> &vView, const Vector3<T> &vUp, const Vector3<T> &vRight) {
		return Matrix4x4<T>(
			Vector4<T>(vRight.x, vUp.x, -vView.x, 0),
			Vector4<T>(vRight.y, vUp.y, -vView.y, 0),
			Vector4<T>(vRight.z, vUp.z, -vView.z, 0),
			Vector4<T>(-(vEye | vRight), -(vEye | vUp), -(vEye | -vView), 1)
		);
	}

	/// Builds and returns a model matrix.
	/// Assumes vView, vRight, and vUp are already normalized and perpendicular.
	/// @param[in] vEye The eye (or camera) position for this model
	/// @param[in] vView The view direction vector for this model
	/// @param[in] vUp The up direction vector for this model
	/// @param[in] vRight The right direction vector for this model
	static Matrix4x4<T> Model(const Vector3<T> &vEye, const Vector3<T> &vView, const Vector3<T> &vUp, const Vector3<T> &vRight) {
		return Matrix4x4<T>(
			Vector4<T>(vRight, 0),
			Vector4<T>(vUp, 0),
			Vector4<T>(-vView, 0),
			Vector4<T>(vEye, 1)
		);
	}

	/// Builds a view matrix equivalent to gluLookAt()
	static Matrix4x4<T> LookAt(const Vector3<T> &vEye, const Vector3<T> &vAt, const Vector3<T> &vUp) {
		Vector3<T> vView = (vAt - vEye).normalize();
		Vector3<T> vRight = vView.cross(vUp).normalize();
		Vector3<T> vTrueUp = vRight.cross(vView).normalize();
		return View(vEye, vView, vTrueUp, vRight);
	}

	/// Builds a projection matrix equivalent to glFrustum()
	static Matrix4x4<T> Frustum(T left, T right, T bottom, T top, T zNear, T zFar) {
		return Matrix4x4<T>(
			Vector4<T>((2*zNear)/(right-left), 0, 0, 0),
			Vector4<T>(0, (2*zNear)/(top-bottom), 0, 0),
			Vector4<T>((right+left)/(right-left), (top+bottom)/(top-bottom), (zFar+zNear)/(zNear-zFar), -1),
			Vector4<T>(0, 0, (2*zFar*zNear)/(zNear-zFar), 0)
		);
	}

	/// Builds a projection matrix equivalent to gluPerspective()
	static Matrix4x4<T> Perspective(T fovy, T aspect, T zNear, T zFar) {
		T xmin, xmax, ymin, ymax;
		ymax = zNear * tan(fovy * (T)(M_PI / 360.0));
		ymin = -ymax;
		xmin = ymin * aspect;
		xmax = ymax * aspect;
		return Frustum(xmin, xmax, ymin, ymax, zNear, zFar);
	}

	/// Builds a projection matrix equivalent to glOrtho()
	static Matrix4x4<T> Ortho(T left, T right, T bottom, T top, T zNear, T zFar) {
		return Matrix4x4<T>(
			Vector4<T>(2 / (right-left), 0, 0, 0),
			Vector4<T>(0, 2 / (top-bottom), 0, 0),
			Vector4<T>(0, 0, -2 / (zFar-zNear), 0),
			Vector4<T>((right+left)/(left-right), (top+bottom)/(bottom-top), (zFar+zNear)/(zNear-zFar), 1)
		);
	}
	//@}

	/// @name Casting and unary operators
	//@{
	operator Matrix4x4<float>() const	{ return Matrix4x4<float>((vec4)v[0], (vec4)v[1], (vec4)v[2], (vec4)v[3]); }
	operator Matrix4x4<double>() const	{ return Matrix4x4<double>((dvec4)v[0], (dvec4)v[1], (dvec4)v[2], (dvec4)v[3]); }
	operator T*()						{ return &v[0].x; }
	operator const T*() const			{ return &v[0].x; }
	Vector4<T> &operator[](unsigned int n)	{ if(n > 3) VKLogException("Matrix4x4[%u] - Invalid index", n); return v[n]; }
	const Vector4<T> &operator[](unsigned int n) const { if(n > 3) VKLogException("Matrix4x4[%u] - Invalid index", n); return v[n]; }
	//@}

	/// @name Assignment operators
	//@{
	const Matrix4x4<T> &operator=(const T *p) {
		v[0] = Vector4<T>(p[0], p[1], p[2], p[3]);
		v[1] = Vector4<T>(p[4], p[5], p[6], p[7]);
		v[2] = Vector4<T>(p[8], p[9], p[10], p[11]);
		v[3] = Vector4<T>(p[12], p[13], p[14], p[15]);
		return *this;
	}
	const Matrix4x4<T> &operator=(const Matrix3x3<T> &m) {
		v[0] = Vector4<T>(m.v[0], 0);
		v[1] = Vector4<T>(m.v[1], 0);
		v[2] = Vector4<T>(m.v[2], 0);
		v[3] = Vector4<T>(0, 0, 0, 1);
		return *this;
	}
	const Matrix4x4<T> &operator=(const Quaternion<T> &q) {
		*this = Matrix3x3<T>(q);
		return *this;
	}
	//@}

	/// @name Arithmetic operators
	//@{
	Matrix4x4<T> operator*(T f) const { return Matrix4x4<T>(v[0] * f, v[1] * f, v[2] * f, v[3] * f); }
	Matrix4x4<T> operator*(const Matrix4x4<T> &m) const {
		Matrix4x4<T> t = transpose();
		return Matrix4x4<T>( // 64 muls, 48 adds
			Vector4<T>(t.v[0] | m.v[0], t.v[1] | m.v[0], t.v[2] | m.v[0], t.v[3] | m.v[0]),
			Vector4<T>(t.v[0] | m.v[1], t.v[1] | m.v[1], t.v[2] | m.v[1], t.v[3] | m.v[1]),
			Vector4<T>(t.v[0] | m.v[2], t.v[1] | m.v[2], t.v[2] | m.v[2], t.v[3] | m.v[2]),
			Vector4<T>(t.v[0] | m.v[3], t.v[1] | m.v[3], t.v[2] | m.v[3], t.v[3] | m.v[3])
		);
	}
	Matrix4x4<T> operator*(const Quaternion<T> &q) const { return operator*(Matrix4x4<T>(q)); }
	void operator*=(T f) { v[0] *= f; v[1] *= f; v[2] *= f; v[3] *= f; }
	void operator*=(const Matrix4x4<T> &m) { *this = operator*(m); }
	void operator*=(const Quaternion<T> &q) { *this = operator*(q); }
	//@}

	/// Returns the transpose of the current transformation matrix
	Matrix4x4<T> transpose() const {
		return Matrix4x4<T>(
			Vector4<T>(v[0].x, v[1].x, v[2].x, v[3].x),
			Vector4<T>(v[0].y, v[1].y, v[2].y, v[3].y),
			Vector4<T>(v[0].z, v[1].z, v[2].z, v[3].z),
			Vector4<T>(v[0].w, v[1].w, v[2].w, v[3].w)
		);
	}

	/// Scales the current matrix and returns the result (faster than a generic multiply).
	/// @param[in] s A vector containing the x, y, and z scaling factors
	Matrix4x4<T> scale(const Vector3<T> &s) const {
		return Matrix4x4<T>(
			Vector4<T>(v[0].x * s.x, v[0].y * s.x, v[0].z * s.x, v[0].w),
			Vector4<T>(v[1].x * s.y, v[1].y * s.y, v[1].z * s.y, v[1].w),
			Vector4<T>(v[2].x * s.z, v[2].y * s.z, v[2].z * s.z, v[2].w),
			Vector4<T>(v[3].x, v[3].y, v[3].z, v[3].w)
		);
	}

	/// Translates the current matrix and returns the result (faster than a generic multiply).
	/// @param[in] t A vector containing the x, y, and z translating factors
	Matrix4x4<T> translate(const Vector3<T> &t) const {
		return Matrix4x4<T>(
			Vector4<T>(v[0].x, v[0].y, v[0].z, v[0].w),
			Vector4<T>(v[1].x, v[1].y, v[1].z, v[1].w),
			Vector4<T>(v[2].x, v[2].y, v[2].z, v[2].w),
			Vector4<T>(
				v[3].x + v[0].x * t.x + v[1].x * t.y + v[2].x * t.z,
				v[3].y + v[0].y * t.x + v[1].y * t.y + v[2].y * t.z,
				v[3].z + v[0].z * t.x + v[1].z * t.y + v[2].z * t.z,
				v[3].w)
		);
	}

	/// Transforms a vector by the current transformation matrix
	Vector3<T> vTransform(const Vector3<T> &vec) const {
		// 9 muls, 6 adds
		return Vector3<T>(
			v[0].x*vec.x + v[1].x*vec.y + v[2].x*vec.z + v[3].x,
			v[0].y*vec.x + v[1].y*vec.y + v[2].y*vec.z + v[3].y,
			v[0].z*vec.x + v[1].z*vec.y + v[2].z*vec.z + v[3].z
		);
	}

	/// Transforms a vector by the current transformation matrix
	Vector4<T> vTransform(const Vector4<T> &vec) const {
		// 9 muls, 6 adds
		return Vector4<T>(
			v[0].x*vec.x + v[1].x*vec.y + v[2].x*vec.z + v[3].x*vec.w,
			v[0].y*vec.x + v[1].y*vec.y + v[2].y*vec.z + v[3].y*vec.w,
			v[0].z*vec.x + v[1].z*vec.y + v[2].z*vec.z + v[3].z*vec.w,
			v[0].w*vec.x + v[1].w*vec.y + v[2].w*vec.z + v[3].w*vec.w
		);
	}

	/// Transforms a normal by the current rotation matrix
	Vector3<T> nTransform(const Vector3<T> &n) const {
		// 9 muls, 6 adds
		return Vector3<T>(
			v[0].x*n.x + v[0].y*n.y + v[0].z*n.z,
			v[1].x*n.x + v[1].y*n.y + v[1].z*n.z,
			v[2].x*n.x + v[2].y*n.y + v[2].z*n.z
		);
	}

	char *to_s(char *psz, bool front=true) const {
		char *pszEnd = psz;
		*pszEnd++ = 'm';
		*pszEnd++ = '[';
		for(int i=0; i<4; i++) {
			if(i != 0) {
				*pszEnd++ = ',';
				*pszEnd++ = ' ';
			}
			pszEnd = v[i].to_s(pszEnd, false);
		}
		*pszEnd++ = ']';
		*pszEnd = 0;
		return front ? psz : pszEnd;
	}
	std::string to_s() const {
		char szBuffer[256];
		return to_s(szBuffer);
	}
};

// Methods that required forward declarations
template<class T> inline Matrix3x3<T>::Matrix3x3(const Matrix4x4<T> &m) { *this = m; }
template<class T> inline const Matrix3x3<T> &Matrix3x3<T>::operator=(const Matrix4x4<T> &m) { v[0] = m.v[0]; v[1] = m.v[1]; v[2] = m.v[2]; return *this; }

typedef Matrix3x3<float> mat3;
typedef Matrix4x4<float> mat4;
typedef Matrix3x3<double> dmat3;
typedef Matrix4x4<double> dmat4;

} // namespace VK

#endif // __Matrix_h__
