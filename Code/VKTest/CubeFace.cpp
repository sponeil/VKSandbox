// CubeFace.cpp
//
#include "../VKContext/VKCore.h"
#include "../VKContext/VKVector.h"
#include "CubeFace.h"
//#include "PlanetaryMapCoord.h"


uint8_t CubeFace::GetFace(const VK::ivec3 &v)
{
	uint8_t nFace;
	int x = VK::Math::Abs(v.x), y = VK::Math::Abs(v.y), z = VK::Math::Abs(v.z);
	if(x > y) {
		if(x > z)
			nFace = (uint8_t)((v.x > 0) ? RightFace : LeftFace);
		else
			nFace = (uint8_t)((v.z > 0) ? FrontFace : BackFace);
	} else {
		if(y > z)
			nFace = (uint8_t)((v.y > 0) ? TopFace : BottomFace);
		else
			nFace = (uint8_t)((v.z > 0) ? FrontFace : BackFace);
	}
	return nFace;
}

uint8_t CubeFace::GetFaceCoordinates(const VK::ivec3 &v, int &x, int &y)
{
	uint8_t nFace;
	int ax = VK::Math::Abs(v.x), ay = VK::Math::Abs(v.y), az = VK::Math::Abs(v.z);
	int sc, tc, ma;
	if(ax > ay && ax > az) {
		ma = ax;
		sc = v.z;
		tc = -v.y;
		if(v.x > 0) {
			nFace = RightFace;
			sc = -sc;
		} else {
			nFace = LeftFace;
		}
	} else if(ay > az) {
		ma = ay;
		sc = v.x;
		tc = v.z;
		if(v.y > 0) {
			nFace = TopFace;
		} else {
			nFace = BottomFace;
			tc = -tc;
		}
	} else {
		ma = az;
		sc = v.x;
		tc = -v.y;
		if(v.z > 0) {
			nFace = FrontFace;
		} else {
			nFace = BackFace;
			sc = -sc;
		}
	}

	// x and y should be approximately from MinCoord to MaxCoord
	x = (muldiv(sc, MaxCoord, ma) + MaxCoord) >> 1;
	y = (muldiv(tc, MaxCoord, ma) + MaxCoord) >> 1;
	return nFace;
}

