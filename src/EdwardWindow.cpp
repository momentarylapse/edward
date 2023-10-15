/*
 * EdwardWindow.cpp
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */

#include "EdwardWindow.h"
#include "Edward.h"
#include "Session.h"
#include "mode/administration/ModeAdministration.h"
#include "mode/administration/dialog/ConfigurationDialog.h"
#include "mode/ModeCreation.h"
#include "mode/ModeNone.h"
#include "multiview/MultiView.h"
#include "multiview/ColorScheme.h"
#include "multiview/DrawingHelper.h"
#include "storage/Storage.h"
#include "stuff/Progress.h"
#include "stuff/PluginManager.h"
#include <y/world/World.h>
#include <y/world/Camera.h>
#include <y/helper/ResourceManager.h>
#include <y/EngineData.h>
#include <y/meta.h>
#include <y/gui/Font.h>
#include "lib/os/config.h"
#include "lib/kaba/kaba.h"
#include "lib/nix/nix.h"

extern string AppName;

#ifndef _X_USE_SOUND_
string SoundDir;
#endif

namespace hui {
	extern bool color_button_linear;
}


void EdwardWindow::on_close() {
	session->allow_termination().then([this] {
		//request_destroy();
		hui::run_later(0.01f, [this] {
			delete this;
		});
	});
}

#define IMPLEMENT_EVENT_V2(EVENT) \
void EdwardWindow::EVENT(const vec2& v) { \
	if (session->cur_mode->multi_view) \
		session->cur_mode->multi_view->EVENT(v); \
	session->cur_mode->EVENT(); \
}

#define IMPLEMENT_EVENT(EVENT) \
void EdwardWindow::EVENT() { \
	if (session->cur_mode->multi_view) \
		session->cur_mode->multi_view->EVENT(); \
	session->cur_mode->EVENT(); \
}

IMPLEMENT_EVENT_V2(on_mouse_move)
IMPLEMENT_EVENT_V2(on_mouse_wheel)
IMPLEMENT_EVENT_V2(on_mouse_enter)
IMPLEMENT_EVENT(on_mouse_leave)
IMPLEMENT_EVENT_V2(on_left_button_down)
IMPLEMENT_EVENT_V2(on_left_button_up)
IMPLEMENT_EVENT_V2(on_middle_button_down)
IMPLEMENT_EVENT_V2(on_middle_button_up)
IMPLEMENT_EVENT_V2(on_right_button_down)
IMPLEMENT_EVENT_V2(on_right_button_up)

void EdwardWindow::on_key_down(int key_code) {
	if (session->cur_mode->multi_view)
		session->cur_mode->multi_view->on_key_down(key_code);
	session->cur_mode->on_key_down(key_code);
}

void EdwardWindow::on_key_up(int key_code) {
	if (session->cur_mode->multi_view)
		session->cur_mode->multi_view->on_key_up(key_code);
	session->cur_mode->on_key_up(key_code);
}

void EdwardWindow::on_event() {
	string id = hui::get_event()->id;
	if (id.num == 0)
		id = hui::get_event()->message;
	if (session->cur_mode->multi_view)
		session->cur_mode->multi_view->on_command(id);
	session->cur_mode->on_command_recursive(id);
	on_command(id);
}

void ExternalModelCleanup(Model *m){}

void EdwardWindow::idle_function()
{
	/*msg_db_f("Idle", 3);

	if (force_redraw)
		onDraw();
	else
		hui::Sleep(0.010f);*/
}

