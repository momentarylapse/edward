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
#include <lib/xhui/controls/MenuBar.h>
#include <lib/xhui/dialogs/FileSelectionDialog.h>
#include <lib/xhui/dialogs/QuestionDialog.h>
#include <lib/yrenderer/Context.h>
#include <sys/stat.h>
#include <y/EngineData.h>

#include "ActionController.h"
#include "DrawingHelper.h"
#include "MultiView.h"
#include "DocumentSession.h"
#include "dialogs/DocumentSwitcher.h"
#include "lib/os/msg.h"
#include "lib/xhui/Theme.h"
#include <lib/yrenderer/Renderer.h>
#include <lib/yrenderer/TextureManager.h>
#include "y/helper/ResourceManager.h"
#include <storage/Storage.h>
#include <stuff/PluginManager.h>
#include "Mode.h"
#include <data/Data.h>
#include "Session.h"
#include <cmath>


extern string AppName;

namespace xhui {
	extern bool color_button_linear;
}

Session* session;

void ExternalModelCleanup(Model *m) {}

namespace yrenderer {
	rect dynamicly_scaled_area(ygfx::FrameBuffer *fb) {
		return {};
	}

	rect dynamicly_scaled_source() {
		return rect::ID;
	}
}

string nice_path(const Path& p) {
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
	string home = getenv("HOME");
	if (str(p).head(home.num) == home)
		return "~" + str(p).sub_ref(home.num);
#endif
	return str(p);
}


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
		auto am = session->cur_doc->cur_mode->get_data()->action_manager;
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

	xhui::color_button_linear = true;

	from_source(R"foodelim(
Dialog x x padding=0
	Grid grid '' spacing=0
		Grid ? '' spacing=0
			MenuBar menu '' main expandx
			MenuBar project-menu '' main
		---|
		Toolbar toolbar '' main expandx
		---|
		Grid main-grid ''
			TabControl tab 'a' nobar
)foodelim");

#ifdef OS_MAC
	int mod = xhui::KEY_SUPER;
	int key_next_doc = xhui::KEY_TAB + xhui::KEY_ALT;//xhui::KEY_CONTROL;
#else
	int mod = xhui::KEY_CONTROL;
	int key_next_doc = xhui::KEY_TAB + xhui::KEY_CONTROL;
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
	set_key_code("next-document", key_next_doc);
	set_key_code("previous-document", key_next_doc + xhui::KEY_SHIFT);

	menu_bar = (xhui::MenuBar*)get_control("menu");
	tool_bar = (xhui::Toolbar*)get_control("toolbar");
	project_menu_bar = (xhui::MenuBar*)get_control("project-menu");

	Array<string> ids = {"new", "open", "save", "save-as", "exit", "undo", "redo", "copy", "paste", "delete"};
	for (const string& id: ids)
		event(id, [this, id=id] {
			session->cur_doc->cur_mode->on_command(id);
		});

	event_xp(id, xhui::event_id::Initialize, [this] (Painter* p) {
		auto pp = (xhui::Painter*)p;
		session->ctx = yrenderer::api_init_xhui(pp);
		session->resource_manager = new ResourceManager(session->ctx, engine.texture_dir, engine.material_dir, engine.shader_dir);
		session->ctx->texture_manager = session->resource_manager->texture_manager;
		session->ctx->shader_manager = session->resource_manager->shader_manager;
		session->ctx->material_manager = session->resource_manager->material_manager;
		session->ctx->shader_manager->default_shader = "default.shader";
		session->drawing_helper = new DrawingHelper(session->ctx, pp->context);
		try {
			session->ctx->load_shader_module("module-basic-data.shader");
			session->ctx->load_shader_module("module-basic-interface.shader");
			session->ctx->load_shader_module("module-vertex-default.shader");
			session->ctx->load_shader_module("module-vertex-animated.shader");
			session->ctx->load_shader_module("module-light-sources-default.shader");
			session->ctx->load_shader_module("module-shadows-pcf.shader");
			session->ctx->load_shader_module("module-lighting-pbr.shader");
			session->ctx->load_shader_module("forward/module-surface.shader");
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
		session->cur_doc->cur_mode->multi_view->select_all();
	});
	event("select_none", [this] {
		session->cur_doc->cur_mode->multi_view->clear_selection();
	});
	event("invert_selection", [this] {
		session->cur_doc->cur_mode->multi_view->invert_selection();
	});
	event("execute-plugin", [this] {
		xhui::FileSelectionDialog::ask(this, "Execute plugin", session->plugin_manager->directory, {}).then( [this] (const Path& path) {
			session->plugin_manager->execute(path);
		});
	});
	event("next-document", [this] {
		if (switcher) {
			switcher->next();
		} else {
			switcher = new DocumentSwitcher(this);
			open_dialog(switcher);
		}
	});
	event("previous-document", [this] {
		if (switcher) {
			switcher->previous();
		} else {
			switcher = new DocumentSwitcher(this);
			open_dialog(switcher);
		}
	});
	auto quit = [this] {
		if (session->cur_doc->cur_mode->get_data()->action_manager->is_save())
			request_destroy();
		else xhui::QuestionDialog::ask(this, "Question", "You have unsaved changes. Do you want to save?").then([this] (xhui::Answer a) {
			if (a == xhui::Answer::Yes)
				session->storage->auto_save(session->cur_doc->cur_mode->get_data()).then([this] {
					request_destroy();
				});
			else if (a == xhui::Answer::No)
				request_destroy();
		});
	};
	event_x(id, xhui::event_id::Close, quit);
	event("exit", quit);

	auto update_project_menu = [this] {
		auto sub = new xhui::Menu;
		if (session->project_dir)
			sub->add_item("project-dir", nice_path(session->project_dir));
		sub->enable("project-dir", false);
		sub->add_item("project-settings", "Project settings..");
		sub->add_item("import-project", "Import project..");
		sub->add_item("create-project", "New project..");
		auto mm = new xhui::Menu;
		if (session->project_dir)
			mm->add_item_menu("project", "[" + session->project_dir.basename() + "]", sub);
		else
			mm->add_item_menu("project", "Project", sub);
		project_menu_bar->set_menu(mm);
	};
	update_project_menu();
	session->out_project_loaded >> create_sink(update_project_menu);

	xhui::run_repeated(0.5f, [this] {
		request_redraw();
	});
}

void EdwardWindow::on_key_down(int key) {
}

void EdwardWindow::on_key_up(int key_code) {
	int key = key_code & 0xff;
	if ((key == xhui::KEY_LCONTROL or key == xhui::KEY_LALT) and switcher) {
		switcher->request_destroy();
		switcher = nullptr;
	}
}

void EdwardWindow::update_menu() {
	if (!session->cur_doc or !session->cur_doc->cur_mode)
		return;
	if (auto d = session->cur_doc->cur_mode->get_data()) {
		enable("undo", d->action_manager->undoable());
		enable("redo", d->action_manager->redoable());

		string prefix = d->action_manager->is_save() ? "" : "*";
		set_title(format("%s%s  - [%s]", prefix, d->filename.relative_to(session->storage->root_dir), nice_path(session->storage->root_dir)));
	}
}



