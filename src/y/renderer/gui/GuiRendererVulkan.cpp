/*
 * GuiRendererVulkan.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "GuiRendererVulkan.h"
#ifdef USING_VULKAN
#include "../base.h"
#include "../helper/PipelineManager.h"
#include "../../graphics-impl.h"
#include "../../gui/gui.h"
#include "../../gui/Picture.h"
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include <lib/math/mat4.h>
#include <lib/math/rect.h>



struct UBOGUI {
	mat4 m,v,p;
	color col;
	rect source;
	float blur, exposure, gamma;
};


GuiRendererVulkan::GuiRendererVulkan() : Renderer("ui") {

	shader = resource_manager->load_shader("vulkan/2d.shader");

	vb = new VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID);
}

void GuiRendererVulkan::draw(const RenderParams& params) {
	PerformanceMonitor::begin(ch_draw);
	gpu_timestamp_begin(params.command_buffer, ch_draw);
	prepare_gui(params.frame_buffer, params);
	draw_gui(params.command_buffer, params.render_pass);
	gpu_timestamp_end(params.command_buffer, ch_draw);
	PerformanceMonitor::end(ch_draw);
}

void GuiRendererVulkan::prepare_gui(FrameBuffer *source, const RenderParams& params) {
	gui::update();

	UBOGUI u;
	u.v = mat4::ID;
	u.p = mat4::scale(2.0f, 2.0f, 1) * mat4::translation(vec3(-0.5f, -0.5f, 0)); // nix::set_projection_ortho_relative()
	u.gamma = 2.2f;
	u.exposure = 1.0f;

	int index = 0;

	for (auto *n: gui::sorted_nodes) {
		if (!n->eff_visible)
			continue;
		if (n->type == gui::Node::Type::PICTURE or n->type == gui::Node::Type::TEXT) {
			auto *p = (gui::Picture*)n;

			if (index >= ubo.num) {
				dset.add(pool->create_set("buffer,sampler"));
				ubo.add(new UniformBuffer(sizeof(UBOGUI)));
			}

			if (p->angle == 0) {
				u.m = mat4::translation(vec3(p->eff_area.x1, p->eff_area.y1, /*0.999f - p->eff_z/1000*/ 0.5f)) * mat4::scale(p->eff_area.width(), p->eff_area.height(), 0);
			} else {
				// TODO this should use the physical ratio
				float r = params.desired_aspect_ratio;
				u.m = mat4::translation(vec3(p->eff_area.x1, p->eff_area.y1, /*0.999f - p->eff_z/1000*/ 0.5f)) * mat4::scale(1/r, 1, 0) * mat4::rotation_z(p->angle) * mat4::scale(p->eff_area.width() * r, p->eff_area.height(), 0);
			}
			u.blur = p->bg_blur;
			u.col = p->eff_col;
			u.source = p->source;
			ubo[index]->update(&u);

			dset[index]->set_uniform_buffer(0, ubo[index]);
			dset[index]->set_texture(1, p->texture.get());
//			dset[index]->set_texture(2, source->...);
			dset[index]->update();
			index ++;
		}
	}
}

void GuiRendererVulkan::draw_gui(CommandBuffer *cb, RenderPass *render_pass) {
	if (!pipeline)
		pipeline = PipelineManager::get_gui(shader.get(), render_pass, "3f,3f,2f");

	cb->bind_pipeline(pipeline);

	int index = 0;
	for (auto *n: gui::sorted_nodes) {
		if (!n->eff_visible)
			continue;
		if (n->type == gui::Node::Type::PICTURE or n->type == gui::Node::Type::TEXT) {
			auto *p = (gui::Picture*)n;
			if (p->shader) {
				auto pl = PipelineManager::get_gui(p->shader.get(), render_pass, "3f,3f,2f");
				cb->bind_pipeline(pl);
			}

			cb->bind_descriptor_set(0, dset[index]);
			cb->draw(vb.get());

			if (p->shader)
				cb->bind_pipeline(pipeline);
			index ++;
		}
	}
}

#endif
