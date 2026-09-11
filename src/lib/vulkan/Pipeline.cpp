//
//  Pipeline.cpp
//  3
//
//  Created by <author> on 06/02/2019.
//
//

#if HAS_LIB_VULKAN


#include "Pipeline.h"
#include "vulkan.h"
#include "common.h"
#include "Shader.h"
#include "helper.h"
#include "RenderPass.h"
#include "VertexBuffer.h"
#include "../base/sort.h"
#include "../os/msg.h"
#include "../math/rect.h"

namespace vulkan {

PrimitiveTopology parse_topology(const string &t) {
	if (t == "points")
		return PrimitiveTopology::POINTS;
	if (t == "lines")
		return PrimitiveTopology::LINES;
	if (t == "line-strip")
		return PrimitiveTopology::LINE_STRIP;
	if (t == "triangles")
		return PrimitiveTopology::TRIANGLES;
	if (t == "triangle-fan")
		return PrimitiveTopology::TRIANGLE_FAN;
	if (t == "patch-list")
		return PrimitiveTopology::PATCHES;
	msg_error("invalid topology: " + t);
	return PrimitiveTopology::TRIANGLES;
}


Array<VkPipelineShaderStageCreateInfo> create_shader_stages(Shader *shader) {
	Array<VkPipelineShaderStageCreateInfo> shader_stages;
	for (auto &m: shader->modules) {
		VkPipelineShaderStageCreateInfo shader_stage_info = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			.stage = m.stage,
			.module = m.module,
			.pName = "main"
		};
		shader_stages.add(shader_stage_info);
	}
	shader_stages = base::sorted(shader_stages, [](const auto& a, const auto& b) {
		return (int)a.stage <= (int)b.stage;
	});
	return shader_stages;
}

VkPipelineLayout create_pipeline_layout(int push_size, const Array<VkDescriptorSetLayout> &dset_layouts) {
	VkPipelineLayoutCreateInfo info = {};
	VkPushConstantRange pci = {VK_SHADER_STAGE_ALL, 0, (uint32_t)push_size};
	info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	if (push_size > 0) {
		pci.stageFlags = VK_SHADER_STAGE_ALL; //VK_SHADER_STAGE_VERTEX_BIT /*| VK_SHADER_STAGE_GEOMETRY_BIT*/ | VK_SHADER_STAGE_FRAGMENT_BIT;
		pci.offset = 0;
		pci.size = push_size;
		info.pushConstantRangeCount = 1;
		info.pPushConstantRanges = &pci;
	}
	info.setLayoutCount = dset_layouts.num;
	info.pSetLayouts = &dset_layouts[0];
	if (verbosity >= 2)
		msg_write(format("create pipeline with %d layouts, %d push size", dset_layouts.num, push_size));

	VkPipelineLayout layout = VK_NULL_HANDLE;
	if (vkCreatePipelineLayout(default_device->device, &info, nullptr, &layout) != VK_SUCCESS)
		throw Exception("failed to create pipeline layout!");
	return layout;
}

BasePipeline::BasePipeline(VkPipelineBindPoint bp, Shader *s) {
	bind_point = bp;
	shader = s;
	descr_layouts = shader->descr_layouts;

	shader_stages = create_shader_stages(shader);

	layout = create_pipeline_layout(shader->push_size, descr_layouts);
}

BasePipeline::BasePipeline(VkPipelineBindPoint bp, const Array<VkDescriptorSetLayout> &dset_layouts) {
	bind_point = bp;
	descr_layouts = dset_layouts;
	layout = create_pipeline_layout(0, dset_layouts);
}

BasePipeline::~BasePipeline() {
	destroy();
	if (layout)
		vkDestroyPipelineLayout(default_device->device, layout, nullptr);
}

void BasePipeline::destroy() {
	if (pipeline)
		vkDestroyPipeline(default_device->device, pipeline, nullptr);
	pipeline = nullptr;
}

Array<VkVertexInputAttributeDescription> parse_attr_descr(const string &format);
VkVertexInputBindingDescription parse_binding_descr(const string &format);

GraphicsPipeline::GraphicsPipeline(Shader *_shader, RenderPass *_render_pass, int _subpass, PrimitiveTopology topology, VertexBuffer *vb) : GraphicsPipeline(_shader, _render_pass, _subpass, topology, vb->binding_description, vb->attribute_descriptions) {}