EdwardWindow::EdwardWindow(Session *_session) :
	obs::Node<hui::Window>(AppName, 800, 600),
	in_data_selection_changed(this, [this] {
		session->cur_mode->multi_view->force_redraw();
		update_menu();
	}),
	in_data_changed(this, [this] {
		session->cur_mode->on_set_multi_view();
		session->cur_mode->multi_view->force_redraw();
		update_menu();
	}),
	in_action_failed(this, [this] {
		auto am = session->cur_mode->get_data()->action_manager;
		session->error(format(_("Action failed: %s\nReason: %s"), am->error_location.c_str(), am->error_message.c_str()));
	}),
	in_saved(this, [this] {
		session->set_message(_("Saved!"));
		update_menu();
	})
{
	session = _session;
	side_panel = nullptr;
	bottom_panel = nullptr;

	event_x("nix-area", hui::EventID::DRAW_GL, [this] { on_draw_gl(); });
	event_x("nix-area", hui::EventID::REALIZE, [this] { on_realize_gl(); });

	set_border_width(0);
	add_basic_layout("menubar|toolbar-top|toolbar-left");
	add_grid("", 0, 0, "vgrid");
	set_target("vgrid");
	add_grid("", 0, 0, "root-table");
	set_target("root-table");
	add_drawing_area("!grabfocus,opengl=4.5", 0, 0, "nix-area");
	add_expander("!slide=left", 1, 0, "side-bar-revealer");
	set_target("side-bar-revealer");
	add_grid("!noexpandx,width=360", 0, 0, "side-bar-grid");
	expand("side-bar-revealer", false);
	set_border_width(5);
	show();


	add_action_checkable("light");
	add_action_checkable("wire");
	add_action_checkable("grid");
	add_action_checkable("whole_window");
	add_action_checkable("show_fx");
	add_action_checkable("snap_to_grid");

	add_action_checkable("mode_model_vertex");
	add_action_checkable("mode_model_polygon");
	add_action_checkable("mode_model_texture_coord");
	add_action_checkable("mode_model_deform");
	add_action_checkable("mode_model_skeleton");
	add_action_checkable("mode_model_animation");
	add_action_checkable("select_cw");
	add_action_checkable("rotate");
	add_action_checkable("scale");
	add_action_checkable("mirror");
	add_action_checkable("detail_physical");
	add_action_checkable("detail_high");
	add_action_checkable("detail_2");
	add_action_checkable("detail_3");

	hui::color_button_linear = true;

	load_key_codes();

	// configuration
	int w = hui::config.get_int("Window.Width", 800);
	int h = hui::config.get_int("Window.Height", 600);
	bool maximized = hui::config.get_bool("Window.Maximized", false);
	set_size(w, h);

	//HuiConfigread_int("Api", api, NIX_API_OPENGL);
	/*bool LocalDocumentation = HuiConfig.get_bool("LocalDocumentation", false);
	WorldScriptVarFile = HuiConfig.get_str("WorldScriptVarFile", "");
	ObjectScriptVarFile = HuiConfig.get_str("ObjectScriptVarFile", "");
	ItemScriptVarFile = HuiConfig.get_str("ItemScriptVarFile", "");

	LoadKeyCodes();
	SaveKeyCodes();

	int n = dir_search(HuiAppDirectory + "Data/", "*.txt", false);
	for (int i=0;i<n;i++)
		if (dir_search_name[i].find("scriptvars_") >= 0)
			ScriptVarFile[NumScriptVarFiles ++] = dir_search_name[i];
	LoadScriptVarNames(0, "");
	LoadScriptVarNames(1, "");
	LoadScriptVarNames(2, "");*/

	// create the main window
	set_maximized(maximized);

	get_toolbar(hui::TOOLBAR_TOP)->configure(false, true);
	get_toolbar(hui::TOOLBAR_LEFT)->configure(false, true);


	event("hui:close", [this] { on_close(); });
	event("exit", [this] { on_close(); });
	event("*", [this] { on_event(); });
	event("what_the_fuck", [this] { on_about(); });
	event("send_bug_report", [this] { on_send_bug_report(); });
	event("execute_plugin", [this] { on_execute_plugin(); });
	event("abort_creation_mode", [this] { on_abort_creation_mode(); });
	set_key_code("abort_creation_mode", hui::KEY_ESCAPE, "hui:cancel");


	hui::run_later(0.010f, [this] {
		if (session->cur_mode->multi_view)
			session->cur_mode->multi_view->force_redraw();
	});
	hui::run_later(0.100f, [this] {
		optimize_current_view();
	});
}

EdwardWindow::~EdwardWindow() {
	// auto unsubscribe()...
	session->set_mode_now(session->mode_none);

	// saving the configuration data...
	int w, h;
	get_size_desired(w, h);
	hui::config.set_int("Window.X", -1);//r.x1);
	hui::config.set_int("Window.Y", -1);//r.y1);
	hui::config.set_int("Window.Width", w);
	hui::config.set_int("Window.Height", h);
	hui::config.set_bool("Window.Maximized", is_maximized());
}

