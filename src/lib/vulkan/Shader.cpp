#if HAS_LIB_VULKAN
#include "lib/base/error.h"
#include "lib/base/error.h"


#include "Shader.h"
#include "DescriptorSet.h"
#include "Texture.h"
#include "Device.h"
#include <vulkan/vulkan.h>

#include "common.h"
#include "../os/file.h"
#include "../os/filesystem.h"
#include "../os/formatter.h"
#include "../os/msg.h"

#include <glslang/Include/glslang_c_interface.h>
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>

static TBuiltInResource create_glslang_resources() {
    TBuiltInResource resources{};
    resources.maxLights                                 = 32;
    resources.maxClipPlanes                             = 6;
    resources.maxTextureUnits                           = 32;
    resources.maxTextureCoords                          = 32;
    resources.maxVertexAttribs                          = 64;
    resources.maxVertexUniformComponents                = 4096;
    resources.maxVaryingFloats                          = 64;
    resources.maxVertexTextureImageUnits                = 32;
    resources.maxCombinedTextureImageUnits              = 80;
    resources.maxTextureImageUnits                      = 32;
    resources.maxFragmentUniformComponents              = 4096;
    resources.maxDrawBuffers                            = 32;
    resources.maxVertexUniformVectors                   = 128;
    resources.maxVaryingVectors                         = 8;
    resources.maxFragmentUniformVectors                 = 16;
    resources.maxVertexOutputVectors                    = 16;
    resources.maxFragmentInputVectors                   = 15;
    resources.minProgramTexelOffset                     = -8;
    resources.maxProgramTexelOffset                     = 7;
    resources.maxClipDistances                          = 8;
    resources.maxComputeWorkGroupCountX                 = 65535;
    resources.maxComputeWorkGroupCountY                 = 65535;
    resources.maxComputeWorkGroupCountZ                 = 65535;
    resources.maxComputeWorkGroupSizeX                  = 1024;
    resources.maxComputeWorkGroupSizeY                  = 1024;
    resources.maxComputeWorkGroupSizeZ                  = 64;
    resources.maxComputeUniformComponents               = 1024;
    resources.maxComputeTextureImageUnits               = 16;
    resources.maxComputeImageUniforms                   = 8;
    resources.maxComputeAtomicCounters                  = 8;
    resources.maxComputeAtomicCounterBuffers            = 1;
    resources.maxVaryingComponents                      = 60;
    resources.maxVertexOutputComponents                 = 64;
    resources.maxGeometryInputComponents                = 64;
    resources.maxGeometryOutputComponents               = 128;
    resources.maxFragmentInputComponents                = 128;
    resources.maxImageUnits                             = 8;
    resources.maxCombinedImageUnitsAndFragmentOutputs   = 8;
    resources.maxCombinedShaderOutputResources          = 8;
    resources.maxImageSamples                           = 0;
    resources.maxVertexImageUniforms                    = 0;
    resources.maxTessControlImageUniforms               = 0;
    resources.maxTessEvaluationImageUniforms            = 0;
    resources.maxGeometryImageUniforms                  = 0;
    resources.maxFragmentImageUniforms                  = 8;
    resources.maxCombinedImageUniforms                  = 8;
    resources.maxGeometryTextureImageUnits              = 16;
    resources.maxGeometryOutputVertices                 = 256;
    resources.maxGeometryTotalOutputComponents          = 1024;
    resources.maxGeometryUniformComponents              = 1024;
    resources.maxGeometryVaryingComponents              = 64;
    resources.maxTessControlInputComponents             = 128;
    resources.maxTessControlOutputComponents            = 128;
    resources.maxTessControlTextureImageUnits           = 16;
    resources.maxTessControlUniformComponents           = 1024;
    resources.maxTessControlTotalOutputComponents       = 4096;
    resources.maxTessEvaluationInputComponents          = 128;
    resources.maxTessEvaluationOutputComponents         = 128;
    resources.maxTessEvaluationTextureImageUnits        = 16;
    resources.maxTessEvaluationUniformComponents        = 1024;
    resources.maxTessPatchComponents                    = 120;
    resources.maxPatchVertices                          = 32;
    resources.maxTessGenLevel                           = 64;
    resources.maxViewports                              = 16;
    resources.maxVertexAtomicCounters                   = 0;
    resources.maxTessControlAtomicCounters              = 0;
    resources.maxTessEvaluationAtomicCounters           = 0;
    resources.maxGeometryAtomicCounters                 = 0;
    resources.maxFragmentAtomicCounters                 = 8;
    resources.maxCombinedAtomicCounters                 = 8;
    resources.maxAtomicCounterBindings                  = 1;
    resources.maxVertexAtomicCounterBuffers             = 0;
    resources.maxTessControlAtomicCounterBuffers        = 0;
    resources.maxTessEvaluationAtomicCounterBuffers     = 0;
    resources.maxGeometryAtomicCounterBuffers           = 0;
    resources.maxFragmentAtomicCounterBuffers           = 1;
    resources.maxCombinedAtomicCounterBuffers           = 1;
    resources.maxAtomicCounterBufferSize                = 16384;
    resources.maxTransformFeedbackBuffers               = 4;
    resources.maxTransformFeedbackInterleavedComponents = 64;
    resources.maxCullDistances                          = 8;
    resources.maxCombinedClipAndCullDistances           = 8;
    resources.maxSamples                                = 4;
    resources.maxMeshOutputVerticesNV                   = 256;
    resources.maxMeshOutputPrimitivesNV                 = 512;
    resources.maxMeshWorkGroupSizeX_NV                  = 32;
    resources.maxMeshWorkGroupSizeY_NV                  = 1;
    resources.maxMeshWorkGroupSizeZ_NV                  = 1;
    resources.maxTaskWorkGroupSizeX_NV                  = 32;
    resources.maxTaskWorkGroupSizeY_NV                  = 1;
    resources.maxTaskWorkGroupSizeZ_NV                  = 1;
    resources.maxMeshViewCountNV                        = 4;

    resources.limits.nonInductiveForLoops                 = true;
    resources.limits.whileLoops                           = true;
    resources.limits.doWhileLoops                         = true;
    resources.limits.generalUniformIndexing               = true;
    resources.limits.generalAttributeMatrixVectorIndexing = true;
    resources.limits.generalVaryingIndexing               = true;
    resources.limits.generalSamplerIndexing               = true;
    resources.limits.generalVariableIndexing              = true;
    resources.limits.generalConstantMatrixVectorIndexing  = true;

    return resources;
}

