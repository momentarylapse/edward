/*
 * Edward.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"

#include "Edward.h"
#include "Mode/Administration/ModeAdministration.h"
#include "Mode/Model/ModeModel.h"
#include "Mode/Model/Mesh/ModeModelMesh.h"
#include "Mode/Material/ModeMaterial.h"
#include "Mode/World/ModeWorld.h"
#include "Mode/Font/ModeFont.h"
#include "Mode/ModeCreation.h"
#include "Mode/ModeNone.h"
#include "MultiView/MultiView.h"
#include "MultiView/ColorScheme.h"
#include "MultiView/DrawingHelper.h"
#include "Storage/Storage.h"
#include "x/world.h"
#include "x/camera.h"
#include "meta.h"
#include "x/model_manager.h"
#include "x/font.h"
#include "lib/kaba/kaba.h"
#include "lib/nix/nix.h"

Edward *ed = NULL;
EdwardApp *app = NULL;

#ifndef _X_USE_SOUND_
string SoundDir;
#endif

extern string AppName;



void read_color_4i(File *f,int *c)
{
	// argb (file) -> rgba (editor)
	c[3] = f->read_int();
	c[0] = f->read_int();
	c[1] = f->read_int();
	c[2] = f->read_int();
}

void write_color_4i(File *f,int *c)
{
	// rgba (editor) -> argb (file)
	f->write_int(c[3]);
	f->write_int(c[0]);
	f->write_int(c[1]);
	f->write_int(c[2]);
}

void write_color_rgba(File *f, const color &c)
{
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
	f->write_int((int)(c.a * 255.0f));
}

void read_color_rgba(File *f, color &c)
{
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
	c.a = (float)f->read_int() / 255.0f;
}

void write_color_argb(File *f, const color &c)
{
	f->write_int((int)(c.a * 255.0f));
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
}

void read_color_argb(File *f, color &c)
{
	c.a = (float)f->read_int() / 255.0f;
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
}

void write_color_3i(File *f, const color &c)
{
	f->write_int((int)(c.r * 255.0f));
	f->write_int((int)(c.g * 255.0f));
	f->write_int((int)(c.b * 255.0f));
}

void read_color_3i(File *f, color &c)
{
	c.r = (float)f->read_int() / 255.0f;
	c.g = (float)f->read_int() / 255.0f;
	c.b = (float)f->read_int() / 255.0f;
	c.a = 1;
}

color i42c(int *c)
{
	return color(float(c[3])/255.0f,float(c[0])/255.0f,float(c[1])/255.0f,float(c[2])/255.0f);
}

void Edward::on_close()
{
	if (allow_termination())
		exit();
}

void Edward::exit()
{
	destroy();
	app->end();
}

#define IMPLEMENT_EVENT(EVENT) \
void Edward::EVENT() \
{ \
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

void Edward::on_key_down()
{
	int key_code = hui::GetEvent()->key_code;
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_key_down(key_code);
	cur_mode->on_key_down(key_code);
}

void Edward::on_key_up()
{
	int key_code = hui::GetEvent()->key_code;
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_key_up(key_code);
	cur_mode->on_key_up(key_code);
}

void Edward::on_event()
{
	string id = hui::GetEvent()->id;
	if (id.num == 0)
		id = hui::GetEvent()->message;
	if (cur_mode->multi_view)
		cur_mode->multi_view->on_command(id);
	cur_mode->on_command_recursive(id);
	on_command(id);
}

void Edward::on_abort_creation_mode()
{
	ModeCreationBase *m = dynamic_cast<ModeCreationBase*>(cur_mode);
	if (m)
		m->abort();
}

void Edward::idle_function()
{
	/*msg_db_f("Idle", 3);

	if (force_redraw)
		onDraw();
	else
		hui::Sleep(0.010f);*/
}

