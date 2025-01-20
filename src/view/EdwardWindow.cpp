//
// Created by michi on 19.01.25.
//

#include "EdwardWindow.h"
#include <lib/math/quaternion.h>

#include "lib/xhui/xhui.h"
#include "lib/xhui/controls/Button.h"
#include "lib/xhui/controls/Label.h"
#include "lib/xhui/controls/Edit.h"
#include "lib/xhui/controls/Grid.h"
#include "lib/xhui/controls/DrawingArea.h"
#include "lib/xhui/controls/Overlay.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/ContextVulkan.h>
#include <mode_world/ModeWorld.h>
#include <renderer/world/geometry/RenderViewData.h>
#include <renderer/world/geometry/SceneView.h>
#include <renderer/base.h>
#include <renderer/path/RenderPath.h>
#include <world/Model.h>
#include <world/ModelManager.h>
#include <world/Terrain.h>
#include <y/EngineData.h>

#include "MultiView.h"
#include "lib/os/msg.h"
#include "lib/xhui/Theme.h"
#include "lib/xhui/draw/font.h"
#include "y/renderer/Renderer.h"
#include "y/helper/ResourceManager.h"
#include "y/world/Material.h"
#include "y/world/Camera.h"
#include "y/world/Light.h"
#include "y/y/Entity.h"
#include "mode_world/data/DataWorld.h"
#include "storage/Storage.h"
#include "Session.h"


extern string AppName;


Session* session;

rect dynamicly_scaled_area(FrameBuffer*) { return {}; }
rect dynamicly_scaled_source() { return {}; }
void ExternalModelCleanup(Model *m) {}

#if 0
class TestRenderer : public Renderer {
public:
	vulkan::VertexBuffer* vbx;
	SceneView scene_view;
	RenderViewData rvd;
	shared<Shader> shader;
	Material* material;
	quaternion ang = quaternion::ID;
	Camera* cam;
	Light* light;
	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!

	TestRenderer() : Renderer("test") {
		resource_manager = _resource_manager;
		vbx = new VertexBuffer("3f,3f,2f");
		vbx->create_quad(rect::ID_SYM);
		try {
			shader = resource_manager->load_surface_shader("default.shader", "forward", "default", "");
			material = resource_manager->load_material("");
			material->albedo = White;
			material->metal = 0.0f;
			material->roughness = 0.9f;
			material->emission = color(1, 0.1f, 0.1f, 0.1f);
			material->textures = {tex_white};
			material->pass0.cull_mode = 0;
		} catch(Exception& e) {
			msg_error(e.message());
		}

		cam = new Camera();
		cam->owner = new Entity;
		scene_view.cam = cam;
		cam->owner->pos = {0, 0,-10};
		cam->min_depth = 1;
		cam->max_depth = 100;
		rvd.scene_view = &scene_view;

		light = new Light(White, -1, -1);
		light->owner = new Entity;
		//light->owner->ang = quaternion::rotation({0,1,0}, pi);
		//light->light.harshness = 0.5f;
	}
	void prepare(const RenderParams& params) override {
		ang = quaternion::rotation({0,1,0}, 0.02f) * ang;
	}
	void draw(const RenderParams& params) override {
		auto cb = params.command_buffer;
		cb->clear(params.area, {data_world->meta_data.background_color}, 1.0);

	//	scene_view.choose_lights();
		{
			scene_view.lights.clear();
			scene_view.shadow_index = -1;
			//	if (l->allow_shadow)
			//		scene_view.shadow_index = scene_view.lights.num;
			scene_view.lights.add(light);
		}

		scene_view.cam->update_matrices(params.desired_aspect_ratio);
		rvd.set_projection_matrix(scene_view.cam->m_projection);
		rvd.set_view_matrix(scene_view.cam->m_view);
		rvd.update_lights();
		rvd.ubo.num_lights = scene_view.lights.num;
		rvd.ubo.shadow_index = scene_view.shadow_index;

		rvd.begin_draw();


		auto& rd = rvd.start(params,  mat4::rotation(ang), shader.get(), *material, 0, PrimitiveTopology::TRIANGLES, vbx);
		rd.apply(params);
		cb->draw(vbx);
	}
};
#endif


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