#if 0
bool EdwardWindow::handle_arguments(Array<string> arg)
{
	if (arg.num < 2)
		return false;


	if (arg[1] == "--new-material") {
		universal_new(FD_MATERIAL);
		return true;
	} else if (arg[1] == "--new-world") {
		universal_new(FD_WORLD);
		return true;
	} else if (arg[1] == "--new-font") {
		universal_new(FD_FONT);
		return true;
	}

	for (int i=1; i<arg.num; i++){
		string param = arg[i];

// convert file types...

	/*if (param == "-cftmodel"){		ConvertFileFormat(FDModel, true);		End();	}
	if (param == "-cftobject"){	ConvertFileFormat(FDObject, true);		End();	}
	if (param == "-cftitem"){		ConvertFileFormat(FDItem, true);		End();	}
	if (param == "-cftmaterial"){	ConvertFileFormat(FDMaterial, true);	End();	}
	if (param == "-cftmap"){		ConvertFileFormat(FDTerrain, true);		End();	}
	if (param == "-cftworld"){		ConvertFileFormat(FDWorld, true);		End();	}
// test files
	if (param == "-tftmodel"){		ConvertFileFormat(FDModel, false);		End();	}
	if (param == "-tftobject"){	ConvertFileFormat(FDObject, false);		End();	}
	if (param == "-tftitem"){		ConvertFileFormat(FDItem, false);		End();	}
	if (param == "-tftmaterial"){	ConvertFileFormat(FDMaterial, false);	End();	}
	if (param == "-tftmap"){		ConvertFileFormat(FDTerrain, false);	End();	}
	if (param == "-tftworld"){		ConvertFileFormat(FDWorld, false);		End();	}*/

		if (param == "--execute"){
			i ++;
			if (i < arg.num){
				plugins->execute(arg[i]);
			}
			continue;
		}

// loading...
	if (param[0]=='"')
		param.delete_single(0);
	if (param[param.num-1]=='"')
		param.resize(param.num-1);

	int type = storage->guess_type(param);

	if (type >= 0) {
		universal_edit(type, param, false);
	} else {
		error_box(_("Unknown file extension: ") + param);
		app->end();
	}
	}
	return true;
}
#endif


void EdwardWindow::on_abort_creation_mode() {
	ModeCreationBase *m = dynamic_cast<ModeCreationBase*>(session->cur_mode);
	if (m)
		m->abort();
}

void EdwardWindow::optimize_current_view() {
	session->cur_mode->optimize_view_recursice();
}


void EdwardWindow::on_about() {
	hui::about_box(this);
}

void EdwardWindow::on_send_bug_report()
{}//	hui::SendBugReport();	}


void EdwardWindow::on_realize_gl() {
	auto gl = nix::init();
	session->create_initial_resources(gl);
}

void EdwardWindow::on_draw_gl() {
	auto e = hui::get_event();
	nix::start_frame_hui(session->gl);
	nix::set_viewport(rect(0, e->column, 0, e->row));

#if 0

	nix::clear(Green);
	nix::set_z(false, false);
	nix::set_cull(nix::CullMode::NONE);

	nix::set_projection_perspective();
	nix::set_view_matrix(mat4::ID);
	nix::set_model_matrix(mat4::translation(vec3(0,0,5)));
	nix::set_shader(gl->default_3d.get());
	nix::set_material(White, 0, 0, White);
	nix::bind_texture(0, drawing_helper->tex_white.get());

	gl->vb_temp->create_cube(vec3(-1, -1, -1), vec3(1, 1, 1));
	nix::draw_triangles(gl->vb_temp);

	nix::set_projection_ortho_pixel();
	nix::set_shader(gl->default_2d.get());
	drawing_helper->set_color(Black);
	drawing_helper->draw_str(100, 100, "Test", TextAlign::CENTER);
#endif

	//nix::set_srgb(true);
	if (session->cur_mode->multi_view)
		session->cur_mode->multi_view->on_draw();
	session->cur_mode->on_draw();

	// messages
	nix::set_shader(session->gl->default_2d.get());
	foreachi(string &m, session->message_str, i)
	session->drawing_helper->draw_str(nix::target_width / 2, nix::target_height / 2 - 20 - i * 20, m, TextAlign::CENTER);

	nix::end_frame_hui();
}

