// VKShaderManager.cpp
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKShaderTechnique.h"

namespace VK {

void ShaderTechnique::clear() {
	// Set all states to the default
	topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	fill = VK_POLYGON_MODE_FILL;
	cull = VK_CULL_MODE_BACK_BIT;
	front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	blendEnable = VK_FALSE;
	depthTestEnable = depthWriteEnable = VK_TRUE;
	depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	srcColorBlend = srcAlphaBlend = VK_BLEND_FACTOR_ONE;
	dstColorBlend = dstAlphaBlend = VK_BLEND_FACTOR_ZERO;
	colorBlendOp = alphaBlendOp = VK_BLEND_OP_ADD;

	m_vInputs.clear();
	m_vGeometry.clear();
	m_vFragment.clear();
	m_vOutputs.clear();
	m_vUniforms.clear();
}

void ShaderTechnique::destroy() {
	clear();
	if (pipeline) {
		vkDestroyPipeline(vk, pipeline, NULL);
		pipeline = NULL;
	}
	if (pipelineLayout) {
		vkDestroyPipelineLayout(vk, pipelineLayout, NULL);
		pipelineLayout = NULL;
	}
	ShaderProgram::destroy();
}

bool ShaderTechnique::addState(const char *pszName, const char *pszValue) {
	// Handle the glEnable states
	if(strcmp(pszName, "Enable") == 0) {
		if (strcmp(pszValue, "BLEND") == 0)
			blendEnable = VK_TRUE;
		else if (strcmp(pszValue, "DEPTH_TEST") == 0)
			depthTestEnable = VK_TRUE;
		else if (strcmp(pszValue, "DEPTH_WRITE") == 0)
			depthWriteEnable = VK_TRUE;
		else
			return false;
		return true;
	}

	// Handle the glDisable states
	if (strcmp(pszName, "Disable") == 0) {
		if (strcmp(pszValue, "BLEND") == 0)
			blendEnable = VK_FALSE;
		else if (strcmp(pszValue, "DEPTH_TEST") == 0)
			depthTestEnable = VK_FALSE;
		else if (strcmp(pszValue, "DEPTH_WRITE") == 0)
			depthWriteEnable = VK_FALSE;
		else
			return false;
		return true;
	}

	// Handle the enum states

	// glFrontFace() states
	if (strcmp(pszName, "FrontFace") == 0) {
		if (strcmp(pszValue, "CW") == 0)
			front = VK_FRONT_FACE_CLOCKWISE;
		else
			front = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		return true;
	}

	// glCullFace() states
	if (strcmp(pszName, "CullFace") == 0) {
		if(strcmp(pszValue, "FRONT_AND_BACK") == 0)
			cull = VK_CULL_MODE_FRONT_AND_BACK;
		else if (strcmp(pszValue, "FRONT") == 0)
			cull = VK_CULL_MODE_FRONT_BIT;
		else if (strcmp(pszValue, "BACK") == 0)
			cull = VK_CULL_MODE_BACK_BIT;
		else
			cull = VK_CULL_MODE_NONE;
		return true;
	}

	// glDepthFunc() states
	if (strcmp(pszName, "DepthFunc") == 0) {
		if (strcmp(pszValue, "NEVER") == 0)
			depthCompareOp = VK_COMPARE_OP_NEVER;
		else if (strcmp(pszValue, "LESS") == 0)
			depthCompareOp = VK_COMPARE_OP_LESS;
		else if (strcmp(pszValue, "EQUAL") == 0)
			depthCompareOp = VK_COMPARE_OP_EQUAL;
		else if (strcmp(pszValue, "GREATER") == 0)
			depthCompareOp = VK_COMPARE_OP_GREATER;
		else if (strcmp(pszValue, "VK_COMPARE_OP_NOT_EQUAL") == 0)
			depthCompareOp = VK_COMPARE_OP_NOT_EQUAL;
		else if (strcmp(pszValue, "GEQUAL") == 0)
			depthCompareOp = VK_COMPARE_OP_GREATER_OR_EQUAL;
		else if (strcmp(pszValue, "ALWAYS") == 0)
			depthCompareOp = VK_COMPARE_OP_ALWAYS;
		else
			depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		return true;
	}

	// glPolygonMode() states
	if (strcmp(pszName, "PolygonMode") == 0) {
		if(strcmp(pszValue, "POINT") == 0)
			fill = VK_POLYGON_MODE_POINT;
		else if(strcmp(pszValue, "LINE") == 0)
			fill = VK_POLYGON_MODE_LINE;
		else
			fill = VK_POLYGON_MODE_FILL;
		return true;
	}

	// glBlendEquation() states
	if (strcmp(pszName, "BlendEquation") == 0) {
		if (strcmp(pszValue, "SUBTRACT") == 0)
			colorBlendOp = VK_BLEND_OP_SUBTRACT;
		else if (strcmp(pszValue, "REVERSE_SUBTRACT") == 0)
			colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
		else if (strcmp(pszValue, "MIN") == 0)
			colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
		else if (strcmp(pszValue, "MIN") == 0)
			colorBlendOp = VK_BLEND_OP_REVERSE_SUBTRACT;
		else
			colorBlendOp = VK_BLEND_OP_ADD;
		return true;
	}

	// glBlendFunc() states
	if (strcmp(pszName, "BlendSrc") == 0 || strcmp(pszName, "BlendDest") == 0) {
		VkBlendFactor &f = pszName[5] == 'S' ? srcColorBlend : dstColorBlend;
		if (strcmp(pszValue, "ZERO") == 0)
			f = VK_BLEND_FACTOR_ZERO;
		else if (strcmp(pszValue, "ONE") == 0)
			f = VK_BLEND_FACTOR_ONE;
		else if (strcmp(pszValue, "SRC_COLOR") == 0)
			f = VK_BLEND_FACTOR_SRC_COLOR;
		else if (strcmp(pszValue, "ONE_MINUS_SRC_COLOR") == 0)
			f = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		else if (strcmp(pszValue, "DST_COLOR") == 0)
			f = VK_BLEND_FACTOR_DST_COLOR;
		else if (strcmp(pszValue, "ONE_MINUS_DST_COLOR") == 0)
			f = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		else if (strcmp(pszValue, "SRC_ALPHA") == 0)
			f = VK_BLEND_FACTOR_SRC_ALPHA;
		else if (strcmp(pszValue, "ONE_MINUS_SRC_ALPHA") == 0)
			f = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		else if (strcmp(pszValue, "DST_ALPHA") == 0)
			f = VK_BLEND_FACTOR_DST_ALPHA;
		else if (strcmp(pszValue, "ONE_MINUS_DST_ALPHA") == 0)
			f = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		else if (strcmp(pszValue, "CONSTANT_COLOR") == 0)
			f = VK_BLEND_FACTOR_CONSTANT_COLOR;
		else if (strcmp(pszValue, "ONE_MINUS_CONSTANT_COLOR") == 0)
			f = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		else if (strcmp(pszValue, "CONSTANT_ALPHA") == 0)
			f = VK_BLEND_FACTOR_CONSTANT_ALPHA;
		else if (strcmp(pszValue, "ONE_MINUS_CONSTANT_ALPHA") == 0)
			f = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		else if (strcmp(pszValue, "SRC_ALPHA_SATURATE") == 0)
			f = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		else
			return false;
		return true;
	}

	return false;
}

bool ShaderTechnique::parse(String::Parser &parser, std::string &strPreparedCode, int nVersion) {
	char szToken[256], szName[256], szStatement[256], szLayout[256], szOp1[256], szOp2[16384];
	char szVertex[16384], szFragment[16384], szGeometry[16384];
	char szGeometryIn[256], szGeometryOut[256];
	unsigned int nLength;

	// Clear all technique states, inputs, varyings, outputs, etc.
	*szVertex = *szFragment = *szGeometry = *szGeometryIn = *szGeometryOut = *szLayout = 0;
	clear();

	// Strip "technique", <name>, and "{"
	nLength = parser.nextToken(szToken); // The check above ensures the first token will always be "technique"
	nLength = parser.nextToken(szName); // This token should be the technique name
	nLength = parser.nextToken(szToken); // This token should always be "{"

	// Parse all statements until we reach "}"
	while(nLength = parser.nextToken(szStatement)) {
		if(nLength == 1 && *szStatement == '}') {
			break; // Close the technique
		} else if (strcmp(szStatement, "layout") == 0) { // If this is a layer description for a const/uniform/etc
			parser.nextToken(szOp1);
			parser.nextDelimiter(szLayout, ')'); // Cache it in szLayout to use when we parse the const/uniform/etc
			parser.nextToken(szOp1);
			continue;
		} else if(strcmp(szStatement, "state") == 0) {
			parser.nextToken(szOp1);
			parser.nextDelimiter(szOp2, ';');
			if(!addState(szOp1, szOp2)) // If the state is not valid, add a warning but keep going
				vk.addWarning(Logger::format("Technique %s has an invalid state \"%s\" \"%s\"", szName, szOp1, szOp2));
		} else if(strcmp(szStatement, "in") == 0) {
			parser.nextToken(szOp1);
			parser.nextDelimiter(szOp2, ';');
			addInput(szOp1, szOp2, szLayout);
			*szLayout = 0;
		} else if(strcmp(szStatement, "geom") == 0) {
			parser.nextToken(szOp1);
			parser.nextDelimiter(szOp2, ';');
			addGeometryAttribute(szOp1, szOp2, szLayout);
			*szLayout = 0;
		} else if(strcmp(szStatement, "frag") == 0) {
			parser.nextToken(szOp1);
			parser.nextDelimiter(szOp2, ';');
			addFragmentAttribute(szOp1, szOp2, szLayout);
			*szLayout = 0;
		} else if(strcmp(szStatement, "out") == 0) {
			parser.nextToken(szOp1);
			parser.nextDelimiter(szOp2, ';');
			addOutput(szOp1, szOp2, szLayout);
			*szLayout = 0;
		} else if(strcmp(szStatement, "uniform") == 0) {
			parser.nextToken(szOp1);
			unsigned int n = parser.nextCodeBlock(szOp2);
			if (n > 0) szOp2[n++] = ' ';
			parser.nextDelimiter(szOp2+n, ';');
			addUniform(szOp1, szOp2, szLayout);
			*szLayout = 0;
		} else if(strcmp(szStatement, "Vertex") == 0) {
			parser.nextCodeBlock(szVertex);
			continue;
		} else if(strcmp(szStatement, "Geometry") == 0) {
			if(parser.nextToken(szOp1) != 1 || *szOp1 != '(') {
				vk.addWarning(Logger::format("Technique %s defines a geometry shader without defined inputs/outputs", szName));
				return false;
			}
			parser.nextToken(szGeometryIn);
			if(parser.nextToken(szOp1) != 1 || *szOp1 != ',') {
				vk.addWarning(Logger::format("Technique %s defines a geometry shader with improperly defined inputs/outputs", szName));
				return false;
			}
			parser.nextDelimiter(szGeometryOut, ')');
			if(parser.nextToken(szOp1) != 1 || *szOp1 != ')') {
				vk.addWarning(Logger::format("Technique %s defines a geometry shader with improperly defined inputs/outputs", szName));
				return false;
			}
			parser.nextCodeBlock(szGeometry);
			continue;
		} else if(strcmp(szStatement, "Fragment") == 0) {
			parser.nextCodeBlock(szFragment);
			continue;
		} else {
			vk.addWarning(Logger::format("Technique %s has an invalid statement \"%s\"", szName, szStatement));
			return false;
		}

		// Make sure every statement is terminated with a ';'
		nLength = parser.nextToken(szToken);
		if(nLength != 1 || *szToken != ';') {
			vk.addWarning(Logger::format("Technique %s has a malformed statement - %s %s %s %s", szName, szStatement, szOp1, szOp2, szToken));
			return false;
		}
	}

	if(*szVertex) {
		// Build the main() function for the vertex shader and attempt to compile it
		std::ostringstream ostr;
		for (unsigned int n = 0; n < m_vUniforms.size(); n++) {
			if (!m_vUniforms[n].strLayout.empty())
				ostr << "layout(" << m_vUniforms[n].strLayout << ") ";
			ostr << "uniform " << m_vUniforms[n].strType << " " << m_vUniforms[n].strName << ";" << std::endl;
		}
		for (unsigned int n = 0; n < m_vInputs.size(); n++) {
			if (!m_vInputs[n].strLayout.empty())
				ostr << "layout(" << m_vInputs[n].strLayout << ") ";
			ostr << "in " << m_vInputs[n].strType << " " << m_vInputs[n].strName << ";" << std::endl;
		}
		std::vector<ShaderAttribute> &outputs = *szGeometry ? m_vGeometry : (*szFragment ? m_vFragment : m_vOutputs);
		for (unsigned int n = 0; n < outputs.size(); n++) {
			if (!outputs[n].strLayout.empty())
				ostr << "layout(" << outputs[n].strLayout << ") ";
			ostr << "out " << outputs[n].strType << " " << outputs[n].strName << ";" << std::endl;
		}
		ostr << "out gl_PerVertex { vec4 gl_Position; float gl_PointSize; };" << std::endl;
		ostr << std::endl << "void main() " << szVertex << std::endl;
		setStage(VK::VertexStage, (strPreparedCode + ostr.str()).c_str());
	}

	if(*szGeometry) {
		// Build the main() function for the geometry shader and attempt to compile it
		std::ostringstream ostr;
		int nInputs = szGeometryIn[0] == 't' ? 3 : szGeometryIn[0] == 'l' ? 2 : 1;
		ostr << "layout(" << szGeometryIn << ") in;" << std::endl;
		ostr << "layout(" << szGeometryOut << ") out;" << std::endl;
		for (unsigned int n = 0; n < m_vUniforms.size(); n++) {
			if (!m_vUniforms[n].strLayout.empty())
				ostr << "layout(" << m_vUniforms[n].strLayout << ") ";
			ostr << "uniform " << m_vUniforms[n].strType << " " << m_vUniforms[n].strName << ";" << std::endl;
		}
		for (unsigned int n = 0; n < m_vGeometry.size(); n++) {
			if (!m_vGeometry[n].strLayout.empty())
				ostr << "layout(" << m_vGeometry[n].strLayout << ") ";
			ostr << "in " << m_vGeometry[n].strType << " " << m_vGeometry[n].strName << "[" << nInputs << "];" << std::endl;
		}
		std::vector<ShaderAttribute> &outputs = *szFragment ? m_vFragment : m_vOutputs;
		for (unsigned int n = 0; n < outputs.size(); n++) {
			if (!outputs[n].strLayout.empty())
				ostr << "layout(" << outputs[n].strLayout << ") ";
			ostr << "out " << outputs[n].strType << " " << outputs[n].strName << ";" << std::endl;
		}
		ostr << "in gl_PerVertex { vec4 gl_Position; float gl_PointSize; } gl_in[];" << std::endl;
		ostr << "out gl_PerVertex { vec4 gl_Position; float gl_PointSize; };" << std::endl;
		//ostr << "out int gl_Layer;" << std::endl;
		ostr << std::endl << "void main() " << szGeometry << std::endl;
		setStage(VK::GeometryStage, (strPreparedCode + ostr.str()).c_str());
	}

	if(*szFragment) {
		// Build the main() function for the fragment shader and attempt to compile it
		std::ostringstream ostr;
		for (unsigned int n = 0; n < m_vUniforms.size(); n++) {
			if (!m_vUniforms[n].strLayout.empty())
				ostr << "layout(" << m_vUniforms[n].strLayout << ") ";
			ostr << "uniform " << m_vUniforms[n].strType << " " << m_vUniforms[n].strName << ";" << std::endl;
		}
		for (unsigned int n = 0; n < m_vFragment.size(); n++) {
			if (!m_vFragment[n].strLayout.empty())
				ostr << "layout(" << m_vFragment[n].strLayout << ") ";
			ostr << "in " << m_vFragment[n].strType << " " << m_vFragment[n].strName << ";" << std::endl;
		}
		std::vector<ShaderAttribute> &outputs = m_vOutputs;
		for (unsigned int n = 0; n < outputs.size(); n++) {
			if (!outputs[n].strLayout.empty())
				ostr << "layout(" << outputs[n].strLayout << ") ";
			ostr << "out " << outputs[n].strType << " " << outputs[n].strName << ";" << std::endl;
		}
		//ostr << "in vec4 gl_FragCoord;" << std::endl;
		//ostr << "in flat int gl_Layer;" << std::endl;
		ostr << std::endl << "void main() " << szFragment << std::endl;
		setStage(VK::FragmentStage, (strPreparedCode + ostr.str()).c_str());
	}

	return compile(szName);
}

void ShaderTechnique::buildPipeline(VkRenderPass pass, VkPipelineLayout pipelineLayout) {
	if (!ShaderProgram::isValid())
		return;

	std::vector<PipelineShaderStageCreateInfo> stageInfo;
	ShaderProgram::getStages(stageInfo);

	uint32_t offset = 0;
	std::vector<VertexInputAttributeDescription> attributes;
	attributes.resize(m_vInputs.size());
	for (uint32_t i = 0; i < (uint32_t)m_vInputs.size(); i++) {
		VkFormat f = VK_FORMAT_UNDEFINED;
		uint32_t size = 0;
		if (m_vInputs[i].strType == "float" || memcmp(m_vInputs[i].strType.c_str(), "vec", 3) == 0) {
			size = m_vInputs[i].strType[0] == 'f' ? 1 : (m_vInputs[i].strType[3] - '0');
			switch (size) {
				case 1: f = VK_FORMAT_R32_SFLOAT; break;
				case 2: f = VK_FORMAT_R32G32_SFLOAT; break;
				case 3: f = VK_FORMAT_R32G32B32_SFLOAT; break;
				case 4: f = VK_FORMAT_R32G32B32A32_SFLOAT; break;
			}
			size *= sizeof(float);
		}
		// TODO: Add support for non-float types here
		if (f == VK_FORMAT_UNDEFINED) {
			VKLogException("Unrecognized vertex attribute type: %s", m_vInputs[i].strType.c_str());
		} else {
			attributes[i] = VertexInputAttributeDescription(0, i, f, offset);
			offset += size;
		}
	}
	std::vector<VertexInputBindingDescription> bindings = { VertexInputBindingDescription(0, offset) };

	GraphicsPipelineCreateInfo pipelineInfo(pass, pipelineLayout, &stageInfo);
	if (offset > 0) // Do we even have any vertex shader inputs?
		pipelineInfo.vertex = PipelineVertexInputStateCreateInfo(&bindings, &attributes);
	pipelineInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; // These are already the default values.
	pipelineInfo.rasterization.polygonMode = fill;
	pipelineInfo.rasterization.cullMode = cull;
	pipelineInfo.rasterization.frontFace = front;
	pipelineInfo.dynamic.enabled[pipelineInfo.dynamic.dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;
	pipelineInfo.dynamic.enabled[pipelineInfo.dynamic.dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;
	pipelineInfo.viewport.viewportCount = pipelineInfo.viewport.scissorCount = 1;
	std::vector<PipelineColorBlendAttachmentState> attachments(m_vOutputs.size());
	for (size_t i = 0; i < attachments.size(); i++) {
		attachments[i].blendEnable = blendEnable;
		attachments[i].colorBlendOp = colorBlendOp;
		attachments[i].srcColorBlendFactor = srcColorBlend;
		attachments[i].dstColorBlendFactor = dstColorBlend;
	}
	pipelineInfo.blend.attachmentCount = (uint32_t)attachments.size();
	pipelineInfo.blend.pAttachments = &attachments[0];
	pipelineInfo.depth.depthTestEnable = depthTestEnable;
	pipelineInfo.depth.depthWriteEnable = depthWriteEnable;
	if (pipeline) {
		vkDestroyPipeline(vk, pipeline, NULL);
		pipeline = NULL;
	}
	OBJ_CHECK(vkCreateGraphicsPipelines(vk, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &pipeline));
}

void ShaderTechnique::destroyPipeline() {
	if (pipeline) {
		vkDestroyPipeline(vk, pipeline, NULL);
		pipeline = NULL;
	}
	if (pipelineLayout) {
		vkDestroyPipelineLayout(vk, pipelineLayout, NULL);
		pipelineLayout = NULL;
	}
}

} // namespace VK