static TBuiltInResource default_resource;

base::result<bytes> glslang_to_spirv(const string& source, VkShaderStageFlagBits type) {
	static bool initialized = false;
	if (!initialized) {
		[[maybe_unused]] int r = ShInitialize();
		default_resource = create_glslang_resources();
		initialized = true;
	}

	auto stage = EShLangVertex;
	if (type == VK_SHADER_STAGE_GEOMETRY_BIT)
		stage = EShLangGeometry;
	if (type == VK_SHADER_STAGE_FRAGMENT_BIT)
		stage = EShLangFragment;
	if (type == VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT)
		stage = EShLangTessControl;
	if (type == VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT)
		stage = EShLangTessEvaluation;
	if (type == VK_SHADER_STAGE_MESH_BIT_EXT)
		stage = EShLangMesh;
	if (type == VK_SHADER_STAGE_TASK_BIT_EXT)
		stage = EShLangTask;
	if (type == VK_SHADER_STAGE_RAYGEN_BIT_KHR)
		stage = EShLangRayGen;
	if (type == VK_SHADER_STAGE_INTERSECTION_BIT_KHR)
		stage = EShLangIntersect;
	if (type == VK_SHADER_STAGE_ANY_HIT_BIT_KHR)
		stage = EShLangAnyHit;
	if (type == VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)
		stage = EShLangClosestHit;
	if (type == VK_SHADER_STAGE_MISS_BIT_KHR)
		stage = EShLangMiss;
	if (type == VK_SHADER_STAGE_COMPUTE_BIT)
		stage = EShLangCompute;

	glslang::TShader shader(stage);
	const char* s = (char*)source.data;
	int l = source.num;
	shader.setStringsWithLengths(&s, &l, 1);
	shader.setPreamble("#define vulkan 1\n#define gl 0\n");

//	shader.setEnvInput(glslang::EShSourceGlsl, EShLangVertex, glslang::EShClientVulkan, 330);
	shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_3);
	shader.setEnvTarget(glslang::EshTargetSpv, glslang::EShTargetSpv_1_6);

	if (!shader.parse(&default_resource, 330, false, (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		return base::Error{format("compiling shader: %s", shader.getInfoLog())};
	}

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link((EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		return base::Error{format("linking shader: %s", program.getInfoLog())};
	}

	glslang::TIntermediate *intermediate = program.getIntermediate(stage);
	std::vector<uint32_t> spirv;
	glslang::GlslangToSpv(*intermediate, spirv);

	return bytes(&spirv[0], (int)spirv.size() * 4);
}


/*string with_line_numbers(const string& s) {
	auto xx = s.explode("\n");
	string r;
	for (int i=0; i<xx.num; i++)
		r += format("%03d  %s\n", i+1, xx[i]);
	return r;
}*/

namespace vulkan {


	base::result<VkShaderModule> create_shader_module(const bytes &code) {
		if (code.num == 0)
			return nullptr;
		VkShaderModuleCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = code.num;
		info.pCode = reinterpret_cast<const uint32_t*>(code.data);

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(default_device->device, &info, nullptr, &shaderModule) != VK_SUCCESS)
			return base::Error{"failed to create shader module!"};

		return shaderModule;
	}

	//string shader_error;

	base::result<VkShaderModule> create_vk_shader(const string &source, VkShaderStageFlagBits type) {
		if (source.num == 0)
			return nullptr;

		RESULT_PROPAGATE_ERROR(_code, glslang_to_spirv(source, type), x2);
		return create_shader_module(_code);

		/*
		 *
			shader_error = shaderc_result_get_error_message(result);
			shaderc_result_release(result);
			msg_error(shader_error);
			throw Exception("while compiling shader: " + shader_error);
		}
		return nullptr;*/
	}

	base::result<VkShaderStageFlagBits> parse_stage(const string& tag) {
		if (tag == "VertexShader") {
			return VK_SHADER_STAGE_VERTEX_BIT;
		} else if (tag == "FragmentShader") {
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		} else if (tag == "ComputeShader") {
			return VK_SHADER_STAGE_COMPUTE_BIT;
		} else if (tag == "TessellationControlShader") {
			return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		} else if (tag == "TessellationEvaluationShader") {
			return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		} else if (tag == "GeometryShader") {
			return VK_SHADER_STAGE_GEOMETRY_BIT;
		} else if (tag == "RayGenShader") {
			return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		} else if (tag == "RayClosestHitShader") {
			return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		} else if (tag == "RayAnyHitShader") {
			return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		} else if (tag == "RayMissShader") {
			return VK_SHADER_STAGE_MISS_BIT_KHR;
		} else if (tag == "TaskShader") {
			return VK_SHADER_STAGE_TASK_BIT_EXT;
		} else if (tag == "MeshShader") {
			return VK_SHADER_STAGE_MESH_BIT_EXT;
		} else {
			return base::Error("unknown shader tag: '" + tag + "'");
		}
	}

	base::result_void Shader::add_module(const string& _stage, const string &source) {
		RESULT_PROPAGATE_ERROR(stage, parse_stage(_stage), x1);
		RESULT_PROPAGATE_ERROR(mm, create_vk_shader(source, stage), x2);
		modules.add({mm, stage});
		return base::result_success();
	}

	void Shader::set_meta(const string& bindings, int _push_size) {
		push_size = push_size;
		descr_layouts = DescriptorSet::parse_bindings(bindings);
	}



	Shader::Shader() {
		push_size = 0;
	}

	Shader::~Shader() {
		if (verbosity >= 1)
			msg_write("delete shader");
		for (auto &m: modules)
			vkDestroyShaderModule(default_device->device, m.module, nullptr);
		for (auto &l: descr_layouts)
			DescriptorSet::destroy_layout(l);
	}

	VkShaderModule Shader::get_module(VkShaderStageFlagBits stage) const {
		for (auto &m: modules)
			if (m.stage == stage)
				return m.module;
		return VK_NULL_HANDLE;
	}


};

#endif
