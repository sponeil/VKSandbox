// VKFont.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKFont_h__
#define __VKFont_h__

#include "VKShaderTechnique.h"
#include "VKBufferObject.h"
#include "VKTransform.h"

namespace VK {

/// Encapsulates a font texture and simple text rendering.
/// A TTF file is loaded into a PixelBuffer using FreeType. A set of character
/// positions is built, and the buffer is used to initialize the font texture.
/// Default text rendering routines are applied for 2D coordinates (ortho
/// projection) and 3D coordinates (perspective projection).
class Font : public VK::Object {
protected:
	struct Symbol {
		std::vector<vec3> vertices;  // Vertices (x,y)
		std::vector<usvec3> outline; // Outline faces
		std::vector<usvec3> faces;   // Faces
		std::vector<usvec2> lines;   // Lines
		unsigned int vOffset, oOffset, fOffset, wOffset;

		float leftx;           // Smaller x coordinate
		float rightx;          // Right x coordinate
		float topy;            // Top y coordinate
		float bottomy;         // Bottom y coordinate

		Symbol() : leftx(0), rightx(0), topy(0), bottomy(0) {}
	};

	std::string name;
	Symbol symbols[256];
	std::vector<float> vb; //VK::VBO vbo;
	std::vector<uint16_t> ib; //VK::IBOMedium ibo;
	BufferObject vbo, ibo;
	VK::vec4 m_vColor;
	int m_nCharPos, m_nCharColor;
	float m_fSize;

public:
	/// Used to specify text alignment in the x direction
	enum AlignX {
		AlignXLeft = 0, ///< The X render position represents the left edge of the text
		AlignXCenter = 1, ///< The X render position represents the center of the text
		AlignXRight = 2, ///< The X render position represents the right edge of the text
	};
	/// Used to specify text alignment in the y direction
	enum AlignY {
		AlignYTop = 0, ///< The Y render position represents the top edge of the text
		AlignYCenter = 1, ///< The Y render position represents the center of the text
		AlignYBottom = 2, ///< The Y render position represents the bottom edge of the text
	};

	Font() {}
	virtual ~Font() { destroy(); }
	virtual void destroy();

	/// Loads a TTF file and builds a font texture.
	bool load(const char *pszFile);

	float getSize() { return m_fSize; }
	void setSize(float f) { m_fSize = f; }
	void setColor(const VK::vec4 &v) { m_vColor = v; }

	/// Measures the width of a text string using this font
	float measure(const char *pszText);

	/// Call to enable the states needed for rendering text using the specified shader technique
	void begin(VkCommandBuffer cmd, const vec4 &color, float size);

	/// Call to disable the states needed for rendering text using the specified shader technique
	void end(VkCommandBuffer cmd);

	/// Call to draw text using the Text2D shader technique
	void draw2D(VkCommandBuffer cmd, vec4 *pData, uint32_t &nInstance, const char *pszText, VK::vec2 vPos, AlignX xAlign = AlignXLeft, AlignY yAlign = AlignYTop);

#if 0
	/// Call to draw text using the Text3D shader technique
	void draw3D(const char *pszText, VK::vec3 vPos, VK::vec2 vSize, AlignX xAlign = AlignXCenter, AlignY yAlign = AlignYCenter);
#endif
};

} // namespace VK

#endif // __VKFont_h__