GraphicsPipeline::GraphicsPipeline(Shader *_shader, RenderPass *_render_pass, int _subpass, PrimitiveTopology topology, const string &format) : GraphicsPipeline(_shader, _render_pass, _subpass, topology, parse_binding_descr(format), parse_attr_descr(format)) {}

GraphicsPipeline::GraphicsPipeline(Shader *_shader, RenderPass *_render_pass, int _subpass, PrimitiveTopology topology, VkVertexInputBindingDescription _binding_description, const Array<VkVertexInputAttributeDescription> &_attribute_descriptions) : BasePipeline(VK_PIPELINE_BIND_POINT_GRAPHICS, _shader) {
	render_pass = _render_pass;
	subpass = _subpass;

	binding_description = _binding_description;
	attribute_descriptions = _attribute_descriptions;
	vertex_input_info = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		.vertexBindingDescriptionCount = 1,
		.pVertexBindingDescriptions = &binding_description,
		.vertexAttributeDescriptionCount = (unsigned)attribute_descriptions.num,
		.pVertexAttributeDescriptions = &attribute_descriptions[0]
	};

	input_assembly = {
	.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
	.topology = (VkPrimitiveTopology)topology,
#ifdef OS_MAC
	.primitiveRestartEnable = VK_TRUE // molten vk/metal seem to lack the feature of "turning this off" (O_O)'
#else
	.primitiveRestartEnable = VK_FALSE
#endif
	};

	for (int i=0; i<render_pass->num_color_attachments(subpass); i++) {
		VkPipelineColorBlendAttachmentState a = {
			.blendEnable = VK_FALSE,
			.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
		};
		color_blend_attachments.add(a);
	}

	color_blending = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = (unsigned)color_blend_attachments.num,
		.pAttachments = &color_blend_attachments[0],
		.blendConstants = {0.0f, 0.0f, 0.0f, 0.0f}
	};


	rasterizer = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.depthClampEnable = VK_FALSE,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		//.polygonMode = VK_POLYGON_MODE_LINE,
		//.cullMode = VK_CULL_MODE_NONE,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1
	};

	multisampling = {};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	if (render_pass->samples > VK_SAMPLE_COUNT_1_BIT) {
		multisampling.sampleShadingEnable = VK_TRUE;
		multisampling.rasterizationSamples = render_pass->samples;
		multisampling.minSampleShading = 0.2f;
	} else {
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	}

	depth_stencil = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_TRUE,
		.depthWriteEnable = VK_TRUE,
		.depthCompareOp = VK_COMPARE_OP_LESS,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE
	};

	tessellation = {};
	if (input_assembly.topology == VK_PRIMITIVE_TOPOLOGY_PATCH_LIST) {
		// TODO expose
		tessellation.patchControlPoints = 4;
	}

	dynamic_states.add(VK_DYNAMIC_STATE_VIEWPORT);
	set_viewport(rect(0, 400, 0, 400)); // always override dynamically!

	rebuild();
}

GraphicsPipeline::~GraphicsPipeline() {
	destroy();
}



void GraphicsPipeline::disable_blend() {
	color_blend_attachments[0].blendEnable = VK_FALSE;
}

void GraphicsPipeline::set_blend(Alpha src, Alpha dst) {
	color_blend_attachments[0].blendEnable = VK_TRUE;
	color_blend_attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachments[0].srcColorBlendFactor = (VkBlendFactor)src;
	color_blend_attachments[0].dstColorBlendFactor = (VkBlendFactor)dst;
	color_blend_attachments[0].alphaBlendOp = VK_BLEND_OP_MAX;
	color_blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
}

