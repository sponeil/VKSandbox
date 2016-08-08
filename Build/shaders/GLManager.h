// GLManager.h
//

struct SceneData {
	// Shared viewport settings (generally remains static during each render pass)
	vec4 vSize;			// The current viewport size (NOTE: packing will round it up to vec4)

	// Shared transformation matrices (generally remains static during each render pass)
	mat4 mProjection;	// The perspective projection matrix
	mat4 mOrtho;		// The orthographic projection matrix
	mat4 mView;			// The view matrix
	mat4 mViewProj;		// Memory vs. speed trade-off (when working in world space)
};

struct GUIData {
	vec4 vGUIRect;
	vec4 vGUIColor;
	vec4 vGUIOptions;
};

struct TextData {
	vec4 vCharPos;
	vec4 vCharColor;
};

