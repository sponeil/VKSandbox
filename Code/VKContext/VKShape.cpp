// GLShape.cpp
// This code is part of the GLContext library, an object-oriented class
// library designed to make OpenGL 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifdef GL
#include "VKCore.h"
#include "VKShape.h"

namespace VK {

void Shape::buildSquare() {
	float fVert[4][3] = {
		{0, 1, 0},	// Left, top
		{0, 0, 0},	// Left, bottom
		{1, 0, 0},	// Right, bottom
		{1, 1, 0},	// Right, top
	};
	unsigned short nIndex[] = { 0, 1, 2, 0, 2, 3 };

	VK::Vertex<3,3> vert[4];
	for(int i=0; i<4; i++) {
		VK::vec3 v(fVert[i][0], fVert[i][1], fVert[i][2]);
		vert[i].setAttr0(v);
		vert[i].setAttr1(VK::vec3(0, 0, 1));
	}
	vbo.create(4, vert);
	ibo.create(6, nIndex);
}

void Shape::buildCube() {
	float fVert[8][3] = {
		{-1, 1, -1},	// Left, top, back
		{-1, 1, 1},		// Left, top, front
		{1, 1, 1},		// Right, top, front
		{1, 1, -1},		// Right, top, back
		{-1, -1, -1},	// Left, bottom, back
		{-1, -1, 1},	// Left, bottom, front
		{1, -1, 1},		// Right, bottom, front
		{1, -1, -1}		// Right, bottom, back
	};

	VK::Vertex<3,3> vert[8];
	for(int i=0; i<8; i++) {
		VK::vec3 v(fVert[i][0], fVert[i][1], fVert[i][2]);
		vert[i].setAttr0(v);
		vert[i].setAttr1(v.normalize());
	}
	vbo.create(8, vert);

	unsigned short nIndex[] = {
		0, 1, 2, 0, 2, 3, // Top face
		4, 7, 6, 4, 6, 5, // Bottom face
		0, 4, 5, 0, 5, 1, // Left face
		2, 6, 7, 2, 7, 3, // Right face
		0, 7, 4, 0, 3, 7, // Back face
		1, 5, 6, 1, 6, 2, // Front face
	};
	ibo.create(36, nIndex);
}

void Shape::buildSphere(GLushort nSlices, GLushort nSections) {
	GLushort nVertices = nSlices * (nSections-1) + 2;
	std::vector< VK::Vertex<3,3> > vVertex(nVertices);
	std::vector<float> fRingz(nSections+1);
	std::vector<float> fRingSize(nSections+1);
	std::vector<float> fRingx(nSlices+1);
	std::vector<float> fRingy(nSlices+1);

	float fSliceArc = (2*(float)M_PI) / nSlices;
	float fSectionArc = (float)M_PI / nSections;
	for(int i=0; i<=nSections; i++) {
		fRingz[i] = cosf(fSectionArc * i);
		fRingSize[i] = sinf(fSectionArc * i);
	}
	for(int i=0; i<=nSlices; i++) {
		fRingx[i] = cosf(fSliceArc * i);
		fRingy[i] = sinf(fSliceArc * i);
	}

	int nIndex = 0;
	vVertex[nIndex].setAttr0(vec3(0, 0, 1));
	vVertex[nIndex++].setAttr1(vec3(0, 0, 1));
	for(int j=1; j<nSections; j++) {
		for(int i=0; i<nSlices; i++) {
			vec3 v = vec3(fRingx[i] * fRingSize[j], fRingy[i] * fRingSize[j], fRingz[j]).normalize();
			vVertex[nIndex].setAttr0(v.normalize());
			vVertex[nIndex++].setAttr1(v.normalize());
		}
	}
	vVertex[nIndex].setAttr0(vec3(0, 0, -1));
	vVertex[nIndex++].setAttr1(vec3(0, 0, -1));

	std::vector<GLushort> vIndex;
	// Add the triangle fan for the front cap
	for(int i=0; i<=nSlices; i++) {
		vIndex.push_back(0);
		vIndex.push_back(i);
		vIndex.push_back(i == nSlices ? 1 : i+1);
	}

	// Add the belts that go around the middle
	int nIndex1 = 1;
	int nIndex2 = 1 + nSlices;
	for(int j=1; j<nSections-1; j++) {
		for(int i=0; i<nSlices; i++) {
			vIndex.push_back(nIndex1+i);
			vIndex.push_back(nIndex2+i);
			vIndex.push_back(i+1 == nSlices ? nIndex2 : nIndex2+i+1);
			vIndex.push_back(nIndex1+i);
			vIndex.push_back(i+1 == nSlices ? nIndex2 : nIndex2+i+1);
			vIndex.push_back(i+1 == nSlices ? nIndex1 : nIndex1+i+1);
		}
		nIndex1 += nSlices;
		nIndex2 += nSlices;
	}

	// Add the triangle fan for the back cap
	for(int i=1; i<=nSlices; i++) {
		vIndex.push_back(nVertices-1);
		vIndex.push_back(nVertices-1-i);
		vIndex.push_back(nVertices-2 - (i == nSlices ? 0 : i));
	}

	vbo.create((GLuint)vVertex.size(), &vVertex[0]);
	ibo.create((GLuint)vIndex.size(), &vIndex[0]);
}

} // namespace VK
#endif