void EdwardWindow::load_key_codes() {
	Configuration con;

	// first installed version
	con.load(app->directory_static | "keys.txt");
	for (auto &id: con.map.keys())
		set_key_code(id, hui::parse_key_code(con.get_str(id, "")));

	// then override by user keys
	con.load(app->directory | "keys.txt");
	for (auto &id: con.map.keys())
		set_key_code(id, hui::parse_key_code(con.get_str(id, "")));
}

void EdwardWindow::on_command(const string &id) {
	if (id == "model_new")
		session->universal_new(FD_MODEL);
	if (id == "model_open")
		session->universal_open(FD_MODEL);
	if (id == "material_new")
		session->universal_new(FD_MATERIAL);
	if (id == "material_open")
		session->universal_open(FD_MATERIAL);
	if (id == "world_new")
		session->universal_new(FD_WORLD);
	if (id == "world_open")
		session->universal_open(FD_WORLD);
	if (id == "font_new")
		session->universal_new(FD_FONT);
	if (id == "font_open")
		session->universal_open(FD_FONT);
	if (id == "project_new")
		session->mode_admin->_new();
	if (id == "project_open")
		session->mode_admin->open();
	if (id == "project_settings") {
		hui::fly(new ConfigurationDialog(this, session->mode_admin->data, false));
	}
	if (id == "administrate")
		session->set_mode(session->mode_admin);
	if (id == "opt_view")
		optimize_current_view();
}


string title_filename(const Path &filename) {
	if (filename)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unknown");
}


void EdwardWindow::update_menu() {
	session->cur_mode->on_update_menu_recursive();

	Data *d = session->cur_mode->get_data();
	if (d) {
		enable("undo", d->action_manager->undoable());
		enable("redo", d->action_manager->redoable());
		string title = title_filename(d->filename) + " - " + AppName;
		if (!d->action_manager->is_save())
			title = "*" + title;
		set_title(title);
		if (session->cur_mode->multi_view)
			enable("view_pop", session->cur_mode->multi_view->view_stage > 0);
	} else {
		set_title(AppName);
	}

	// general multiview stuff
	MultiView::MultiView *mv = session->cur_mode->multi_view;
	if (mv) {
		check("whole_window", mv->whole_window);
		check("grid", mv->grid_enabled);
		check("light", mv->light_enabled);
		check("wire", mv->wire_mode);
		check("snap_to_grid", mv->snap_to_grid);
	}
}

void EdwardWindow::set_side_panel(shared<hui::Panel> panel) {
	if (side_panel) {
		// close
		expand("side-bar-revealer", false);
		unembed(side_panel.get());
		side_panel = nullptr;
	}
	if (panel) {
		// open
		side_panel = panel;
		embed(panel, "side-bar-grid", 0, 0);
		expand("side-bar-revealer", true);
	}
}

void EdwardWindow::set_bottom_panel(shared<hui::Panel> panel) {
	if (bottom_panel) {
		// close
		//reveal("side-bar-revealer", false);
		unembed(bottom_panel.get());
		bottom_panel = nullptr;
	}
	if (panel) {
		// open
		bottom_panel = panel;
		embed(panel.get(), "vgrid", 0, 1);
		//reveal("side-bar-revealer", true);
	}
}

void EdwardWindow::on_execute_plugin() {
	auto temp = session->storage->last_dir[FD_SCRIPT];
	session->storage->last_dir[FD_SCRIPT] = PluginManager::directory;

	session->storage->file_dialog(FD_SCRIPT, false, false).then([this, temp] (const auto& p) {
		app->plugins->execute(session, p.complete);
		session->storage->last_dir[FD_SCRIPT] = temp;
	}).on_fail([this, temp] {
		session->storage->last_dir[FD_SCRIPT] = temp;
	});
}



