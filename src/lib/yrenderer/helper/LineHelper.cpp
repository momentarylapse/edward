#include "LineHelper.h"
#include "../scene/RenderViewData.h"
#include "../Context.h"
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/math/vec2.h>

#include "lib/os/msg.h"
#include "lib/yrenderer/scene/SceneView.h"

namespace ygfx {
	void draw_simple(DrawingHelperData* aux, const Array<Vertex1>& p, const mat4& mat, const color& _color, bool use_z);
}

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
#ifdef USING_VULKAN
	if (params.render_pass != render_pass) {
		aux->rebuild(params.render_pass);
		render_pass = params.render_pass;
	}
	aux->cb = params.command_buffer;
#else
	nix::set_view_matrix(mat4::ID);
	if (params.target_is_window)
		nix::set_projection_matrix(mat4::scale(1,-1,1));
	else
		nix::set_projection_matrix(mat4::ID);
#endif
	area = params.area;
	mat = rvd->scene_view->main_camera_params.projection_matrix(params.desired_aspect_ratio) * rvd->scene_view->main_camera_params.view_matrix();
}

void LineHelper::set_color(const color& c) {
	_color = c;
}

void LineHelper::set_line_width(float w) {
	line_width = w;
}

void LineHelper::set_z(bool enabled) {
	use_z = enabled;
}

static void add_vb_line(Array<ygfx::Vertex1>& vertices, const vec3& a, const vec3& b, const rect& area, float line_width, float ui_scale) {
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
	vertices.add({a0, v_0, 0,0});
	vertices.add({a1, v_0, 0,0});
	vertices.add({b0, v_0, 0,0});
	vertices.add({b0, v_0, 0,0});
	vertices.add({a1, v_0, 0,0});
	vertices.add({b1, v_0, 0,0});
}

void LineHelper::draw_lines(const Array<vec3>& points, bool contiguous) {
	Array<ygfx::Vertex1> vertices;

	if (contiguous) {
		for (int i=0; i<points.num-1; i++)
			add_vb_line(vertices, mat.project(points[i]), mat.project(points[i+1]), area, line_width, ui_scale);
	} else {
		for (int i=0; i<points.num-1; i+=2)
			add_vb_line(vertices, mat.project(points[i]), mat.project(points[i+1]), area, line_width, ui_scale);
	}

	ygfx::draw_simple(aux, vertices, mat4::ID, _color, use_z);
}
} // yrenderer