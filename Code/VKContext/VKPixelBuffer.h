// VKPixelBuffer.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan API easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKPixelBuffer_h__
#define __VKPixelBuffer_h__

//#include "VKContext.h"
#include "VKTransform.h"

namespace VK {

/// Encapsulates a pixel buffer in system memory (for initializing textures).
/// It supports 1D, 2D, and 3D buffers and is templatized to provide type-
/// safety and to automate values passed to VK functions like VK_FLOAT.
/// Specific instantiations are forced to be built into VKContext.lib,
/// which allows me to put some of the methods in the cpp. It also limits the
/// type T to signed and unsigned: char, short, int, float, and double.
/// @note This class contains a number of advanced methods for manipulating
/// buffers. It also contains methods for building common types of textures,
/// like a glow map or a noise map.
template <class T> class PixelBuffer {
protected:
	uint16_t m_nWidth;			///< The width of the buffer (x axis)
	uint16_t m_nHeight;			///< The height of the buffer (y axis)
	uint16_t m_nDepth;			///< The depth of the buffer (z axis)
	uint32_t m_nPixels;			///< The number of pixels in the buffer
	uint8_t m_nChannels;		///< The number of channels of data stored in the buffer
	uint32_t m_nFormat;			///< The format of the pixel data (i.e. VK_RGB, VK_RGBA)
	bool m_bAlloc;				///< Set to true if this class allocated the buffer
	T *m_pBuffer;				///< A pointer to the buffer (aligned to a 64-byte boundary for things like SSE operations)

	/// Used internally to validate the buffer before operating on it
	void checkBuffer() const {
		if(!isValid()) VKLogException("VK::PixelBuffer - Attempting to access NULL buffer");
	}

	/// Used internally to validate the buffer before operating on it
	void checkBuffer(uint32_t nPixel) const {
		checkBuffer();
		if(nPixel > m_nPixels) VKLogException("VK::PixelBuffer - Attempting to access invalid buffer index");
	}

	static void splineStretchRow(T *pDest, uint16_t wDest, const T *pSrc, uint16_t wSrc, uint8_t nChannels, uint32_t nStride, bool bRepeat) {
		if(wSrc < 2)
			VKLogException("Attempting to stretch an empty dimension");

		// We need a buffer of 4 neighboring values for spline interpolation
		T val[4][4]; // (Actually, we need 4 neighboring values for each channel)
		int nLast; // This is always the leading edge in our buffer of values
		int nStart[4] = {bRepeat ? wSrc-2 : 0, bRepeat ? wSrc-1 : 0, 0, 1};
		for(int i=0; i<4; i++) {
			nLast = nStart[i];
			val[i][0] = pSrc[nLast*nStride+0];
			if(nChannels > 1) {
				val[i][1] = pSrc[nLast*nStride+1];
				if(nChannels > 2) {
					val[i][2] = pSrc[nLast*nStride+2];
					if(nChannels > 3)
						val[i][3] = pSrc[nLast*nStride+3];
				}
			}
		}

		int index = 0; // Shift the index into the val array so we don't have to shift the values
		float inc = (float)wSrc / (float)wDest; // This is how much t is incremented when we move over a pixel
		float t = 0.5f + inc * 0.5f; // Remember to start at the center of the pixel or the texture will shift toward the origin

		// Calculate the value for each destination pixel
		for(int x = 0; x < wDest; x++) {
			// See if we have moved to the next interval between pixels
			// (When shrinking, the initial t may be >= 1.0, so this needs to come first in the loop)
			while(t >= 1.0f) { // A while loop is needed when shrinking
				t -= 1.0f;
				if(++nLast == wSrc)
					nLast = bRepeat ? 0 : nLast-1;
				index++;
				val[(index+3)&3][0] = pSrc[nLast*nStride];
				if(nChannels > 1) {
					val[(index+3)&3][1] = pSrc[nLast*nStride+1];
					if(nChannels > 2) {
						val[(index+3)&3][2] = pSrc[nLast*nStride+2];
						if(nChannels > 3)
							val[(index+3)&3][3] = pSrc[nLast*nStride+3];
					}
				}
			}

			// Perform the spline interpolation for each channel
			for(int i=0; i<nChannels; i++)
				pDest[i] = clamp(Math::CatmullRom(val[(index)&3][i], val[(index+1)&3][i], val[(index+2)&3][i], val[(index+3)&3][i], t));
			pDest += nStride; // Increment pDest to the next destination pixel
			t += inc; // Increment t to the next sample point
		}
	}

public:
	/// Default constructor, initializes memebers to NULL.
	PixelBuffer() : m_bAlloc(false), m_pBuffer(NULL) {}

	/// Creates a buffer at construction time.
	/// @param[in] nWidth The width of the buffer you wish to create
	/// @param[in] nHeight The height of the buffer you wish to create (set to 1 for 1D textures)
	/// @param[in] nDepth The depth of the buffer you wish to create (set to 1 for 1D or 2D textures)
	/// @param[in] nChannels The number of channels in the buffer
	/// @param[in] nFormat The format of the pixel data (i.e. VK_RGB, VK_RGBA)
	/// @param[in] pBuffer Allows you to use an existing buffer instead of allocating a new one
	PixelBuffer(uint16_t nWidth, uint16_t nHeight, uint16_t nDepth=1, uint8_t nChannels=1, uint32_t nFormat=(uint32_t)-1, T *pBuffer=NULL) : m_bAlloc(false), m_pBuffer(NULL) {
		create(nWidth, nHeight, nDepth, nChannels, nFormat, pBuffer);
	}

	/// Creates a copy of an existing PixelBuffer at construction time.
	/// @param[in] pb The pixel buffer you wish to copy
	PixelBuffer(const PixelBuffer &pb) : m_bAlloc(false), m_pBuffer(NULL) { *this = pb; }
	/// Default destructor, calls cleanup()
	~PixelBuffer()					{ destroy(); }

	/// Creates a new pixel buffer (does not initialize it).
	/// @param[in] nWidth The width of the buffer you wish to create
	/// @param[in] nHeight The height of the buffer you wish to create (set to 1 for 1D textures)
	/// @param[in] nDepth The depth of the buffer you wish to create (set to 1 for 1D or 2D textures)
	/// @param[in] nChannels The number of channels in the buffer
	/// @param[in] nFormat The format of the pixel data (i.e. VK_RGB, VK_RGBA)
	/// @param[in] pBuffer Allows you to use an existing buffer instead of allocating a new one
	void create(uint16_t nWidth, uint16_t nHeight, uint16_t nDepth=1, uint8_t nChannels=1, uint32_t nFormat=(uint32_t)-1, T *pBuffer=NULL) {
		destroy();
		m_nWidth = nWidth;
		m_nHeight = nHeight;
		m_nDepth = nDepth;
		m_nPixels = m_nWidth * m_nHeight * m_nDepth;
		m_nChannels = nChannels;
		m_pBuffer = pBuffer ? pBuffer : (T *)malloc(getBufferSize());
		m_bAlloc = m_pBuffer != pBuffer;

		m_nFormat = nFormat;
		// TODO: Choose correct formats based on template args (these assume uint8_t)
		if(m_nFormat == -1)
			m_nFormat = nChannels == 1 ? VK_FORMAT_R8_UNORM : nChannels == 2 ? VK_FORMAT_R8G8_UNORM : nChannels == 3 ? VK_FORMAT_R8G8B8_UNORM : VK_FORMAT_R8G8B8A8_UNORM;
	}

	/// Destroys the pixel buffer and frees any memory it allocated
	void destroy() {
		if(m_bAlloc)
			free(m_pBuffer);
		m_pBuffer = NULL;
		m_bAlloc = false;
	}

	/// Call to make sure the buffer is valid before operating on it
	bool isValid() const		{ return m_pBuffer != NULL; }
	/// Call to get the buffer width
	uint16_t getWidth() const 	{ return m_nWidth; }
	/// Call to get the buffer height
	uint16_t getHeight() const	{ return m_nHeight; }
	/// Call to get the buffer depth
	uint16_t getDepth() const	{ return m_nDepth; }
	/// Call to get the number of channels in the buffer
	uint8_t getChannels() const	{ return m_nChannels; }
	/// Call to get the buffer format
	uint32_t getFormat() const	{ return m_nFormat; }
	/// Call to get the number of pixels in the buffer
	int getNumPixels() const	{ return m_nPixels; }
	/// Call to get the number of T elements in the buffer
	int getNumElements() const	{ return getNumPixels() * m_nChannels; }
	/// Call to get the size of the entire buffer in bytes
	int getBufferSize() const	{ return getNumElements() * sizeof(T); }
	/// Call to get a pointer to the buffer
	T *getBuffer() const		{ return m_pBuffer; }
	/// Call to get the VK data type when creating a texture or using functions like glReadPixels
	//static uint32_t getDataType() { return Context::VKDataType<T>(); }

	/// Returns true if two buffers have the same size and format
	bool operator==(const PixelBuffer<T> &buf) { return (m_nWidth == buf.m_nWidth && m_nHeight == buf.m_nHeight && m_nDepth == buf.m_nDepth && m_nChannels == buf.m_nChannels && m_nFormat == buf.m_nFormat); }
	/// Returns true if two buffers DO NOT have the same size and format
	bool operator!=(const PixelBuffer<T> &buf) { return !operator==(buf); }

	/// Creates a copy of a PixelBuffer
	void operator=(const PixelBuffer<T> &buf) {
		if(buf.isValid()) {
			create(buf.m_nWidth, buf.m_nHeight, buf.m_nDepth, buf.m_nChannels);
			memcpy(m_pBuffer, buf.m_pBuffer, getBufferSize());
		}
	}

	/// Converts a float buffer to a type T buffer, clamping values to the max-min range
	void clamp(const PixelBuffer<float> &buf) {
		if(buf.isValid()) {
			create(buf.getWidth(), buf.getHeight(), buf.getDepth(), buf.getChannels());
			float *pBuf = buf.getBuffer();
			unsigned int nSize = getNumElements();
			for(unsigned int i=0; i<nSize; i++)
				m_pBuffer[i] = clamp(pBuf[i]);
		}
	}

	/// Converts a float buffer to a type T buffer, scaling values up to the max-min range
	void scale(const PixelBuffer<float> &buf) {
		if(buf.isValid()) {
			create(buf.getWidth(), buf.getHeight(), buf.getDepth(), buf.getChannels());
			float *pBuf = buf.getBuffer();
			unsigned int nSize = getNumElements();
			for(unsigned int i=0; i<nSize; i++)
				m_pBuffer[i] = scale(pBuf[i]);
		}
	}

	/// Fills the buffer with a specific value
	void operator=(T t) {
		checkBuffer();
		if(t == (T)0 || sizeof(T) == 1) {
			memset(m_pBuffer, (int)t, getBufferSize());
		} else {
			unsigned int nSize = getNumElements();
			for(unsigned int i=0; i<nSize; i++)
				m_pBuffer[i] = t;
		}
	}

	/// Adds a specific value to every element in the buffer
	void operator+=(T t) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] += t;
	}

	/// Subtracts a specific value from every element in the buffer
	void operator-=(T t) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] -= t;
	}

	/// Multiplies every element in the buffer by a specific value
	void operator*=(T t) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] *= t;
	}

	/// Divides every element in the buffer by a specific value
	void operator/=(T t) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] /= t;
	}

	/// Performs a shift right on every element in the buffer
	void operator>>=(int nShift) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] = (T)((int)m_pBuffer[i] >> nShift);
	}

	/// Performs a shift left on every element in the buffer
	void operator<<=(int nShift) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] = (T)((int)m_pBuffer[i] << nShift);
	}

	/// Adds the contents of another buffer to this one
	void operator+=(const PixelBuffer<T> &pb) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] += pb.m_pBuffer[i];
	}

	/// Subtracts the contents of another buffer to this one
	void operator-=(const PixelBuffer<T> &pb) {
		checkBuffer();
		unsigned int nSize = getNumElements();
		for(unsigned int i=0; i<nSize; i++)
			m_pBuffer[i] -= pb.m_pBuffer[i];
	}

	/// Clears the buffer using memset(0)
	void clear() {
		checkBuffer();
		memset(m_pBuffer, 0, getBufferSize());
	}

	/// Copies another buffer to this one using memcpy
	void copy(const T *p) {
		checkBuffer();
		memcpy(m_pBuffer, p, getBufferSize());
	}

	/// Swaps pointers for two buffers (they must have the same size and format)
	void swap(PixelBuffer<T> &buf) {
		checkBuffer();
		if(*this != buf)
			VKLogException("PixelBuffer::swap() - Attempting to swap buffers of different types");
		Math::Swap(m_pBuffer, buf.m_pBuffer);
	}

	/// Returns a pointer to the start of any pixel in the buffer.
	/// @note All the other pixel accessor methods call this one. To avoid redundant
	/// calls to checkBuffer(), it is only called from this accessor method.
	      T *operator[](uint32_t nPixel)                         { checkBuffer(nPixel); return &m_pBuffer[nPixel*m_nChannels]; }
	const T *operator[](uint32_t nPixel) const                   { return ((PixelBuffer *)this)->operator[](nPixel); }

	/// Returns a pointer to the start of any pixel in the buffer using 1D, 2D, or 3D coordinates
	      T *operator()(uint32_t x)                              { return operator[](x); }
	const T *operator()(uint32_t x) const                        { return ((PixelBuffer *)this)->operator()(x); }
	      T *operator()(uint32_t x, uint32_t y)                    { return operator()(m_nWidth * y + x); }
	const T *operator()(uint32_t x, uint32_t y) const              { return ((PixelBuffer *)this)->operator()(x, y); }
	      T *operator()(uint32_t x, uint32_t y, uint32_t z)          { return operator()(x, m_nHeight * z + y); }
	const T *operator()(uint32_t x, uint32_t y, uint32_t z) const    { return ((PixelBuffer *)this)->operator()(x, y, z); }

	/// Returns a pixel as a reference to a Vector1 (should generally only be used with 1-channel buffers)
	      Vector1<T> &vec1(uint32_t x)                           { return *(Vector1<T> *)operator()(x); }
	const Vector1<T> &vec1(uint32_t x) const                     { return *(Vector1<T> *)operator()(x); }
	      Vector1<T> &vec1(uint32_t x, uint32_t y)                 { return *(Vector1<T> *)operator()(x, y); }
	const Vector1<T> &vec1(uint32_t x, uint32_t y) const           { return *(Vector1<T> *)operator()(x, y); }
	      Vector1<T> &vec1(uint32_t x, uint32_t y, uint32_t z)       { return *(Vector1<T> *)operator()(x, y, z); }
	const Vector1<T> &vec1(uint32_t x, uint32_t y, uint32_t z) const { return *(Vector1<T> *)operator()(x, y, z); }

	/// Returns a pixel as a reference to a Vector2 (should generally only be used with 2-channel buffers)
	      Vector2<T> &vec2(uint32_t x)                           { return *(Vector2<T> *)operator()(x); }
	const Vector2<T> &vec2(uint32_t x) const                     { return *(Vector2<T> *)operator()(x); }
	      Vector2<T> &vec2(uint32_t x, uint32_t y)                 { return *(Vector2<T> *)operator()(x, y); }
	const Vector2<T> &vec2(uint32_t x, uint32_t y) const           { return *(Vector2<T> *)operator()(x, y); }
	      Vector2<T> &vec2(uint32_t x, uint32_t y, uint32_t z)       { return *(Vector2<T> *)operator()(x, y, z); }
	const Vector2<T> &vec2(uint32_t x, uint32_t y, uint32_t z) const { return *(Vector2<T> *)operator()(x, y, z); }

	/// Returns a pixel as a reference to a Vector3 (should generally only be used with 3-channel buffers)
	      Vector3<T> &vec3(uint32_t x)                           { return *(Vector3<T> *)operator()(x); }
	const Vector3<T> &vec3(uint32_t x) const                     { return *(Vector3<T> *)operator()(x); }
	      Vector3<T> &vec3(uint32_t x, uint32_t y)                 { return *(Vector3<T> *)operator()(x, y); }
	const Vector3<T> &vec3(uint32_t x, uint32_t y) const           { return *(Vector3<T> *)operator()(x, y); }
	      Vector3<T> &vec3(uint32_t x, uint32_t y, uint32_t z)       { return *(Vector3<T> *)operator()(x, y, z); }
	const Vector3<T> &vec3(uint32_t x, uint32_t y, uint32_t z) const { return *(Vector3<T> *)operator()(x, y, z); }

	/// Returns a pixel as a reference to a Vector4 (should generally only be used with 4-channel buffers)
	      Vector4<T> &vec4(uint32_t x)                           { return *(Vector4<T> *)operator()(x); }
	const Vector4<T> &vec4(uint32_t x) const                     { return *(Vector4<T> *)operator()(x); }
	      Vector4<T> &vec4(uint32_t x, uint32_t y)                 { return *(Vector4<T> *)operator()(x, y); }
	const Vector4<T> &vec4(uint32_t x, uint32_t y) const           { return *(Vector4<T> *)operator()(x, y); }
	      Vector4<T> &vec4(uint32_t x, uint32_t y, uint32_t z)       { return *(Vector4<T> *)operator()(x, y, z); }
	const Vector4<T> &vec4(uint32_t x, uint32_t y, uint32_t z) const { return *(Vector4<T> *)operator()(x, y, z); }

	/// Returns a linear-interpolated value for the specified coordinate (in the specified channel)
	T linear(uint8_t c, float x, float y=0, float z=0) const {
		float fX = x*(m_nWidth-1);
		int nX = VK::Math::Min(m_nWidth-2, VK::Math::Max(0, (int)fX));
		float rX = fX - nX;
		if(m_nHeight == 1) {
			T *p = m_pBuffer + nX * m_nChannels + c;
			return (T)(p[0] * (1-rX) + p[m_nChannels] * rX);
		}
		float fY = y*(m_nHeight-1);
		int nY = VK::Math::Min(m_nHeight-2, VK::Math::Max(0, (int)fY));
		float rY = fY - nY;
		if(m_nDepth == 1) {
			T *p0 = m_pBuffer + (nY * m_nWidth + nX) * m_nChannels + c;
			T *p1 = p0 + m_nWidth*m_nChannels;
			return	(T)(p0[0] * (1-rX) * (1-rY) + p0[m_nChannels] * rX * (1-rY) +
					p1[0] * (1-rX) * rY + p1[m_nChannels] * rX * rY);
		}
		float fZ = z*(m_nDepth-1);
		int nZ = VK::Math::Min(m_nDepth-2, VK::Math::Max(0, (int)fZ));
		float rZ = fZ - nZ;
		T *p00 = m_pBuffer + ((nZ * m_nHeight + nY) * m_nWidth + nX) * m_nChannels + c;
		T *p01 = p00 + m_nWidth*m_nChannels;
		T *p10 = p00 + m_nWidth*m_nHeight*m_nChannels;
		T *p11 = p10 + m_nWidth*m_nChannels;
		return	(T)(p00[0] * (1-rX) * (1-rY) * (1-rZ) +
				p00[m_nChannels] * rX * (1-rY) * (1-rZ) +
				p01[0] * (1-rX) * rY * (1-rZ) +
				p01[m_nChannels] * rX * rY * (1-rZ) +
				p10[0] * (1-rX) * (1-rY) * rZ +
				p10[m_nChannels] * rX * (1-rY) * rZ +
				p11[0] * (1-rX) * rY * rZ +
				p11[m_nChannels] * rX * rY * rZ);
	}

	/// Flips the buffer horizontally, vertically, or both (in place).
	/// @param[in] bHorz Set to true to flip horizontally
	/// @param[in] bVert Set to true to flip vertically
	void flip(bool bHorz=true, bool bVert=true) {
		T *p = (T *)m_pBuffer;
		int xmax = !bHorz || bVert ? m_nWidth : (m_nWidth+1)/2;
		int ymax = !bVert ? m_nHeight : (m_nHeight+1)/2;
		int x, y;
		for(y=0; y < ymax; ++y) {
			int y2 = bVert ? ((m_nHeight-1) - y) : y;
			int off1 = y*m_nWidth*m_nChannels, off2 = y2*m_nWidth*m_nChannels;
			if(bHorz) {
				if(bVert && y == y2) xmax = (m_nWidth+1)/2;
				off2 += (m_nWidth-1)*m_nChannels;
				for(x=0; x < xmax; ++x) {
					for(int c = 0; c < m_nChannels; c++)
						Math::Swap(p[off1++], p[off2+c]);
					off2 -= m_nChannels;
				}
			} else {
				for(x=0; x < xmax; ++x) {
					for(int c = 0; c < m_nChannels; c++)
						Math::Swap(p[off1++], p[off2++]);
				}
			}
		}
	}

	/// Shrinks a buffer by cropping it.
	/// @param[in] pb The PixelBuffer to crop
	/// @param[in] w The width of the cropped buffer
	/// @param[in] h The height of the cropped buffer
	/// @param[in] d The depth of the cropped buffer
	/// @param[in] x0 The starting x position of the cropped buffer
	/// @param[in] y0 The starting y position of the cropped buffer
	/// @param[in] z0 The starting z position of the cropped buffer
	void crop(const PixelBuffer<T> &pb, int w, int h, int d=1, int x0=0, int y0=0, int z0=0) {
		create(w, h, d, pb.getChannels());
		for(int z=0; z<d; ++z) {
			for(int y=0; y<h; ++y) {
				memcpy(operator()(0, y, z), pb(0+x0, y+y0, z+z0), w*m_nChannels*sizeof(T));
			}
		}
	}

	/// Expands a buffer by tiling it.
	/// @param[in] pb The PixelBuffer to tile
	/// @param[in] w The width of the expanded buffer
	/// @param[in] h The height of the expanded buffer
	/// @param[in] d The depth of the expanded buffer
	void tile(const PixelBuffer<T> &pb, int w, int h, int d=1) {
		create(w, h, d, pb.getChannels());
		T *pDest = m_pBuffer;
		for(int z=0; z<d; z++) {
			int tz = z % pb.getDepth();
			for(int y=0; y<h; y++) {
				int ty = y % pb.getHeight();
				for(int x=0; x<w; x++) {
					int tx = x % pb.getWidth();
					const T *pSrc = pb(tx, ty, tz);
					for(int c=0; c<m_nChannels; c++) {
						*pDest++ = *pSrc++;
					}
				}
			}
		}
	}

	/// Stretches a buffer using spline interpolation (can be used to expand or shrink).
	/// The spline interpolation makes the stretched values very smooth.
	/// @param[in] pb The PixelBuffer to tile
	/// @param[in] w The width of the new buffer
	/// @param[in] h The height of the new buffer
	/// @param[in] d The depth of the new buffer
	/// @param[in] bRepeat Set to true for repeating textures, false to clamp to edge
	void stretch(const PixelBuffer<T> &pb, int w, int h=1, int d=1, bool bRepeat=true) {
		// Do we need to stretch the x dimension?
		if(w == pb.getWidth()) {
			*this = pb;
		} else {
			create(w, pb.getHeight(), pb.getDepth(), pb.getChannels());
			for(int z=0; z<m_nDepth; z++) {
				for(int y=0; y<m_nHeight; y++)
					splineStretchRow((*this)(0, y, z), m_nWidth, pb(0, y, z), pb.getWidth(), m_nChannels, m_nChannels, bRepeat);
			}
		}

		// Do we need to stretch in the y dimension?
		if(h != pb.getHeight()) {
			PixelBuffer<T> pbTemp = *this;
			create(w, h, pbTemp.getDepth(), pbTemp.getChannels());
			for(int z=0; z<m_nDepth; z++) {
				for(int x=0; x<m_nWidth; x++)
					splineStretchRow((*this)(x, 0, z), m_nHeight, pbTemp(x, 0, z), pbTemp.getHeight(), m_nChannels, m_nWidth*m_nChannels, bRepeat);
			}
		}

		// Do we need to stretch in the z dimension?
		if(d != pb.getDepth()) {
			PixelBuffer<T> pbTemp = *this;
			create(w, h, d, pbTemp.getChannels());
			for(int y=0; y<m_nHeight; y++) {
				for(int x=0; x<m_nWidth; x++)
					splineStretchRow((*this)(x, y, 0), m_nDepth, pbTemp(x, y, 0), pbTemp.getDepth(), m_nChannels, m_nWidth*m_nHeight*m_nChannels, bRepeat);
			}
		}
	}

	/// Clamps a float to the min..max range of T (without scaling).
	static T clamp(float f);

	/// Scales f by the max value of T and then calls clamp() to clamp it.
	static T scale(float f);

	/// Like scale, but for unsigned types T, shifts f from -1 .. 1 to the 0 .. 1 range to avoid clamping to 0.
	static T shift(float f);

	/// Like clamp, but using the absolute value
	static T clampAbs(float f) { return clamp(Math::Abs(f)); }

	/// Like scale, but using the absolute value
	static T scaleAbs(float f) { return scale(Math::Abs(f)); }

	/// Builds a glow map using an exponential fade from the center.
	/// @param[in] fExpose The exponent used for the falloff rate
	/// @param[in] fSizeDisc The routine leaves a solid white disc in the center this size
	void makeGlow(float fExpose=5.0f, float fSizeDisc=0.01f) {
		int n = 0;
		VK::vec2 vFactor(2.0f / m_nWidth, 2.0f / m_nHeight);
		for(int y=0; y<m_nHeight; y++) {
			float fDy = (y+0.5f) * vFactor.y - 1.0f;
			for(int x=0; x<m_nWidth; x++) {
				float fDx = (x+0.5f) * vFactor.x - 1.0f;
				float fDist = sqrtf(fDx*fDx + fDy*fDy);
//#define JITTER_VKOW
#ifdef JITTER_VKOW
				fDist += (rand() - RAND_MAX / 2) * (0.005f / RAND_MAX);
#endif
				float fIntensity = fDist < fSizeDisc ? 1.0f : fDist > 1.0f ? 0.0f : expf((fSizeDisc-fDist)*fExpose);
				switch(m_nChannels) {
					case 4:
						m_pBuffer[n++] = 255;
						m_pBuffer[n++] = 255;
						m_pBuffer[n++] = 255;
						m_pBuffer[n++] = scale(fIntensity);
						break;
					default:
						for(int i=0; i<m_nChannels; i++)
							m_pBuffer[n++] = scale(fIntensity);
						break;
				}
			}
		}
	}

	/// Builds a noise map of random values
	/// @param[in] nSeed The random seed to use
	void makeNoise(int nSeed) {
		srand(nSeed);
		T *pDest = m_pBuffer;
		for(int z=0; z<m_nDepth; z++) {
			for(int y=0; y<m_nHeight; y++) {
				for(int x=0; x<m_nWidth; x++) {
					float f[4];
					for(int n=0; n<m_nChannels; n++)
						f[n] = rand()/(float)RAND_MAX - 0.5f;
					VK::Math::Normalize(f, m_nChannels);
					for(int n=0; n<m_nChannels; n++)
						*pDest++ = scale(f[n]);
				}
			}
		}
	}

	/// Builds a packed buffer from a single channel of another texture (as used in chapter 1 of GPU Gems 3)
	/// @param[in] pbFull The full noise buffer to pack
	/// @param[in] nChannel The channel to pack
	void makePacked(const PixelBuffer<T> &pbFull, int nChannel=0) {
		create(pbFull.getWidth(), pbFull.getHeight(), pbFull.getDepth(), 4);
		int nChannels = pbFull.getChannels();
		T *pSrc = pbFull.getBuffer() + nChannel;
		T *pDest = m_pBuffer;
		for(int z=0; z<m_nDepth; z++) {
			T *pTop = pSrc;
			for(int y=0; y<m_nHeight; y++) {
				T *pRow0 = pSrc;
				T *pRow1 = y < m_nHeight-1 ? &pSrc[m_nWidth*nChannels] : pTop;
				int n = 0;
				for(int x=0; x<m_nWidth-1; x++) {
					*pDest++ = pRow0[n];
					*pDest++ = pRow0[n+nChannels];
					*pDest++ = pRow1[n];
					*pDest++ = pRow1[n+nChannels];
					n += nChannels;
				}
				*pDest++ = pRow0[n];
				*pDest++ = pRow0[0];
				*pDest++ = pRow1[n];
				*pDest++ = pRow1[0];
				pSrc += m_nWidth*nChannels;
			}
		}
	}

	bool load(const char *pszFile);
	bool loadPPM(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels);
	bool loadRAW(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels);
	bool loadJPG(const char *pszFile, uint16_t nWidth=0, uint16_t nHeight=0, uint8_t nChannels=0);
	bool loadPNG(const char *pszFile, uint16_t nWidth=0, uint16_t nHeight=0, uint8_t nChannels=0);

	bool saveJPG(const char *pszFile);
	bool savePNG(const char *pszFile);
};

