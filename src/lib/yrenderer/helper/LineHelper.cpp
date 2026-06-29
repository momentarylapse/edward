#include "LineHelper.h"
#include "../scene/RenderViewData.h"
#include "../Context.h"
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Painter.h>
#include <lib/math/vec2.h>
#include <cmath>

namespace yrenderer {

void LineHelper::start_frame(float _ui_scale) {
	ui_scale = _ui_scale;
	if (aux)
		aux->reset_frame();
}

void LineHelper::begin_draw(const RenderParams& params, RenderViewData& _rvd) {
	rvd = &_rvd;
	if (!aux)
		aux = _rvd.ctx->context->_create_auxiliary_stuff();
	dummy_projection = mat4::ID;
#ifdef USING_VULKAN
	if (params.render_pass != render_pass) {
		aux->rebuild(params.render_pass);
		render_pass = params.render_pass;
	}
	aux->cb = params.command_buffer;
#else
	if (params.target_is_window)
		dummy_projection = mat4::scale(1,-1,1);
#endif
	area = params.area;
	mat = rvd->camera_params.projection_matrix(params.desired_aspect_ratio) * rvd->camera_params.view_matrix();
	aux->projection_matrix = &dummy_projection;
}

void LineHelper::set_color(const color& c) {
	_color = c;
}

void LineHelper::set_line_width(float w) {
	line_width = w;
}

void LineHelper::set_z_test(bool enabled) {
	use_z = enabled;
}

void LineHelper::set_blending(bool enabled) {
	use_blending = enabled;
}

static void add_vb_line(Array<ygfx::VertexX>& vertices, const vec3& a, const vec3& b, const rect& area, float line_width, float ui_scale, const color& col) {
	float w = area.width();
	float h = area.height();
	vec2 ba_pixel = vec2((b.x - a.x) * w, (b.y - a.y) * h);
	vec2 dir_pixel = ba_pixel.normalized();
	vec2 o_pixel = dir_pixel.ortho();
	vec3 r = vec3(o_pixel.x/ w, o_pixel.y/ h, 0) * (line_width / 2) * ui_scale * 2; // *2 ...because we're mapping to -1:+1 ?!?
	vec3 a0 = a - r;
	vec3 a1 = a + r;
	vec3 b0 = b - r;
	vec3 b1 = b + r;
	vertices.add({a0, v_0, 0,0, col});
	vertices.add({a1, v_0, 0,0, col});
	vertices.add({b0, v_0, 0,0, col});
	vertices.add({b0, v_0, 0,0, col});
	vertices.add({a1, v_0, 0,0, col});
	vertices.add({b1, v_0, 0,0, col});
}

void LineHelper::draw_lines(const Array<vec3>& points, bool contiguous) {
	Array<ygfx::VertexX> vertices;

	if (contiguous) {
		for (int i=0; i<points.num-1; i++)
			add_vb_line(vertices, mat.project(points[i]), mat.project(points[i+1]), area, line_width, ui_scale, White);
	} else {
		for (int i=0; i<points.num-1; i+=2)
			add_vb_line(vertices, mat.project(points[i]), mat.project(points[i+1]), area, line_width, ui_scale, White);
	}

	ygfx::draw_simple(aux, vertices, mat4::ID, _color, use_z, use_blending);
}


void LineHelper::draw_lines_colored(const Array<vec3>& points, const Array<color>& cols, bool contiguous) {
	Array<ygfx::VertexX> vertices;

	if (contiguous) {
		for (int i=0; i<points.num-1; i++)
			add_vb_line(vertices, mat.project(points[i]), mat.project(points[i+1]), area, line_width, ui_scale, cols[i]);
	} else {
		for (int i=0; i<points.num-1; i+=2)
			add_vb_line(vertices, mat.project(points[i]), mat.project(points[i+1]), area, line_width, ui_scale, cols[i]);
	}

	ygfx::draw_simple(aux, vertices, mat4::ID, White, use_z, use_blending);
}

void LineHelper::draw_circle(const vec3& center, const vec3& axis, float r) {
	int N = 128;
	Array<vec3> points;
	vec3 e1 = axis.ortho() * r;
	vec3 e2 = vec3::cross(axis, e1);
	for (int i=0; i<=N; i++) {
		float w = (float)i / (float)N * 2 * pi;
		points.add(center + e1 * cosf(w) + e2 * sinf(w));
	}
	draw_lines(points, true);
}
} // yrenderer