Edward::Edward(Array<string> arg) :
	Observer("Edward"),
	hui::Window(AppName, 800, 600)
{
	set_border_width(0);
	add_grid("", 0, 0, "vgrid");
	set_target("vgrid");
	add_grid("", 0, 0, "root-table");
	set_target("root-table");
	add_drawing_area("!grabfocus,opengl", 0, 0, "nix-area");
	add_revealer("!slide=left", 1, 0, "side-bar-revealer");
	set_target("side-bar-revealer");
	add_grid("!noexpandx,width=360", 0, 0, "side-bar-grid");
	reveal("side-bar-revealer", false);
	set_border_width(5);
	show();

	ed = this;
	no_mode = new ModeNone;
	cur_mode = no_mode;
	side_panel = nullptr;
	prev_side_panel = nullptr;

	progress = new Progress;

	load_key_codes();

	possible_sub_dir.add("Fonts");
	possible_sub_dir.add("Maps");
	possible_sub_dir.add("Materials");
	possible_sub_dir.add("Objects");
	possible_sub_dir.add("Scripts");
	possible_sub_dir.add("Sounds");
	possible_sub_dir.add("Textures");

	// configuration
	int w = hui::Config.get_int("Window.Width", 800);
	int h = hui::Config.get_int("Window.Height", 600);
	bool maximized = hui::Config.get_bool("Window.Maximized", false);
	set_size(w, h);
	root_dir = hui::Config.get_str("RootDir", "");
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

	// initialize engine
	nix::Init("OpenGL", w, h);
	nix::render_str = &render_text;

	MetaInit();
	CameraInit();
	GodInit();

	/*RegisterFileTypes();

	for (int i=0;i<NumFDs;i++)
		DialogDir[i] = "";
	for (int i=0;i<4;i++){
		BgTextureFile[i] = "";
		BgTexture[i] = -1;
	}

	msg_db_r("init modes", 1);*/


	ed->toolbar[hui::TOOLBAR_TOP]->configure(false, true);
	ed->toolbar[hui::TOOLBAR_LEFT]->configure(false, true);

	multi_view_3d = new MultiView::MultiView(true);
	multi_view_2d = new MultiView::MultiView(false);
	mode_model = new ModeModel;
	mode_material = new ModeMaterial;
	mode_world = new ModeWorld;
	mode_font = new ModeFont;
	mode_administration = new ModeAdministration;

	storage = new Storage();

	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;*/

	make_dirs(root_dir,true);

	// subscribe to all data to automatically redraw...
	subscribe(mode_model->data);
	subscribe(mode_material->data);
	subscribe(mode_world->data);
	subscribe(mode_font->data);
	subscribe(multi_view_2d);
	subscribe(multi_view_3d);

	plugins = new PluginManager();

	if (!handle_arguments(arg))
		mode_model->_new();


	event("hui:close", std::bind(&Edward::on_close, this));
	event("exit", std::bind(&Edward::on_close, this));
	event("*", std::bind(&Edward::on_event, this));
	event("what_the_fuck", std::bind(&Edward::on_about, this));
	event("send_bug_report", std::bind(&Edward::on_send_bug_report, this));
	event("execute_plugin", std::bind(&Edward::on_execute_plugin, this));
	event("abort_creation_mode", std::bind(&Edward::on_abort_creation_mode, this));
	event_x("nix-area", "hui:draw-gl", std::bind(&Edward::on_draw_gl, this));
	set_key_code("abort_creation_mode", hui::KEY_ESCAPE, "hui:cancel");

	//hui::SetIdleFunction(std::bind(&Edward::idleFunction, this));
	hui::RunLater(0.010f, [=]{ cur_mode->multi_view->force_redraw(); });
	hui::RunLater(0.100f, [=]{ optimize_current_view(); });
}

Edward::~Edward()
{
}

void Edward::on_destroy()
{
	delete plugins;
	delete multi_view_2d;
	delete multi_view_3d;
	// saving the configuration data...
	int w, h;
	get_size_desired(w, h);
	hui::Config.set_int("Window.X", -1);//r.x1);
	hui::Config.set_int("Window.Y", -1);//r.y1);
	hui::Config.set_int("Window.Width", w);
	hui::Config.set_int("Window.Height", h);
	hui::Config.set_bool("Window.Maximized", is_maximized());
	hui::Config.set_str("RootDir", root_dir);
	hui::Config.set_str("Language", hui::GetCurLanguage());
	/*HuiConfig.set_bool("LocalDocumentation", LocalDocumentation);
	HuiConfig.set_str("WorldScriptVarFile", WorldScriptVarFile);
	HuiConfig.set_str("ObjectScriptVarFile", ObjectScriptVarFile);
	HuiConfig.set_str("ItemScriptVarFile", ItemScriptVarFile);*/
	//HuiConfig.set_int("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));
	hui::Config.save();
}

