/*
 * GuiRenderer.cpp
 *
 *  Created on: 23 Nov 2021
 *      Author: michi
 */

#include "GuiRenderer.h"
#include <gui/gui.h>
#include <gui/Picture.h>
#include <gui/Canvas.h>
#include <gui/Font.h>
#include <gui/Label.h>
#include <lib/yrenderer/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/Painter.h>
#include <lib/ygraphics/TextCache.h>
#include <lib/profiler/Profiler.h>
#include <lib/math/rect.h>
#include "lib/os/msg.h"


using namespace yrenderer;


GuiRenderer::GuiRenderer(Context* ctx) : Renderer(ctx, "ui") {
}

GuiRenderer::~GuiRenderer() = default;

void GuiRenderer::prepare(const RenderParams &params) {
	if (!aux) {
		aux = ctx->context->_create_auxiliary_stuff();
#ifdef USING_VULKAN
		aux->rebuild(params.render_pass);
#endif
	}
	aux->reset_frame();

	if (!text_cache)
		text_cache = new ygfx::TextCache(aux.get());
	text_cache->iterate();
	gui::text_cache = text_cache.get();

	gui::ui_scale = params.area.height();

	gui::update(params.desired_aspect_ratio);
}


void GuiRenderer::draw(const RenderParams& params) {
	profiler::begin(channel);
	ctx->gpu_timestamp_begin(params, channel);

#ifdef USING_VULKAN
	aux->cb = params.command_buffer;
#endif
#ifdef USING_OPENGL
	nix::set_front(nix::Orientation::CCW);
#endif
	ygfx::Painter painter(aux.get(), gui::font_manager, text_cache.get(), params.area, rect(0,params.desired_aspect_ratio, 0, 1));

	draw_node(painter, gui::toplevel.get(), White);

#ifdef USING_OPENGL
	nix::set_front(nix::Orientation::CW);
#endif

	ctx->gpu_timestamp_end(params, channel);
	profiler::end(channel);
}

void GuiRenderer::draw_node(ygfx::Painter& painter, gui::Node* n, const color& group_col) {
	color col = group_col;

	if (n->type == gui::Node::Type::LABEL) {
		auto t = static_cast<gui::Label*>(n);
		//painter.set_color(color(0.1f, 1,1,1));
		//painter.draw_rect(t->area);

		painter.set_font_size(t->font_size);
		painter.set_color(t->col);// * col);
		painter.draw_str(t->area.p00(), t->text);
	} else if (n->type == gui::Node::Type::PICTURE) {
		auto p = static_cast<gui::Picture*>(n);
		//col *= p->col;
		painter.set_fill(true);
		painter.set_color(p->col);
		painter.set_roundness(p->radius);
		painter.softness = p->softness;
		painter.set_texture(p->texture.get());
		if (p->shader) {
			painter.set_shader(p->shader.get());
			painter.set_shader_data(p->shader_data);
		}
		painter.draw_rect(p->area);
		painter.set_roundness(0);
		painter.softness = 0;
		painter.set_shader(nullptr);
		painter.set_texture(nullptr);
	} else if (n->type == gui::Node::Type::CANVAS) {
		auto c = static_cast<gui::Canvas*>(n);
	//	float fx = params.desired_aspect_ratio;
	//	painter._area = {c->area.x1 * fx, c->area.x2 * fx, c->area.y1, c->area.y2};
		//const auto area0 = painter._area;
		//painter._area = c->area;
		c->on_draw(&painter);
		//painter._area = area0;
	}

	for (auto c: weak(n->children))
		if (c->visible)
			draw_node(painter, c, col);
}

