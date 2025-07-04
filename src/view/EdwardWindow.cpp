//
// Created by michi on 19.01.25.
//

#include "EdwardWindow.h"
#include <lib/xhui/xhui.h>
#include <lib/xhui/Painter.h>
#include <lib/xhui/Context.h>
#include <lib/xhui/Dialog.h>
#include <lib/xhui/controls/DrawingArea.h>
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
#include <storage/Storage.h>
#include <stuff/PluginManager.h>
#include "Mode.h"
#include <data/Data.h>
#include "Session.h"


extern string AppName;


Session* session;

rect dynamicly_scaled_area(FrameBuffer*) { return {}; }
rect dynamicly_scaled_source() { return {}; }
void ExternalModelCleanup(Model *m) {}



EdwardWindow::EdwardWindow(xfer<Session> _session) : obs::Node<xhui::Window>(AppName, 1024, 768),
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
	set_key_code("exit", mod + xhui::KEY_Q);
	set_key_code("undo", mod + xhui::KEY_Z);
	set_key_code("redo", mod + xhui::KEY_Y);
	set_key_code("copy", mod + xhui::KEY_C);
	set_key_code("paste", mod + xhui::KEY_V);
#ifdef OS_MAC
	set_key_code("delete", xhui::KEY_BACKSPACE);
#else
	set_key_code("delete", xhui::KEY_DELETE);
#endif
	set_key_code("select_all", mod + xhui::KEY_A);
	set_key_code("execute-plugin", mod + xhui::KEY_RETURN);

	toolbar = (xhui::Toolbar*)get_control("toolbar");

	Array<string> ids = {"new", "open", "save", "save-as", "exit", "undo", "redo", "copy", "paste", "delete"};
	for (const string& id: ids)
		event(id, [this, id=id] {
			session->cur_mode->on_command(id);
		});

	event_xp(id, xhui::event_id::Initialize, [this] (Painter* p) {
		auto pp = (xhui::Painter*)p;
		session->ctx = api_init_xhui(pp);
		session->resource_manager = new ResourceManager(session->ctx);
		session->resource_manager->default_shader = "default.shader";
		session->resource_manager->texture_dir = engine.texture_dir;
		session->resource_manager->shader_dir = engine.shader_dir;
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

		xhui::run_later(0.01f, [this] {
			session->promise_started(session.get());
		});
	});
	event_xp(id, xhui::event_id::JustBeforeDraw, [this] (Painter* p) {
		if (!session->cur_mode or !session->cur_mode->multi_view)
			return;
		if (auto da = static_cast<xhui::DrawingArea*>(get_control("area")))
			da->for_painter_do(static_cast<xhui::Painter*>(p), [this] (Painter* p) {
				session->cur_mode->multi_view->set_area(p->area());
				renderer->prepare(p);
			});
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
			drawing2d::draw_boxed_str(p, _area.center() + vec2(0, 20*i), session->message_str[i], 0);
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
		auto ac = session->cur_mode->multi_view->action_controller.get();
		const auto mode = ac->action_mode();
		if (mode == MouseActionMode::MOVE) {
			ac->set_action_mode(MouseActionMode::ROTATE);
			set_options("mouse-action", "image=rf-rotate");
		} else if (mode == MouseActionMode::ROTATE) {
			ac->set_action_mode(MouseActionMode::SCALE);
			set_options("mouse-action", "image=rf-scale");
		} else if (mode == MouseActionMode::SCALE) {
			ac->set_action_mode(MouseActionMode::MOVE);
			set_options("mouse-action", "image=rf-translate");
		}
		set_string("mouse-action", session->cur_mode->multi_view->action_controller->action_name().sub(0, 1).upper());
	});
	event("model_new", [this] {
		session->universal_new(FD_MODEL);
	});
	event("material_new", [this] {
		session->universal_new(FD_MATERIAL);
	});
	event("world_new", [this] {
		session->universal_new(FD_WORLD);
	});
	event("select_all", [this] {
		session->cur_mode->multi_view->select_all();
	});
	event("select_none", [this] {
		session->cur_mode->multi_view->clear_selection();
	});
	event("invert_selection", [this] {
		session->cur_mode->multi_view->invert_selection();
	});
	event("execute-plugin", [this] {
		xhui::FileSelectionDialog::ask(this, "Execute plugin", session->plugin_manager->directory, {}).then( [this] (const Path& path) {
			session->plugin_manager->execute(session.get(), path);
		});
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

	renderer = new XhuiRenderer();

	xhui::run_repeated(0.5f, [this] {
		request_redraw();
	});
}

void EdwardWindow::on_key_down(int key) {
}

string nice_path(const Path& p) {
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
	string home = getenv("HOME");
	if (str(p).head(home.num) == home)
		return "~" + str(p).sub_ref(home.num);
#endif
	return str(p);
}

void EdwardWindow::update_menu() {
	if (auto d = session->cur_mode->get_data()) {
		enable("undo", d->action_manager->undoable());
		enable("redo", d->action_manager->redoable());

		string prefix = d->action_manager->is_save() ? "" : "*";
		set_title(format("%s%s  - [%s]", prefix, d->filename.relative_to(session->storage->root_dir), nice_path(session->storage->root_dir)));
	}
}



