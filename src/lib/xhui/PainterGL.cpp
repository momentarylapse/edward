#if HAS_LIB_GL

#include "Painter.h"
#include "Theme.h"
#include "draw/font.h"
#include "Context.h"

#include "../nix/nix.h"
#include "../image/image.h"

namespace nix {
	mat4 create_pixel_projection_matrix();
}


namespace xhui {


nix::Texture *tex_text = nullptr;
nix::Texture *tex_white = nullptr;
nix::VertexBuffer *vb_rect = nullptr;
nix::Shader *shader = nullptr;
nix::Shader *shader_round = nullptr;
nix::Texture *tex_xxx = nullptr;

bool _nix_inited = false;
owned<nix::Context> __nix_context;

nix::Context* init_nix() {
	if (_nix_inited)
		return __nix_context.get();
	nix::default_shader_bindings = false;
	__nix_context = nix::init();
	tex_text = new nix::Texture();
	tex_white = new nix::Texture();
	Image im;
	im.create(8, 8, White);
	tex_white->write(im);
	vb_rect = new nix::VertexBuffer("3f,3f,2f");
	vb_rect->create_quad(rect::ID, rect::ID);



#if 0
	<VertexShader>
	#version 330 core
	#extension GL_ARB_separate_shader_objects : enable

	struct Matrix { mat4 model, view, project; };
	/*layout(binding = 0)*/ uniform Matrix matrix;

	layout(location = 0) in vec3 in_position;
	layout(location = 1) in vec3 in_normal;
	layout(location = 2) in vec2 in_uv;

	layout(location = 0) out vec2 out_uv;

	void main() {
		gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
		out_uv = in_uv;
	}

	</VertexShader>
#endif

	shader = __nix_context->create_shader(
			R"foodelim(
<Layout>
	version = 420
</Layout>
<VertexShader>

#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
layout(binding=0) uniform sampler2D tex0;
uniform vec4 _color_;
out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= _color_;
}
</FragmentShader>
)foodelim");
	shader->filename = "-my-shader-";


	shader_round  = __nix_context->create_shader(
			R"foodelim(
<Layout>
	version = 420
</Layout>
<VertexShader>

#extension GL_ARB_separate_shader_objects : enable

struct Matrix { mat4 model, view, project; };
/*layout(binding = 0)*/ uniform Matrix matrix;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec2 out_uv;
layout(location = 2) out vec4 out_pos; // camera space

void main() {
	gl_Position = matrix.project * matrix.view * matrix.model * vec4(in_position, 1);
	out_normal = (matrix.view * matrix.model * vec4(in_normal, 0)).xyz;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_normal;
layout(location = 1) in vec2 in_uv;
layout(location = 2) in vec4 in_pos;
uniform sampler2D tex0;
uniform vec4 _color_;
uniform vec2 size;
uniform float radius = 0;
uniform float softness = 0;
out vec4 out_color;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= _color_;
	if (softness >= 0.5) {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-softness-radius));
		pp = clamp(pp, 0, 1000); 
		out_color.a *= 1 - clamp((length(pp) - radius) / softness, 0, 1);
	} else {
		vec2 pp = (abs(in_uv - 0.5) * size - (0.5*size-radius));
		pp = clamp(pp, 0, 1000);
		out_color.a *= 1 - clamp((length(pp) - radius), 0, 1);
	}
	//out_color = vec4(length(pp)/radius, 0, 1, 1);
}
</FragmentShader>
)foodelim");
	shader_round->filename = "-my-shader-round-";

	tex_xxx = new nix::Texture();

	_nix_inited = true;
	return __nix_context.get();
}

Painter::Painter(Window *w) {
	window = w;
	context = window->context;
	if (window) {
		ui_scale = window->ui_scale;
		glfwMakeContextCurrent(w->window);
		if (!_nix_inited)
			context->ctx = init_nix();

		Painter::set_color(Theme::_default.text);
		Painter::set_font(Theme::_default.font_name /*"CAC Champagne"*/, Theme::_default.font_size, false, false);

		offset_x = 0;
		offset_y = 0;


		int ww, hh;
		glfwGetWindowSize(window->window, &ww, &hh);
		width = (float)ww / ui_scale;
		height = (float)hh / ui_scale;

		_area = {0, (float)width, 0, (float)height};
		native_area = {0, (float)ww, 0, (float)hh};
		native_area_window = native_area;
		_clip = _area;

		window->handle_event_p(window->id, event_id::JustBeforeDraw, this);

		// in case the event_id::JustBeforeDraw triggers off-screen rendering...
		nix::bind_frame_buffer(context->ctx->default_framebuffer);

		nix::start_frame_glfw(context->ctx, window->window);
		nix::set_projection_matrix(nix::create_pixel_projection_matrix() * mat4::translation({0,0,0.5f}) * mat4::scale(ui_scale, ui_scale, 1));
		//nix::clear(color(1, 0.15f, 0.15f, 0.3f));
		nix::set_cull(nix::CullMode::NONE);
		nix::set_z(false, false);
	}
}

