// CubeFace.h
//
#ifndef __CubeFace_h__
#define __CubeFace_h__

namespace VK {

#include "../../Build/shaders/VKTest.h"

}

/// Encapsulates the complexities of representing a sphere with 6 cube faces.
/// This class handles converting 3D vectors to and from cube face coordinates,
/// as well as finding nearby coordinates in neighboring cube faces.
/// These tasks are not trivial because each face has top, bottom, left, and
/// right edges, but one face's top edge doesn't always match its neighbor's
/// bottom edge (to the face containing the north pole, every edge would be
/// the bottom).
class CubeFace
{
private:
	/// Safely multiplies two 32-bit ints (which creates a 64-bit int) and divides by another.
	/// It is only safe if d is does not overflow 32 bits, but various checks in this class will ensure that.
	static int muldiv(int n, int m, int d) {
#ifdef _WIN64
		// In 64-bit mode on my CPU, this "should" be faster (haven't tried it yet).
		return (int)(((__int64)n * (__int64)m) / d);
#else
		// In 32-bit mode on my CPU, the 64-bit divide kills the performance.
		// A double should have enough precision, and it's faster.
		double ret = (((double)n * (double)m) / d);
		return (int)(ret + (ret > 0 ? 0.5 : -0.5));
#endif
	}

public:

	enum { MaxCoord = 1 << 24 };

	/// Use to find a cube face's neighbor on any side.
	/// \param nFace The starting cube face
	/// \param nEdge The edge (or side) of the neighbor you want
	/// \return The neighboring face on the specified side
	static uint8_t NeighborFace(uint8_t nFace, uint8_t nEdge) { return (uint8_t)VK::NeighborFace[nFace][nEdge]; }

	/// Use to find the neighbor's edge that matches the specified edge.
	/// \param nFace The starting cube face
	/// \param nEdge The edge (or side) of the neighbor you want
	/// \return The neighboring face on the specified side
	static uint8_t NeighborEdge(uint8_t nFace, uint8_t nEdge) { return (uint8_t)VK::NeighborEdge[nFace][nEdge]; }

	static int to_i(double f) { return (int)(f * MaxCoord + 0.5); }
	static double to_f(int i) { return (double)i / (double)MaxCoord; }
	static double to_f(int i, double length) { return (i*length) / (double)MaxCoord; }
	static double to_f(int i, int max) { return (double)i / (double)max; }
	static double to_f(int i, int max, double length) { return (i*length) / (double)max; }

	static VK::ivec3 to_i(const VK::dvec3 &v) { double f = 1 / v.mag(); return VK::ivec3(to_i(v.x*f), to_i(v.y*f), to_i(v.z*f)); }
	static VK::ivec3 to_i(const VK::vec3 &v) { double f = 1 / v.mag(); return VK::ivec3(to_i(v.x*f), to_i(v.y*f), to_i(v.z*f)); }
	static VK::dvec3 to_f(const VK::ivec3 &v) { return VK::dvec3(to_f(v.x), to_f(v.y), to_f(v.z)); }
	static VK::dvec3 to_f(const VK::ivec3 &v, double length) { return VK::dvec3(to_f(v.x, length), to_f(v.y, length), to_f(v.z, length)); }
	static VK::dvec3 to_f(const VK::ivec3 &v, int max) { return VK::dvec3(to_f(v.x, max), to_f(v.y, max), to_f(v.z, max)); }
	static VK::dvec3 to_f(const VK::ivec3 &v, int max, double length) { return VK::dvec3(to_f(v.x, max, length), to_f(v.y, max, length), to_f(v.z, max, length)); }


	/// Takes any 3D vector and determines what cube face it's in.
	/// \param v A 3D vector (relative to the center of the cube)
	/// \return The cube face v is in
	static uint8_t GetFace(const VK::ivec3 &v);
	static uint8_t GetFace(const VK::dvec3 &v) { return GetFace(to_i(v)); }

	//************************************************************
	// floating-point coordinate methods
	//************************************************************

	/// Takes a set of cube face coordinates and converts them into a 3D vector.
	/// \param nFace The desired cube face
	/// \param x The x coordinate within the cube face (0.0 - MaxCoord)
	/// \param y The y coordinate within the cube face (0.0 - MaxCoord)
	/// \param nLength The desired length, or magnitude, of the vector
	/// \return A 3D vector going through the specified face coordinates (and of the specified length)
	static VK::ivec3 GetPlanetaryVector(uint8_t nFace, int x, int y, int nLength);
	static VK::dvec3 GetPlanetaryVector(uint8_t nFace, double x, double y) {
		return to_f(GetPlanetaryVector(nFace, to_i(x), to_i(y), MaxCoord), MaxCoord);
	}
	static VK::dvec3 GetPlanetaryVector(uint8_t nFace, double x, double y, double fLength) {
		return to_f(GetPlanetaryVector(nFace, to_i(x), to_i(y), MaxCoord), MaxCoord, fLength);
	}

