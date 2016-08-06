#ifndef __VKNoise_h__
#define __VKNoise_h__

#include "VKMath.h"

namespace VK {

/*******************************************************************************
* Class: Random
********************************************************************************
* This class wraps a random number generator. I plan to implement my own random
* number generator so I can keep the seeds as member variables (which is more
* flexible than using statics or globals). I was using one I found on the
* Internet implemented in assembler, but I was having problems with it so I
* removed it for this demo.
*******************************************************************************/
#define BITS		32
#define MSB			0x80000000L
#define ALL_BITS	0xffffffffL
#define HALF_RANGE	0x40000000L
#define STEP		7
class Random
{
protected:
	long m_nSeed;
	long m_nQuotient;
	long m_nRemainder;

public:
	Random()						{}
	Random(unsigned int nSeed)		{ init(nSeed); }
	void init(unsigned int nSeed)	{ srand(nSeed); }
	double random()					{ return (double)rand()/(double)RAND_MAX; }
	double random(double dMin, double dMax) {
		double dInterval = dMax - dMin;
		double d = dInterval * random();
		return dMin + Math::Min(d, dInterval);
	}
	unsigned int random(unsigned int nMin, unsigned int nMax) {
		if(nMax <= nMin) return nMin;
		unsigned int nInterval = nMax - nMin;
		unsigned int i = (unsigned int)((nInterval+1.0) * random());
		return nMin + Math::Min(i, nInterval);
	}
	int random(int nMin, int nMax) {
		if(nMax <= nMin) return nMin;
		unsigned int nInterval = (unsigned int)(nMax - nMin);
		unsigned int i = (unsigned int)((nInterval+1.0) * random());
		return nMin + (int)Math::Min(i, nInterval);
	}
};

class RandomLCG
{
protected:
	unsigned int m_nSeed;
	unsigned int m_nQuotient;
	unsigned int m_nRemainder;

public:
	RandomLCG(unsigned int nSeed)
	{
		m_nSeed = nSeed;
		m_nQuotient = LONG_MAX / 16807L;
		m_nRemainder = LONG_MAX % 16807L;
	}

	unsigned int rand()
	{
		if(m_nSeed <= m_nQuotient)
			m_nSeed = (m_nSeed * 16807L) % LONG_MAX;
		else
		{
			int nHigh = m_nSeed / m_nQuotient;
			int nLow  = m_nSeed % m_nQuotient;

			int test = 16807L * nLow - m_nRemainder * nHigh;

			if(test > 0)
				m_nSeed = test;
			else
				m_nSeed = test + LONG_MAX;
		}

		return m_nSeed;
	}
};

class RandomR250
{
protected:
	unsigned int m_nR250buffer[250];
	int m_nR250index;

public:
	RandomR250(unsigned int nSeed)
	{
		RandomLCG lcg(nSeed);

		int j, k;
		unsigned int mask, msb;
		m_nR250index = 0;
		for(j = 0; j < 250; j++)
			m_nR250buffer[j] = rand();
		for(j = 0; j < 250; j++)	// set some MSBs to 1
			if(lcg.rand() > HALF_RANGE)
				m_nR250buffer[j] |= MSB;

		msb = MSB;	        // turn on diagonal bit
		mask = ALL_BITS;	// turn off the leftmost bits

		for (j = 0; j < BITS; j++)
		{
			k = STEP * j + 3;	// select a word to operate on
			m_nR250buffer[k] &= mask; // turn off bits left of the diagonal
			m_nR250buffer[k] |= msb;	// turn on the diagonal bit
			mask >>= 1;
			msb  >>= 1;
		}
	}

	unsigned int r250()
	{
		register int j;
		register unsigned int new_rand;

		if ( m_nR250index >= 147 )
			j = m_nR250index - 147;
		else
			j = m_nR250index + 103;

		new_rand = m_nR250buffer[ m_nR250index ] ^ m_nR250buffer[ j ];
		m_nR250buffer[ m_nR250index ] = new_rand;

		if ( m_nR250index >= 249 )
			m_nR250index = 0;
		else
			m_nR250index++;

		return new_rand;

	}

