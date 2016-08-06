// GLContext.glsl - Shared uniform and function declarations

// Vertices and indices for a regular (from 0:0 to 1:1) and unit square (a unit circle has a diameter of 2).
const vec4 vSquare[4] = vec4[4] (
	vec4(0, 1, 0, 1),
	vec4(0, 0, 0, 1),
	vec4(1, 0, 0, 1),
	vec4(1, 1, 0, 1)
);
const vec4 vUnitSquare[4] = vec4[4] (
	vec4(-1, 1, 0, 1),
	vec4(-1, -1, 0, 1),
	vec4(1, -1, 0, 1),
	vec4(1, 1, 0, 1)
);
const int iSquare[6] = int[6] (0, 1, 2, 0, 2, 3);
#define iUnitSquare iSquare

// Vertices and indices for a unit cube (a unit circle has a diameter of 2)
const vec4 vUnitCube[8] = vec4[8] (
	vec4(-1, 1, -1, 1),	// Left, top, back
	vec4(-1, 1, 1, 1),	// Left, top, front
	vec4(1, 1, 1, 1),	// Right, top, front
	vec4(1, 1, -1, 1),	// Right, top, back
	vec4(-1, -1, -1, 1),// Left, bottom, back
	vec4(-1, -1, 1, 1),	// Left, bottom, front
	vec4(1, -1, 1, 1),	// Right, bottom, front
	vec4(1, -1, -1, 1)	// Right, bottom, back
);
const int iUnitCube[36] = int[36] (
	0, 1, 2, 0, 2, 3, // Top face
	4, 7, 6, 4, 6, 5, // Bottom face
	0, 4, 5, 0, 5, 1, // Left face
	2, 6, 7, 2, 7, 3, // Right face
	0, 7, 4, 0, 3, 7, // Back face
	1, 5, 6, 1, 6, 2  // Front face
);

// Cubic smoothing, yields C1 continuous curve from 0..1
float cubic(float r) { return r*r*(3.0-2.0*r); }
vec2 cubic(vec2 r) { return r*r*(3.0-2.0*r); }
vec3 cubic(vec3 r) { return r*r*(3.0-2.0*r); }
vec4 cubic(vec4 r) { return r*r*(3.0-2.0*r); }

// Quintic smoothing, yields C2 continuous curve from 0..1
float quintic(float r) { return r*r*r*(r*(r*6.0-15.0)+10.0); }
vec2 quintic(vec2 r) { return r*r*r*(r*(r*6.0-15.0)+10.0); }
vec3 quintic(vec3 r) { return r*r*r*(r*(r*6.0-15.0)+10.0); }
vec4 quintic(vec4 r) { return r*r*r*(r*(r*6.0-15.0)+10.0); }


float bspline(float x) {
	float f = abs(x);
	if(f <= 1.0) {
		float ff = f*f;
		return (2.0 / 3.0) + 0.5 * f*ff - ff;
	}
	if(f <= 2.0) {
		f = 2.0 - f;
		return (1.0 / 6.0) * f*f*f;
	}
	return 1.0;
}  

// Calculates a single bspline value
float bspline(vec4 p, float t)
{
	float tt = t*t;
	float ttt = tt*t;
	float s = 1.0 - t;
	float ss = s*s;
	float sss = ss*s;
	return
		p.x * ((1.0 / 6.0) * sss) +
		p.y * ((2.0 / 3.0) + 0.5 * ttt - tt) +
		p.z * ((2.0 / 3.0) + 0.5 * sss - ss) +
		p.w * ((1.0 / 6.0) * ttt);
}

// Calculates a single Catmull-Rom spline value
float catmull_rom(vec4 p, float t)
{
	const mat4 m4catmull_rom = mat4(
		vec4(0, -1,  2, -1),
		vec4(2,  0, -5,  3),
		vec4(0,  1,  4, -3),
		vec4(0,  0, -1,  1)
	);

	vec4 f4 = vec4(1.0, t, t*t, t*t*t);
	return 0.5 * dot(f4, m4catmull_rom * p);
}