bool Edward::handle_arguments(Array<string> arg)
{
	if (arg.num < 2)
		return false;


	if (arg[1] == "--update-model"){
		if (arg.num >= 3){
			DataModel m;
			storage->load(arg[2], &m, false);
			//m.save(arg[2]);
			::exit(0);
		}
		return false;
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

	string ext = param.extension();

	if (ext == "model"){
		make_dirs(param);
		storage->load(param, mode_model->data);
		set_mode(mode_model);
		/*if (mmodel->Skin[1].Sub[0].Triangle.num==0)
			mmodel->SetEditMode(EditModeVertex);*/
	}else if (ext == "material"){
		make_dirs(param);
		storage->load(param, mode_material->data);
		set_mode(mode_material);
	/*}else if ((ext == "map") || (ext == "terrain")){
		MakeDirs(param);
		mworld->Terrain.resize(1);
		mworld->LoadFromFileTerrain(0, v0, param, true);
		mworld->OptimizeView();
		set_mode(ModeWorld);*/
	}else if (ext == "world"){
		make_dirs(param);
		storage->load(param, mode_world->data);
		set_mode(mode_world);
		multi_view_3d->whole_window = true;
	}else if (ext == "xfont"){
		make_dirs(param);
		storage->load(param, mode_font->data);
		set_mode(mode_font);
	}else if (ext == "js"){
		storage->load(param, mode_model->data);
		set_mode(mode_model);
	}else if (ext == "ply"){
		storage->load(param, mode_model->data);
		set_mode(mode_model);
	/*}else if (ext == "mdl"){
		mmodel->LoadImportFromGameStudioMdl(param);
		set_mode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();
		//mmodel->Changed=false;
	}else if (ext == "wmb"){
		mmodel->LoadImportFromGameStudioWmb(param);
		set_mode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();*/
	}else if (ext == "3ds"){
		storage->load(param, mode_model->data);
	}else{
		error_box(_("Unknown file extension: ") + param);
		app->end();
	}
	}
	return true;
}

void Edward::optimize_current_view()
{
	cur_mode->optimize_view_recursice();
}


// do we change roots?
//  -> data loss?
bool mode_switch_allowed(ModeBase *m)
{
	if (m->equal_roots(ed->cur_mode))
		return true;
	return ed->allow_termination();
}

