//
// Created by michi on 25.01.25.
//

#include "DrawingHelper.h"
#include "multiview/MultiView.h"
#include "SingleData.h"
#include "VisibilityStack.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/yrenderer/Context.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/Context.h>
#include <lib/polymesh/MeshEdit.h>
#include <cmath>

const float DrawingHelper::LINE_THIN = 2;
const float DrawingHelper::LINE_MEDIUM = 3;
const float DrawingHelper::LINE_THICK = 5;
const float DrawingHelper::LINE_EXTRA_THICK = 7;
const color DrawingHelper::COLOR_X = color(1, 0.9f, 0.6f, 0.1f).srgb_to_linear();

yrenderer::Material* create_material(yrenderer::Context* ctx, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false) {
	auto material = ctx->create_internal_material();
	material->albedo = albedo;
	material->roughness = roughness;
	material->metal = metal;
	material->emission = emission;
	material->textures = {ctx->tex_white};
	if (transparent) {
		material->pass0.cull_mode = ygfx::CullMode::NONE;
		material->pass0.mode = yrenderer::TransparencyMode::FUNCTIONS;
		material->pass0.source = ygfx::Alpha::SOURCE_ALPHA;
		material->pass0.destination = ygfx::Alpha::SOURCE_INV_ALPHA;
		material->pass0.z_write = false;
	}
	return material;
}

DrawingHelper::DrawingHelper(yrenderer::Context* _ctx, xhui::Context* _xhui_ctx) {
	ctx = _ctx;

	try {
		material_hover = create_material(ctx, {0.3f, 0,0,0}, 0.9f, 0, White, true);
		material_selection = create_material(ctx, {0.3f, 0,0,0}, 0.9f, 0, Red, true);
		material_creation = create_material(ctx, {0.3f, 0,0.5f,0}, 0.9f, 0, color(1,0,0.5f,0).srgb_to_linear(), true);

		material_shadow = new yrenderer::Material();
		material_shadow->pass0.shader_path = "shadow.shader";
	} catch(Exception& e) {
		msg_error(e.message());
	}
}

void DrawingHelper::clear(const yrenderer::RenderParams& params, const color& c) {
#ifdef USING_VULKAN
	auto cb = params.command_buffer;
	cb->clear(params.area, {c}, 1.0);
#else
	nix::clear(c);
#endif
}

void DrawingHelper::draw_mesh(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd, const mat4& matrix, ygfx::VertexBuffer* vertex_buffer, yrenderer::Material* material, int pass_no, const string& vertex_module) {
	auto shader = rvd.get_shader(material, pass_no, vertex_module, "");
	auto& rd = rvd.start(params, matrix, shader, material, pass_no, ygfx::PrimitiveTopology::TRIANGLES, vertex_buffer);
	rd.draw_triangles(params, vertex_buffer);
}

namespace drawing2d {

void draw_boxed_str(Painter* p, const vec2& _pos, const string& str, int align, Style style) {
	vec2 size = p->get_str_size(str);
	vec2 pos = _pos;
	if (align == 0)
		pos.x -= size.x / 2;
	if (align == 1)
		pos.x -= size.x;
	if (style == Style::ERROR)
		p->set_color(color(1, 0.6f, 0.1f, 0.1f));
	else if (style == Style::WARNING)
		p->set_color(xhui::Theme::_default.background_button_danger);
	else
		p->set_color(xhui::Theme::_default.background_button);
	p->set_roundness(7);
	p->draw_rect(rect(pos, pos + size).grow(7));
	p->set_color(xhui::Theme::_default.text_label);
	p->set_roundness(0);
	p->draw_str(pos, str);
}

void draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& _a, MultiViewType kind, const base::optional<Hover>& hover, const base::set<int>& sel, const VisibilityFilter& filter) {
	int _hover = -1;
	if (hover and hover->type == kind)
		_hover = hover->index;
	auto& a = const_cast<DynamicArray&>(_a);
	for (int i=0; i<a.num; i++) {
		if (!filter(i))
			continue;
		const auto v = static_cast<multiview::SingleData*>(a.simple_element(i));
		p->set_color(sel.contains(i) ? Red : color(1, 0.25f, 0.25f, 1.0f));
		auto p1 = win->project(v->pos);
		if (p1.z < 0 or p1.z > 1)
			continue;
		float r = 2.5f;
		if (i == _hover) {
			r = 5;
			p->set_color(sel.contains(i) ? color(1, 1, 0.2f, 0.2f) : color(1, 0.45f, 0.45f, 1.0f));
		}
		p->draw_rect({p1.x - r,p1.x + r, p1.y - r,p1.y + r});
	}
}

}

Array<vec3> mesh_edit_to_lines(const polymesh::Mesh& mesh, const polymesh::MeshEdit& ed) {
	Array<vec3> points;
	for (const auto& p: ed._new_polygons) {
		for (int k=0; k<p.p.side.num; k++) {
			int v0 = p.p.side[k].vertex;
			int v1 = p.p.side[(k+1) % p.p.side.num].vertex;
			points.add((v0 >= 0) ? mesh.vertices[v0].pos : ed._new_vertices[-(v0+1)].v.pos);
			points.add((v1 >= 0) ? mesh.vertices[v1].pos : ed._new_vertices[-(v1+1)].v.pos);
		}
	}
	return points;
}