// Non-templatized methods for loading and saving image files
bool Load(PixelBuffer<uint8_t> &pb, const char *pszFile);
bool LoadPPM(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels);
bool LoadRAW(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels);
bool LoadJPG(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels);
bool LoadPNG(PixelBuffer<uint8_t> &pb, const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels);
bool SaveJPG(PixelBuffer<uint8_t> &pb, const char *pszFile);
bool SavePNG(PixelBuffer<uint8_t> &pb, const char *pszFile);

template<> inline bool PixelBuffer<unsigned char>::load(const char *pszFile) {
	return Load(*this, pszFile);
}
template<> inline bool PixelBuffer<unsigned char>::loadPPM(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	return LoadPPM(*this, pszFile, nWidth, nHeight, nChannels);
}
template<> inline bool PixelBuffer<unsigned char>::loadRAW(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	return LoadRAW(*this, pszFile, nWidth, nHeight, nChannels);
}
template<> inline bool PixelBuffer<unsigned char>::loadJPG(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	return LoadJPG(*this, pszFile, nWidth, nHeight, nChannels);
}
template<> inline bool PixelBuffer<unsigned char>::loadPNG(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	return LoadPNG(*this, pszFile, nWidth, nHeight, nChannels);
}
template<> inline bool PixelBuffer<unsigned char>::saveJPG(const char *pszFile) {
	return SaveJPG(*this, pszFile);
}
template<> inline bool PixelBuffer<unsigned char>::savePNG(const char *pszFile) {
	return SavePNG(*this, pszFile);
}
template<class T> inline bool PixelBuffer<T>::loadRAW(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	VKLogException("PixelBuffer load/save methods are only supported for unsigned char buffers");
	return false;
}
template<class T> inline bool PixelBuffer<T>::loadJPG(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	VKLogException("PixelBuffer load/save methods are only supported for unsigned char buffers");
	return false;
}
template<class T> inline bool PixelBuffer<T>::loadPNG(const char *pszFile, uint16_t nWidth, uint16_t nHeight, uint8_t nChannels) {
	VKLogException("PixelBuffer load/save methods are only supported for unsigned char buffers");
	return false;
}
template<class T> inline bool PixelBuffer<T>::saveJPG(const char *pszFile) {
	VKLogException("PixelBuffer load/save methods are only supported for unsigned char buffers");
	return false;
}
template<class T> inline bool PixelBuffer<T>::savePNG(const char *pszFile) {
	VKLogException("PixelBuffer load/save methods are only supported for unsigned char buffers");
	return false;
}

