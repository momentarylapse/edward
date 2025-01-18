#include <lib/math/quaternion.h>

#include "lib/xhui/xhui.h"
#include "lib/xhui/controls/Button.h"
#include "lib/xhui/controls/Label.h"
#include "lib/xhui/controls/Edit.h"
#include "lib/xhui/controls/Grid.h"
#include "lib/xhui/controls/DrawingArea.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/ContextVulkan.h>
#include <renderer/world/geometry/RenderViewData.h>
#include <renderer/world/geometry/SceneView.h>
#include <renderer/base.h>

#include "lib/os/msg.h"
#include "lib/xhui/Theme.h"
#include "lib/xhui/draw/font.h"
#include "y/renderer/Renderer.h"
#include "y/helper/ResourceManager.h"
#include "y/world/Material.h"

string AppVersion = "0.5.-1.0";
string AppName = "Edward";

//EdwardApp *app = nullptr;
void* app = nullptr;

ResourceManager* _resource_manager;

class TestRenderer : public Renderer {
public:
	vulkan::VertexBuffer* vb;
	SceneView scene_view;
	RenderViewData rvd;
	shared<Shader> shader;
	Material* material;
	quaternion ang = quaternion::ID;
	TestRenderer() : Renderer("test") {
		resource_manager = _resource_manager;
		vb = new VertexBuffer("3f,3f,2f");
		vb->create_quad(rect::ID_SYM);
		try {
			shader = resource_manager->load_surface_shader("default.shader", "forward", "default", "");
			material = resource_manager->load_material("");
		} catch(Exception& e) {
			msg_error(e.message());
		}
	}
	void prepare(const RenderParams& params) override {
		ang = quaternion::rotation({0,0,1}, 0.01f) * ang;
	}
	void draw(const RenderParams& params) override {
		auto cb = params.command_buffer;
		cb->clear(params.area, {Green}, 1.0);

		rvd.set_projection_matrix(mat4::perspective(0.7, params.desired_aspect_ratio, 0.1f, 1000.0f, false));
		rvd.set_view_matrix(mat4::translation({0,0,10}));

		rvd.begin_draw();
		auto rd = rvd.start(params, mat4::rotation(ang), shader.get(), *material, 0, PrimitiveTopology::TRIANGLES, vb);
		rd.apply(params);
		cb->draw(vb);
	}
};
class XhuiRenderer : public RenderTask {
public:
	rect native_area_window = rect::ID;
	XhuiRenderer() : RenderTask("xhui") {
	}
	void render(const RenderParams& params) override {
		params.command_buffer->set_viewport(params.area);
		for (auto c: children)
			c->prepare(params);
		for (auto c: children)
			c->draw(params);
		params.command_buffer->set_viewport(native_area_window);
	}
	void render(Painter* p) {
		auto pp = (xhui::Painter*)p;
		RenderParams params;
		params.command_buffer = pp->cb;
		params.area = pp->native_area;
		params.render_pass = pp->context->render_pass;
		params.frame_buffer = pp->context->current_frame_buffer();
		params.desired_aspect_ratio = pp->native_area.width() / pp->native_area.height();
		native_area_window = pp->native_area_window;
		render(params);
	}
};

int hui_main(const Array<string>& args) {

	try {
		xhui::init(args, "edward");
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	auto w = new xhui::Window("test", 1024, 768);
	auto g = new xhui::Grid("grid");
	w->add(g);
	auto g2 = new xhui::Grid("grid2");
	g->add(g2, 0, 0);
	g2->add(new xhui::Label("label1", "label"), 0, 0);
	g2->add(new xhui::Button("button1", "a small test g"), 1, 0);
	g2->add(new xhui::Button("button2", "a small test g"), 2, 0);
	g->add(new xhui::DrawingArea("area"), 0, 1);
	auto g3 = new xhui::Grid("grid3");
	g->add(g3, 0, 2);
	g3->add(new xhui::Button("button3", "a"), 0, 0);
	g3->add(new xhui::Button("button4", "b"), 1, 0);

	w->event("button1", [] {
		msg_write("event button1 click");
	});

	auto renderer = new XhuiRenderer();

	w->event_xp("area", "hui:initialize", [renderer] (Painter* p) {
		auto pp = (xhui::Painter*)p;
		vulkan::default_device = pp->context->device;
		api_init_external(pp->context->instance, pp->context->device);
		_resource_manager = new ResourceManager({});
		try {
			_resource_manager->load_shader_module("module-basic-data.shader");
			_resource_manager->load_shader_module("module-basic-interface.shader");
			_resource_manager->load_shader_module("module-vertex-default.shader");
			_resource_manager->load_shader_module("module-lighting-pbr.shader");
			_resource_manager->load_shader_module("forward/module-surface.shader");
		} catch(Exception& e) {
			msg_error(e.message());
		}
		renderer->add_child(new TestRenderer());
	});
	w->event_xp("area", "hui:draw", [renderer] (Painter* p) {
		if (true) {
			renderer->render(p);
		} else {
			p->set_color(xhui::Theme::_default.background_low);
			p->set_roundness(8);
			p->draw_rect(p->area());
			p->set_roundness(0);
			float font_size = 50;
			p->set_font_size(font_size);
			vec2 p0 = p->area().p00() + vec2(20, 20);
			string text = "Test  g";
			auto dims = font::get_text_dimensions(text);
			p->set_color(xhui::Theme::_default.border);
			p->draw_line({p0.x, p0.y + dims.bounding_top_to_line}, {p0.x + dims.bounding_width, p0.y + dims.bounding_top_to_line});
			p->set_fill(false);
			p->draw_rect(dims.bounding_box(p0));
			p->set_fill(true);

			p->set_color(xhui::Theme::_default.text);
			p->draw_str(p0, text);

			//p->draw_str({50, 200}, "Test g\nbla gg");

			p->set_roundness(20);
			p->draw_rect({50, 300, 150, 250});
			((xhui::Painter*)p)->softness = 10;
			p->draw_rect({50, 300, 300, 400});
			((xhui::Painter*)p)->softness = 0;
		}
	});

	xhui::run();

	return 0;
}

