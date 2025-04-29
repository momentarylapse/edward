//
// Created by michi on 19.01.25.
//

#include "EdwardWindow.h"
#include "lib/xhui/xhui.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/Context.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/dialogs/QuestionDialog.h>
#include <lib/xhui/dialogs/ColorSelectionDialog.h>
#include <renderer/base.h>
#include <renderer/path/RenderPath.h>
#include <sys/stat.h>
#include <y/EngineData.h>

#include "ActionController.h"
#include "DrawingHelper.h"
#include "MultiView.h"
#include "lib/os/msg.h"
#include "lib/xhui/Theme.h"
#include "lib/xhui/draw/font.h"
#include "y/renderer/Renderer.h"
#include "y/renderer/target/XhuiRenderer.h"
#include "y/helper/ResourceManager.h"
#include "mode_world/ModeWorld.h"
#include "storage/Storage.h"
#include "Session.h"


extern string AppName;


Session* session;

rect dynamicly_scaled_area(FrameBuffer*) { return {}; }
rect dynamicly_scaled_source() { return {}; }
void ExternalModelCleanup(Model *m) {}

namespace xhui {
#ifdef HAS_LIB_GL
	void init_nix();
	extern owned<nix::Context> _nix_context;
#endif
}

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
	maximize(true);
	session = _session;

	from_source(R"foodelim(
Dialog x x padding=0
	Grid grid '' spacing=0
		MenuBar menu '' main
		---|
		Toolbar toolbar '' main
		---|
		Grid main-grid ''
			Overlay ? ''
				DrawingArea area '' grabfocus
				Grid overlay-main-grid '' margin=25
					Grid overlay-button-grid-left '' spacing=20
						Button mouse-action 'T' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
					.
					Label ? '' ignorehover expandx
					Grid overlay-button-grid-right '' spacing=20
						Button cam-rotate 'R' image=rf-rotate height=50 width=50 padding=7 noexpandx ignorefocus
						---|
						Button cam-move 'M' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");

#ifdef OS_MAC
	int mod = xhui::KEY_SUPER;
#else
	int mod = xhui::KEY_CONTROL;
#endif

	set_key_code("new", mod + xhui::KEY_N);
	set_key_code("open", mod + xhui::KEY_O);
	set_key_code("save", mod + xhui::KEY_S);
	set_key_code("save-as", mod + xhui::KEY_SHIFT + xhui::KEY_S);
	set_key_code("quit", mod + xhui::KEY_Q);
	set_key_code("undo", mod + xhui::KEY_Z);
	set_key_code("redo", mod + xhui::KEY_Y);
	set_key_code("copy", mod + xhui::KEY_C);
	set_key_code("paste", mod + xhui::KEY_V);

	toolbar = (xhui::Toolbar*)get_control("toolbar");

	Array<string> ids = {"new", "open", "save", "save-as", "exit", "undo", "redo", "copy", "paste"};
	for (const string& id: ids)
		event(id, [this, id=id] {
			session->cur_mode->on_command(id);
		});

	event_xp("area", xhui::event_id::Initialize, [this] (Painter* p) {
		auto pp = (xhui::Painter*)p;
#ifdef USING_VULKAN
		vulkan::default_device = pp->context->device;
		api_init_external(pp->context->instance, pp->context->device);
#else
		xhui::init_nix();
		//api_init()
#endif
		session->resource_manager = new ResourceManager({});
		session->resource_manager->default_shader = "default.shader";
#ifdef USING_OPENGL
		session->resource_manager->ctx = xhui::_nix_context.get();
#endif
		session->drawing_helper = new DrawingHelper(pp->context, session->resource_manager);
		try {
			session->resource_manager->load_shader_module("module-basic-data.shader");
			session->resource_manager->load_shader_module("module-basic-interface.shader");
			session->resource_manager->load_shader_module("module-vertex-default.shader");
			session->resource_manager->load_shader_module("module-vertex-animated.shader");
			session->resource_manager->load_shader_module("module-light-sources-default.shader");
			session->resource_manager->load_shader_module("module-shadows-pcf.shader");
			session->resource_manager->load_shader_module("module-lighting-pbr.shader");
			session->resource_manager->load_shader_module("forward/module-surface.shader");
		} catch(Exception& e) {
			msg_error(e.message());
		}

		engine.file_errors_are_critical = false;
		engine.ignore_missing_files = true;
		engine.resource_manager = session->resource_manager;

		session->promise_started(session);
	});
	event_xp("area", xhui::event_id::Draw, [this] (Painter* p) {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		session->cur_mode->multi_view->set_area(p->area());
		renderer->render(p);
		session->cur_mode->multi_view->on_draw(p);
		session->cur_mode->on_draw_post(p);
		p->set_color(White);
		p->set_font_size(xhui::Theme::_default.font_size * 1.5f);
		for (int i=0; i<session->message_str.num; i++)
			session->drawing_helper->draw_boxed_str(p, _area.center() + vec2(0, 20*i), session->message_str[i], 0);
	});
	event_x("area", xhui::event_id::MouseMove, [this] {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		session->cur_mode->multi_view->on_mouse_move(state.m, state.m - state_prev.m);
		session->cur_mode->on_mouse_move(state.m, state.m - state_prev.m);
	});
	event_x("area", xhui::event_id::MouseWheel, [this] {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		session->cur_mode->multi_view->on_mouse_wheel(state.m, state.scroll);
	});
	event_x("area", xhui::event_id::MouseLeave, [this] {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		session->cur_mode->multi_view->on_mouse_leave();
		session->cur_mode->on_mouse_leave(state.m);
	});
	event_x("area", xhui::event_id::LeftButtonDown, [this] {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		session->cur_mode->multi_view->on_left_button_down(state.m);
		session->cur_mode->on_left_button_down(state.m);
	});
	event_x("area", xhui::event_id::LeftButtonUp, [this] {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		session->cur_mode->multi_view->on_left_button_up(state.m);
		session->cur_mode->on_left_button_up(state.m);
	});
	event_x("area", xhui::event_id::KeyDown, [this] {
		session->cur_mode->multi_view->on_key_down(state.key_code);
		session->cur_mode->on_key_down(state.key_code);
	});
	event_x("cam-move", xhui::event_id::LeftButtonDown, [this] {
		set_mouse_mode(0);
	});
	event_x("cam-move", xhui::event_id::LeftButtonUp, [this] {
		set_mouse_mode(1);
	});
	event_x("cam-move", xhui::event_id::MouseMove, [this] {
		vec2 d = state.m - state_prev.m;
		if (state.lbut) {
			if (is_key_pressed(xhui::KEY_SHIFT))
				session->cur_mode->multi_view->view_port.move(vec3(0,0,d.y) / 800.0f);
			else
				session->cur_mode->multi_view->view_port.move(vec3(-d.x, d.y, 0) / 800.0f);
		}
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
	event("mouse-action", [this] {
		auto& mode = session->cur_mode->multi_view->action_controller->action.mode;
		if (mode == MouseActionMode::MOVE) {
			mode = MouseActionMode::ROTATE;
			set_options("mouse-action", "image=rf-rotate");
		} else if (mode == MouseActionMode::ROTATE) {
			mode = MouseActionMode::SCALE;
			set_options("mouse-action", "image=rf-scale");
		} else if (mode == MouseActionMode::SCALE) {
			mode = MouseActionMode::MOVE;
			set_options("mouse-action", "image=rf-translate");
		}
		set_string("mouse-action", session->cur_mode->multi_view->action_controller->action.name().sub(0, 1).upper());
	});
	event("model_new", [this] {
		session->universal_new(FD_MODEL);
	});
	event("world_new", [this] {
		session->universal_new(FD_WORLD);
	});
	auto quit = [this] {
		if (session->cur_mode->get_data()->action_manager->is_save())
			request_destroy();
		else xhui::QuestionDialog::ask(this, "Question", "You have unsaved changes. Do you want to save?").then([this] (xhui::Answer a) {
			if (a == xhui::Answer::Yes)
				session->storage->auto_save(session->cur_mode->get_data()).then([this] {
					request_destroy();
				});
			else if (a == xhui::Answer::No)
				request_destroy();
		});
	};
	event_x(id, xhui::event_id::Close, quit);
	event("exit", quit);

	xhui::run_repeated(0.5f, [this] {
		request_redraw();
	});
}

void EdwardWindow::on_key_down(int key) {
}

void EdwardWindow::update_menu() {
	if (session->cur_mode->get_data()) {
		enable("undo", session->cur_mode->get_data()->action_manager->undoable());
		enable("redo", session->cur_mode->get_data()->action_manager->redoable());
	}
}



