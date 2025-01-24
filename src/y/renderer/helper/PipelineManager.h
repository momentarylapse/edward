/*
 * PipelineManager.h
 *
 *  Created on: 15 Dec 2021
 *      Author: michi
 */

#pragma once
#include "../../graphics-fwd.h"
#ifdef USING_VULKAN

class string;

namespace PipelineManager {

GraphicsPipeline *get(Shader *s, RenderPass *rp, PrimitiveTopology top, VertexBuffer *vb, vulkan::CullMode culling, bool test_z, bool write_z);
GraphicsPipeline *get_alpha(Shader *s, RenderPass *rp, PrimitiveTopology top, VertexBuffer *vb, Alpha src, Alpha dst, vulkan::CullMode culling, bool test_z, bool write_z);
GraphicsPipeline *get_gui(Shader *s, RenderPass *rp, const string &format);

void clear();

}

#endif
