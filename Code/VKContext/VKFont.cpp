// GLFont.cpp
// This code is part of the GLContext library, an object-oriented class
// library designed to make OpenGL 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKFont.h"

using namespace VK;

namespace VK {

inline float LittleEndianFloat(float f) {
	int test = 1;
	if(*((char *)&test) == 0) { // Big-endian, need to swap bytes
		unsigned int &i = *(unsigned int *)&f;
		i = (i << 24) | ((i << 8) & 0x00FF0000) | ((i >> 8) & 0x0000FF00) | (i >> 24);
	}
	return f;
}

void Font::destroy() {
	vbo.destroy();
	ibo.destroy();
}

bool Font::load(const char *pszFile) {
	VKLogDebug("Loading font: %s", pszFile);
	
	std::string strFile;
	int nFile = 0;
#ifdef ANDROID
	strFile = Path(pszFile).apk_read();
#else
	strFile = Path(pszFile).read();
#endif

	if(strFile.length() < 3+96+1+28) {
		VKLogException("Invalid font file: %s (too small for header)", pszFile);
		return false;
	}
	
	char buffer[128];
	memcpy(buffer, &strFile[nFile], 3); // ifFont.read(buffer, 3);
	nFile += 3;
	buffer[3] = 0;
	if(strcmp(buffer, "GLF") != 0) {
		VKLogException("Invalid font file header: %s", pszFile);
		return false;
	}

	memcpy(buffer, &strFile[nFile], 96); // ifFont.read(buffer, 96);
	nFile += 96;
	buffer[96] = 0;
	name = buffer;

	unsigned char sym_total = strFile[nFile++]; // ifFont.read((char *)&sym_total, 1); // Read total symbols in font
	memcpy(buffer, &strFile[nFile], 28); // ifFont.read(buffer, 28); // Read unused data
	nFile += 28;

	enum { INVALID = 65535 };
	struct EdgeKey {
		usvec2 vert; // The indexes to the 2 vertices that make up this edge
		EdgeKey(unsigned short v1, unsigned short v2) {
			vert[0] = VK::Math::Min(v1, v2); // For comparison purposes, always store the lower index in v[0]
			vert[1] = VK::Math::Max(v1, v2); // For comparison purposes, always store the upper index in v[1]
		}
		bool operator<(const EdgeKey &e) const { return *(unsigned int *)&vert < *(unsigned int *)&e.vert; }
	};
	struct EdgeInfo {
		usvec2 tri; // The indexes to the 2 triangles sharing this edge
		EdgeInfo(unsigned short t=INVALID) {
			tri[0] = t;
			tri[1] = INVALID;
		}
		bool isShared() const { return tri[1] != INVALID; }
		unsigned short findOpposite(const EdgeKey &edge, const std::vector<usvec3> &faces) const {
			const usvec3 &face = faces[tri[0]];
			if(face.x != edge.vert[0] && face.x != edge.vert[1])
				return face.x;
			if(face.y != edge.vert[0] && face.y != edge.vert[1])
				return face.y;
			return face.z;
		}
	};

	struct VertexInfo {
		unsigned short neighbor[2]; // The neighboring vertices found along outer edges
		unsigned short outline; // The index of the outline vertex created for this one
		vec2 pos; // The position of this vertex (for calculating normals)
		vec2 normal; // The calculated normal of this vertex (based on the neighboring outer edges)
		VertexInfo(const vec2 &p) : pos(p) { neighbor[0] = neighbor[1] = INVALID; }
		int add(unsigned short n) {
			if(neighbor[0] == INVALID) {
				neighbor[0] = n;
				return 1;
			}
			neighbor[1] = n;
			return 2;
		}
	};

	typedef std::map<EdgeKey, EdgeInfo> EdgeInfoMap;
	typedef std::vector<VertexInfo> VertexInfoVector;

	// Now read each symbol
	unsigned int nTotalVertices = 0, nTotalIndices = 0;
	for(int sindex=0; sindex<sym_total; sindex++) {
		if(strFile.length() - nFile < 4) {
			VKLogException("Invalid font file: %s (truncated at symbol %d)", pszFile, sindex);
			return false;
		}
		unsigned char code = strFile[nFile++];
		size_t verts = (unsigned char)strFile[nFile++];
		size_t faces = (unsigned char)strFile[nFile++];
		size_t lines = (unsigned char)strFile[nFile++];

		Symbol &symbol = symbols[code];
		if(!symbol.vertices.empty()) {
			VKLogException("Invalid font file: %s (encountered same symbol twice in font)", pszFile);
			return false;
		}
		if(strFile.length() - nFile < lines + faces*3 + verts*2 * sizeof(float)) {
			VKLogException("Invalid font file: %s (truncated at symbol %d)", pszFile, sindex);
			return false;
		}

		// Fetch the vertices from the file and convert them from vec2 to vec3
		VertexInfoVector vInfo;
		vInfo.reserve(verts);
		symbol.vertices.reserve(verts);
		for(size_t i=0; i < verts; i++, nFile += 2*sizeof(float)) {
			float x = LittleEndianFloat(*(float *)&strFile[nFile]);
			float y = LittleEndianFloat(*(float *)&strFile[nFile+sizeof(float)]);
			symbol.vertices.push_back(vec3(x, y, 0.0f));
			vInfo.push_back(VertexInfo(symbol.vertices.back()));
		}

		// Fetch the face definitions from the file
		symbol.faces.reserve(faces);
		for(size_t i = 0; i<faces; i++, nFile += 3) {
			symbol.faces.push_back(usvec3((unsigned char)strFile[nFile+0], (unsigned char)strFile[nFile+1], (unsigned char)strFile[nFile+2]));
		}

		// Skip over any lines in the file
		nFile += (int)lines;

		// Now build a set of edge info to detect shared edges vs. outer edges
		EdgeInfoMap edges;
		for(size_t i=0; i < symbol.faces.size(); i++) { // For each face
			EdgeKey key[3] = { // Each face has 3 edges
				EdgeKey(symbol.faces[i].x, symbol.faces[i].y),
				EdgeKey(symbol.faces[i].x, symbol.faces[i].z),
				EdgeKey(symbol.faces[i].y, symbol.faces[i].z),
			};
			for(size_t e = 0; e < 3; e++) { // For each edge in the face
				EdgeInfoMap::iterator itEdge = edges.find(key[e]);
				if(itEdge == edges.end()) { // If it's a new edge, add it to the map tied to this triangle
					edges[key[e]] = EdgeInfo((unsigned short)i);
				} else { // If we have seen it before, it is a shared edge with 2 triangles
					itEdge->second.tri[1] = (unsigned short)i;
				}
			}
		}

		// Now tie outer (not shared) edges to the vertices they link
		for(EdgeInfoMap::iterator itEdge = edges.begin(); itEdge != edges.end(); itEdge++) {
			if(!itEdge->second.isShared()) {
				// Tell each vertex in this edge that they're neighbors
				vInfo[itEdge->first.vert[0]].add(itEdge->first.vert[1]);
				vInfo[itEdge->first.vert[1]].add(itEdge->first.vert[0]);
			}
		}

		// Add a new outline vertex for each existing vertex
		for(size_t i = 0; i < vInfo.size(); i++) {
			// Find the outer edges tied to this vertex so we can calculate a normal vector
			VertexInfo &vert = vInfo[i];
			vert.outline = 0;
			if(vert.neighbor[1] == INVALID)
				continue;
			EdgeInfoMap::iterator itEdge1 = edges.find(EdgeKey((unsigned short)i, vert.neighbor[0]));
			EdgeInfoMap::iterator itEdge2 = edges.find(EdgeKey((unsigned short)i, vert.neighbor[1]));
			vec2 v0 = vert.pos;
			vec2 v1 = vInfo[vert.neighbor[0]].pos;
			vec2 v2 = vInfo[vert.neighbor[1]].pos;

			// Calculate the normal of the first outer edge
			vec2 n1 = vec2(v1.y-v0.y, v0.x-v1.x).normalize(); // (y, -x) should be perpendicular
			vec2 o1 = vInfo[itEdge1->second.findOpposite(itEdge1->first, symbol.faces)].pos;
			if(n1.dot(o1-v0) > 0) // Make sure it faces away from the opposite vertex in its triangle
				n1 = -n1;

			// Calculate the normal of the second outer edge
			vec2 n2 = vec2(v2.y-v0.y, v0.x-v2.x).normalize(); // (y, -x) should be perpendicular
			vec2 o2 = vInfo[itEdge2->second.findOpposite(itEdge2->first, symbol.faces)].pos;
			if(n2.dot(o2-v0) > 0) // Make sure it faces away from the opposite vertex in its triangle
				n2 = -n2;

			// Average the two normals
			vert.normal = (n1 + n2).normalize();

			// Create a new vertex pushed out along the normal vector
			vert.outline = (unsigned short)symbol.vertices.size();
			symbol.vertices.push_back(vec3(vert.pos + vert.normal * 0.1f, 1.0f));
		}

		for(EdgeInfoMap::iterator itEdge = edges.begin(); itEdge != edges.end(); itEdge++) {
			if(!itEdge->second.isShared()) { // Skip shared edges (we only care about outer edges)
				// Add the outer edge and its outline to the wireframe line list
				usvec2 vOutline(vInfo[itEdge->first.vert[0]].outline, vInfo[itEdge->first.vert[1]].outline);
				symbol.lines.push_back(itEdge->first.vert);
				if(vOutline.x == 0 || vOutline.y == 0)
					continue;
				symbol.lines.push_back(vOutline);

				// Add the new triangles we need for the outline
				usvec3 t1(itEdge->first.vert.x, itEdge->first.vert.y, vOutline.x);
				usvec3 t2(itEdge->first.vert.y, vOutline.y, vOutline.x);
				vec3 v1, v2;
				v1 = vec3(symbol.vertices[t1.y] - symbol.vertices[t1.x]);
				v2 = vec3(symbol.vertices[t1.z] - symbol.vertices[t1.x]);
				v1.z = v2.z = 0;
				if(v1.cross(v2).z < 0)
					VK::Math::Swap(t1.y, t1.z);
				v1 = vec3(symbol.vertices[t2.y] - symbol.vertices[t2.x]);
				v2 = vec3(symbol.vertices[t2.z] - symbol.vertices[t2.x]);
				v1.z = v2.z = 0;
				if(v1.cross(v2).z < 0)
					VK::Math::Swap(t2.y, t2.z);
				symbol.outline.push_back(t1);
				symbol.outline.push_back(t2);
			}
		}

		nTotalVertices += (unsigned int)symbol.vertices.size();
		nTotalIndices += (unsigned short)(symbol.outline.size() * 3 + symbol.faces.size() * 3 + symbol.lines.size() * 2);

		// Read vertex data
		symbol.leftx = 10;
		symbol.rightx = -10;
		symbol.topy = 10;
		symbol.bottomy = -10;
		for(size_t i = 0; i < symbol.vertices.size(); i++) {
			float x = symbol.vertices[i].x, y = symbol.vertices[i].y;
			if(x < symbol.leftx) symbol.leftx = x;
			if(x > symbol.rightx) symbol.rightx = x;
			if(y < symbol.topy) symbol.topy = y;
			if(y > symbol.bottomy) symbol.bottomy = y;
		}
	}
	//ifFont.close();

	ib.resize(nTotalIndices);
	vb.resize(nTotalVertices*3);
	unsigned int nVertex = 0, nIndex = 0;
	for(size_t i = 0; i < 256; i++) {
		Symbol &symbol = symbols[i];
		if(symbol.vertices.empty())
			continue;

		// Add the indices for the character outline (render these before the character)
		symbol.oOffset = nIndex;
		for(std::vector<usvec3>::iterator it = symbol.outline.begin(); it != symbol.outline.end(); it++) {
			ib[nIndex++] = nVertex + it->x;
			ib[nIndex++] = nVertex + it->y;
			ib[nIndex++] = nVertex + it->z;
		}

		// Add the indices for the solid characters
		symbol.fOffset = nIndex;
		for(std::vector<usvec3>::iterator it = symbol.faces.begin(); it != symbol.faces.end(); it++) {
			ib[nIndex++] = nVertex + it->x;
			ib[nIndex++] = nVertex + it->y;
			ib[nIndex++] = nVertex + it->z;
		}

		// Add the indices for the wireframe characters
		symbol.wOffset = nIndex;
		for(std::vector<usvec2>::iterator it = symbol.lines.begin(); it != symbol.lines.end(); it++) {
			ib[nIndex++] = nVertex + it->x;
			ib[nIndex++] = nVertex + it->y;
		}

		// Add the vertices last (or nVertex will be wrong when setting up indices)
		symbol.vOffset = nVertex;
		memcpy(&vb[nVertex*3], &symbol.vertices[0], symbol.vertices.size() * 3 * sizeof(float));
		nVertex += (unsigned int)symbol.vertices.size();
	}

	vbo.create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, vb.size() * sizeof(float));
	vbo.update(&vb[0]);
	ibo.create(VK_BUFFER_USAGE_INDEX_BUFFER_BIT, ib.size() * sizeof(uint16_t));
	ibo.update(&ib[0]);

