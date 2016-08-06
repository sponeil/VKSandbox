// VKGeometry.h
//

#ifndef __VKGeometry_h__
#define __VKGeometry_h__

#include "VKMatrix.h"

namespace VK {

// The plane is represented as Dot(N,X) = c where N is the plane normal
// vector, not necessarily unit length, c is the plane constant, and X is
// any point on the plane.
class Plane {
public:
	vec3 normal;
	float constant;

public:
	Plane() {}
	Plane(const vec3 &p1, const vec3 &p2, const vec3 &p3) { init(p1, p2, p3); }
	Plane(const vec3 &n, const vec3 &p) { init(n, p); }
	Plane(const vec3 &n, const float f) { init(n, f); }

	void init(const vec3 &p1, const vec3 &p2, const vec3 &p3) { init((p2-p1).cross(p3-p1).normalize(), p1); }
	void init(const vec3 &n, const vec3 &p) { init(n, -(p | n)); }
	void init(const vec3 &n, const float f) { normal = n; constant = f; }

	// A positive, 0, or negative result indicates the point is in front of, on, or behind the plane
	float distance(const vec3 &p) const { return (normal | p) + constant; }

	// Returns true if the line intersects the plane and changes vPos to the location of the intersection
	bool intersection(vec3 &vPos, vec3 &vDir) const {
		float f = normal | vDir;
		if(VK::Math::Abs(f) < DELTA)
			return false;
		vPos -= vDir * (distance(vPos) / f);
		return true;
	}
};

class Frustum
{
protected:
	Plane m_plFrustum[6];

public:
	Frustum() {}

