/*
 * GuiRendererVulkan.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "GuiRenderer.h"
#ifdef USING_VULKAN
#include "../../gui/gui.h"
#include "../../gui/Picture.h"
#include "../../gui/Canvas.h"
#include "../../helper/ResourceManager.h"
#include <lib/yrenderer/Context.h>
#include <lib/yrenderer/helper/PipelineManager.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/Painter.h>
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/helper/Bindable.h>
#include <lib/profiler/Profiler.h>
#include <lib/math/mat4.h>
#include <lib/math/rect.h>

using namespace yrenderer;


struct UBOGUI {
	mat4 m,v,p;
	color col;
	rect source;
	float blur, exposure, gamma;
};


GuiRenderer::GuiRenderer(Context* ctx) : Renderer(ctx, "ui") {

	shader = shader_manager->load_shader("vulkan/2d.shader");

	vb = new ygfx::VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID);
}

GuiRenderer::~GuiRenderer() = default;

void GuiRenderer::prepare(const RenderParams &params) {
	if (!aux) {
		aux = ctx->context->_create_auxiliary_stuff();
		aux->rebuild(params.render_pass);
	}
	aux->reset_frame();
}


void GuiRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);
	prepare_gui(params.frame_buffer, params);
	draw_gui(params.command_buffer, params);


//	aux->cb = params.command_buffer;
//	auto p = new ygfx::Painter(aux.get(), params.area, rect(0,params.desired_aspect_ratio, 0, 1), params.area.height(), nullptr);

	/*p->set_color(Red);
	p->draw_rect({0.1f, 0.5f, 0.1f, 0.5f});

	p->set_color(White);
	p->set_line_width(0.02f);
	p->set_fill(false);
	p->draw_circle({0.4f, 0.3f}, 0.2f);*/

//	delete p;


	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

void GuiRenderer::prepare_gui(ygfx::FrameBuffer *source, const RenderParams& params) {
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
			if (!p->texture)
				continue;

			if (index >= ubo.num) {
				dset.add(ctx->pool->create_set("sampler,sampler,buffer"));
				ubo.add(new ygfx::UniformBuffer(sizeof(UBOGUI)));
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

			dset[index]->set_texture(0, p->texture.get());
//			dset[index]->set_texture(1, source->...);
			dset[index]->set_uniform_buffer(2, ubo[index]);
			dset[index]->update();
			index ++;
		}
	}
}

void GuiRenderer::draw_gui(ygfx::CommandBuffer *cb, const RenderParams& params) {
	if (!pipeline)
		pipeline = PipelineManager::get_gui(shader.get(), params.render_pass, "3f,3f,2f");

	cb->bind_pipeline(pipeline);

	aux->cb = cb;
	ygfx::Painter painter(aux.get(), params.area, rect(0,params.desired_aspect_ratio, 0, 1), params.area.height(), nullptr);


	int index = 0;
	for (auto *n: gui::sorted_nodes) {
		if (!n->eff_visible)
			continue;
		if (n->type == gui::Node::Type::PICTURE or n->type == gui::Node::Type::TEXT) {
			auto *p = static_cast<gui::Picture*>(n);
			if (!p->texture)
				continue;
			if (p->shader) {
				auto pl = PipelineManager::get_gui(p->shader.get(), params.render_pass, "3f,3f,2f");
				cb->bind_pipeline(pl);
				apply_shader_data(params, p->shader.get(), p->shader_data);
			}

			cb->bind_descriptor_set(0, dset[index]);
			cb->draw(vb.get());

			if (p->shader)
				cb->bind_pipeline(pipeline);
			index ++;
		} else if (n->type == gui::Node::Type::CANVAS) {
			auto *c = static_cast<gui::Canvas*>(n);
			float fx = params.desired_aspect_ratio;
			painter._area = {c->eff_area.x1 * fx, c->eff_area.x2 * fx, c->eff_area.y1, c->eff_area.y2};
			c->on_draw(&painter);

			cb->bind_pipeline(pipeline);
		}
	}
}

#endif
