// VKManager.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#ifndef __VKManager_h__
#define __VKManager_h__

#include "VKContext.h"
#include "VKTransform.h"
#include "VKBufferObject.h"
#include "VKImage.h"
#include "VKRenderPass.h"
#include "VKShaderFile.h"
#include "VKFont.h"
#include "VKTransform.h"

namespace VK {

#include "../../Build/shaders/GLManager.h"
#define MAX_GUI_INSTANCES 10000

/// Provides a simple management layer for loading Vulkan shaders and fonts.
/// It also manages the scene, gui, and text uniform buffers and GUI rendering calls.
/// GUI rendering can be done in its own pass or as part of another pass.
class Manager {
public:
	Context &vk; ///< The Vulkan context we want to manage

protected:
	float m_fFOV; ///< The field of view angle (in degrees)
	float m_fNear; ///< The distance to the near clipping plane
	float m_fFar; ///< The distance to the far clipping plane

	double m_dLastShaderUpdate; ///< The last time the shader files were checked for updates
	std::map<std::string, ShaderTechnique> m_mapTechniques; ///< Map of managed shader techniques (from glfx files)
	std::map<std::string, ShaderFile> m_mapGLSLFiles; ///< Map of managed GLSL shader files
	std::map<std::string, FXFile> m_mapGLFXFiles; ///< Map of GLFX effect files
	std::map<std::string, Font> m_mapFonts; ///< Map of managed fonts

	VkDescriptorPool descriptorPool;
	VkPipelineLayout pipelineLayout;
	UniformBuffer sceneBuffer, guiBuffer, textBuffer;

	SceneData scene;
	GUIData gui[MAX_GUI_INSTANCES];
	TextData text[MAX_GUI_INSTANCES];
	uint32_t nGUIElements, nTextElements;
	ShaderTechnique *pLastTechnique;

public:
	Manager() : vk(*Context::GetCurrent()), m_fFOV(45.0f), m_fNear(0.1f), m_fFar(1000.0f), descriptorPool(NULL), pipelineLayout(NULL) {} ///< Default constructor
	Manager(Context &context) : vk(context), m_fFOV(45.0f), m_fNear(0.1f), m_fFar(1000.0f), descriptorPool(NULL), pipelineLayout(NULL) {} ///< Default constructor
	~Manager() { destroy(); } ///< Default destructor (destroys Vulkan objects and context)

	void init(); ///< Creates a number of useful default managed objects
	void destroy(); ///< Destroys all managed objects
	bool isValid() const { return !m_mapTechniques.empty(); }

	/// Call once per frame to check for updated shader files
	/// @param[in] fPeriodLength The amount of time (in seconds) between checks for managed files that have changed
	void update(float fPeriodLength);

	float getFOV() const { return m_fFOV; }
	float getNear() const { return m_fNear; }
	float getFar() const { return m_fFar; }
	void setFOV(float f) { m_fFOV = f; }
	void setNear(float f) { m_fNear = f; }
	void setFar(float f) { m_fFar = f; }

	void cleanup(); ///< Call before rebuilding the swapchain
	void reinit(RenderPass &guiPass, uint16_t nWidth, uint16_t nHeight); ///< Call after rebuilding the swapchain

	UniformBuffer &getSceneBuffer() { return sceneBuffer; }
	VkDescriptorPool &getDescriptorPool() { return descriptorPool; }

	void setViewMatrix(VK::mat4 &m) {
		scene.mView = m;
		scene.mViewProj = scene.mProjection * scene.mView;
		sceneBuffer.update(&scene);
	}

	/// @name Managed shader technique methods
	//@{
	/// Call before Manager::init() to specify which VKFX files this app should load.
	void loadFX(const char *pszFile) {
		VK::Path path = VK::Path::Shader() + pszFile;
		if(!path.exists() && !path.file())
			VKLogException("Unable to find FX file: %s", (const char *)path);
		m_mapGLFXFiles[pszFile];
	}

	/// Checks for modified shader files and rebuilds affected techniques.
	/// If any modified techniques fail to compile/link, a warning is logged
	/// and the previous iteration is kept to avoid interrupting the program
	/// while it is running. It is unbelievably useful to be able to see the
	/// results of a shader change as soon as you save it without breaking
	/// your work flow.
	bool updateShaders();

	/// Call to retrieve a pointer to a managed shader technique by name.
	/// @param[in] pszName The name of the shader technique
	/// @return A pointer to the VK::ShaderTechnique object if it exists, NULL otherwise
	ShaderTechnique *getTechnique(const char *pszName) {
		std::map<std::string, ShaderTechnique>::iterator it = m_mapTechniques.find(pszName);
		return it == m_mapTechniques.end() ? NULL : &it->second;
	}
	///@}

	/// @name Managed font methods
	//@{
	/// Call to load a font texture into a VK::Font object.
	/// @param[in] pszName The name of the TTF file (without the filename extension)
	bool loadFont(const char *pszName);
	/// Call to remove a managed VK::Font object.
	/// @param[in] pszName The name of the font (without the filename extension)
	void removeFont(const char *pszName) { m_mapFonts.erase(pszName); }
	/// Call to get a pointer to a managed font by name.
	/// @param[in] pszName The name of the font (without the filename extension)
	/// @return A pointer to the VK::Font object if it exists, NULL otherwise
	Font *getFont(const char *pszName) {
		std::map<std::string, Font>::iterator it = m_mapFonts.find(pszName);
		return it == m_mapFonts.end() ? NULL : &it->second;
	}
	//@}

	void begin(VkCommandBuffer cmd) {
		nGUIElements = nTextElements = 0;
		pLastTechnique = NULL;

		VkDescriptorSet descriptorSets[] = { sceneBuffer, guiBuffer, textBuffer };
		vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 3, descriptorSets, 0, NULL);
	}

	void end() {
		if(nGUIElements)
			guiBuffer.update(&gui, 0, sizeof(GUIData)*nGUIElements);
		if (nTextElements)
			textBuffer.update(&text, 0, sizeof(TextData)*nTextElements);
	}

	void addGUIElements(VkCommandBuffer cmd, const char *type, GUIData *data, uint32_t instances) {
		ShaderTechnique *p = getTechnique(type);
		if (pLastTechnique != p) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *p);
			pLastTechnique = p;
		}
		vkCmdDraw(cmd, 6, instances, 0, nGUIElements);
		memcpy(&gui[nGUIElements], data, sizeof(GUIData)*instances);
		nGUIElements += instances;
	}

	void addText(VkCommandBuffer cmd, const char *pszFont, const char *pszText, vec2 pos, vec4 color, float size, Font::AlignX xAlign = Font::AlignXLeft, Font::AlignY yAlign = Font::AlignYTop) {
		ShaderTechnique *p = getTechnique("Text2D");
		Font *font = getFont(pszFont);
		if (pLastTechnique != p) {
			vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, *p);
			pLastTechnique = p;
		}
		font->begin(cmd, color, size);
		font->draw2D(cmd, &text[nTextElements].vCharPos, nTextElements, pszText, pos, xAlign, yAlign);
	}
};


} // namespace VK

#endif // __VKManager_h__