EdwardWindow::EdwardWindow(Session* _session) : xhui::Window(AppName, 1024, 768) {
	session = _session;

	auto g = new xhui::Grid("grid");
	add(g);
	auto g2 = new xhui::Grid("grid2");
	g->add(g2, 0, 0);
	g2->add(new xhui::Label("label1", "label"), 0, 0);
	g2->add(new xhui::Button("button1", "a small test g"), 1, 0);
	g2->add(new xhui::Button("button2", "a small test g"), 2, 0);
	auto o = new xhui::Overlay("overlay");
	g->add(o, 0, 1);
	o->add(new xhui::DrawingArea("area"));
	auto g3 = new xhui::Grid("grid3");
	g->add(g3, 0, 2);
	g3->add(new xhui::Button("button3", "a"), 0, 0);
	g3->add(new xhui::Button("button4", "b"), 1, 0);


	auto g4 = new xhui::Grid("grid4");
	o->add(g4);
	g4->margin = 25;
	g4->add(new xhui::Button("button5", "a"), 0, 0);
	g4->add(new xhui::Button("button6", "b"), 0, 1);
	for (auto c: g4->children) {
		c.control->min_width_user = 50;
		c.control->min_height_user = 50;
		c.control->expand_x = false;
	}

	event("button1", [] {
		msg_write("event button1 click");
	});

	renderer = new XhuiRenderer();

	event_xp("area", "hui:initialize", [this] (Painter* p) {
		auto pp = (xhui::Painter*)p;
		vulkan::default_device = pp->context->device;
		api_init_external(pp->context->instance, pp->context->device);
		session->resource_manager = new ResourceManager({});
		session->resource_manager->default_shader = "default.shader";
		try {
			session->resource_manager->load_shader_module("module-basic-data.shader");
			session->resource_manager->load_shader_module("module-basic-interface.shader");
			session->resource_manager->load_shader_module("module-vertex-default.shader");
			session->resource_manager->load_shader_module("module-vertex-animated.shader");
			session->resource_manager->load_shader_module("module-lighting-pbr.shader");
			session->resource_manager->load_shader_module("forward/module-surface.shader");
		} catch(Exception& e) {
			msg_error(e.message());
		}
		auto mode = new ModeWorld(session);
		renderer->add_child(mode->multi_view);
		mode->multi_view->add_child(mode->create_renderer(mode->multi_view->cam.scene_view.get()));
		session->storage = new Storage(session);
		session->set_mode(mode);


		engine.file_errors_are_critical = false;
		engine.ignore_missing_files = true;
		engine.resource_manager = session->resource_manager;

		if (args.num >= 2)
			session->storage->load(args[1], mode->data);
	});
	event_xp("area", "hui:draw", [this] (Painter* p) {
		renderer->render(p);
	});

	xhui::run_repeated(0.02f, [this] {
		request_redraw();
	});
}

void EdwardWindow::on_mouse_move(const vec2& m, const vec2& d) {
	if (state.lbut)
		session->cur_mode->multi_view->cam.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));
	if (state.rbut)
		session->cur_mode->multi_view->cam.move(vec3(-d.x, d.y, 0) / 800.0f); // / window size?
}
void EdwardWindow::on_mouse_wheel(const vec2& d) {
	if (session->cur_mode)
		session->cur_mode->multi_view->cam.radius *= exp(- d.y * 0.1f);
}
void EdwardWindow::on_key_down(int key) {
	float d = 0.05f;
	if (key == xhui::KEY_UP)
		session->cur_mode->multi_view->cam.move({0, d, 0});
	if (key == xhui::KEY_DOWN)
		session->cur_mode->multi_view->cam.move({0, -d, 0});
	if (key == xhui::KEY_LEFT)
		session->cur_mode->multi_view->cam.move({-d, 0, 0});
	if (key == xhui::KEY_RIGHT)
		session->cur_mode->multi_view->cam.move({d, 0, 0});
}


