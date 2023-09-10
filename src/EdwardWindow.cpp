/*
 * EdwardWindow.cpp
 *
 *  Created on: 10 Sept 2023
 *      Author: michi
 */

#include "EdwardWindow.h"
#include "Edward.h"
#include "mode/administration/ModeAdministration.h"
#include "mode/administration/dialog/ConfigurationDialog.h"
#include "mode/model/ModeModel.h"
#include "mode/model/mesh/ModeModelMesh.h"
#include "mode/material/ModeMaterial.h"
#include "mode/world/ModeWorld.h"
#include "mode/font/ModeFont.h"
#include "mode/ModeCreation.h"
#include "mode/ModeNone.h"
#include "multiview/MultiView.h"
#include "multiview/ColorScheme.h"
#include "multiview/DrawingHelper.h"
#include "storage/Storage.h"
#include "stuff/Progress.h"
#include "stuff/PluginManager.h"
#include "y/World.h"
#include "y/Camera.h"
#include "y/ResourceManager.h"
#include "y/EngineData.h"
#include "meta.h"
#include "y/Font.h"
#include "lib/os/config.h"
#include "lib/kaba/kaba.h"
#include "lib/nix/nix.h"
#include "y/ModelManager.h"

extern string AppName;

#ifndef _X_USE_SOUND_
string SoundDir;
#endif

namespace hui {
	extern bool color_button_linear;
}


void EdwardWindow::on_close() {
	allow_termination().on([this] {
		//request_destroy();
		hui::run_later(0.01f, [this] {
			delete this;
		});
	});
}

#define IMPLEMENT_EVENT(EVENT) \
void EdwardWindow::EVENT() { \
	if (cur_mode->multi_view) \
		cur_mode->multi_view->EVENT(); \
	cur_mode->EVENT(); \
}

IMPLEMENT_EVENT(on_mouse_move)
IMPLEMENT_EVENT(on_mouse_wheel)
IMPLEMENT_EVENT(on_mouse_enter)
IMPLEMENT_EVENT(on_mouse_leave)
IMPLEMENT_EVENT(on_left_button_down)
IMPLEMENT_EVENT(on_left_button_up)
IMPLEMENT_EVENT(on_middle_button_down)
IMPLEMENT_EVENT(on_middle_button_up)
IMPLEMENT_EVENT(on_right_button_down)
IMPLEMENT_EVENT(on_right_button_up)

void EdwardWindow::on_key_down()
{
	int key_code = hui::get_event()->key_code;
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_key_down(key_code);
	cur_mode->on_key_down(key_code);
}

void EdwardWindow::on_key_up()
{
	int key_code = hui::get_event()->key_code;
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_key_up(key_code);
	cur_mode->on_key_up(key_code);
}

void EdwardWindow::on_event()
{
	string id = hui::get_event()->id;
	if (id.num == 0)
		id = hui::get_event()->message;
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_command(id);
	cur_mode->on_command_recursive(id);
	on_command(id);
}

void ExternalModelCleanup(Model *m){}

void EdwardWindow::on_abort_creation_mode()
{
	ModeCreationBase *m = dynamic_cast<ModeCreationBase*>(cur_mode);
	if (m)
		m->abort();
}

void EdwardWindow::idle_function()
{
	/*msg_db_f("Idle", 3);

	if (force_redraw)
		onDraw();
	else
		hui::Sleep(0.010f);*/
}

class XWindow : public hui::Window {
public:
	nix::Context *gl = nullptr;
	ResourceManager *resource_manager;
	DrawingHelper *drawing_helper;

	XWindow() : hui::Window("a", 800, 600) {
		event_x("area", hui::EventID::REALIZE, [this] { msg_write("x"); on_realize(); });
		event_x("area", hui::EventID::DRAW_GL, [this] { on_draw_gl(); });

		add_drawing_area("!opengl=4.5", 0, 0, "area");
		show();
	}

	void on_realize() {
		msg_write("realize");
		gl = nix::init();
		resource_manager = new ResourceManager(gl);
		drawing_helper = new DrawingHelper(gl, resource_manager, app->directory_static);
	}

	void on_draw_gl() {
		msg_write("draw");
		if (!gl)
			return;
		auto e = hui::get_event();
		nix::start_frame_hui(gl);
		nix::set_viewport(rect(0, e->column, 0, e->row));

		nix::clear(Red);
		nix::set_z(false, false);
		nix::set_cull(nix::CullMode::NONE);

		msg_write(format("%s  %d  %d", p2s(this), gl->default_framebuffer->frame_buffer, gl->default_3d->program));
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
		drawing_helper->set_font_size(20);
		drawing_helper->draw_str(100, 100, "Test", TextAlign::CENTER);

		nix::end_frame_hui();
	}

};