	double dr250()
	{
		register int j;
		register unsigned int new_rand;

		if ( m_nR250index >= 147 )
			j = m_nR250index - 147;
		else
			j = m_nR250index + 103;

		new_rand = m_nR250buffer[ m_nR250index ] ^ m_nR250buffer[ j ];
		m_nR250buffer[ m_nR250index ] = new_rand;

		if ( m_nR250index >= 249 )	/* increment pointer for next time */
			m_nR250index = 0;
		else
			m_nR250index++;

		return (double)new_rand / ALL_BITS;
	}
};

/*******************************************************************************
* Class: Noise
********************************************************************************
* This class implements the Perlin noise function. Initialize it with the number
* of dimensions (1 to 4) and a random seed. I got the source for the first 3
* dimensions from "Texturing & Modeling: A Procedural Approach". I added the
* extra dimension because it may be desirable to use 3 spatial dimensions and
* one time dimension. The noise buffers are set up as member variables so that
* there may be several instances of this class in use at the same time, each
* initialized with different parameters.
*******************************************************************************/
class Noise
{
protected:
	static const int MAX_DIMENSIONS = 4;
	static const int MAX_OCTAVES = 12;
	
	int m_nDimensions;						// Number of dimensions used by this object
	unsigned char m_nMap[256];				// Randomized map of indexes into buffer
	float m_nBuffer[256][MAX_DIMENSIONS];	// Random n-dimensional buffer

	float lattice(int ix, float fx, int iy=0, float fy=0, int iz=0, float fz=0, int iw=0, float fw=0)
	{
		int n[4] = {ix, iy, iz, iw};
		float f[4] = {fx, fy, fz, fw};
		int i, nIndex = 0;
		for(i=0; i<m_nDimensions; i++)
			nIndex = m_nMap[(nIndex + n[i]) & 0xFF];
		float fValue = 0;
		for(i=0; i<m_nDimensions; i++)
			fValue += m_nBuffer[nIndex][i] * f[i];
		return fValue;
	}

public:
	Noise()	{}
	Noise(int nDimensions, unsigned int nSeed)	{ init(nDimensions, nSeed); }
	void init(int nDimensions, unsigned int nSeed);
	float noise(float *f);
};

/*******************************************************************************
* Class: CFractal
********************************************************************************
* This class implements fBm, or fractal Brownian motion. Since fBm uses Perlin
* noise, this class is derived from CNoise. Initialize it with the number of
* dimensions (1 to 4), a random seed, H (roughness ranging from 0 to 1), and
* the lacunarity (2.0 is often used). Many of the fractal routines came from
* "Texturing & Modeling: A Procedural Approach". fBmTest() is my own creation,
* and I created it to generate my first planet.
*******************************************************************************/
class Fractal : public Noise
{
protected:
	float m_fH;
	float m_fLacunarity;
	float m_fExponent[MAX_OCTAVES];

public:
	Fractal()	{}
	Fractal(int nDimensions, unsigned int nSeed, float fH, float fLacunarity)
	{
		init(nDimensions, nSeed, fH, fLacunarity);
	}
	void init(int nDimensions, unsigned int nSeed, float fH, float fLacunarity)
	{
		Noise::init(nDimensions, nSeed);
		m_fH = fH;
		m_fLacunarity = fLacunarity;
		float f = 1;
		for(int i=0; i<MAX_OCTAVES; i++) 
		{
			m_fExponent[i] = powf(f, -m_fH);
			f *= m_fLacunarity;
		}
	}
	float fBm(float *f, float fOctaves);
	float Turbulence(float *f, float fOctaves);
	float Multifractal(float *f, float fOctaves, float fOffset);
	float Heterofractal(float *f, float fOctaves, float fOffset);
	float HybridMultifractal(float *f, float fOctaves, float fOffset, float fGain);
	float RidgedMultifractal(float *f, float fOctaves, float fOffset, float fThreshold);
	float fBmTest(float *f, float fOctaves, float fOffset=-0.1f);
	float fBmTest2(float *f, float fOctaves, float fGain=2.0f, float fOffset=-0.1f);
	float fBmTest3(float *f, float fOctaves, float fGain=2.0f, float fOffset=-0.1f);
};

} // namespace VK
#endif // __Noise_h__
