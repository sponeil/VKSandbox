// GLShape.h
// This code is part of the GLContext library, an object-oriented class
// library designed to make OpenGL 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __GLShape_h__
#define __GLShape_h__

#ifdef GL
#include "VKIndexBufferObject.h"
#include "VKVertexBufferObject.h"

namespace VK {

/// This class encapsulates simple common shapes (rectangles for billboards, boxes, spheres).
/// If you have a shape you use a lot, this would be a good place to add it.
/// Each shape creates its own VBO and IBO, and is generally intended to be used
/// for simple things. For example, the unit rectangle is used for drawing text using bitmap fonts.
struct Shape {
	VertexBufferObject vbo; ///< A VBO created for this shape
	IndexBufferObject<GLushort> ibo; ///< An IBO created for this shape
	GLenum nMode; ///< The rendering mode to use for this shape (i.e. GL_TRIANGLES)

	Shape() : nMode(GL_TRIANGLES), vbo(3, 3) {}

	/// Enables this shape for rendering
	void enable() {
		vbo.enable();
		ibo.bind(true);
	}
	/// Disables this shape for rendering
	void disable() {
		vbo.disable();
		ibo.bind(false);
	}

	/// Draws one instance of this shape
	void draw() { ibo.draw(nMode); }

#ifdef GL_VERSION_3_1
	/// Draws multiple instances of this shape
	void drawInstances(GLuint nInstances) { ibo.drawInstanced(nInstances, nMode); }
#endif

	/// Builds a unit square (cornersfrom 0,0 to 1,1)
	void buildSquare();

	/// Builds a unit cube (corners from -1,-1,-1 to 1,1,1)
	void buildCube();

	/// Builds a unit sphere using the specified number of slices and sections (axes from -1 to 1)
	void buildSphere(GLushort nSlices, GLushort nSections);
};

} // namespace VK
#endif

#endif // __GLShape_h__