// Type-specific methods for PixelBuffer
template<> inline char PixelBuffer<char>::clamp(float f) { return (char)(Math::Clamp(f, (float)-0x7F, (float)0x7F) + 0.5f); }
template<> inline short PixelBuffer<short>::clamp(float f) { return (short)(Math::Clamp(f, (float)-0x7FFF, (float)0x7FFF) + 0.5f); }
template<> inline int PixelBuffer<int>::clamp(float f) { return (int)(Math::Clamp(f, (float)-0x7FFFFFFF, (float)0x7FFFFFFF) + 0.5f); }
template<> inline float PixelBuffer<float>::clamp(float f) { return Math::Clamp(f, -1.0f, 1.0f); }
template<> inline double PixelBuffer<double>::clamp(float f) { return Math::Clamp(f, -1.0f, 1.0f); }
template<> inline unsigned char PixelBuffer<unsigned char>::clamp(float f) { return (unsigned char)(Math::Clamp(f, 0.0f, (float)0xFF) + 0.5f); }
template<> inline unsigned short PixelBuffer<unsigned short>::clamp(float f) { return (unsigned short)(Math::Clamp(f, 0.0f, (float)0xFFFF) + 0.5f); }
template<> inline unsigned int PixelBuffer<unsigned int>::clamp(float f) { return (unsigned int)(Math::Clamp(f, 0.0f, (float)0xFFFFFFFF) + 0.5f); }

