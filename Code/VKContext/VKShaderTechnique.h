// VKShaderTechnique.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKShaderTechnique_h__
#define __VKShaderTechnique_h__

#include "VKShaderProgram.h"
#include "VKString.h"

namespace VK {

// This simple struct merely stores the type and name of attributes parsed for a technique.
struct ShaderAttribute {
	std::string strType, strName, strLayout;
	ShaderAttribute(const char *pszType, const char *pszName, const char *pszLayout) {
		strType = pszType;
		strName = pszName;
		strLayout = pszLayout;
	}
};

/// This class encapsulates an FX-like technique for a shader program.
/// In addition to a shader program, it manages a list of OpenVK states
/// associated with the technique. It also parses "technique {}" code blocks
/// from .glfx files. A single .glfx file can contain several techniques,
/// so the code to parse a .glfx file must be encapsulated outside this
/// class. That external parser is responsible for:
/// 1) Handling any pre-processing (stripping comments, expanding #include statements).
/// 2) Stripping all "technique {}" code blocks from the source.
/// 3) Passing what is left to this class along with a single technique block.
class ShaderTechnique : public ShaderProgram {
protected:
	VkPipeline pipeline;
	VkPipelineLayout pipelineLayout;

	VkPrimitiveTopology topology;// = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
	VkPolygonMode fill;// = VK_POLYGON_MODE_FILL
	VkCullModeFlags cull;// = VK_CULL_MODE_BACK_BIT
	VkFrontFace front;// = VK_FRONT_FACE_COUNTER_CLOCKWISE
	VkBool32 blendEnable; // = VK_FALSE
	VkBool32 depthTestEnable; // = VK_TRUE
	VkBool32 depthWriteEnable; // = VK_TRUE
	VkCompareOp depthCompareOp; // = VK_COMPARE_OP_LESS_OR_EQUAL
	VkBlendFactor srcColorBlend, dstColorBlend; // = VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
	VkBlendFactor srcAlphaBlend, dstAlphaBlend; // = VK_BLEND_FACTOR_ONE, VK_BLEND_FACTOR_ZERO
	VkBlendOp colorBlendOp, alphaBlendOp; // = VK_BLEND_OP_ADD, VK_BLEND_OP_ADD

	// Technique attributes
	std::vector<ShaderAttribute> m_vInputs; ///< A list of input attributes for this technique's vertex shader
	std::vector<ShaderAttribute> m_vGeometry; ///< A list of attributes passed to the geometry shader
	std::vector<ShaderAttribute> m_vFragment; ///< A list of attributes passed to the fragment shader
	std::vector<ShaderAttribute> m_vOutputs; ///< A list of output attributes for this technique's fragment shader
	std::vector<ShaderAttribute> m_vUniforms; ///< A list of uniforms define in the "technique" code block

	void clear();
	bool addState(const char *pszName, const char *pszValue);
	void addInput(const char *pszType, const char *pszName, const char *layout) { m_vInputs.push_back(ShaderAttribute(pszType, pszName, layout)); }
	void addGeometryAttribute(const char *pszType, const char *pszName, const char *layout) { m_vGeometry.push_back(ShaderAttribute(pszType, pszName, layout)); }
	void addFragmentAttribute(const char *pszType, const char *pszName, const char *layout) { m_vFragment.push_back(ShaderAttribute(pszType, pszName, layout)); }
	void addOutput(const char *pszType, const char *pszName, const char *layout) { m_vOutputs.push_back(ShaderAttribute(pszType, pszName, layout)); }
	void addUniform(const char *pszType, const char *pszName, const char *layout) { m_vUniforms.push_back(ShaderAttribute(pszType, pszName, layout)); }

public:
	ShaderTechnique() : pipeline(NULL), pipelineLayout(NULL) {}
	virtual ~ShaderTechnique() { destroy(); }
	virtual void destroy();

	operator VkPipeline() const { return pipeline; }
	operator VkPipelineLayout() const { return pipelineLayout; }

	VkPolygonMode getFillMode() const { return fill; }
	void setFillMode(VkPolygonMode f) { fill = f; }

	const unsigned int inputs() const { return (unsigned int)m_vInputs.size(); }
	const unsigned int geometryAttributes() const { return (unsigned int)m_vGeometry.size(); }
	const unsigned int fragmentAttributes() const { return (unsigned int)m_vFragment.size(); }
	const unsigned int outputs() const { return (unsigned int)m_vOutputs.size(); }
	const unsigned int uniforms() const { return (unsigned int)m_vUniforms.size(); }
	const ShaderAttribute &input(unsigned int n) const { return m_vInputs[n]; }
	const ShaderAttribute &geometryAttr(unsigned int n) const { return m_vGeometry[n]; }
	const ShaderAttribute &fragmentAttr(unsigned int n) const { return m_vFragment[n]; }
	const ShaderAttribute &output(unsigned int n) const { return m_vOutputs[n]; }
	const ShaderAttribute &uniform(unsigned int n) const { return m_vUniforms[n]; }

	/// Parses a "technique {}" code block. If you already have a String::Parser
	/// object, this version of parse() saves an alloc and a string copy.
	/// @param[in] parser A String::Parser object containing a "technique {}" code block
	/// @param[in] strPreparedCode Prepared code from a .glfx file
	/// @return true on success, false on failure
	bool parse(String::Parser &parser, std::string &strPreparedCode, int nVersion);

	/// Parses a "technique {}" code block.
	/// This version merely creates a String::Parser and calls that version of parse().
	bool parse(const char *pszTechniqueBlock, unsigned int nLength, std::string &strPreparedCode, int nVersion) {
        String::Parser p(pszTechniqueBlock, nLength);
		parse(p, strPreparedCode, nVersion);
	}

	/// Parses a "technique {}" code block.
	/// This version merely creates a String::Parser and calls that version of parse().
	bool parse(std::string &strTechniqueBlock, std::string &strPreparedCode, int nVersion) {
        String::Parser p(strTechniqueBlock);
		parse(p, strPreparedCode, nVersion);
	}

	void buildPipeline(VkRenderPass pass, VkPipelineLayout pipelineLayout);
	void buildPipeline(VkRenderPass pass, VkDescriptorSetLayout *layouts, uint32_t count) {
		VK::PipelineLayoutCreateInfo pipelineLayoutInfo(layouts, count);
		vkCreatePipelineLayout(vk, &pipelineLayoutInfo, NULL, &pipelineLayout);
		buildPipeline(pass, pipelineLayout);
	}
	void destroyPipeline();

	/// Call to enable the technique for rendering.
	//void enable();
	/// Call to disable the technique for rendering.
	//void disable();
};

} // namespace VK

#endif // __VKShaderTechnique_h__