	Symbol &sym = symbols[' '];
	sym.leftx = -0.25f;
	sym.rightx = 0.25f;
	VKLogInfo("Loaded font file: %s (vertices = %u, indices = %u)", pszFile, nTotalVertices, nTotalIndices);
	return true;
}

float Font::measure(const char *pszText) {
	float w = 0.0f;
	for(const char *psz=pszText; *psz; ++psz) {
		Symbol &sym = symbols[*psz];
		w += (0.1f + sym.rightx - sym.leftx);
	}
	w -= 0.1f;
	return w * m_fSize;
}

void Font::begin(VkCommandBuffer cmd, const vec4 &color, float size) {
	VkDeviceSize offsets[1] = { 0 };
	VkBuffer buffers[] = { vbo };
	vkCmdBindVertexBuffers(cmd, 0, 1, buffers, offsets);
	vkCmdBindIndexBuffer(cmd, ibo, 0, VK_INDEX_TYPE_UINT16);
	m_fSize = size;
	m_vColor = color;
}

void Font::end(VkCommandBuffer cmd) {
}

void Font::draw2D(VkCommandBuffer cmd, vec4 *pData, uint32_t &nInstance, const char *pszText, vec2 vPos, AlignX xAlign, AlignY yAlign) {
	vec4 v4(vPos, 0, m_fSize);
	float w = measure(pszText);

	switch (yAlign) {
		case AlignYBottom:
			v4.y += m_fSize * 1.5f  * 0.5f;
			break;
		case AlignYTop:
			v4.y -= m_fSize * 1.5f  * 0.5f;
			break;
	}
	switch (xAlign) {
		case AlignXCenter:
			v4.x -= w * 0.5f;
			break;
		case AlignXRight:
			v4.x -= w;
			break;
	}

	for (const char *psz = pszText; *psz; ++psz) {
		Symbol &sym = symbols[*psz];
		v4.x -= sym.leftx * m_fSize;
		if (!sym.vertices.empty()) {
			*pData++ = v4;
			*pData++ = m_vColor;
			vkCmdDrawIndexed(cmd, (uint32_t)(sym.outline.size() + sym.faces.size()) * 3, 1, sym.oOffset, 0, nInstance++);
		}
		v4.x += (sym.rightx + 0.1f) * m_fSize;
	}
}

