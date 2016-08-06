// VKShaderObject.h
// This code is part of the VKContext library, an object-oriented class
// library designed to make Vulkan easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//
#ifndef __VKShaderObject_h__
#define __VKShaderObject_h__

#include "VKContext.h"

namespace VK {

// Copy of EShLanguage from glslang compiler (to avoid pulling its headers into the client app)
typedef enum {
	VertexStage,
	TessControlStage,
	TessEvaluationStage,
	GeometryStage,
	FragmentStage,
	ComputeStage,
	StageCount,
} ShaderStage;

class ShaderProgram : public Object {
private:
	struct Stage {
		std::string entry;
		std::string glsl;
		std::vector<uint32_t> spirv;
		VkShaderModule module;

		Stage() : module(NULL) {}
	};

	Stage stages[StageCount];
	bool dirty;

public:
	ShaderProgram(): dirty(false) {}
	virtual ~ShaderProgram() { destroy(); }
	virtual void destroy();
	virtual bool isValid() const {
		for (int i = 0; i < StageCount; i++) {
			if (stages[i].module)
				return true;
		}
		return false;
	}

	void setStage(ShaderStage s, const char *glsl, const char *entry="main");

	bool compile(const char *name);
	void getStages(std::vector<PipelineShaderStageCreateInfo> &s);
};

} // namespace VK

#endif // __GLShaderObject_h__
