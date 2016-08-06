// Perlin noise shader functions
// This code is part of the GLContext library, an object-oriented class
// library designed to make OpenGL 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

// NOTES: A 1D texture is MUCH faster than a uniform array.
// Apparently array lookups are cached, and cache misses are PAINFUL.
// Texture lookup cache misses hurt a little, but not nearly as much.
// I also replaced the lattice with prime multipliers to speed it up.

vec4 noise_lut[64] = {
  {-0.3280, 0.6129, -0.7189, 7},
  {0.6176, 0.2840, -0.7335, 49},
  {-0.0370, 0.5567, 0.8299, 45},
  {-0.0822, 0.6353, -0.7679, 31},
  {0.4764, 0.8754, -0.0819, 4},
  {-0.2732, 0.2359, -0.9326, 23},
  {0.2692, 0.4051, 0.8737, 13},
  {-0.5912, -0.7494, -0.2982, 55},
  {0.0548, 0.9694, -0.2395, 61},
  {-0.2699, 0.4398, 0.8566, 26},
  {-0.0221, 0.4292, -0.9030, 35},
  {0.0100, 0.4162, -0.9092, 58},
  {-0.5314, 0.6666, -0.5227, 2},
  {-0.4019, 0.3937, 0.8268, 12},
  {0.7308, -0.6728, -0.1150, 28},
  {0.6933, -0.6372, 0.3367, 63},
  {-0.6809, 0.1810, 0.7097, 6},
  {-0.0546, 0.7006, -0.7114, 10},
  {0.5587, 0.0095, -0.8293, 19},
  {0.2757, -0.8836, -0.3784, 39},
  {0.5770, 0.6652, 0.4739, 20},
  {-0.3303, -0.9419, -0.0617, 46},
  {0.0297, -0.8112, 0.5840, 22},
  {-0.9110, -0.1700, 0.3757, 27},
  {0.0802, 0.6819, 0.7271, 3},
  {-0.6239, -0.1411, -0.7686, 11},
  {-0.1112, -0.4638, 0.8789, 36},
  {-0.3540, 0.8457, -0.3994, 37},
  {-0.7742, 0.1714, 0.6092, 52},
  {-0.2323, 0.7707, 0.5933, 56},
  {-0.7711, 0.6367, 0.0015, 43},
  {-0.6686, 0.1080, 0.7358, 53},
  {0.7446, -0.6640, 0.0680, 29},
  {-0.8310, 0.4335, 0.3487, 44},
  {-0.6084, -0.5875, 0.5337, 42},
  {0.8216, -0.3463, 0.4529, 17},
  {-0.8005, -0.3642, -0.4760, 33},
  {0.4544, 0.7310, 0.5092, 62},
  {-0.6798, 0.6797, 0.2756, 24},
  {-0.5827, -0.6616, -0.4719, 57},
  {-0.5267, 0.6418, 0.5574, 16},
  {-0.7813, 0.0856, -0.6183, 60},
  {0.5442, -0.0981, -0.8332, 47},
  {-0.9124, 0.0931, 0.3986, 15},
  {0.5415, -0.8224, -0.1746, 9},
  {-0.7739, -0.1831, -0.6063, 34},
  {0.8361, 0.2118, 0.5060, 51},
  {0.8062, 0.0072, -0.5916, 38},
  {-0.5525, 0.7570, 0.3489, 32},
  {0.8080, -0.5248, -0.2677, 8},
  {-0.6272, 0.7761, -0.0663, 21},
  {-0.1451, -0.9454, 0.2918, 30},
  {0.5341, -0.6112, 0.5841, 5},
  {0.6159, -0.6242, -0.4806, 0},
  {0.3807, -0.8858, -0.2653, 18},
  {-0.2810, -0.4042, 0.8704, 14},
  {0.9053, 0.3438, 0.2496, 48},
  {0.5319, 0.3005, 0.7917, 40},
  {-0.9130, -0.3610, 0.1902, 1},
  {0.8564, 0.1166, 0.5030, 59},
  {0.2512, -0.7460, -0.6167, 54},
  {-0.5674, 0.3229, 0.7575, 25},
  {0.4976, -0.8673, -0.0114, 41},
  {0.7580, 0.4774, 0.4444, 50},
};