#if 0
void Font::draw3D(const char *pszText, vec3 vPos, vec2 vSize, AlignX xAlign, AlignY yAlign) {
	vec4 v4(vPos, m_fSize);
	float w = measure(pszText);
	gl.uniform4f(m_nCharColor, m_vColor.x, m_vColor.y, m_vColor.z, m_vColor.w);

	switch(yAlign) {
		case AlignYBottom:
			v4.y += m_fSize * 1.5f  * 0.5f;
			break;
		case AlignYTop:
			v4.y -= m_fSize * 1.5f  * 0.5f;
			break;
	}
	switch(xAlign) {
		case AlignXCenter:
			v4.x -= w * 0.5f;
			break;
		case AlignXRight:
			v4.x -= w;
			break;
	}

	for(const char *psz=pszText; *psz; ++psz) {
		Symbol &sym = symbols[*psz];
		v4.x -= sym.leftx * m_fSize;
		if(!sym.vertices.empty()) {
			gl.uniform4f(m_nCharPos, v4.x, v4.y, v4.z, v4.w);
			ibo.draw(GL_TRIANGLES, sym.fOffset, (GLuint)sym.faces.size()*3);
			//ibo.draw(GL_LINES, sym.wOffset, (GLuint)sym.lines.size()*2);
		}
		v4.x += (sym.rightx+0.1f) * m_fSize;
	}
}
#endif

} // namespace VK