void CubeFace::GetFaceCoordinates(unsigned char nFace, const VK::ivec3 &v, int &x, int &y)
{
	// The vector passed in may not be in the specified face.
	// If not, the coordinates within nFace closest to v are returned.
	// (This helps find the shortest distance from a point to a node in the quad-tree)
	switch(nFace) {
		case RightFace:
			if(v.x < 0) { // If v is in the left, force the coordinates to the nearest corner of this face.
				x = v.z > 0 ? -MaxCoord : MaxCoord;
				y = v.y > 0 ? -MaxCoord : MaxCoord;
			} else {
				x = v.x <= VK::Math::Abs(v.z) ? (v.z > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.z, MaxCoord, v.x);
				y = v.x <= VK::Math::Abs(v.y) ? (v.y > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.y, MaxCoord, v.x);
			}
			break;
		case LeftFace:
			if(v.x > 0) { // If v is in the right, force the coordinates to the nearest corner of this face.
				x = v.z > 0 ? MaxCoord : -MaxCoord;
				y = v.y > 0 ? -MaxCoord : MaxCoord;
			} else {
				x = -v.x <= VK::Math::Abs(v.z) ? (v.z > 0 ? MaxCoord : -MaxCoord) : muldiv(v.z, MaxCoord, -v.x);
				y = -v.x <= VK::Math::Abs(v.y) ? (v.y > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.y, MaxCoord, -v.x);
			}
			break;
		case TopFace:
			if(v.y < 0) { // If v is in the bottom, force the coordinates to the nearest corner of this face.
				x = v.x > 0 ? MaxCoord : -MaxCoord;
				y = v.z > 0 ? MaxCoord : -MaxCoord;
			} else {
				x = v.y <= VK::Math::Abs(v.x) ? (v.x > 0 ? MaxCoord : -MaxCoord) : muldiv(v.x, MaxCoord, v.y);
				y = v.y <= VK::Math::Abs(v.z) ? (v.z > 0 ? MaxCoord : -MaxCoord) : muldiv(v.z, MaxCoord, v.y);
			}
			break;
		case BottomFace:
			if(v.y > 0) { // If v is in the top, force the coordinates to the nearest corner of this face.
				x = v.x > 0 ? MaxCoord : -MaxCoord;
				y = v.z > 0 ? -MaxCoord : MaxCoord;
			} else {
				x = -v.y <= VK::Math::Abs(v.x) ? (v.x > 0 ? MaxCoord : -MaxCoord) : muldiv(v.x, MaxCoord, -v.y);
				y = -v.y <= VK::Math::Abs(v.z) ? (v.z > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.z, MaxCoord, -v.y);
			}
			break;
		case FrontFace:
			if(v.z < 0) { // If v is in the back, force the coordinates to the nearest corner of this face.
				x = v.x > 0 ? MaxCoord : -MaxCoord;
				y = v.y > 0 ? -MaxCoord : MaxCoord;
			} else {
				x = v.z <= VK::Math::Abs(v.x) ? (v.x > 0 ? MaxCoord : -MaxCoord) : muldiv(v.x, MaxCoord, v.z);
				y = v.z <= VK::Math::Abs(v.y) ? (v.y > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.y, MaxCoord, v.z);
			}
			break;
		case BackFace:
			if(v.z > 0) { // If v is in the front, force the coordinates to the nearest corner of this face.
				x = v.x > 0 ? -MaxCoord : MaxCoord;
				y = v.y > 0 ? -MaxCoord : MaxCoord;
			} else {
				x = -v.z <= VK::Math::Abs(v.x) ? (v.x > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.x, MaxCoord, -v.z);
				y = -v.z <= VK::Math::Abs(v.y) ? (v.y > 0 ? -MaxCoord : MaxCoord) : muldiv(-v.y, MaxCoord, -v.z);
			}
			break;
	}

	x = (x + MaxCoord) >> 1;
	y = (y + MaxCoord) >> 1;
}

VK::ivec3 CubeFace::GetPlanetaryVector(uint8_t nFace, int x, int y, int nLength)
{
	int z = 0;
	x = (x << 1) - MaxCoord;
	y = (y << 1) - MaxCoord;
	switch(nFace) {
		case RightFace:
			z = -x;
			x = MaxCoord;
			y = -y;
			break;
		case LeftFace:
			z = x;
			x = -MaxCoord;
			y = -y;
			break;
		case TopFace:
			z = y;
			y = MaxCoord;
			x = x;
			break;
		case BottomFace:
			z = -y;
			y = -MaxCoord;
			x = x;
			break;
		case FrontFace:
			z = MaxCoord;
			x = x;
			y = -y;
			break;
		case BackFace:
			z = -MaxCoord;
			x = -x;
			y = -y;
			break;
	}

	/*
	int nScale = (int)(sqrt((double)x*x + (double)y*y + (double)z*z) + 0.5);
	return VK::ivec3(
		muldiv(x, nLength, nScale),
		muldiv(y, nLength, nScale),
		muldiv(z, nLength, nScale)
	);
	*/
	// This is more precise and faster due to having fewer conversions between double and int.
	// (May not be more precise or faster using 64-bit ints on 64-bit Windows. Need to test.)
	double d = nLength / sqrt((double)x*x + (double)y*y + (double)z*z);
	return VK::ivec3(
		(int)(x * d + (x < 0 ? -0.5 : 0.5)),
		(int)(y * d + (y < 0 ? -0.5 : 0.5)),
		(int)(z * d + (z < 0 ? -0.5 : 0.5)));
}