	void init(const mat4 &mProj, const mat4 &mView, const mat4 &mModel)
	{
		mat4 mModelview = mView * mModel;
		const float *proj = mProj;
		const float *modl = mModelview;
		float   clip[16];
		float   t;

		/* Combine the two matrices (multiply projection by modelview) */
		clip[ 0] = modl[ 0] * proj[ 0] + modl[ 1] * proj[ 4] + modl[ 2] * proj[ 8] + modl[ 3] * proj[12];
		clip[ 1] = modl[ 0] * proj[ 1] + modl[ 1] * proj[ 5] + modl[ 2] * proj[ 9] + modl[ 3] * proj[13];
		clip[ 2] = modl[ 0] * proj[ 2] + modl[ 1] * proj[ 6] + modl[ 2] * proj[10] + modl[ 3] * proj[14];
		clip[ 3] = modl[ 0] * proj[ 3] + modl[ 1] * proj[ 7] + modl[ 2] * proj[11] + modl[ 3] * proj[15];

		clip[ 4] = modl[ 4] * proj[ 0] + modl[ 5] * proj[ 4] + modl[ 6] * proj[ 8] + modl[ 7] * proj[12];
		clip[ 5] = modl[ 4] * proj[ 1] + modl[ 5] * proj[ 5] + modl[ 6] * proj[ 9] + modl[ 7] * proj[13];
		clip[ 6] = modl[ 4] * proj[ 2] + modl[ 5] * proj[ 6] + modl[ 6] * proj[10] + modl[ 7] * proj[14];
		clip[ 7] = modl[ 4] * proj[ 3] + modl[ 5] * proj[ 7] + modl[ 6] * proj[11] + modl[ 7] * proj[15];

		clip[ 8] = modl[ 8] * proj[ 0] + modl[ 9] * proj[ 4] + modl[10] * proj[ 8] + modl[11] * proj[12];
		clip[ 9] = modl[ 8] * proj[ 1] + modl[ 9] * proj[ 5] + modl[10] * proj[ 9] + modl[11] * proj[13];
		clip[10] = modl[ 8] * proj[ 2] + modl[ 9] * proj[ 6] + modl[10] * proj[10] + modl[11] * proj[14];
		clip[11] = modl[ 8] * proj[ 3] + modl[ 9] * proj[ 7] + modl[10] * proj[11] + modl[11] * proj[15];

		clip[12] = modl[12] * proj[ 0] + modl[13] * proj[ 4] + modl[14] * proj[ 8] + modl[15] * proj[12];
		clip[13] = modl[12] * proj[ 1] + modl[13] * proj[ 5] + modl[14] * proj[ 9] + modl[15] * proj[13];
		clip[14] = modl[12] * proj[ 2] + modl[13] * proj[ 6] + modl[14] * proj[10] + modl[15] * proj[14];
		clip[15] = modl[12] * proj[ 3] + modl[13] * proj[ 7] + modl[14] * proj[11] + modl[15] * proj[15];

		/* Extract the numbers for the RIGHT plane */
		m_plFrustum[0].normal.x = clip[ 3] - clip[ 0];
		m_plFrustum[0].normal.y = clip[ 7] - clip[ 4];
		m_plFrustum[0].normal.z = clip[11] - clip[ 8];
		m_plFrustum[0].constant = clip[15] - clip[12];
		t = m_plFrustum[0].normal.mag();
		m_plFrustum[0].normal /= t;
		m_plFrustum[0].constant /= t;

		/* Extract the numbers for the LEFT plane */
		m_plFrustum[1].normal.x = clip[ 3] + clip[ 0];
		m_plFrustum[1].normal.y = clip[ 7] + clip[ 4];
		m_plFrustum[1].normal.z = clip[11] + clip[ 8];
		m_plFrustum[1].constant = clip[15] + clip[12];
		t = m_plFrustum[1].normal.mag();
		m_plFrustum[1].normal /= t;
		m_plFrustum[1].constant /= t;

		/* Extract the BOTTOM plane */
		m_plFrustum[2].normal.x = clip[ 3] + clip[ 1];
		m_plFrustum[2].normal.y = clip[ 7] + clip[ 5];
		m_plFrustum[2].normal.z = clip[11] + clip[ 9];
		m_plFrustum[2].constant = clip[15] + clip[13];
		t = m_plFrustum[2].normal.mag();
		m_plFrustum[2].normal /= t;
		m_plFrustum[2].constant /= t;

		/* Extract the TOP plane */
		m_plFrustum[3].normal.x = clip[ 3] - clip[ 1];
		m_plFrustum[3].normal.y = clip[ 7] - clip[ 5];
		m_plFrustum[3].normal.z = clip[11] - clip[ 9];
		m_plFrustum[3].constant = clip[15] - clip[13];
		t = m_plFrustum[3].normal.mag();
		m_plFrustum[3].normal /= t;
		m_plFrustum[3].constant /= t;

		/* Extract the FAR plane */
		m_plFrustum[4].normal.x = clip[ 3] - clip[ 2];
		m_plFrustum[4].normal.y = clip[ 7] - clip[ 6];
		m_plFrustum[4].normal.z = clip[11] - clip[10];
		m_plFrustum[4].constant = clip[15] - clip[14];
		t = m_plFrustum[4].normal.mag();
		m_plFrustum[4].normal /= t;
		m_plFrustum[4].constant /= t;

		/* Extract the NEAR plane */
		m_plFrustum[5].normal.x = clip[ 3] + clip[ 2];
		m_plFrustum[5].normal.y = clip[ 7] + clip[ 6];
		m_plFrustum[5].normal.z = clip[11] + clip[10];
		m_plFrustum[5].constant = clip[15] + clip[14];
		t = m_plFrustum[5].normal.mag();
		m_plFrustum[5].normal /= t;
		m_plFrustum[5].constant /= t;
	}

	bool isInFrustum(const vec3 &vPos, const float fRadius) const
	{
		for(int i=0; i<4; ++i)
		{
			if(m_plFrustum[i].distance(vPos) < -fRadius)
				return false;
		}
		return true;
	}
};

} // namespace VK

#endif // __VKGeometry_h__
