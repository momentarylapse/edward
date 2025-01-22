//
// Created by michi on 19.01.25.
//

#include "EdwardWindow.h"
#include "lib/xhui/xhui.h"
#include "lib/xhui/controls/Button.h"
#include "lib/xhui/controls/Label.h"
#include "lib/xhui/controls/Edit.h"
#include "lib/xhui/controls/Grid.h"
#include "lib/xhui/controls/DrawingArea.h"
#include "lib/xhui/controls/Overlay.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/ContextVulkan.h>
#include <renderer/base.h>
#include <renderer/path/RenderPath.h>
#include <sys/stat.h>
#include <y/EngineData.h>
#include "MultiView.h"
#include "lib/os/msg.h"
#include "lib/xhui/Theme.h"
#include "lib/xhui/draw/font.h"
#include "y/renderer/Renderer.h"
#include "y/helper/ResourceManager.h"
#include "mode_world/ModeWorld.h"
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
		engine.physical_aspect_ratio = pp->native_area.width() / pp->native_area.height();
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

class ToolButton : public xhui::Button {
public:
	explicit ToolButton(const string& id, const string& title) : Button(id, title) {
		min_height_user = min_width_user;
		expand_x = false;
		expand_y = false;
	}
};

class TouchButton : public xhui::Button {
public:
	explicit TouchButton(const string& id, const string& title) : Button(id, title) {
		min_height_user = 50;
		min_width_user = 50;
		expand_x = false;
		expand_y = false;
	}
	/*void _draw(xhui::Painter* p) override {
	}*/
};


EdwardWindow::EdwardWindow(Session* _session) : obs::Node<xhui::Window>(AppName, 1024, 768),
	in_redraw(this, [this] {
		request_redraw();
	}),
	in_data_selection_changed(this, [this] {
		//msg_write("SEL CHANGED");
		request_redraw();
		update_menu();
	}),
	in_data_changed(this, [this] {
		//msg_write("DATA CHANGED");
		//session->cur_mode->on_set_multi_view();
		//session->cur_mode->multi_view->force_redraw();
		request_redraw();
		update_menu();
	}),
	in_action_failed(this, [this] {
		auto am = session->cur_mode->get_data()->action_manager;
		session->error(format("Action failed: %s\nReason: %s", am->error_location.c_str(), am->error_message.c_str()));
	}),
	in_saved(this, [this] {
		msg_write("SAVED");
		session->set_message("Saved!");
		update_menu();
	})
{
	session = _session;

	auto g = new xhui::Grid("grid");
	add(g);
	auto g2 = new xhui::Grid("grid2");
	g->add(g2, 0, 0);
	g2->add(new xhui::Label("label1", "label"), 0, 0);
	g2->add(new ToolButton("undo", "undo"), 1, 0);
	g2->add(new ToolButton("redo", "redo"), 2, 0);
	auto o = new xhui::Overlay("overlay");
	g->add(o, 0, 1);
	o->add(new xhui::DrawingArea("area"));
	/*auto g3 = new xhui::Grid("grid3");
	g->add(g3, 0, 2);
	g3->add(new xhui::Button("button3", "a"), 0, 0);
	g3->add(new xhui::Button("button4", "b"), 1, 0);*/


	auto g4 = new xhui::Grid("grid4");
	o->add(g4);
	g4->margin = 25;
	g4->add(new TouchButton("button5", ""), 0, 0);
	g4->add(new TouchButton("cam-rotate", "R"), 0, 1);
	g4->add(new TouchButton("cam-move", "M"), 0, 2);

	event("undo", [this] {
		session->cur_mode->on_command("undo");
	});
	event("redo", [this] {
		session->cur_mode->on_command("redo");
	});

	renderer = new XhuiRenderer();

	event_xp("area", xhui::event_id::Initialize, [this] (Painter* p) {
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
		mode->multi_view->add_child(mode->create_renderer(mode->multi_view->view_port.scene_view.get()));
		session->storage = new Storage(session);
		session->set_mode(mode);


		engine.file_errors_are_critical = false;
		engine.ignore_missing_files = true;
		engine.resource_manager = session->resource_manager;

		if (args.num >= 2) {
			session->storage->load(args[1], mode->data);
			xhui::run_later(0.2f, [mode] {
				mode->optimize_view();
			});
		}
	});
	event_xp("area", xhui::event_id::Draw, [this] (Painter* p) {
		session->cur_mode->multi_view->area = p->area();
		renderer->render(p);
		session->cur_mode->multi_view->on_draw(p);
		session->cur_mode->on_draw_post(p);
	});
	event_x("area", xhui::event_id::MouseMove, [this] {
		session->cur_mode->multi_view->on_mouse_move(state.m, state.m - state_prev.m);
		session->cur_mode->on_mouse_move(state.m, state.m - state_prev.m);
	});
	event_x("area", xhui::event_id::MouseWheel, [this] {
		session->cur_mode->multi_view->on_mouse_wheel(state.m, state.scroll);
	});
	event_x("area", xhui::event_id::MouseLeave, [this] {
		session->cur_mode->multi_view->on_mouse_leave();
		session->cur_mode->on_mouse_leave(state.m);
	});
	event_x("area", xhui::event_id::LeftButtonDown, [this] {
		session->cur_mode->on_left_button_down(state.m);
	});
	event_x("area", xhui::event_id::LeftButtonUp, [this] {
		session->cur_mode->on_left_button_up(state.m);
	});
	event_x("cam-move", xhui::event_id::LeftButtonDown, [this] {
		set_mouse_mode(0);
	});
	event_x("cam-move", xhui::event_id::LeftButtonUp, [this] {
		set_mouse_mode(1);
	});
	event_x("cam-move", xhui::event_id::MouseMove, [this] {
		vec2 d = state.m - state_prev.m;
		if (state.lbut)
			session->cur_mode->multi_view->view_port.move(vec3(-d.x, d.y, 0) / 800.0f);
	});
	event_x("cam-rotate", xhui::event_id::LeftButtonDown, [this] {
		set_mouse_mode(0);
	});
	event_x("cam-rotate", xhui::event_id::LeftButtonUp, [this] {
		set_mouse_mode(1);
	});
	event_x("cam-rotate", xhui::event_id::MouseMove, [this] {
		vec2 d = state.m - state_prev.m;
		if (state.lbut)
			session->cur_mode->multi_view->view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));
	});

	xhui::run_repeated(0.02f, [this] {
		//request_redraw();
	});
}

void EdwardWindow::on_key_down(int key) {
	session->cur_mode->multi_view->on_key_down(key);
	session->cur_mode->on_key_down(key);
}

void EdwardWindow::update_menu() {
	if (session->cur_mode->get_data()) {
		enable("undo", session->cur_mode->get_data()->action_manager->undoable());
		enable("redo", session->cur_mode->get_data()->action_manager->redoable());
	}
}