float perlin_noise(vec2 v) {
	vec2 n = floor(v);
	vec2 r1 = fract(v);
	vec2 r2 = r1 - vec2(1.0, 1.0);
	vec2 w = cubic(r1);

	// Find the lattice offsets (kept in the alpha channel)
	ivec2 ix = ivec2(noise_lut[int(n.x) & 0x3F].a, noise_lut[int(n.x+1) & 0x3F].a);
	ivec4 iy = ivec4(
		noise_lut[(int(n.y) + ix.x) & 0x3F].a,
		noise_lut[(int(n.y) + ix.y) & 0x3F].a,
		noise_lut[(int(n.y+1) + ix.x) & 0x3F].a,
		noise_lut[(int(n.y+1) + ix.y) & 0x3F].a
	);

	// Grab the actual xy values for those lattice offsets
	vec2 v0 = normalize(noise_lut[iy.x].xy);
	vec2 v1 = normalize(noise_lut[iy.y].xy);
	vec2 v2 = normalize(noise_lut[iy.z].xy);
	vec2 v3 = normalize(noise_lut[iy.w].xy);

	// Interpolate
#if 1
	vec2 vx = mix(
		vec2(dot(v0, r1), dot(v2, vec2(r1.x,r2.y))),
		vec2(dot(v1, vec2(r2.x,r1.y)), dot(v3, r2)),
		w.x
	);
	return mix(vx.x, vx.y, w.y);
#else
	return mix(
		mix(v0.x*r1.x + v0.y*r1.y, v1.x*r2.x + v1.y*r1.y, w.x),
		mix(v2.x*r1.x + v2.y*r2.y, v3.x*r2.x + v3.y*r2.y, w.x),
		w.y
	);
#endif
}

float perlin_noise(vec3 v) {
	vec3 n = floor(v);
	vec3 r1 = fract(v);
	vec3 r2 = r1 - vec3(1.0, 1.0, 1.0);
	vec3 w = cubic(r1);

	// Find the lattice offsets (kept in the alpha channel)
	ivec2 ix = ivec2(noise_lut[int(n.x) & 0x3F].a, noise_lut[int(n.x+1) & 0x3F].a);
	ivec4 iy = ivec4(
		noise_lut[int(n.y + ix.x) & 0x3F].a,
		noise_lut[int(n.y + ix.y) & 0x3F].a,
		noise_lut[int(n.y+1 + ix.x) & 0x3F].a,
		noise_lut[int(n.y+1 + ix.y) & 0x3F].a
	);
	ivec4 iz0 = ivec4(
		noise_lut[int(n.z + iy.x) & 0x3F].a,
		noise_lut[int(n.z + iy.y) & 0x3F].a,
		noise_lut[int(n.z + iy.z) & 0x3F].a,
		noise_lut[int(n.z + iy.w) & 0x3F].a
	);
	n.z += 1;
	ivec4 iz1 = ivec4(
		noise_lut[int(n.z + iy.x) & 0x3F].a,
		noise_lut[int(n.z + iy.y) & 0x3F].a,
		noise_lut[int(n.z + iy.z) & 0x3F].a,
		noise_lut[int(n.z + iy.w) & 0x3F].a
	);

	// Grab the xyz values for those lattice offsets
	vec3 v0 = noise_lut[iz0.x].xyz;
	vec3 v1 = noise_lut[iz0.y].xyz;
	vec3 v2 = noise_lut[iz0.z].xyz;
	vec3 v3 = noise_lut[iz0.w].xyz;
	vec3 v4 = noise_lut[iz1.x].xyz;
	vec3 v5 = noise_lut[iz1.y].xyz;
	vec3 v6 = noise_lut[iz1.z].xyz;
	vec3 v7 = noise_lut[iz1.w].xyz;

	// Interpolate
#if 1 // Vectorized versions of these functions should be faster
	vec4 vx = mix(
		vec4(dot(v0, r1), dot(v2, vec3(r1.x,r2.y,r1.z)), dot(v4, vec3(r1.x,r1.y,r2.z)), dot(v6, vec3(r1.x,r2.y,r2.z))),
		vec4(dot(v1, vec3(r2.x,r1.y,r1.z)), dot(v3, vec3(r2.x,r2.y,r1.z)), dot(v5, vec3(r2.x,r1.y,r2.z)), dot(v7, r2)),
		w.x
	);
	vec2 vy = mix(vx.xz, vx.yw, w.y);
	return mix(vy.x, vy.y, w.z);
#else
	return mix(
		mix(
			mix(v0.x*r1.x + v0.y*r1.y + v0.z*r1.z, v1.x*r2.x + v1.y*r1.y + v1.z*r1.z, w.x),
			mix(v2.x*r1.x + v2.y*r2.y + v2.z*r1.z, v3.x*r2.x + v3.y*r2.y + v3.z*r1.z, w.x),
			w.y
		),
		mix(
			mix(v4.x*r1.x + v4.y*r1.y + v4.z*r2.z, v5.x*r2.x + v5.y*r1.y + v5.z*r2.z, w.x),
			mix(v6.x*r1.x + v6.y*r2.y + v6.z*r2.z, v7.x*r2.x + v7.y*r2.y + v7.z*r2.z, w.x),
			w.y
		),
		w.z
	);
#endif
}

float fBm(vec2 v) {
	return perlin_noise(v) * 0.0625 + perlin_noise(v*0.5) * 0.125 + perlin_noise(v*0.25) * 0.25 + perlin_noise(v*0.125) * 0.5;
}

float fBm(vec3 v) {
	return perlin_noise(v) * 0.0625 + perlin_noise(v*0.5) * 0.125 + perlin_noise(v*0.25) * 0.25 + perlin_noise(v*0.125) * 0.5;
}