base::future<EdwardWindow*> emit_session() {
	base::promise<EdwardWindow*> promise;
	auto ed = new EdwardWindow;
	ed->promise_started.get_future().on([promise, ed] () mutable {
		promise(ed);
	});
	return promise.get_future();
}

void test_gl() {
	//auto ww = new XWindow();
	auto ww = new EdwardWindow;
	hui::fly(ww);


	hui::run_later(2, [] {
		auto ww = new XWindow();
		hui::fly(ww);
	});
}

EdwardWindow::EdwardWindow() :
	obs::Node<hui::Window>(AppName, 800, 600),
	in_data_selection_changed(this, [this] {
			cur_mode->multi_view->force_redraw();
			update_menu();
	}),
	in_data_changed(this, [this] {
			cur_mode->on_set_multi_view();
			cur_mode->multi_view->force_redraw();
			update_menu();
	}),
	in_action_failed(this, [this] {
			auto am = cur_mode->get_data()->action_manager;
			error_box(format(_("Action failed: %s\nReason: %s"), am->error_location.c_str(), am->error_message.c_str()));
	}),
	in_saved(this, [this] {
			set_message(_("Saved!"));
			update_menu();
	})
{
	gl = nullptr;
	mode_none = new ModeNone(this);
	cur_mode = mode_none;
	side_panel = nullptr;
	bottom_panel = nullptr;
	progress = new Progress;

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
		if (cur_mode->multi_view)
			cur_mode->multi_view->force_redraw();
	});
	hui::run_later(0.100f, [this] {
		optimize_current_view();
	});
}

