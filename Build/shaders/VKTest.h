// VKTest.h
//

#define TestWidth 65

#define MAX_LEVELS 6

#define TopEdge 0
#define RightEdge 1
#define BottomEdge 2
#define LeftEdge 3

#define RightFace 0
#define LeftFace 1
#define TopFace 2
#define BottomFace 3
#define FrontFace 4
#define BackFace 5
#define FaceCount 6

// Each face has 4 neighboring faces across the TopEdge, RightEdge, BottomEdge, and LeftEdge
const ivec4 NeighborFace[FaceCount] = {
	{TopFace, BackFace, BottomFace, FrontFace}, // Right face
	{TopFace, FrontFace, BottomFace, BackFace}, // Left face
	{BackFace, RightFace, FrontFace, LeftFace}, // Top face
	{FrontFace, RightFace, BackFace, LeftFace}, // Bottom face
	{TopFace, RightFace, BottomFace, LeftFace}, // Front face
	{TopFace, LeftFace, BottomFace, RightFace}, // Back face
};

// These are the edges on those neighboring faces that lead back to it.
const ivec4 NeighborEdge[FaceCount] = {
	{RightEdge, LeftEdge, RightEdge, RightEdge},      // Right face
	{LeftEdge, LeftEdge, LeftEdge, RightEdge},        // Left face
	{TopEdge, TopEdge, TopEdge, TopEdge},             // Top face
	{BottomEdge, BottomEdge, BottomEdge, BottomEdge}, // Bottom face
	{BottomEdge, LeftEdge, TopEdge, RightEdge},       // Front face
	{TopEdge, LeftEdge, BottomEdge, RightEdge},       // Back face
};

// Example:
// NeighborFace[FrontFace][LeftEdge] would be LeftFace, and
// NeighborEdge[FrontFace][LeftEdge] would be RightEdge, so
// NeighborFace[LeftFace][RightEdge] would point back to FrontFace

struct PlanetFaceData {
	ivec4 iFace;
	vec4 vCorners;
	vec4 vHole;
};