	/// Takes a 3D vector and converts it into cube face coordinates.
	/// This version of the function always returns coordinates in the specified face.
	/// If the vector is not in the specified face, the nearest coordinates are returned.
	/// See: "Mapping Texture Coordinates to Cube Map Faces" in http://developer.nvidia.com/object/cube_map_ogl_tutorial.html
	/// \param nFace The desired cube face
	/// \param v A 3D vector (relative to the center of the cube)
	/// \param x (Out) Set to the closest x coordinate in the specified cube face
	/// \param y (Out) Set to the closest y coordinate in the specified cube face
	static void GetFaceCoordinates(uint8_t nFace, const VK::ivec3 &v, int &x, int &y);
	static void GetFaceCoordinates(uint8_t nFace, const VK::vec3 &v, float &x, float &y) {
		int ix, iy;
		GetFaceCoordinates(nFace, to_i(v), ix, iy);
		x = (float)to_f(ix);
		y = (float)to_f(iy);
	}
	static void GetFaceCoordinates(uint8_t nFace, const VK::dvec3 &v, double &x, double &y) {
		int ix, iy;
		GetFaceCoordinates(nFace, to_i(v), ix, iy);
		x = to_f(ix);
		y = to_f(iy);
	}

	/// Takes a 3D vector and converts it into cube face coordinates.
	/// This version of the function determines which cube face the vector is in.
	/// See: "Mapping Texture Coordinates to Cube Map Faces" in http://developer.nvidia.com/object/cube_map_ogl_tutorial.html
	/// \param v A 3D vector (relative to the center of the cube)
	/// \param x (Out) Set to the closest x coordinate in the specified cube face
	/// \param y (Out) Set to the closest y coordinate in the specified cube face
	/// \return The cube face the specified vector is in
	static uint8_t GetFaceCoordinates(const VK::ivec3 &v, int &x, int &y);
	static uint8_t GetFaceCoordinates(const VK::dvec3 &v, double &x, double &y) {
		int ix, iy;
		uint8_t nFace = GetFaceCoordinates(to_i(v), ix, iy);
		x = to_f(ix);
		y = to_f(iy);
		return nFace;
	}

	/// Takes a set of coordinates in one cube face and finds coordinates close to it in a neighboring cube face.
	/// \param w The width of the integer coordinate system (not counting the shared edge)
	/// \param nEdge The edge to cross over
	/// \param nFace (In/Out) Takes the initial cube face, returns the new cube face
	/// \param x (In/Out) Takes the initial x coordinate, returns the new x coordinate
	/// \param y (In/Out) Takes the initial y coordinate, returns the new y coordinate
	/// \param n The distance to cross over the edge (0 returns a point on the edge)
	static void CrossEdge(int w, uint8_t nEdge, uint8_t &nFace, int &x, int &y, int n=0);

	/// Takes a set of coordinates in one cube face and finds coordinates close to it in a neighboring cube face.
	/// \param nEdge The edge to cross over
	/// \param nFace (In/Out) Takes the initial cube face, returns the new cube face
	/// \param x (In/Out) Takes the initial x coordinate, returns the new x coordinate
	/// \param y (In/Out) Takes the initial y coordinate, returns the new y coordinate
	/// \param n The distance to cross over the edge (0 returns a point on the edge)
	static void CrossEdge(uint8_t nEdge, uint8_t &nFace, double &x, double &y, double n=0.0f);
	
	// Same as CrossEdge above, but instead of crossing over, leaves x and y where they are (may be outside the face boundaries)
	static void GetNeighborCoordinates(uint8_t nEdge, uint8_t &nFace, double &x, double &y);

	static void AdjustCoords(uint8_t &nFace, double &x, double &y) {
		if(x < 0) {
			y = VK::Math::Max(0.0, VK::Math::Min(1.0, y)); // There are no diagonal neighbors
			CrossEdge(LeftEdge, nFace, x, y, 0-x);
		}
		else if(x > 1.0) {
			y = VK::Math::Max(0.0, VK::Math::Min(1.0, y)); // There are no diagonal neighbors
			CrossEdge(RightEdge, nFace, x, y, x-1.0);
		}
		else if(y < 0)
			CrossEdge(TopEdge, nFace, x, y, 0-y);
		else if(y > 1.0)
			CrossEdge(BottomEdge, nFace, x, y, y-1.0);
	}

	static void AdjustCoords(int w, uint8_t &nFace, int &x, int &y) {
		if(x < 0) {
			y = (DWORD)VK::Math::Max(0, VK::Math::Min(w, y)); // There are no diagonal neighbors
			CrossEdge(w, LeftEdge, nFace, x, y, 0-x);
		}
		if(x > w) {
			y = (DWORD)VK::Math::Max(0, VK::Math::Min(w, y)); // There are no diagonal neighbors
			CrossEdge(w, RightEdge, nFace, x, y, x-w);
		}
		if(y < 0)
			CrossEdge(w, TopEdge, nFace, x, y, 0-y);
		if(y > w)
			CrossEdge(w, BottomEdge, nFace, x, y, y-w);
	}
};

#endif