EdwardWindow::~EdwardWindow() {
	// auto unsubscribe()...
	set_mode_now(mode_none);

	delete mode_world;
	/*delete mode_material;
	delete mode_model;
	delete mode_font;
	delete mode_admin;*/

	delete multi_view_2d;
	delete multi_view_3d;
	// saving the configuration data...
	int w, h;
	get_size_desired(w, h);
	hui::config.set_int("Window.X", -1);//r.x1);
	hui::config.set_int("Window.Y", -1);//r.y1);
	hui::config.set_int("Window.Width", w);
	hui::config.set_int("Window.Height", h);
	hui::config.set_bool("Window.Maximized", is_maximized());
	hui::config.set_str("RootDir", storage->root_dir.str());
	hui::config.set_str("Language", hui::get_cur_language());
	/*HuiConfig.set_bool("LocalDocumentation", LocalDocumentation);
	HuiConfig.set_str("WorldScriptVarFile", WorldScriptVarFile);
	HuiConfig.set_str("ObjectScriptVarFile", ObjectScriptVarFile);
	HuiConfig.set_str("ItemScriptVarFile", ItemScriptVarFile);*/
	//HuiConfig.set_int("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));
	hui::config.save(app->directory | "config.txt");
	delete storage;


	app->end();
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

void EdwardWindow::optimize_current_view() {
	cur_mode->optimize_view_recursice();
}


// do we change roots?
//  -> data loss?
base::future<void> mode_switch_allowed(ModeBase *m) {
	if (!m->ed->cur_mode or m->equal_roots(m->ed->cur_mode)) {
		base::promise<void> promise;
		promise();
		return promise.get_future();
	} else {
		return m->ed->allow_termination();
	}
}

void EdwardWindow::set_mode(ModeBase *m) {
	if (cur_mode == m)
		return;
	mode_switch_allowed(m).on([this, m] {
		set_mode_now(m);
	});
}

void EdwardWindow::set_mode_now(ModeBase *m) {
	if (cur_mode == m)
		return;

	// recursive use...
	mode_queue.add(m);
	if (mode_queue.num > 1)
		return;

	cur_mode->on_leave();
	if (cur_mode->get_data()) {
		cur_mode->get_data()->unsubscribe(this);
		cur_mode->get_data()->action_manager->unsubscribe(this);
	}

	m = mode_queue[0];
	while (m) {

		// close current modes
		while (cur_mode) {
			if (cur_mode->is_ancestor_of(m))
				break;
			msg_write("end " + cur_mode->name);
			cur_mode->on_end();
			if (cur_mode->multi_view)
				cur_mode->multi_view->pop_settings();
			cur_mode = cur_mode->parent_untyped;
		}

		//multi_view_3d->ResetMouseAction();
		//multi_view_2d->ResetMouseAction();

		// start new modes
		while (cur_mode != m) {
			cur_mode = cur_mode->get_next_child_to(m);
			msg_write("start " + cur_mode->name);
			if (cur_mode->multi_view)
				cur_mode->multi_view->push_settings();
			cur_mode->on_start();
		}
		cur_mode->on_enter();
		cur_mode->on_set_multi_view();

		// nested set calls?
		mode_queue.erase(0);
		m = nullptr;
		if (mode_queue.num > 0)
			m = mode_queue[0];
	}

	set_menu(hui::create_resource_menu(cur_mode->menu_id, this));
	update_menu();
	//cur_mode->on_enter(); // ????
	if (cur_mode->get_data()) {
		cur_mode->get_data()->out_selection >> in_data_selection_changed;
		cur_mode->get_data()->out_changed >> in_data_changed;
		auto *am = cur_mode->get_data()->action_manager;
		am->out_failed >> in_action_failed;
		am->out_saved >> in_saved;
	}

	if (cur_mode->multi_view)
		cur_mode->multi_view->force_redraw();
}

void EdwardWindow::on_about() {
	hui::about_box(this);
}

void EdwardWindow::on_send_bug_report()
{}//	hui::SendBugReport();	}

void EdwardWindow::on_execute_plugin() {
	auto temp = storage->last_dir[FD_SCRIPT];
	storage->last_dir[FD_SCRIPT] = PluginManager::directory;

	storage->file_dialog(FD_SCRIPT, false, false).on([this, temp] (const auto& p) {
		app->plugins->execute(this, p.complete);
		storage->last_dir[FD_SCRIPT] = temp;
	});
}


void EdwardWindow::on_realize_gl() {

	msg_error("REALIZE");

	// initialize engine
	gl = nix::init();
	resource_manager = new ResourceManager(gl);
	drawing_helper = new DrawingHelper(gl, resource_manager, app->directory_static);

	engine.ignore_missing_files = true;
	resource_manager->load_shader("module-vertex-default.shader");
	//ResourceManager::default_shader

	material_manager = new MaterialManager(resource_manager);
	model_manager = new ModelManager(resource_manager, material_manager);
	CameraInit();
	GodInit(0);



	multi_view_3d = new MultiView::MultiView(this, true);
	multi_view_2d = new MultiView::MultiView(this, false);
	mode_model = new ModeModel(this, multi_view_3d, multi_view_2d);
	mode_world = new ModeWorld(this, multi_view_3d);
	mode_font = new ModeFont(this, multi_view_2d);
	mode_admin = new ModeAdministration(this);

	storage = new Storage(this);
	storage->set_root_directory(hui::config.get_str("RootDir", ""));

	mode_material = new ModeMaterial(this, multi_view_3d);


	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;*/

	multi_view_3d->out_settings_changed >> create_sink([this] { update_menu(); });
	multi_view_3d->out_selection_changed >> create_sink([this] {
		cur_mode->on_selection_change();
		update_menu();
	});
	multi_view_3d->out_viewstage_changed >> create_sink([this] {
		cur_mode->on_view_stage_change();
		update_menu();
	});
	multi_view_3d->out_redraw >> create_sink([this] { redraw("nix-area"); });

	multi_view_2d->out_settings_changed >> create_sink([this]{ update_menu(); });
	multi_view_2d->out_selection_changed >> create_sink([this] {
		cur_mode->on_selection_change();
		update_menu();
	});
	multi_view_2d->out_viewstage_changed >> create_sink([this] {
		cur_mode->on_view_stage_change();
		update_menu();
	});
	multi_view_2d->out_redraw >> create_sink([this] {
		redraw("nix-area");
	});

	promise_started();
}

void EdwardWindow::on_draw_gl() {
	auto e = hui::get_event();
	nix::start_frame_hui(gl);
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
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_draw();
	cur_mode->on_draw();

	// messages
	nix::set_shader(gl->default_2d.get());
	foreachi(string &m, message_str, i)
		drawing_helper->draw_str(nix::target_width / 2, nix::target_height / 2 - 20 - i * 20, m, TextAlign::CENTER);

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


void EdwardWindow::remove_message() {
	message_str.erase(0);
	cur_mode->multi_view->force_redraw();
}

void EdwardWindow::set_message(const string &message) {
	msg_write(message);
	message_str.add(message);
	cur_mode->multi_view->force_redraw();
	hui::run_later(2.0f, [this]{ remove_message(); });
}


void EdwardWindow::error_box(const string &message) {
	//set_info_text(message, {"error", "allow-close"});
	hui::error_box(this, _("Error"), message);
}

void EdwardWindow::on_command(const string &id) {
	if (id == "model_new")
		universal_new(FD_MODEL);
	if (id == "model_open")
		universal_open(FD_MODEL);
	if (id == "material_new")
		universal_new(FD_MATERIAL);
	if (id == "material_open")
		universal_open(FD_MATERIAL);
	if (id == "world_new")
		universal_new(FD_WORLD);
	if (id == "world_open")
		universal_open(FD_WORLD);
	if (id == "font_new")
		universal_new(FD_FONT);
	if (id == "font_open")
		universal_open(FD_FONT);
	if (id == "project_new")
		mode_admin->_new();
	if (id == "project_open")
		mode_admin->open();
	if (id == "project_settings") {
		hui::fly(new ConfigurationDialog(this, mode_admin->data, false));
	}
	if (id == "administrate")
		set_mode(mode_admin);
	if (id == "opt_view")
		optimize_current_view();
}

ModeBase *EdwardWindow::get_mode(int preferred_type) {
	if (preferred_type == FD_MODEL)
		return mode_model;
	if (preferred_type == FD_WORLD)
		return mode_world;
	if (preferred_type == FD_MATERIAL)
		return mode_material;
	if (preferred_type == FD_FONT)
		return mode_font;
	return mode_none;
}

void EdwardWindow::universal_new(int preferred_type) {
#if 1
	allow_termination().on([this, preferred_type] {
		/*auto m = get_mode(preferred_type);
		m->_new();
		set_mode(m);
		m->optimize_view();*/
		if (preferred_type == FD_MODEL) {
			mode_model->_new();
			set_mode(mode_model);
			mode_model->mode_model_mesh->optimize_view();
		} else if (preferred_type == FD_WORLD) {
			mode_world->_new();
			set_mode(mode_world);
			mode_world->optimize_view();
		} else if (preferred_type == FD_MATERIAL) {
			mode_material->_new();
			set_mode(mode_material);
			mode_material->optimize_view();
		} else if (preferred_type == FD_FONT) {
			mode_font->_new();
			set_mode(mode_font);
			mode_font->optimize_view();
		}
	});
#else

	msg_error("UNIVERSAL NEW");

	auto ed = new EdwardWindow();
	msg_write("----a");
	if (preferred_type == FD_MODEL) {
		ed->mode_model->_new();
		ed->set_mode(ed->mode_model);
		ed->mode_model->mode_model_mesh->optimize_view();
	} else if (preferred_type == FD_WORLD) {
		ed->mode_world->_new();
		ed->set_mode(ed->mode_world);
		ed->mode_world->optimize_view();
	} else if (preferred_type == FD_MATERIAL) {
		ed->mode_material->_new();
		ed->set_mode(ed->mode_material);
		ed->mode_material->optimize_view();
	} else if (preferred_type == FD_FONT) {
		ed->mode_font->_new();
		ed->set_mode(ed->mode_font);
		ed->mode_font->optimize_view();
	}
	//hui::fly(ed);
#endif
}

void EdwardWindow::universal_open(int preferred_type) {
	storage->file_dialog_x({FD_MODEL, FD_MATERIAL, FD_WORLD}, preferred_type, false, false).on([this] (const auto& p) {
		auto ed = this;//new EdwardWindow;
		if (p.kind == FD_MODEL) {
			ed->storage->load(p.complete, ed->mode_model->data);
			ed->set_mode(ed->mode_model);
			ed->mode_model->mode_model_mesh->optimize_view();
		} else if (p.kind == FD_WORLD) {
			ed->storage->load(p.complete, ed->mode_world->data);
			ed->set_mode(ed->mode_world);
			ed->mode_world->optimize_view();
		} else if (p.kind == FD_MATERIAL) {
			ed->storage->load(p.complete, ed->mode_material->data);
			ed->set_mode(ed->mode_material);
			ed->mode_material->optimize_view();
		}
	});
}

Path add_extension_if_needed(EdwardWindow *ed, int type, const Path &filename) {
	auto e = filename.extension();
	if (e.num == 0)
		return filename.with("." + ed->storage->fd_ext(type));
	return filename;
}

Path make_absolute_path(EdwardWindow *ed, int type, const Path &filename, bool relative_path) {
	if (relative_path)
		return ed->storage->get_root_dir(type) | filename;
	return filename;
}

void EdwardWindow::universal_edit(int type, const Path &_filename, bool relative_path) {
	allow_termination().on([this, type, _filename, relative_path] {
		msg_write("EDIT");
		msg_write(_filename.str());
		Path filename = make_absolute_path(this, type, add_extension_if_needed(this, type, _filename), relative_path);
		msg_write(filename.str());
		switch (type){
			case -1:
				if (filename.basename() == "config.txt")
					hui::open_document(filename);
				else if (filename.basename() == "game.ini")
					mode_admin->basic_settings();
				break;
			case FD_MODEL:
				if (storage->load(filename, mode_model->data, true)) {
					set_mode(mode_model);
					mode_model->mode_model_mesh->optimize_view();
				}
				break;
			case FD_MATERIAL:
				if (storage->load(filename, mode_material->data, true)) {
					set_mode(mode_material);
					mode_material->optimize_view();
				}
				break;
			case FD_FONT:
				if (storage->load(filename, mode_font->data, true))
					set_mode(mode_font);
				break;
			case FD_WORLD:
				if (storage->load(filename, mode_world->data, true)) {
					set_mode(mode_world);
					mode_world->optimize_view();
				}
				break;
			case FD_TERRAIN:
				mode_world->data->reset();
				if (mode_world->data->add_terrain(filename.relative_to(engine.map_dir).no_ext(), v_0)) {
					set_mode(mode_world);
					mode_world->optimize_view();
				}
				break;
			case FD_CAMERAFLIGHT:
				/*mode_world->data->Reset();
				strcpy(mworld->CamScriptFile,a->Name);
				if (mworld->LoadCameraScript()){
					SetMode(ModeWorld);
					mworld->OptimizeView();
				}*/
				break;
			case FD_TEXTURE:
			case FD_SOUND:
			case FD_SHADERFILE:
			case FD_SCRIPT:
			case FD_FILE:
				hui::open_document(filename);
				break;
		}
		//return true;
	});
}


string title_filename(const Path &filename) {
	if (filename)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unknown");
}


void EdwardWindow::update_menu() {
	cur_mode->on_update_menu_recursive();

	Data *d = cur_mode->get_data();
	if (d) {
		enable("undo", d->action_manager->undoable());
		enable("redo", d->action_manager->redoable());
		string title = title_filename(d->filename) + " - " + AppName;
		if (!d->action_manager->is_save())
			title = "*" + title;
		set_title(title);
		if (cur_mode->multi_view)
			enable("view_pop", cur_mode->multi_view->view_stage > 0);
	} else {
		set_title(AppName);
	}

	// general multiview stuff
	MultiView::MultiView *mv = cur_mode->multi_view;
	if (mv) {
		check("whole_window", mv->whole_window);
		check("grid", mv->grid_enabled);
		check("light", mv->light_enabled);
		check("wire", mv->wire_mode);
		check("snap_to_grid", mv->snap_to_grid);
	}
}

base::future<void> EdwardWindow::allow_termination() {
	base::promise<void> promise;

	if (!cur_mode) {
		promise();
		return promise.get_future();
	}
	Data *d = cur_mode->get_data();
	if (!d) {
		promise();
		return promise.get_future();
	}
	if (d->action_manager->is_save()) {
		promise();
		return promise.get_future();
	}
	hui::question_box(this,_("Quite a polite question"),_("You increased entropy. Do you wish to save your work?"), true)
		.on([promise] (bool answer) mutable {
			if (!answer) {
				promise();
			} else {
				//bool saved = cur_mode->save();
				//return saved;
				promise.fail();
			}
		}).on_fail([promise] () mutable {
			promise.fail();
		});
	return promise.get_future();
}

string EdwardWindow::get_tex_image(nix::Texture *tex) {
	if (icon_image.contains(tex))
		return icon_image[tex];

	string img;
	if (tex) {
		Image im;
		tex->read(im);
		auto *small = im.scale(48, 48);
		img = hui::set_image(small);
		delete small;
	} else {
		Image empty;
		empty.create(48, 48, White);
		img = hui::set_image(&empty);
	}
	icon_image.set(tex, img);
	return img;
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

ModeBase *EdwardWindow::find_mode_base(const string &name) {
	if (name == "model")
		return mode_model;
	if (name == "model-mesh")
		return mode_model->mode_model_mesh;
	if (name == "model-mesh-texture")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_texture;
	if (name == "model-mesh-deform")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_deform;
	if (name == "model-mesh-material")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_material;
	if (name == "model-mesh-paint")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_paint;
	if (name == "model-skeleton")
		return (ModeBase*)mode_model->mode_model_skeleton;
	if (name == "model-animation")
		return (ModeBase*)mode_model->mode_model_animation;
	if (name == "material")
		return mode_material;
	if (name == "font")
		return mode_font;
	if (name == "world")
		return mode_world;
	return mode_none;
}



