//
// Created by michi on 25.01.25.
//

#include "DrawingHelper.h"
#include <y/graphics-impl.h>
#include <y/helper/ResourceManager.h>
#include <y/renderer/base.h>
#include <lib/math/mat4.h>
#include <lib/math/vec2.h>
#include <lib/os/msg.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/xhui.h>
#include "MultiView.h"
#include "../multiview/SingleData.h"


Material* create_material(ResourceManager* resource_manager, const color& albedo, float roughness, float metal, const color& emission, bool transparent = false) {
	auto material = resource_manager->load_material("");
	material->albedo = albedo;
	material->roughness = roughness;
	material->metal = metal;
	material->emission = emission;
	material->textures = {tex_white};
	if (transparent) {
		material->pass0.cull_mode = 0;
		material->pass0.mode = TransparencyMode::FUNCTIONS;
		material->pass0.source = Alpha::SOURCE_ALPHA;
		material->pass0.destination = Alpha::SOURCE_INV_ALPHA;
		material->pass0.z_buffer = false;
	}
	return material;
}

DrawingHelper::DrawingHelper(xhui::Context* ctx, ResourceManager* rm) {
	context = ctx;
	resource_manager = rm;

	/*light = new Light(White, -1, -1);
	light->owner = new Entity;
	light->owner->ang = quaternion::rotation({1,0,0}, 0.5f);
	light->light.harshness = 0.5f;*/

	try {
		material_hover = create_material(resource_manager, {0.3f, 0,0,0}, 0.9f, 0, White, true);
		material_selection = create_material(resource_manager, {0.3f, 0,0,0}, 0.9f, 0, Red, true);
		material_creation = create_material(resource_manager, {0.3f, 0,0.5f,0}, 0.9f, 0, color(1,0,0.5f,0), true);
	} catch(Exception& e) {
		msg_error(e.message());
	}

#ifdef USING_VULKAN
	shader = vulkan::Shader::create(
		R"foodelim(
<Layout>
	version = 430
	bindings = [[sampler]]
	pushsize = 96
</Layout>
<VertexShader>

//#extension GL_ARB_separate_shader_objects : enable

layout(push_constant, std430) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;
layout(location = 2) in vec2 in_uv;

layout(location = 0) out vec4 out_pos; // camera space
layout(location = 1) out vec2 out_uv;

void main() {
	gl_Position = params.matrix * vec4(in_position, 1);
	out_pos = gl_Position;
	out_uv = in_uv;
}
</VertexShader>
<FragmentShader>

layout(push_constant, std140) uniform Parameters {
	mat4 matrix;
	vec4 color;
	vec2 size;
	float radius;
	float softness;
} params;

layout(location = 0) in vec4 in_pos;
layout(location = 1) in vec2 in_uv;
layout(location = 0) out vec4 out_color;

layout(binding = 0) uniform sampler2D tex0;

void main() {
	out_color = texture(tex0, in_uv);
	out_color *= params.color;
}
</FragmentShader>
)foodelim");
//#ifdef USING_VULKAN
	dset = ctx->pool->create_set(shader);
	dset->set_texture(0, ctx->tex_white);
	dset->update();

	pipeline = new vulkan::GraphicsPipeline(shader, context->render_pass, 0, "triangles", context->vb);
	//pipeline->set_z(false, false);
	pipeline->set_culling(vulkan::CullMode::NONE);
	pipeline->rebuild();
#endif
}

void DrawingHelper::set_window(MultiViewWindow* win) {
	window = win;
}


void DrawingHelper::set_color(const color& color) {
	_color = color;
}

void DrawingHelper::set_line_width(float width) {
	_line_width = width;
}



