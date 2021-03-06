// GLShaderObject.cpp
// This code is part of the GLContext library, an object-oriented class
// library designed to make OpenGL 3.x easier to use with object-oriented
// languages. It was designed and written by Sean O'Neil, who disclaims
// any copyright to release it in the public domain.
//

#include "VKCore.h"
#include "VKShaderProgram.h"
#include "../glslang/glslang/Public/ShaderLang.h"
#include "../glslang/SPIRV/GlslangToSpv.h"
#include "../glslang/SPIRV/disassemble.h"

namespace VK {

const TBuiltInResource DefaultTBuiltInResource = {
	/* .MaxLights = */ 32,
	/* .MaxClipPlanes = */ 6,
	/* .MaxTextureUnits = */ 32,
	/* .MaxTextureCoords = */ 32,
	/* .MaxVertexAttribs = */ 64,
	/* .MaxVertexUniformComponents = */ 4096,
	/* .MaxVaryingFloats = */ 64,
	/* .MaxVertexTextureImageUnits = */ 32,
	/* .MaxCombinedTextureImageUnits = */ 80,
	/* .MaxTextureImageUnits = */ 32,
	/* .MaxFragmentUniformComponents = */ 4096,
	/* .MaxDrawBuffers = */ 32,
	/* .MaxVertexUniformVectors = */ 128,
	/* .MaxVaryingVectors = */ 8,
	/* .MaxFragmentUniformVectors = */ 16,
	/* .MaxVertexOutputVectors = */ 16,
	/* .MaxFragmentInputVectors = */ 15,
	/* .MinProgramTexelOffset = */ -8,
	/* .MaxProgramTexelOffset = */ 7,
	/* .MaxClipDistances = */ 8,
	/* .MaxComputeWorkGroupCountX = */ 65535,
	/* .MaxComputeWorkGroupCountY = */ 65535,
	/* .MaxComputeWorkGroupCountZ = */ 65535,
	/* .MaxComputeWorkGroupSizeX = */ 1024,
	/* .MaxComputeWorkGroupSizeY = */ 1024,
	/* .MaxComputeWorkGroupSizeZ = */ 64,
	/* .MaxComputeUniformComponents = */ 1024,
	/* .MaxComputeTextureImageUnits = */ 16,
	/* .MaxComputeImageUniforms = */ 8,
	/* .MaxComputeAtomicCounters = */ 8,
	/* .MaxComputeAtomicCounterBuffers = */ 1,
	/* .MaxVaryingComponents = */ 60,
	/* .MaxVertexOutputComponents = */ 64,
	/* .MaxGeometryInputComponents = */ 64,
	/* .MaxGeometryOutputComponents = */ 128,
	/* .MaxFragmentInputComponents = */ 128,
	/* .MaxImageUnits = */ 8,
	/* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
	/* .MaxCombinedShaderOutputResources = */ 8,
	/* .MaxImageSamples = */ 0,
	/* .MaxVertexImageUniforms = */ 0,
	/* .MaxTessControlImageUniforms = */ 0,
	/* .MaxTessEvaluationImageUniforms = */ 0,
	/* .MaxGeometryImageUniforms = */ 0,
	/* .MaxFragmentImageUniforms = */ 8,
	/* .MaxCombinedImageUniforms = */ 8,
	/* .MaxGeometryTextureImageUnits = */ 16,
	/* .MaxGeometryOutputVertices = */ 256,
	/* .MaxGeometryTotalOutputComponents = */ 1024,
	/* .MaxGeometryUniformComponents = */ 1024,
	/* .MaxGeometryVaryingComponents = */ 64,
	/* .MaxTessControlInputComponents = */ 128,
	/* .MaxTessControlOutputComponents = */ 128,
	/* .MaxTessControlTextureImageUnits = */ 16,
	/* .MaxTessControlUniformComponents = */ 1024,
	/* .MaxTessControlTotalOutputComponents = */ 4096,
	/* .MaxTessEvaluationInputComponents = */ 128,
	/* .MaxTessEvaluationOutputComponents = */ 128,
	/* .MaxTessEvaluationTextureImageUnits = */ 16,
	/* .MaxTessEvaluationUniformComponents = */ 1024,
	/* .MaxTessPatchComponents = */ 120,
	/* .MaxPatchVertices = */ 32,
	/* .MaxTessGenLevel = */ 64,
	/* .MaxViewports = */ 16,
	/* .MaxVertexAtomicCounters = */ 0,
	/* .MaxTessControlAtomicCounters = */ 0,
	/* .MaxTessEvaluationAtomicCounters = */ 0,
	/* .MaxGeometryAtomicCounters = */ 0,
	/* .MaxFragmentAtomicCounters = */ 8,
	/* .MaxCombinedAtomicCounters = */ 8,
	/* .MaxAtomicCounterBindings = */ 1,
	/* .MaxVertexAtomicCounterBuffers = */ 0,
	/* .MaxTessControlAtomicCounterBuffers = */ 0,
	/* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
	/* .MaxGeometryAtomicCounterBuffers = */ 0,
	/* .MaxFragmentAtomicCounterBuffers = */ 1,
	/* .MaxCombinedAtomicCounterBuffers = */ 1,
	/* .MaxAtomicCounterBufferSize = */ 16384,
	/* .MaxTransformFeedbackBuffers = */ 4,
	/* .MaxTransformFeedbackInterleavedComponents = */ 64,
	/* .MaxCullDistances = */ 8,
	/* .MaxCombinedClipAndCullDistances = */ 8,
	/* .MaxSamples = */ 4,
	/* .limits = */{
		/* .nonInductiveForLoops = */ 1,
		/* .whileLoops = */ 1,
		/* .doWhileLoops = */ 1,
		/* .generalUniformIndexing = */ 1,
		/* .generalAttributeMatrixVectorIndexing = */ 1,
		/* .generalVaryingIndexing = */ 1,
		/* .generalSamplerIndexing = */ 1,
		/* .generalVariableIndexing = */ 1,
		/* .generalConstantMatrixVectorIndexing = */ 1,
	}
};

void ShaderProgram::destroy() {
	for (size_t i = 0; i < StageCount; i++) {
		if (stages[i].module) {
			vkDestroyShaderModule(vk, stages[i].module, NULL);
			stages[i].module = NULL;
		}
		stages[i].spirv.clear();
		stages[i].glsl.clear();
		stages[i].entry.clear();
	}
}

void ShaderProgram::setStage(ShaderStage s, const char *glsl, const char *entry) {
	stages[s].entry = entry;
	stages[s].glsl.assign(glsl);
	dirty = true;
}

#ifdef _WIN32
bool LaunchAndWait(const char *pszCommand, const char *pszWorkingFolder, DWORD dwFlags, DWORD dwExpectedExitCode, WORD nShow) {
	//TMLogDebug("LaunchAndWait(%s)", pszCommand);
	PROCESS_INFORMATION pi;
	STARTUPINFO si;
	memset(&si, 0, sizeof(STARTUPINFO));
	si.cb = sizeof(STARTUPINFO);
	si.dwFlags = STARTF_USESHOWWINDOW;
	si.wShowWindow = nShow;
	if (!CreateProcess(NULL, (char *)pszCommand, NULL, NULL, NULL, dwFlags, NULL, pszWorkingFolder, &si, &pi))
		return false;
	WaitForSingleObject(pi.hProcess, INFINITE);
	DWORD dwExitCode = 0;
	if (!GetExitCodeProcess(pi.hProcess, &dwExitCode) || dwExitCode != dwExpectedExitCode)
		return false;
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);
	return true;
}
#endif

