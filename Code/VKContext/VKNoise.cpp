// VKNoise.cpp
//
#include "VKCore.h"
#include "VKNoise.h"

namespace VK {

void Noise::init(int nDimensions, unsigned int nSeed)
{
	m_nDimensions = Math::Min(nDimensions, MAX_DIMENSIONS);
	Random r(nSeed);

	int i, j;
	for(i=0; i<256; i++)
	{
		m_nMap[i] = i;
		for(j=0; j<m_nDimensions; j++)
			m_nBuffer[i][j] = (float)r.random(-0.5, 0.5);
		if(m_nDimensions == 1)
			;//m_nBuffer[i][0] *= 2.0f;
		else
			Math::Normalize(m_nBuffer[i], m_nDimensions);
	}

	while(--i)
	{
		j = r.random(0, 255);
		Math::Swap(m_nMap[i], m_nMap[j]);
	}
}

float Noise::noise(float *f)
{
	int i, n[MAX_DIMENSIONS];			// Indexes to pass to lattice function
	float r[MAX_DIMENSIONS];		// Remainders to pass to lattice function
	float w[MAX_DIMENSIONS];		// Cubic values to pass to interpolation function

	for(i=0; i<m_nDimensions; i++)
	{
		n[i] = Math::Floor(f[i]);
		r[i] = f[i] - n[i];
		w[i] = Math::Cubic(r[i]);
	}

	float fValue;
	switch(m_nDimensions)
	{
		case 1:
			fValue = Math::Lerp(lattice(n[0], r[0]),
						  lattice(n[0]+1, r[0]-1),
						  w[0]);
			break;
		case 2:
			fValue = Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1]),
							   lattice(n[0]+1, r[0]-1, n[1], r[1]),
							   w[0]),
						  Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1),
							   lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1),
							   w[0]),
						  w[1]);
			break;
		case 3:
			fValue = Math::Lerp(Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1], n[2], r[2]),
									lattice(n[0]+1, r[0]-1, n[1], r[1], n[2], r[2]),
									w[0]),
							   Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1, n[2], r[2]),
									lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2]),
									w[0]),
							   w[1]),
						  Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1], n[2]+1, r[2]-1),
									lattice(n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1),
									w[0]),
							   Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1),
									lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1),
									w[0]),
							   w[1]),
						  w[2]);
			break;
		case 4:
			fValue = Math::Lerp(Math::Lerp(Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1], n[2], r[2], n[3], r[3]),
										 lattice(n[0]+1, r[0]-1, n[1], r[1], n[2], r[2], n[3], r[3]),
										 w[0]),
									Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1, n[2], r[2], n[3], r[3]),
										 lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2], n[3], r[3]),
										 w[0]),
									w[1]),
									Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1], n[2]+1, r[2]-1, n[3], r[3]),
										 lattice(n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1, n[3], r[3]),
										 w[0]),
									Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1),
										 lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1, n[3], r[3]),
										 w[0]),
									w[1]),
							   w[2]),
						  Math::Lerp(Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1], n[2], r[2], n[3]+1, r[3]-1),
										 lattice(n[0]+1, r[0]-1, n[1], r[1], n[2], r[2], n[3]+1, r[3]-1),
										 w[0]),
									Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1, n[2], r[2], n[3]+1, r[3]-1),
										 lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2], r[2], n[3]+1, r[3]-1),
										 w[0]),
									w[1]),
									Math::Lerp(Math::Lerp(lattice(n[0], r[0], n[1], r[1], n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 lattice(n[0]+1, r[0]-1, n[1], r[1], n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 w[0]),
									Math::Lerp(lattice(n[0], r[0], n[1]+1, r[1]-1, n[2]+1, r[2]-1),
										 lattice(n[0]+1, r[0]-1, n[1]+1, r[1]-1, n[2]+1, r[2]-1, n[3]+1, r[3]-1),
										 w[0]),
									w[1]),
							   w[2]),
						  w[3]);
			break;
	}
	return Math::Clamp(fValue*2.0f, -0.99999f, 0.99999f);
}

float Fractal::fBm(float *f, float fOctaves)
{
	// Initialize locals
	int i;
	float fValue = 0;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i];

	// Inner loop of spectral construction, where the fractal is built
	for(i=0; i<fOctaves; i++)
	{
		fValue += noise(fTemp) * m_fExponent[i];
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
	}

	// Take care of remainder in fOctaves
	fOctaves -= (int)fOctaves;
	if(fOctaves > DELTA)
		fValue += fOctaves * noise(fTemp) * m_fExponent[i];
	return Math::Clamp(fValue, -0.99999f, 0.99999f);
}

