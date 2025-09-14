/*
 * GuiRendererGL.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "GuiRenderer.h"
#ifdef USING_OPENGL
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/Painter.h>
#include <lib/os/msg.h>
#include "../../gui/gui.h"
#include "../../gui/Picture.h"
#include <lib/profiler/Profiler.h>
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/helper/Bindable.h>
#include "../../helper/ResourceManager.h"
#include <y/EngineData.h>


using namespace yrenderer;
using namespace ygfx;

GuiRenderer::GuiRenderer(yrenderer::Context* ctx) : Renderer(ctx, "gui") {
	shader = shader_manager->load_shader("forward/2d.shader");

	vb = new VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID);
}

GuiRenderer::~GuiRenderer() = default;


void GuiRenderer::prepare(const RenderParams &params) {
	if (!aux) {
		aux = ctx->context->_create_auxiliary_stuff();
	}
	aux->reset_frame();
}

void GuiRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);
	gui::update();

	//nix::set_projection_ortho_relative();
	auto mt = mat4::translation(vec3(-0.5f, -0.5f, 0));
	auto ms = mat4::scale(2.0f, -2.0f, 1);
	shader->set_matrix_l(shader->location[Shader::LOCATION_MATRIX_P], ms * mt);
	shader->set_matrix_l(shader->location[Shader::LOCATION_MATRIX_V], mat4::ID);
	nix::set_cull(nix::CullMode::NONE);
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_z(false, false);

	for (auto *n: gui::sorted_nodes) {
		if (!n->eff_visible)
			continue;
		if (n->type == gui::Node::Type::PICTURE or n->type == gui::Node::Type::TEXT) {
			auto *p = (gui::Picture*)n;
			if (!p->texture)
				continue;
			auto s = shader.get();
			if (p->shader) {
				s = p->shader.get();
				apply_shader_data(params, s, p->shader_data);
			}
			nix::set_shader(s);
			s->set_float("blur", p->bg_blur);
			s->set_color("color", p->eff_col);
			nix::bind_textures({p->texture.get()});// , source->color_attachments[0].get()});
			if (p->angle == 0) {
				s->set_matrix_l(s->location[Shader::LOCATION_MATRIX_M], mat4::translation(vec3(p->eff_area.x1, p->eff_area.y1, /*0.999f - p->eff_z/1000*/ 0.5f)) * mat4::scale(p->eff_area.width(), p->eff_area.height(), 0));
			} else {
				float r = engine.physical_aspect_ratio;
				s->set_matrix_l(s->location[Shader::LOCATION_MATRIX_M], mat4::translation(vec3(p->eff_area.x1, p->eff_area.y1, /*0.999f - p->eff_z/1000*/ 0.5f)) * mat4::scale(1/r, 1, 0) * mat4::rotation_z(p->angle) * mat4::scale(p->eff_area.width() * r, p->eff_area.height(), 0));
			}
			vb->create_quad(rect::ID, p->source);
			nix::draw_triangles(vb.get());
		}
	}
	nix::set_z(true, true);
	nix::set_cull(nix::CullMode::BACK);

	nix::disable_alpha();

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}


#endif