void Edward::set_mode(ModeBase *m)
{
	if (cur_mode == m)
		return;
	if (!mode_switch_allowed(m))
		return;

	// recursive use...
	mode_queue.add(m);
	if (mode_queue.num > 1)
		return;

	cur_mode->on_leave();
	if (cur_mode->get_data())
		unsubscribe(cur_mode->get_data()->action_manager);

	m = mode_queue[0];
	while(m){

		// close current modes
		while(cur_mode){
			if (cur_mode->is_ancestor_of(m))
				break;
			msg_write("end " + cur_mode->name);
			cur_mode->on_end();
			if (cur_mode->multi_view)
				cur_mode->multi_view->pop_settings();
			cur_mode = cur_mode->parent;
		}

		//multi_view_3d->ResetMouseAction();
		//multi_view_2d->ResetMouseAction();

		// start new modes
		while(cur_mode != m){
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
		m = NULL;
		if (mode_queue.num > 0)
			m = mode_queue[0];
	}

	set_menu(hui::CreateResourceMenu(cur_mode->menu_id));
	update_menu();
	cur_mode->on_enter(); // ????
	if (cur_mode->get_data())
		subscribe(cur_mode->get_data()->action_manager);

	cur_mode->multi_view->force_redraw();
}

void Edward::on_about()
{	hui::AboutBox(this);	}

void Edward::on_send_bug_report()
{}//	hui::SendBugReport();	}

void Edward::on_update(Observable *o, const string &message)
{
	//msg_write(o->getName() + " - " + message);
	if (o->get_name() == "MultiView"){
		if (message == multi_view_3d->MESSAGE_SETTINGS_CHANGE){
			update_menu();
		}else if (message == multi_view_3d->MESSAGE_SELECTION_CHANGE){
			cur_mode->on_selection_change();
			update_menu();
		}else if (message == multi_view_3d->MESSAGE_VIEWSTAGE_CHANGE){
			cur_mode->on_view_stage_change();
			update_menu();
		}
		cur_mode->multi_view->force_redraw();
	}else if (o->get_name() == "ActionManager"){
		ActionManager *am = dynamic_cast<ActionManager*>(o);
		if (message == am->MESSAGE_FAILED){
			error_box(format(_("Action failed: %s\nReason: %s"), am->error_location.c_str(), am->error_message.c_str()));
		}else if (message == am->MESSAGE_SAVED){
			set_message(_("Saved!"));
			update_menu();
		}
	}else if (dynamic_cast<Data*>(o)){
		if (message != Data::MESSAGE_SELECTION)
			cur_mode->on_set_multi_view();
		// data...
		cur_mode->multi_view->force_redraw();
		//if (message != o->MESSAGE_CHANGE)
		update_menu();
	}else{
		cur_mode->multi_view->force_redraw();
		update_menu();
	}
}

void Edward::on_execute_plugin()
{
	string temp = dialog_dir[FD_SCRIPT];
	if (app->installed)
		dialog_dir[FD_SCRIPT] = app->directory_static + "Plugins/";
	else
		dialog_dir[FD_SCRIPT] = app->directory + "Plugins/";

	if (file_dialog(FD_SCRIPT, false, false))
		plugins->execute(dialog_file_complete);
	dialog_dir[FD_SCRIPT] = temp;
}


void Edward::on_draw_gl()
{
	auto e = hui::GetEvent();
	nix::Resize(e->column, e->row);

	if (cur_mode->multi_view)
		cur_mode->multi_view->on_draw();
	cur_mode->on_draw();

	// messages
	nix::SetShader(nix::default_shader_2d);
	foreachi(string &m, message_str, i)
		draw_str(nix::target_width / 2, nix::target_height / 2 - 20 - i * 20, m, TextAlign::CENTER);
}



void Edward::load_key_codes()
{
	File *f = NULL;

	try{
		try{
			f = FileOpenText(app->directory + "keys.txt");
		}catch(...){
			f = FileOpenText(app->directory_static + "keys.txt");
		}
		f->read_comment();
		int nk = f->read_int();
		f->read_comment();
		for (int i=0; i<nk; i++){
			string id = f->read_str();
			int key_code = f->read_int();
			if (id == "execute_plugin")
				key_code = hui::KEY_CONTROL + hui::KEY_P;
			if (id == "move_frame_inc")
				key_code = hui::KEY_CONTROL + hui::KEY_RIGHT;
			if (id == "move_frame_dec")
				key_code = hui::KEY_CONTROL + hui::KEY_LEFT;
			set_key_code(id, key_code);
		}
		event("volume_subtract", NULL);
		set_key_code("volume_subtract", hui::KEY_CONTROL + hui::KEY_J); // TODO ...
		event("invert_selection", NULL);
		set_key_code("invert_selection", hui::KEY_CONTROL + hui::KEY_TAB); // TODO ...
		event("select_all", NULL);
		set_key_code("select_all", hui::KEY_CONTROL + hui::KEY_A); // TODO ...

		event("finish-action", NULL);
		set_key_code("finish-action", hui::KEY_CONTROL + hui::KEY_RETURN); // TODO ...

		event("easify_skin", NULL);
		set_key_code("easify_skin", hui::KEY_CONTROL + hui::KEY_7); // TODO ...

		FileClose(f);
	}catch(...){

	}
}


void Edward::update_dialog_dir(int kind)
{
	if (kind==FD_MODEL)			root_dir_kind[kind] = ObjectDir;
	if (kind==FD_MODEL)			root_dir_kind[kind] = ObjectDir;
	if (kind==FD_TEXTURE)		root_dir_kind[kind] = nix::texture_dir;
	if (kind==FD_SOUND)			root_dir_kind[kind] = SoundDir;
	if (kind==FD_MATERIAL)		root_dir_kind[kind] = MaterialDir;
	if (kind==FD_TERRAIN)		root_dir_kind[kind] = MapDir;
	if (kind==FD_WORLD)			root_dir_kind[kind] = MapDir;
	if (kind==FD_SHADERFILE)		root_dir_kind[kind] = MaterialDir;
	if (kind==FD_FONT)			root_dir_kind[kind] = Gui::FontDir;
	if (kind==FD_SCRIPT)			root_dir_kind[kind] = ScriptDir;
	if (kind==FD_CAMERAFLIGHT)	root_dir_kind[kind] = ScriptDir;
	if (kind==FD_FILE)			root_dir_kind[kind] = root_dir;
}


void Edward::set_root_directory(const string &directory, bool compact_mode)
{
	string object_dir, map_dir, texture_dir, sound_dir, script_dir, material_dir, font_dir;
	bool ufd = (root_dir.find(directory) < 0) and (directory.find(root_dir) < 0);
	root_dir = directory;
	root_dir.dir_ensure_ending();
	if (compact_mode){
		map_dir = root_dir;
		object_dir = root_dir;
		texture_dir = root_dir;
		sound_dir = root_dir;
		script_dir = root_dir;
		material_dir = root_dir;
		font_dir = root_dir;
	}else{
		map_dir = root_dir + "Maps/";
		object_dir = root_dir + "Objects/";
		texture_dir = root_dir + "Textures/";
		sound_dir = root_dir + "Sounds/";
		script_dir = root_dir + "Scripts/";
		material_dir = root_dir + "Materials/";
		font_dir = root_dir + "Fonts/";
	}
	MetaSetDirs(texture_dir, map_dir, object_dir, sound_dir, script_dir, material_dir, font_dir);
#ifndef _X_USE_SOUND_
	SoundDir = sound_dir;
#endif
	if (ufd)
		for (int i=0;i<NUM_FDS;i++){
			dialog_dir[i] = "";
			update_dialog_dir(i);
		}
}


void Edward::make_dirs(const string &original_dir, bool as_root_dir)
{
	string dir = original_dir;
	if (dir.num > 0)
		dir = dir.dirname();
	bool sub_dir = false;
	bool root_dir_correct = false;
	if (!as_root_dir){
		// we are in a sub dir?
		sub_dir=false;
		for (string &p: possible_sub_dir){
			if (dir.find(p) >= 0){
				dir = dir.substr(0, dir.find(p));
				sub_dir=true;
				break;
			}
		}
		root_dir_correct = sub_dir;
		root_dir_correct &= file_test_existence(dir + "game.ini");
	}else{
		root_dir_correct = file_test_existence(dir + "game.ini");
	}
	set_root_directory(dir, !root_dir_correct);
}


string Edward::get_root_dir(int kind)
{
	if (kind==-1)				return root_dir;
	if (kind==FD_MODEL)			return ObjectDir;
	if (kind==FD_TEXTURE)		return nix::texture_dir;
	if (kind==FD_SOUND)			return SoundDir;
	if (kind==FD_MATERIAL)		return MaterialDir;
	if (kind==FD_TERRAIN)		return MapDir;
	if (kind==FD_WORLD)			return MapDir;
	if (kind==FD_SHADERFILE)		return MaterialDir;
	if (kind==FD_FONT)			return Gui::FontDir;
	if (kind==FD_SCRIPT)			return ScriptDir;
	if (kind==FD_CAMERAFLIGHT)	return ScriptDir;
	if (kind==FD_FILE)			return root_dir;
	return root_dir;
}

void Edward::remove_message()
{
	message_str.erase(0);
	cur_mode->multi_view->force_redraw();
}

void Edward::set_message(const string &message)
{
	msg_write(message);
	message_str.add(message);
	cur_mode->multi_view->force_redraw();
	hui::RunLater(2.0f, std::bind(&Edward::remove_message, this));
}


void Edward::error_box(const string &message) {
	set_info_text(message, {"error", "allow-close"});
	//hui::ErrorBox(this, _("Error"), message);
}

void Edward::on_command(const string &id)
{
	if (id == "model_new")
		mode_model->_new();
	if (id == "model_open")
		mode_model->open();
	if (id == "material_new")
		mode_material->_new();
	if (id == "material_open")
		mode_material->open();
	if (id == "world_new")
		mode_world->_new();
	if (id == "world_open")
		mode_world->open();
	if (id == "font_new")
		mode_font->_new();
	if (id == "font_open")
		mode_font->open();
	if (id == "project_new")
		mode_administration->_new();
	if (id == "project_open")
		mode_administration->open();
	if (id == "administrate")
		set_mode(mode_administration);
	if (id == "opt_view")
		optimize_current_view();
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unknown");
}


void Edward::update_menu()
{
	cur_mode->on_update_menu_recursive();

	Data *d = cur_mode->get_data();
	if (d){
		enable("undo", d->action_manager->undoable());
		enable("redo", d->action_manager->redoable());
		string title = title_filename(d->filename) + " - " + AppName;
		if (!d->action_manager->is_save())
			title = "*" + title;
		set_title(title);
		if (cur_mode->multi_view)
			enable("view_pop", cur_mode->multi_view->view_stage > 0);
	}else{
		set_title(AppName);
	}

	// general multiview stuff
	MultiView::MultiView *mv = cur_mode->multi_view;
	if (mv){
		check("whole_window", mv->whole_window);
		check("grid", mv->grid_enabled);
		check("light", mv->light_enabled);
		check("wire", mv->wire_mode);
		check("snap_to_grid", mv->snap_to_grid);
	}
}

static string NoEnding(const string &filename)
{
	int p = filename.rfind(".");
	if (p >= 0)
		return filename.substr(0, p);
	return filename;
}

bool Edward::file_dialog(int kind,bool save,bool force_in_root_dir)
{
	int done;

	update_dialog_dir(kind);
	if (dialog_dir[kind].num < 1)
		dialog_dir[kind] = root_dir_kind[kind];


	string title, show_filter, filter;
	if (kind==FD_MODEL){		title=_("Model file");	show_filter=_("Models (*.model)");			filter="*.model";	}
	if (kind==FD_TEXTURE){	title=_("Texture file");	show_filter=_("Textures (bmp,jpg,tga,png,avi)");filter="*.jpg;*.bmp;*.tga;*.png;*.avi";	}
	if (kind==FD_SOUND){		title=_("Sound file");		show_filter=_("Sounds (wav,ogg)");			filter="*.wav;*.ogg";	}
	if (kind==FD_MATERIAL){	title=_("Material file");	show_filter=_("Materials (*.material)");	filter="*.material";	}
	if (kind==FD_TERRAIN){	title=_("Terrain files");	show_filter=_("Terrains (*.map)");			filter="*.map";	}
	if (kind==FD_WORLD){		title=_("World file");		show_filter=_("Worlds (*.world)");			filter="*.world";	}
	if (kind==FD_SHADERFILE){title=_("Shader file");	show_filter=_("Shader files (*.shader)");	filter="*.shader";	}
	if (kind==FD_FONT){		title=_("Font file");		show_filter=_("Font files (*.xfont)");	filter="*.xfont";	}
	if (kind==FD_SCRIPT){	title=_("Script file");	show_filter=_("Script files (*.kaba)");	filter="*.kaba";	}
	if (kind==FD_CAMERAFLIGHT){title=_("Camera file");	show_filter=_("Camera files (*.camera)");	filter="*.camera";	}
	if (kind==FD_FILE){		title=_("arbitrary file");	show_filter=_("Files (*.*)");				filter="*";	}

	if (save)	done=hui::FileDialogSave(this,title,dialog_dir[kind],show_filter,filter);
	else		done=hui::FileDialogOpen(this,title,dialog_dir[kind],show_filter,filter);
	if (done){

		bool in_root_dir = (hui::Filename.sys_filename().find(root_dir_kind[kind].sys_filename()) >= 0);

		if (force_in_root_dir){
			if (!in_root_dir){
				error_box(hui::Filename.sys_filename());
				error_box(format(_("The file ist not int the appropriate directory: \"%s\"\nor in a subdirectory."), root_dir_kind[kind].sys_filename().c_str()));
				return false;
			}
		}//else
			//MakeDirs(HuiFileDialogPath);

		if (in_root_dir){
			update_dialog_dir(kind);
			dialog_dir[kind] = hui::Filename.dirname();
		}
		dialog_file_complete = hui::Filename;
		dialog_file = dialog_file_complete.substr(root_dir_kind[kind].num, -1);
		dialog_file_no_ending = NoEnding(dialog_file);

		return true;
	}
	return false;
}

bool Edward::allow_termination()
{
	if (!cur_mode)
		return true;
	Data *d = cur_mode->get_data();
	if (!d)
		return true;
	if (d->action_manager->is_save())
		return true;
	string answer = hui::QuestionBox(this,_("Quite a polite question"),_("You increased entropy. Do you wish to save your work?"),true);
	if (answer == "hui:cancel")
		return false;
	if (answer == "hui:no")
		return true;
	bool saved = cur_mode->save();
	return saved;
}

string Edward::get_tex_image(nix::Texture *tex)
{
	if (icon_image.contains(tex))
		return icon_image[tex];

	string img;
	if (tex){
		Image im;
		tex->read(im);
		img = hui::SetImage(&im);
	}else{
		Image empty;
		empty.create(48, 48, White);
		img = hui::SetImage(&empty);
	}
	icon_image.set(tex, img);
	return img;
}

void Edward::set_side_panel(hui::Panel *panel) {
	if (side_panel) {
		// close
		reveal("side-bar-revealer", false);
		delete side_panel;
		side_panel = nullptr;
	}
	if (panel) {
		// open
		side_panel = panel;
		ed->embed(panel, "side-bar-grid", 0, 0);
		reveal("side-bar-revealer", true);
	}
}