void CubeFace::CrossEdge(int w, uint8_t nEdge, uint8_t &nFace, int &x, int &y, int n) {
	uint8_t nReturnEdge = NeighborEdge(nFace, nEdge);
	nFace = NeighborFace(nFace, nEdge);
	switch(nEdge) {
		case TopEdge:
			switch(nReturnEdge) {
				case TopEdge:		y = n; x = w-x; break;
				case BottomEdge:	y = w-n; x = x; break;
				case LeftEdge:		y = x; x = n; break;
				case RightEdge:		y = w-x; x = w-n; break;
			}
			break;
		case BottomEdge:
			switch(nReturnEdge) {
				case TopEdge:		y = n; x = x; break;
				case BottomEdge:	y = w-n; x = w-x; break;
				case LeftEdge:		y = w-x; x = n; break;
				case RightEdge:		y = x; x = w-n; break;
			}
			break;
		case LeftEdge:
			switch(nReturnEdge) {
				case TopEdge:		x = y; y = n; break;
				case BottomEdge:	x = w-y; y = w-n; break;
				case LeftEdge:		x = n; y = w-y; break;
				case RightEdge:		x = w-n; y = y; break;
			}
			break;
		case RightEdge:
			switch(nReturnEdge) {
				case TopEdge:		x = w-y; y = n; break;
				case BottomEdge:	x = y; y = w-n; break;
				case LeftEdge:		x = n; y = y; break;
				case RightEdge:		x = w-n; y = w-y; break;
			}
			break;
	}
}

void CubeFace::CrossEdge(uint8_t nEdge, uint8_t &nFace, double &x, double &y, double n)
{
	const double w = 1.0f;
	uint8_t nReturnEdge = NeighborEdge(nFace, nEdge);
	nFace = NeighborFace(nFace, nEdge);
	switch(nEdge) {
		case TopEdge:
			switch(nReturnEdge) {
				case TopEdge:		y = n; x = w-x; break;
				case BottomEdge:	y = w-n; x = x; break;
				case LeftEdge:		y = x; x = n; break;
				case RightEdge:		y = w-x; x = w-n; break;
			}
			break;
		case BottomEdge:
			switch(nReturnEdge) {
				case TopEdge:		y = n; x = x; break;
				case BottomEdge:	y = w-n; x = w-x; break;
				case LeftEdge:		y = w-x; x = n; break;
				case RightEdge:		y = x; x = w-n; break;
			}
			break;
		case LeftEdge:
			switch(nReturnEdge) {
				case TopEdge:		x = y; y = n; break;
				case BottomEdge:	x = w-y; y = w-n; break;
				case LeftEdge:		x = n; y = w-y; break;
				case RightEdge:		x = w-n; y = y; break;
			}
			break;
		case RightEdge:
			switch(nReturnEdge) {
				case TopEdge:		x = w-y; y = n; break;
				case BottomEdge:	x = y; y = w-n; break;
				case LeftEdge:		x = n; y = y; break;
				case RightEdge:		x = w-n; y = w-y; break;
			}
			break;
	}
}

void CubeFace::GetNeighborCoordinates(uint8_t nEdge, uint8_t &nFace, double &x, double &y) {
	const double w = 1.0f;
	double n = 0.0f;
	uint8_t nReturnEdge = NeighborEdge(nFace, nEdge);
	nFace = NeighborFace(nFace, nEdge);
	switch (nEdge) {
		case TopEdge:
			n = -y;
			switch (nReturnEdge) {
				case TopEdge:		y = n; x = w - x; break;
				case BottomEdge:	y = w - n; x = x; break;
				case LeftEdge:		y = x; x = n; break;
				case RightEdge:		y = w - x; x = w - n; break;
			}
			break;
		case BottomEdge:
			n = y - w;
			switch (nReturnEdge) {
				case TopEdge:		y = n; x = x; break;
				case BottomEdge:	y = w - n; x = w - x; break;
				case LeftEdge:		y = w - x; x = n; break;
				case RightEdge:		y = x; x = w - n; break;
			}
			break;
		case LeftEdge:
			n = -x;
			switch (nReturnEdge) {
				case TopEdge:		x = y; y = n; break;
				case BottomEdge:	x = w - y; y = w - n; break;
				case LeftEdge:		x = n; y = w - y; break;
				case RightEdge:		x = w - n; y = y; break;
			}
			break;
		case RightEdge:
			n = x - w;
			switch (nReturnEdge) {
				case TopEdge:		x = w - y; y = n; break;
				case BottomEdge:	x = y; y = w - n; break;
				case LeftEdge:		x = n; y = y; break;
				case RightEdge:		x = w - n; y = w - y; break;
			}
			break;
	}
}

