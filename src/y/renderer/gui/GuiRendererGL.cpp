/*
 * GuiRendererGL.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "GuiRendererGL.h"
#ifdef USING_OPENGL
#include "../base.h"
#include <graphics-impl.h>
#include <lib/os/msg.h>
#include "../../gui/gui.h"
#include "../../gui/Picture.h"
#include "../../helper/PerformanceMonitor.h"
#include "../../helper/ResourceManager.h"
#include <y/EngineData.h>

GuiRendererGL::GuiRendererGL() : Renderer("gui") {
	shader = resource_manager->load_shader("forward/2d.shader");

	vb = new VertexBuffer("3f,3f,2f");
	vb->create_quad(rect::ID);
}

void GuiRendererGL::draw(const RenderParams& params) {
	draw_gui(nullptr);
}

void apply_shader_data(Shader *s, const Any &shader_data) {
	if (shader_data.is_empty()) {
		return;
	} else if (shader_data.is_dict()) {
		for (auto &key: shader_data.keys()) {
			auto &val = shader_data[key];
			if (val.is_float()) {
				s->set_float(key, val.as_float());
			} else if (val.is_int()) {
				s->set_float(key, val.as_int());
			} else if (val.is_bool()) {
				s->set_int(key, (int)val.as_bool());
			} else if (val.is_list()) {
				float ff[4];
				for (int i=0; i<val.as_list().num; i++)
					ff[i] = val.as_list()[i].as_float();
				s->set_floats(key, ff, val.as_list().num);
			} else {
				msg_write("invalid shader data item: " + val.str());
			}
		}
	} else {
		msg_write("invalid shader data: " + shader_data.str());
	}
}

void GuiRendererGL::draw_gui(FrameBuffer *source) {
	PerformanceMonitor::begin(ch_draw);
	gpu_timestamp_begin(ch_draw);
	gui::update();

	nix::set_projection_ortho_relative();
	nix::set_view_matrix(mat4::ID);
	nix::set_cull(nix::CullMode::NONE);
	nix::set_alpha(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA);
	nix::set_z(false, false);

	for (auto *n: gui::sorted_nodes) {
		if (!n->eff_visible)
			continue;
		if (n->type == gui::Node::Type::PICTURE or n->type == gui::Node::Type::TEXT) {
			auto *p = (gui::Picture*)n;
			auto s = shader.get();
			if (p->shader) {
				s = p->shader.get();
				apply_shader_data(s, p->shader_data);
			}
			nix::set_shader(s);
			s->set_float("blur", p->bg_blur);
			s->set_color("color", p->eff_col);
			nix::set_textures({p->texture.get()});// , source->color_attachments[0].get()});
			if (p->angle == 0) {
				nix::set_model_matrix(mat4::translation(vec3(p->eff_area.x1, p->eff_area.y1, /*0.999f - p->eff_z/1000*/ 0.5f)) * mat4::scale(p->eff_area.width(), p->eff_area.height(), 0));
			} else {
				float r = engine.physical_aspect_ratio;
				nix::set_model_matrix(mat4::translation(vec3(p->eff_area.x1, p->eff_area.y1, /*0.999f - p->eff_z/1000*/ 0.5f)) * mat4::scale(1/r, 1, 0) * mat4::rotation_z(p->angle) * mat4::scale(p->eff_area.width() * r, p->eff_area.height(), 0));
			}
			vb->create_quad(rect::ID, p->source);
			nix::draw_triangles(vb.get());
		}
	}
	nix::set_z(true, true);
	nix::set_cull(nix::CullMode::BACK);

	nix::disable_alpha();

	gpu_timestamp_end(ch_draw);
	PerformanceMonitor::end(ch_draw);
}


#endif