float Fractal::fBmTest(float *f, float fOctaves, float fOffset)
{
	int i, j;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i] * 2;

	float fValue = noise(fTemp) + fOffset;
	for(i=1; i<fOctaves; i++)
	{
		for(j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		fValue += (noise(fTemp) + fOffset) * m_fExponent[i];
	}

	while(Math::Abs(fValue) > 1.0f)
	{
		if(fValue > 0.0f)
			fValue = 2 - fValue;
		else
			fValue = -2 - fValue;
	}

	if(fValue <= 0.0f)
		fValue = (float)-pow(-fValue, 0.7f);
	else
		fValue = (float)pow(fValue, 1 + noise(fTemp) * fValue);
	return fValue;
}

float Fractal::fBmTest2(float *f, float fOctaves, float fGain, float fOffset)
{
	// Initialize locals
	int i, j;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i] * 2;

	float fBase = noise(fTemp) + fOffset;
	for(i=1; i<6; i++)
	{
		for(j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		fBase += (noise(fTemp) + fOffset) * m_fExponent[i];
	}

	fBase *= 0.7f;
	while(Math::Abs(fBase) > 1.0f)
	{
		if(fBase > 0.0f)
			fBase = 2 - fBase;
		else
			fBase = -2 - fBase;
	}

	fGain *= fBase;

	float fValue = 0.0f;
	float fSignal = 1.0f - Math::Abs(noise(fTemp));
	fSignal *= fSignal;
	fValue += fSignal * m_fExponent[6];
	for(i=7; i<13; i++)
	{
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		float fWeight = Math::Clamp(fSignal * fGain, 0.0f, 1.0f);
		fSignal = 1.0f - Math::Abs(noise(fTemp));
		fSignal *= fSignal;
		fSignal *= fSignal;
		fSignal *= fWeight;
		fValue += (fSignal - 0.5f) * m_fExponent[i];
	}

	if(fBase < 0.0f)
		fValue = fBase - fValue * Math::SqrtWithSign(fBase);
	else
		fValue = fBase + fValue * Math::SqrtWithSign(fBase);
	return fValue;
}

float Fractal::fBmTest3(float *f, float fOctaves, float fGain, float fOffset)
{
	// Initialize locals
	int i, j;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i] * 2;

	float fBase = noise(fTemp) + fOffset;
	for(i=1; i<6; i++)
	{
		for(j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		fBase += (noise(fTemp) + fOffset) * m_fExponent[i];
	}
	while(Math::Abs(fBase) > 1.0f)
	{
		if(fBase > 0.0f)
			fBase = 2 - fBase;
		else
			fBase = -2 - fBase;
	}
	if(fBase < 0.0f)
		return fBase;

	float fExtra = fGain * Math::Abs(noise(fTemp)) * m_fExponent[6];
	for(i=6; i<12; i++)
	{
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		fExtra += fGain * Math::Abs(noise(fTemp)) * m_fExponent[i];
	}

	// For numbers from 0..1, 1-Square(1-n) is similar to sqrt(n)
	return fBase - (1-Math::Square(1-fExtra)) * Math::Sqrt(Math::Abs(fBase));
}

float Fractal::Turbulence(float *f, float fOctaves)
{
	// Initialize locals
	int i;
	float fValue = 0;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i];

	// Inner loop of spectral construction, where the fractal is built
	for(i=0; i<fOctaves; i++)
	{
		fValue += Math::Abs(noise(fTemp)) * m_fExponent[i];
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
	}

	// Take care of remainder in fOctaves
	fOctaves -= (int)fOctaves;
	if(fOctaves > DELTA)
		fValue += fOctaves * Math::Abs(noise(fTemp) * m_fExponent[i]);
	return Math::Clamp(fValue, -0.99999f, 0.99999f);
}

float Fractal::Multifractal(float *f, float fOctaves, float fOffset)
{
	// Initialize locals
	int i;
	float fValue = 1;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i];

	// Inner loop of spectral construction, where the fractal is built
	for(i=0; i<fOctaves; i++)
	{
		fValue *= noise(fTemp) * m_fExponent[i] + fOffset;
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
	}

	// Take care of remainder in fOctaves (shouldn't that be a multiply?)
	fOctaves -= (int)fOctaves;
	if(fOctaves > DELTA)
		fValue *= fOctaves * (noise(fTemp) * m_fExponent[i] + fOffset);
	return Math::Clamp(fValue, -0.99999f, 0.99999f);
}