void GraphicsPipeline::set_blend(float alpha) {
	color_blend_attachments[0].blendEnable = VK_TRUE;
	color_blend_attachments[0].colorBlendOp = VK_BLEND_OP_ADD;
	color_blend_attachments[0].srcColorBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
	color_blend_attachments[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
	color_blending.blendConstants[0] = alpha;
	color_blending.blendConstants[1] = alpha;
	color_blending.blendConstants[2] = alpha;
	color_blending.blendConstants[3] = alpha;
	color_blend_attachments[0].alphaBlendOp = VK_BLEND_OP_MAX;
	color_blend_attachments[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
	color_blend_attachments[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
}

void GraphicsPipeline::set_line_width(float line_width) {
	rasterizer.lineWidth = line_width;
}

void GraphicsPipeline::set_depth_bias(bool enabled, float constant_factor, float clamp, float slope_factor) {
	rasterizer.depthBiasEnable = enabled;
	rasterizer.depthBiasConstantFactor = constant_factor;
	rasterizer.depthBiasClamp = clamp;
	rasterizer.depthBiasSlopeFactor = slope_factor;
}

void GraphicsPipeline::set_wireframe(bool wireframe) {
	if (wireframe) {
		rasterizer.polygonMode = VK_POLYGON_MODE_LINE;
	} else {
		rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	}
}

void GraphicsPipeline::set_z(bool test, bool write) {
	depth_stencil.depthTestEnable = test ? VK_TRUE : VK_FALSE;
	depth_stencil.depthWriteEnable = write ? VK_TRUE : VK_FALSE;

	if (test and !write)
		depth_stencil.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
}

void GraphicsPipeline::set_viewport(const rect &r) {
	viewport = {
		.x = r.x1,
		.y = r.y1,
		.width = r.width(),
		.height = r.height(),
		.minDepth = 0.0f,
		.maxDepth = 1.0f
	};
}

void GraphicsPipeline::set_culling(CullMode mode) {
	rasterizer.cullMode = (VkCullModeFlags)mode;
}

VkDynamicState parse_dynamic_state(const string &d) {
	if (d == "viewport")
		return VK_DYNAMIC_STATE_VIEWPORT;
	if (d == "scissor")
		return VK_DYNAMIC_STATE_SCISSOR;
	if (d == "linewidth")
		return VK_DYNAMIC_STATE_LINE_WIDTH;
	if (d == "depthbias")
		return VK_DYNAMIC_STATE_DEPTH_BIAS;
	msg_error("unknown dynamic state: " + d);
	return VK_DYNAMIC_STATE_MAX_ENUM;
}

void GraphicsPipeline::set_dynamic(const Array<string> &_dynamic_states) {
	for (string &d: _dynamic_states) {
		auto ds = parse_dynamic_state(d);
		if (ds != VK_DYNAMIC_STATE_VIEWPORT)
			dynamic_states.add(ds);
	}
}

void GraphicsPipeline::rebuild() {
	destroy();

	// sometimes a dummy scissor is required!
	VkRect2D scissor = {
		.offset = {0, 0},
		.extent = {1000000u, 1000000u}
	};
	//scissor.extent = {(unsigned)width, (unsigned)height};

	VkPipelineViewportStateCreateInfo viewport_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.pViewports = &viewport,
		.scissorCount = 1,
		.pScissors = &scissor
	};



	VkPipelineDynamicStateCreateInfo dynamic_state = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = (unsigned)dynamic_states.num,
		.pDynamicStates = &dynamic_states[0]
	};

	VkGraphicsPipelineCreateInfo pipeline_info = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = (unsigned)shader_stages.num,
		.pStages = &shader_stages[0],
		.pVertexInputState = &vertex_input_info,
		.pInputAssemblyState = &input_assembly,
		.pTessellationState = &tessellation,
		.pViewportState = &viewport_state,
		.pRasterizationState = &rasterizer,
		.pMultisampleState = &multisampling,
		.pDepthStencilState = &depth_stencil,
		.pColorBlendState = &color_blending,
		.pDynamicState = &dynamic_state,
		.layout = layout,
		.renderPass = render_pass->render_pass,
		.subpass = (unsigned)subpass,
		.basePipelineHandle = VK_NULL_HANDLE,
	};

	if (vkCreateGraphicsPipelines(default_device->device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS)
		throw Exception("failed to create graphics pipeline!");
}



ComputePipeline::ComputePipeline(Shader *shader) : BasePipeline(VK_PIPELINE_BIND_POINT_COMPUTE, shader) {
	if (verbosity >= 2)
		msg_write("creating compute pipeline...");
	
	VkComputePipelineCreateInfo info = {
		.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		.stage = shader_stages[0],
		.layout = layout
	};

	if (vkCreateComputePipelines(default_device->device, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline) != VK_SUCCESS)
		throw Exception("failed to create compute pipeline!");
}



RayPipeline::RayPipeline(const string &dset_layouts, const Array<Shader*> &shaders, int recursion_depth) :
		BasePipeline(VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, DescriptorSet::parse_bindings(dset_layouts)),
		sbt(default_device)
{
	if (verbosity >= 2)
		msg_write("creating RTX pipeline...");

	create_groups(shaders);


	VkRayTracingPipelineCreateInfoKHR info = {
		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		.stageCount = (unsigned)shader_stages.num,
		.pStages = &shader_stages[0],
		.groupCount = (unsigned)groups.num,
		.pGroups = &groups[0],
		.maxPipelineRayRecursionDepth = (unsigned)recursion_depth,
		.layout = layout,
		.basePipelineHandle = VK_NULL_HANDLE,
		.basePipelineIndex = 0
	};

	auto result = _vkCreateRayTracingPipelinesKHR(default_device->device, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &info, nullptr, &pipeline);
	if (result != VK_SUCCESS) {
		throw Exception("failed to create ray pipeline! " + i2s(result));
	}
	if (verbosity >= 2)
		msg_write("...done");
}

void RayPipeline::create_groups(const Array<Shader*> &shaders) {

	VkPipelineShaderStageCreateInfo stage = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.pName = "main"
	};

	// ray gen
	stage.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	stage.module = shaders[0]->get_module(VK_SHADER_STAGE_RAYGEN_BIT_KHR);
	shader_stages.add(stage);

	VkRayTracingShaderGroupCreateInfoKHR group_info = {
		.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		.closestHitShader = VK_SHADER_UNUSED_KHR,
		.anyHitShader = VK_SHADER_UNUSED_KHR,
		.intersectionShader = VK_SHADER_UNUSED_KHR
	};
	groups.add(group_info);

	// hit groups
	for (auto *s: shaders.sub_ref(1)) {
		group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
		group_info.generalShader = VK_SHADER_UNUSED_KHR;
		group_info.closestHitShader = VK_SHADER_UNUSED_KHR;
		group_info.anyHitShader = VK_SHADER_UNUSED_KHR;
		group_info.intersectionShader = VK_SHADER_UNUSED_KHR;

		Array<VkPipelineShaderStageCreateInfo> stages;
		if (s->get_module(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR)) {
			group_info.closestHitShader = shader_stages.num + stages.num;
			stage.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
			stage.module = s->get_module(VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
			stages.add(stage);
		}
		if (s->get_module(VK_SHADER_STAGE_ANY_HIT_BIT_KHR)) {
			group_info.anyHitShader = shader_stages.num + stages.num;
			stage.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
			stage.module = s->get_module(VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
			stages.add(stage);
		}
		//VK_SHADER_STAGE_INTERSECTION_BIT_KHR
		shader_stages.append(stages);
		groups.add(group_info);
	}
	miss_group_offset = groups.num;

	// miss groups
	for (auto *s: shaders.sub_ref(1))
		if (s->get_module(VK_SHADER_STAGE_MISS_BIT_KHR)) {
			group_info.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
			group_info.generalShader = shader_stages.num;
			group_info.closestHitShader = VK_SHADER_UNUSED_KHR;
			group_info.anyHitShader = VK_SHADER_UNUSED_KHR;
			group_info.intersectionShader = VK_SHADER_UNUSED_KHR;

			stage.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
			stage.module = s->get_module(VK_SHADER_STAGE_MISS_BIT_KHR);
			shader_stages.add(stage);
			groups.add(group_info);
		}
}

void RayPipeline::create_sbt() {
	if (verbosity >= 2)
		msg_write("SBT");
	const size_t sbt_size = groups.num * default_device->ray_tracing_properties.shaderGroupHandleSize;

	sbt.create(sbt_size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

	void* mem = sbt.map();
	if (_vkGetRayTracingShaderGroupHandlesKHR(default_device->device, pipeline, 0, groups.num, sbt_size, mem))
		throw Exception("vkGetRayTracingShaderGroupHandlesKHR");
	sbt.unmap();
	if (verbosity >= 2)
		msg_write("   ok");
}

};

#endif