static void add_vb_line(Array<Vertex1>& vertices, const vec3& a, const vec3& b, MultiViewWindow* win, float line_width) {
	float w = win->area.width();
	float h = win->area.height();
	vec2 ba_pixel = vec2((b.x - a.x) * w, (b.y - a.y) * h);
	vec2 dir_pixel = ba_pixel.normalized();
	vec2 o_pixel = dir_pixel.ortho();
	vec3 r = vec3(o_pixel.x/ w, o_pixel.y/ h, 0) * (line_width / 2) * xhui::ui_scale;
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

void DrawingHelper::draw_lines(const Array<vec3>& points, bool contiguous) {
#ifdef USING_VULKAN
	auto vb = context->get_line_vb();
	Array<Vertex1> vertices;
	mat4 m = window->projection * window->view;
	if (contiguous) {
		for (int i=0; i<points.num-1; i++)
			add_vb_line(vertices, m.project(points[i]), m.project(points[i+1]), window, _line_width);
	} else {
		for (int i=0; i<points.num-1; i+=2)
			add_vb_line(vertices, m.project(points[i]), m.project(points[i+1]), window, _line_width);
	}
	vb->update(vertices);

	struct Parameters {
		mat4 matrix;
		color col;
		vec2 size;
		float radius, softness;
	};

	Parameters params;
	params.matrix = mat4::ID;
	params.col = _color;
	params.size = {1000,1000};//(float)width, (float)height};
	params.radius = 0;//line_width;
	params.softness = 0;//softness;

	auto cb = context->current_command_buffer();
	cb->bind_pipeline(pipeline);
	cb->push_constant(0, sizeof(params), &params);
	cb->bind_descriptor_set(0, dset);
	cb->draw(vb);
#endif
}

void DrawingHelper::draw_circle(const vec3& center, const vec3& axis, float r) {
	int N = 128;
	Array<vec3> points;
	vec3 e1 = axis.ortho() * r;
	vec3 e2 = vec3::cross(axis, e1);
	for (int i=0; i<=N; i++) {
		float w = (float)i / (float)N * 2 * pi;
		points.add(center + e1 * cos(w) + e2 * sin(w));
	}
	draw_lines(points);
}


void DrawingHelper::clear(const RenderParams& params, const color& c) {
#ifdef USING_VULKAN
	auto cb = params.command_buffer;
	cb->clear(params.area, {c}, 1.0);
#else
	nix::clear(c);
#endif
}

void DrawingHelper::draw_mesh(const RenderParams& params, RenderViewData& rvd, const mat4& matrix, VertexBuffer* vertex_buffer, Material* material, int pass_no, const string& vertex_module) {
#ifdef USING_VULKAN
	auto cb = context->current_command_buffer();
	auto shader = rvd.get_shader(material, 0, vertex_module, "");
	auto& rd = rvd.start(params, matrix, shader, *material, pass_no, PrimitiveTopology::TRIANGLES, vertex_buffer);
	rd.apply(params);
	cb->draw(vertex_buffer);
#else
	msg_error("TODO draw mesh");
#endif
}

void DrawingHelper::draw_boxed_str(Painter* p, const vec2& _pos, const string& str, int align) {
	vec2 size = p->get_str_size(str);
	vec2 pos = _pos;
	if (align == 0)
		pos.x -= size.x / 2;
	if (align == 1)
		pos.x -= size.x;
	p->set_color(xhui::Theme::_default.background_button);
	p->set_roundness(7);
	p->draw_rect(rect(pos, pos + size).grow(7));
	p->set_color(xhui::Theme::_default.text_label);
	p->set_roundness(0);
	p->draw_str(pos, str);
}

void DrawingHelper::draw_data_points(Painter* p, MultiViewWindow* win, const DynamicArray& _a, MultiViewType kind, const base::optional<Hover>& hover) {
	int _hover = -1;
	if (hover and hover->type == kind)
		_hover = hover->index;
	auto& a = const_cast<DynamicArray&>(_a);
	for (int i=0; i<a.num; i++) {
		const auto v = static_cast<multiview::SingleData*>(a.simple_element(i));
		p->set_color(v->is_selected ? Red : Blue);
		auto p1 = win->project(v->pos);
		float r = 2;
		if (i == _hover)
			r = 4;
		p->draw_rect({p1.x - r,p1.x + r, p1.y - r,p1.y + r});
	}
}