float Fractal::Heterofractal(float *f, float fOctaves, float fOffset)
{
	// Initialize locals
	int i;
	float fValue = noise(f) + fOffset;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i] * m_fLacunarity;

	// Inner loop of spectral construction, where the fractal is built
	for(i=1; i<fOctaves; i++)
	{
		fValue += (noise(fTemp) + fOffset) * m_fExponent[i] * fValue;
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
	}

	// Take care of remainder in fOctaves
	fOctaves -= (int)fOctaves;
	if(fOctaves > DELTA)
		fValue += fOctaves * (noise(fTemp) + fOffset) * m_fExponent[i] * fValue;
	return Math::Clamp(fValue, -0.99999f, 0.99999f);
}

float Fractal::HybridMultifractal(float *f, float fOctaves, float fOffset, float fGain)
{
	// Initialize locals
	int i;
	float fValue = (noise(f) + fOffset) * m_fExponent[0];
	float fWeight = fValue;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i] * m_fLacunarity;

	// Inner loop of spectral construction, where the fractal is built
	for(i=1; i<fOctaves; i++)
	{
		if(fWeight > 1)
			fWeight = 1;
		float fSignal = (noise(fTemp) + fOffset) * m_fExponent[i];
		fValue += fWeight * fSignal;
		fWeight *= fGain * fSignal;
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
	}

	// Take care of remainder in fOctaves
	fOctaves -= (int)fOctaves;
	if(fOctaves > DELTA)
	{
		if(fWeight > 1)
			fWeight = 1;
		float fSignal = (noise(fTemp) + fOffset) * m_fExponent[i];
		fValue += fOctaves * fWeight * fSignal;
	}
	return Math::Clamp(fValue, -0.99999f, 0.99999f);
}

float Fractal::RidgedMultifractal(float *f, float fOctaves, float fOffset, float fGain)
{
	float fTemp[MAX_DIMENSIONS];
	int i;

	//float fExponent[10] = {1.0f, 0.75f, 0.5625f, 0.4219f, 0.3164f, 0.2373f, 0.1780f, 0.1335f, 0.1001f, 0.0751f};

	// Initialize locals
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i] * 2;

	// Inner loop of spectral construction, where the fractal is built
	float fValue = Math::SquareWithSign(noise(fTemp));
	for(i=1; i<12; i++)
	{
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		fValue += noise(fTemp) * m_fExponent[i];
	}

	// Take care of remainder in fOctaves
	//fOctaves -= (int)fOctaves;
	//if(fOctaves > DELTA)
	//	fValue += fOctaves * noise(fTemp) * m_fExponent[i];
	
	while(Math::Abs(fValue) > 1.0f)
	{
		if(fValue > 0.0f)
			fValue = 2 - fValue;
		else
			fValue = -2 - fValue;
	}

	if(fValue <= 0.0f)
		fValue = (float)-pow(-fValue, 0.7f);
	else
		fValue = (float)pow(fValue, 1 + noise(fTemp) * fValue);
	return fValue;

	//if(fValue > 0.0f)
	//	fValue = (float)pow(fValue, 1 + noise(fTemp) * fValue);
	//return fValue;

	/*
	// Initialize locals
	float fSignal = fOffset - Abs(Noise(f));
	fSignal *= fSignal;
	float fValue = fSignal;
	float fTemp[MAX_DIMENSIONS];
	for(i=0; i<m_nDimensions; i++)
		fTemp[i] = f[i];

	// Inner loop of spectral construction, where the fractal is built
	for(i=1; i<fOctaves; i++)
	{
		for(int j=0; j<m_nDimensions; j++)
			fTemp[j] *= m_fLacunarity;
		float fWeight = Clamp(fSignal * fGain, 0.0f, 1.0f);
		fSignal = fOffset - Abs(Noise(fTemp));
		fSignal *= fSignal;
		fSignal *= fWeight;
		fValue += fSignal * m_fExponent[i];
	}
	return Clamp(fValue, -0.99999f, 0.99999f);
	*/
}

} // namespace VK