bool ShaderProgram::compile(const char *name) {
	EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
	glslang::TProgram program;
	const char *stageNames[StageCount] = {"Vertex", "TessControl", "TessEvaluation", "Geometry", "Fragment", "Compute"};
	const char *extensions[StageCount] = { "vert", "tesc", "tese", "geom", "frag", "comp" };
	glslang::TShader tShaders[EShLangCount] = {
		glslang::TShader(EShLangVertex),
		glslang::TShader(EShLangTessControl),
		glslang::TShader(EShLangTessEvaluation),
		glslang::TShader(EShLangGeometry),
		glslang::TShader(EShLangFragment),
		glslang::TShader(EShLangCompute)
	};

//#define glslangValidator "C:\\VulkanSDK\\1.0.21.1\\Bin\\glslangValidator.exe"
#ifdef glslangValidator
	char szCommand[1024];
	sprintf(szCommand, glslangValidator " -V");
#endif
	const char *psz = NULL;
	for (int i = 0; i < EShLangCount; i++) {
		if (stages[i].glsl.empty())
			continue;

#ifdef glslangValidator
		VK::Path pGLSL = VK::Path::Shader().add("temp.%s", extensions[i]);
		strcat(szCommand, " ");
		strcat(szCommand, pGLSL.basename().c_str());
		std::ofstream os(pGLSL, std::ios::binary);
		os << stages[i].glsl;
		os.close();
#endif

		psz = stages[i].glsl.c_str();
		tShaders[i].setStrings(&psz, 1);
		if (!stages[i].entry.empty())
			tShaders[i].setEntryPoint(stages[i].entry.c_str());
		VKLogDebug("%s stage for program: %s\n%s", stageNames[i], name, stages[i].glsl.c_str());
		if (!tShaders[i].parse(&DefaultTBuiltInResource, 110, false, messages)) {
			psz = tShaders[i].getInfoLog();
			if (psz && *psz)
				VKLogError("%s", psz);
			return false;
		}
		psz = tShaders[i].getInfoLog();
		if (psz && *psz)
			VKLogInfo("%s stage parsing info for program: %s\n%s", stageNames[i], name, psz);
		psz = tShaders[i].getInfoDebugLog();
		if (psz && *psz)
			VKLogDebug("%s stage debug info for program: %s\n%s", stageNames[i], name, psz);
		program.addShader(&tShaders[i]);
	}

	if (!program.link(messages)) {
		psz = program.getInfoLog();
		if (psz && *psz)
			VKLogError("Link failed for program: %s\n%s", name, psz);
		return false;
	}

#ifdef glslangValidator
	LaunchAndWait(szCommand, VK::Path::Shader(), 0, 0, SW_HIDE);
#endif

	for (int i = 0; i < EShLangCount; i++) {
		stages[i].spirv.clear();
		if (stages[i].module) {
			vkDestroyShaderModule(vk, stages[i].module, NULL);
			stages[i].module = NULL;
		}
		glslang::TIntermediate *intermediate = program.getIntermediate((EShLanguage)i);
		if (intermediate) {
			glslang::GlslangToSpv(*intermediate, stages[i].spirv);

#ifdef glslangValidator
			VK::Path p = VK::Path::Shader().add("%s.spv", extensions[i]);
			std::string str = p.read();
			std::vector<uint32_t> spirv;
			spirv.resize(str.size() / 4);
			memcpy(&spirv[0], str.c_str(), str.size());
			if (spirv.size() != stages[i].spirv.size() || memcmp(&spirv[0], &stages[i].spirv[0], spirv.size() * 4) != 0)
				VKLogWarning("glslangValidator mismatch!");
#endif

			std::ostringstream ostr;
			spv::Disassemble(ostr, stages[i].spirv);
			VKLogDebug("%s stage dissassembly for program: %s\n%s", stageNames[i], name, ostr.str().c_str());
			ShaderModuleCreateInfo shader(stages[i].spirv);
			OBJ_CHECK(vkCreateShaderModule(vk, &shader, nullptr, &stages[i].module));
		}
	}
	return true;
}

void ShaderProgram::getStages(std::vector<PipelineShaderStageCreateInfo> &s) {
	VkShaderStageFlagBits bits[StageCount] = {
		VK_SHADER_STAGE_VERTEX_BIT,
		VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT,
		VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT,
		VK_SHADER_STAGE_GEOMETRY_BIT,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		VK_SHADER_STAGE_COMPUTE_BIT
	};
	for (int i = 0; i < StageCount; i++) {
		if (stages[i].module)
			s.push_back(PipelineShaderStageCreateInfo(bits[i], stages[i].module));
	}
};

} // namespace VK 
