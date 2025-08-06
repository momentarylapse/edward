/*
 * PipelineManager.h
 *
 *  Created on: 15 Dec 2021
 *      Author: michi
 */

#pragma once
#include <lib/ygraphics/graphics-fwd.h>
#ifdef USING_VULKAN

struct string;

namespace yrenderer::PipelineManager {

ygfx::GraphicsPipeline *get(
		ygfx::Shader *s,
		ygfx::RenderPass *rp,
		ygfx::PrimitiveTopology top,
		ygfx::VertexBuffer *vb,
		vulkan::CullMode culling,
		bool test_z, bool write_z);
ygfx::GraphicsPipeline *get_alpha(
		ygfx::Shader *s,
		ygfx::RenderPass *rp,
		ygfx::PrimitiveTopology top,
		ygfx::VertexBuffer *vb,
		ygfx::Alpha src, ygfx::Alpha dst,
		vulkan::CullMode culling,
		bool test_z, bool write_z);
ygfx::GraphicsPipeline *get_gui(
		ygfx::Shader *s,
		ygfx::RenderPass *rp,
		const string &format);

void clear();

}

#endif
