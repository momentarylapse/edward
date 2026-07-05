#if HAS_LIB_VULKAN


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

bytes glslang_to_spirv(const string& source, VkShaderStageFlagBits type) {
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
		throw Exception(format("compiling shader: %s", shader.getInfoLog()));
	}

	glslang::TProgram program;
	program.addShader(&shader);
	if (!program.link((EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		throw Exception(format("linking shader: %s", program.getInfoLog()));
	}

	glslang::TIntermediate *intermediate = program.getIntermediate(stage);
	std::vector<uint32_t> spirv;
	glslang::GlslangToSpv(*intermediate, spirv);

	return bytes(&spirv[0], (int)spirv.size() * 4);
}


string with_line_numbers(const string& s) {
	auto xx = s.explode("\n");
	string r;
	for (int i=0; i<xx.num; i++)
		r += format("%03d  %s\n", i+1, xx[i]);
	return r;
}

namespace vulkan {


	VkShaderModule create_shader_module(const bytes &code) {
		if (code.num == 0)
			return nullptr;
		VkShaderModuleCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		info.codeSize = code.num;
		info.pCode = reinterpret_cast<const uint32_t*>(code.data);

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(default_device->device, &info, nullptr, &shaderModule) != VK_SUCCESS)
			throw Exception("failed to create shader module!");

		return shaderModule;
	}

	Path Shader::directory;


	string overwrite_bindings;
	int overwrite_push_size = -1;



	[[maybe_unused]] const int TYPE_LAYOUT = -41;
	[[maybe_unused]] const int TYPE_MODULE = -42;


	string vertex_module_default = "vertex-default-nix";

	string shader_error;

	struct ShaderSourcePart {
		VkShaderStageFlagBits type;
		string source;
	};

	struct ShaderMetaData {
		string version, name, bindings;
		Array<string> extensions;
		int push_size = 0;
	};

	struct ShaderModule {
		ShaderMetaData meta;
		string source;
	};
	static Array<ShaderModule> shader_modules;

	Array<ShaderSourcePart> get_shader_parts(const string &source) {
		Array<ShaderSourcePart> parts;
		bool has_vertex = false;
		bool has_fragment = false;
		int pos = 0;
		while (pos < source.num - 5) {
			int pos0 = source.find("<", pos);
			if (pos0 < 0)
				break;
			pos = pos0 + 1;
			int pos1 = source.find(">", pos0);
			if (pos1 < 0)
				break;

			string tag = source.sub(pos0 + 1, pos1);
			if ((tag.num > 64) or (tag.find("<") >= 0))
				continue;

			int pos2 = source.find("</" + tag + ">", pos1 + 1);
			if (pos2 < 0)
				continue;
			ShaderSourcePart p;
			p.source = source.sub(pos1 + 1, pos2);
			pos = pos2 + tag.num + 3;
			if (tag == "VertexShader") {
				p.type = VK_SHADER_STAGE_VERTEX_BIT;
				has_vertex = true;
			} else if (tag == "FragmentShader") {
				p.type = VK_SHADER_STAGE_FRAGMENT_BIT;
				has_fragment = true;
			} else if (tag == "ComputeShader") {
				p.type = VK_SHADER_STAGE_COMPUTE_BIT;
			} else if (tag == "TessellationControlShader") {
				p.type = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
			} else if (tag == "TessellationEvaluationShader") {
				p.type = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
			} else if (tag == "GeometryShader") {
				p.type = VK_SHADER_STAGE_GEOMETRY_BIT;
			} else if (tag == "RayGenShader") {
				p.type = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
			} else if (tag == "RayClosestHitShader") {
				p.type = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			} else if (tag == "RayAnyHitShader") {
				p.type = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			} else if (tag == "RayMissShader") {
				p.type = VK_SHADER_STAGE_MISS_BIT_KHR;
			} else if (tag == "TaskShader") {
				p.type = VK_SHADER_STAGE_TASK_BIT_EXT;
			} else if (tag == "MeshShader") {
				p.type = VK_SHADER_STAGE_MESH_BIT_EXT;
			} else if (tag == "Module") {
				p.type = (VkShaderStageFlagBits)TYPE_MODULE;
			} else if (tag == "Layout") {
				p.type = (VkShaderStageFlagBits)TYPE_LAYOUT;
			} else {
				msg_error("unknown shader tag: '" + tag + "'");
				continue;
			}
			parts.add(p);
		}
		if (has_fragment and !has_vertex) {
			if (verbosity >= 1)
				msg_write(" ...auto import " + vertex_module_default);
			parts.add({VK_SHADER_STAGE_VERTEX_BIT, format("#import %s\n", vertex_module_default)});
		}
		return parts;
	}

	string get_inside_of_tag(const string &source, const string &tag) {
		string r;
		int pos0 = source.find("<" + tag + ">");
		if (pos0 < 0)
			return "";
		pos0 += tag.num + 2;
		int pos1 = source.find("</" + tag + ">", pos0);
		if (pos1 < 0)
			return "";
		return source.sub(pos0, pos1);
	}

	string expand_shader_source(const string &source, ShaderMetaData &meta) {
		string r = source;
		while (true) {
			int p = r.find("#import", 0);
			if (p < 0)
				break;
			int p2 = r.find("\n", p);
			string imp = r.sub(p + 7, p2).replace(" ", "");
			//msg_error("import '" + imp + "'");

			bool found = false;
			for (auto &m: shader_modules)
				if (m.meta.name == imp) {
					//msg_error("FOUND " + imp);
					r = r.head(p) + "\n// << + " + imp + "\n" + m.source + "\n// >> " + imp + "\n" + r.sub(p2);
					found = true;
					break;
				}
			if (!found)
				throw Exception(format("shader import '%s' not found", imp));
		}

		string intro;
		if (meta.version != "")
			intro += "#version " + meta.version + "\n";
		for (auto &e: meta.extensions)
			intro += "#extension " + e + " : require\n";
		if (r.find("GL_ARB_separate_shader_objects", 0) < 0)
			intro += "#extension GL_ARB_separate_shader_objects : enable\n";
		if (false) {
			msg_write("\n\n======================================");
			msg_write(with_line_numbers(intro + r));
			msg_write("======================================\n\n");
		}
		return intro + r;
	}

	VkShaderModule create_vk_shader(const string &_source, VkShaderStageFlagBits type, ShaderMetaData &meta) {
		string source = expand_shader_source(_source, meta);
		if (source.num == 0)
			return nullptr;

		bytes _code = glslang_to_spirv(source, type);
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

	ShaderMetaData parse_meta(const string& source) {
		ShaderMetaData m;
		for (auto &x: source.explode("\n")) {
			auto y = x.explode("=");
			if (y.num == 2) {
				string k = y[0].trim();
				string v = y[1].trim();
				if (k == "name") {
					m.name = v;
				} else if (k == "version") {
					m.version = v;
				} else if ((k == "binding") or (k == "bindings")) {
					m.bindings = v;
				} else if (k == "pushsize") {
					m.push_size = v._int();
				} else if (k == "input") {
				} else if (k == "topology") {
				} else if (k == "extensions") {
					m.extensions = v.explode(",");
				} else {
					msg_error("unhandled shader meta: " + x);
				}
			}
		}
		if (overwrite_bindings != "")
			m.bindings = overwrite_bindings;
		if (overwrite_push_size >= 0)
			m.push_size = overwrite_push_size;
		return m;
	}

	xfer<Shader> Shader::create(const string &source) {
		auto parts = get_shader_parts(source);

		if (parts.num == 0)
			throw Exception("no shader tags found (<VertexShader>...</VertexShader> or <FragmentShader>...</FragmentShader>)");

		//int prog = create_empty_shader_program();
		auto s = new Shader();

		ShaderMetaData meta;
		for (auto p: parts) {
			if ((int)p.type == TYPE_MODULE) {
				ShaderModule m;
				m.source = p.source;
				m.meta = meta;
				shader_modules.add(m);
				if (verbosity >= 2)
					msg_write("new module '" + m.meta.name + "'");
				return nullptr;
			} else if ((int)p.type == TYPE_LAYOUT) {
				meta = parse_meta(p.source);
			} else {
				auto mm = create_vk_shader(p.source, p.type, meta);
				if (mm)
					s->modules.add({mm, p.type});
			}
		}

		s->push_size = meta.push_size;
		s->descr_layouts = DescriptorSet::parse_bindings(meta.bindings);
		return s;
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


	xfer<Shader> Shader::load(const Path &_filename) {
		if (!_filename)
			return nullptr;
		Path filename = directory | _filename;
		if (verbosity >= 1)
			msg_write(format("load shader %s", filename));

		if (!os::fs::exists(filename.with(".compiled")))
			return create(os::fs::read_text(filename));

		Shader *s = new Shader();

		auto f = ownify(os::fs::open(filename.with(".compiled"), "rb"));
		try {
			while(true) {
				string tag = f->read_str();
				string value = f->read_str();
				//std::cout << tag << "\n";
				if (tag == "Topology") {
				} else if (tag == "Bindings") {
					s->descr_layouts = DescriptorSet::parse_bindings(value);
				} else if (tag == "PushSize") {
					s->push_size = value._int();
				} else if (tag == "Input") {
				} else if (tag == "Info") {
				} else if (tag == "VertexShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_VERTEX_BIT});
				} else if (tag == "GeometryShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_GEOMETRY_BIT});
				} else if (tag == "FragmentShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_FRAGMENT_BIT});
				} else if (tag == "ComputeShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_COMPUTE_BIT});
				} else if (tag == "RayGenShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_RAYGEN_BIT_KHR});
				} else if (tag == "RayMissShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_MISS_BIT_KHR});
				} else if (tag == "RayClosestHitShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR});
				} else if (tag == "RayAnyHitShader") {
					s->modules.add({create_shader_module(value), VK_SHADER_STAGE_ANY_HIT_BIT_KHR});
				} else {
					msg_write("WARNING: " + value);
				}
			}
		} catch(...) {
		}

		return s;
	}

	VkShaderModule Shader::get_module(VkShaderStageFlagBits stage) const {
		for (auto &m: modules)
			if (m.stage == stage)
				return m.module;
		return VK_NULL_HANDLE;
	}


};

#endif