template<> inline char PixelBuffer<char>::scale(float f) { return clamp(f * 0x7F); }
template<> inline short PixelBuffer<short>::scale(float f) { return clamp(f * 0x7FFF); }
template<> inline int PixelBuffer<int>::scale(float f) { return clamp(f * 0x7FFFFFFF); }
template<> inline float PixelBuffer<float>::scale(float f) { return clamp(f); }
template<> inline double PixelBuffer<double>::scale(float f) { return clamp(f); }
template<> inline unsigned char PixelBuffer<unsigned char>::scale(float f) { return clamp(f * 0xFF); }
template<> inline unsigned short PixelBuffer<unsigned short>::scale(float f) { return clamp(f * 0xFFFF); }
template<> inline unsigned int PixelBuffer<unsigned int>::scale(float f) { return clamp(f * 0xFFFFFFFF); }

template<> inline char PixelBuffer<char>::shift(float f) { return scale(f); }
template<> inline short PixelBuffer<short>::shift(float f) { return scale(f); }
template<> inline int PixelBuffer<int>::shift(float f) { return scale(f); }
template<> inline float PixelBuffer<float>::shift(float f) { return scale(f); }
template<> inline double PixelBuffer<double>::shift(float f) { return scale(f); }
template<> inline unsigned char PixelBuffer<unsigned char>::shift(float f) { return scale(f * 0.5f + 0.5f); }
template<> inline unsigned short PixelBuffer<unsigned short>::shift(float f) { return scale(f * 0.5f + 0.5f); }
template<> inline unsigned int PixelBuffer<unsigned int>::shift(float f) { return scale(f * 0.5f + 0.5f); }

}

#endif // __VKPixelBuffer_h__