void Painter::end() {
	nix::end_frame_glfw();
	iterate_text_caches();
}

void Painter::clear(const color &c) {
	nix::clear(c);
}

void Painter::draw_str(const vec2 &p, const string &str) {
	if (str.num == 0)
		return;
	Image im;
	face->render_text(str, Align::LEFT, im);
	tex_text->write(im);
	tex_text->set_options("minfilter=nearest");
	float w = im.width / ui_scale;
	float h = im.height / ui_scale;
	nix::set_model_matrix(mat4::translation(vec3(offset_x + p.x, offset_y + p.y, 0)) * mat4::scale(w, h, 1));

	nix::set_shader(shader);
	nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	shader->set_color("_color_", _color);
	shader->set_default_data();
	nix::bind_texture(0, tex_text);
	nix::draw_triangles(vb_rect);
	nix::disable_alpha();
}

void Painter::draw_rect(const rect &r) {
	if (fill) {
		nix::set_model_matrix(mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1));
		auto s = shader;
		if (corner_radius > 0) {
			s = shader_round;
			vec2 size = {r.width(), r.height()};
			s->set_floats("size", &size.x, 2);
			s->set_float("radius", corner_radius);
			s->set_float("softness", softness);
		}
		nix::set_shader(s);
		if (_color.a < 1 or corner_radius > 0) {
			if (accumulate_alpha)
				nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ONE, nix::Alpha::ONE);
			else
				nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
		}
		s->set_color("_color_", _color);
		s->set_default_data();
		nix::bind_texture(0, tex_white);
		nix::draw_triangles(vb_rect);
		nix::disable_alpha();
	} else {
		if (corner_radius > 0) {
			draw_line({r.x1 + corner_radius, r.y1}, {r.x2 - corner_radius, r.y1});
			draw_line({r.x1 + corner_radius, r.y2}, {r.x2 - corner_radius, r.y2});
			draw_line({r.x1, r.y1 + corner_radius}, {r.x1, r.y2 - corner_radius});
			draw_line({r.x2, r.y1 + corner_radius}, {r.x2, r.y2 - corner_radius});
			draw_arc({r.x1 + corner_radius, r.y1 + corner_radius}, corner_radius, pi/2, pi);
			draw_arc({r.x2 - corner_radius, r.y1 + corner_radius}, corner_radius, 0, pi/2);
			draw_arc({r.x1 + corner_radius, r.y2 - corner_radius}, corner_radius, -pi/2, -pi);
			draw_arc({r.x2 - corner_radius, r.y2 - corner_radius}, corner_radius, 0, -pi/2);
		} else {
			draw_line({r.x1, r.y1}, {r.x2, r.y1});
			draw_line({r.x1, r.y2}, {r.x2, r.y2});
			draw_line({r.x1, r.y1}, {r.x1, r.y2});
			draw_line({r.x2, r.y1}, {r.x2, r.y2});
		}
	}
}

void Painter::draw_line(const vec2 &a, const vec2 &b) {
	//nix::set_model_matrix(mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1));
	nix::set_model_matrix(mat4::translation(vec3(a.x + offset_x, a.y + offset_y, 0))
			* mat4::rotation_z(atan2(b.y - a.y, b.x - a.x))
			* mat4::scale((b - a).length(), line_width, 1));
	nix::set_shader(shader);
	if (_color.a < 1)
		nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	shader->set_color("_color_", _color);
	shader->set_default_data();
	nix::bind_texture(0, tex_white);
	nix::draw_triangles(vb_rect);
	nix::disable_alpha();
}

void Painter::draw_lines(const Array<vec2> &p) {
	if (contiguous) {
		for (int i=0; i<p.num-1; i++)
			draw_line(p[i], p[i+1]);
	} else {
		for (int i=0; i<p.num-1; i+=2)
			draw_line(p[i], p[i+1]);
	}
}


void Painter::set_transform(float rot[], const vec2 &offset) {
	offset_x = offset.x;
	offset_y = offset.y;
}

void Painter::set_clip(const rect &r) {
	_clip = r;
	nix::set_scissor({r.x1 * ui_scale, max(r.x2, r.x1) * ui_scale, r.y1 * ui_scale, max(r.y2, r.y1) * ui_scale});
}


void Painter::draw_ximage(const rect &r, const XImage *image) {
	auto t = image->texture.get();
	nix::set_model_matrix(mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1));
	nix::set_shader(shader);
	nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	shader->set_color("_color_", _color);
	shader->set_default_data();
	nix::bind_texture(0, t);
	nix::draw_triangles(vb_rect);
	nix::disable_alpha();
}


}

#endif
