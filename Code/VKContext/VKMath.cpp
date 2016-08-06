// VKMath.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKTransform.h"

namespace VK {

// Force specific instantiations to be built (ensures compilation and allows some methods to be defined in the cpp)
//template class Vector1<float>;
//template class Vector1<double>;
//template class Vector2<float>;
//template class Vector2<double>;
//template class Vector3<float>;
//template class Vector3<double>;
//template class Vector4<float>;
//template class Vector4<double>;
//template class Quaternion<float>;
//template class Quaternion<double>;
//template class Matrix3x3<float>;
//template class Matrix3x3<double>;
//template class Matrix4x4<float>;
//template class Matrix4x4<double>;


/// Converts the current rotation matrix to a quaternion
template<class T>
Quaternion<T> Matrix3x3<T>::to_q() const {
	Quaternion<T> q;
	T tr = v[0].x + v[1].y + v[2].z; // Check the sum of the diagonal
	if(tr > 0) { // The sum is positive
		// 4 muls, 1 div, 6 adds, 1 sqrt call
		T s = sqrt(1 + tr);
		q.w = s * (T)0.5;
		s = (T)0.5 / s;
		q.x = (v[1].z - v[2].y) * s;
		q.y = (v[2].x - v[0].z) * s;
		q.z = (v[0].y - v[1].x) * s;
	} else {
		// The sum is negative
		// 4 muls, 1 div, 8 adds, 1 sqrt call
		unsigned int i = v[1].y > v[0].x ? (v[2].z > v[1].y ? 2 : 1) : (v[2].z > v[0].x ? 2 : 0);
		const unsigned int nIndex[3] = {1, 2, 0};
		unsigned int j = nIndex[i];
		unsigned int k = nIndex[j];

		const Matrix3x3 &m = *this;
		T s = sqrt(1 + m[i][i] - m[j][j] - m[k][k]);
		q[i] = s * 0.5f;
		if(s != 0.0)
			s = 0.5f / s;
		q[j] = (m[i][j] + m[j][i]) * s;
		q[k] = (m[i][k] + m[k][i]) * s;
		q[3] = (m[j][k] - m[k][j]) * s;
	}
	return q;
}


} // namespace VK
