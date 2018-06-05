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



static string font_name = "Sans 14";
static float font_size = 20;


void render_text(const string &text, Image &im)
{
	if (text.num == 0){
		im.clear();
		return;
	}
	bool failed = false;
	cairo_surface_t *surface;
	cairo_t *cr;

	// initial surface size guess
	int w_surf = 1024;
	int h_surf = font_size * 2;

	surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w_surf, h_surf);
	cr = cairo_create(surface);

	cairo_set_source_rgba(cr, 0, 0, 0, 1);
	cairo_rectangle(cr, 0, 0, w_surf, h_surf);
	cairo_fill(cr);

	int x = 0, y = 0;

	cairo_set_source_rgba(cr, 1, 1, 1, 1);

	PangoLayout *layout = pango_cairo_create_layout(cr);
	PangoFontDescription *desc = pango_font_description_from_string(font_name.c_str());
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	pango_layout_set_text(layout, (char*)text.data, text.num);
	//int baseline = pango_layout_get_baseline(layout) / PANGO_SCALE;
	int w_used, h_used;
	pango_layout_get_pixel_size(layout, &w_used, &h_used);

	pango_cairo_show_layout(cr, layout);
	g_object_unref(layout);

	cairo_surface_flush(surface);
	unsigned char *c0 = cairo_image_surface_get_data(surface);
	im.create(w_used, h_used, White);
	for (int y=0;y<h_used;y++){
		unsigned char *c = c0 + 4 * y * w_surf;
		for (int x=0;x<w_used;x++){
			float a = (float)c[1] / 255.0f;
			im.setPixel(x, y, color(a, a, 1, 1));
			c += 4;
		}
	}
	im.alpha_used = true;

	cairo_destroy(cr);
	cairo_surface_destroy(surface);
}

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

void Edward::onClose()
{
	if (allowTermination())
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

IMPLEMENT_EVENT(onMouseMove)
IMPLEMENT_EVENT(onMouseWheel)
IMPLEMENT_EVENT(onMouseEnter)
IMPLEMENT_EVENT(onMouseLeave)
IMPLEMENT_EVENT(onLeftButtonDown)
IMPLEMENT_EVENT(onLeftButtonUp)
IMPLEMENT_EVENT(onMiddleButtonDown)
IMPLEMENT_EVENT(onMiddleButtonUp)
IMPLEMENT_EVENT(onRightButtonDown)
IMPLEMENT_EVENT(onRightButtonUp)

void Edward::onKeyDown()
{
	int key_code = hui::GetEvent()->key_code;
	if (cur_mode->multi_view)
		cur_mode->multi_view->onKeyDown(key_code);
	cur_mode->onKeyDown(key_code);
}

void Edward::onKeyUp()
{
	int key_code = hui::GetEvent()->key_code;
	if (cur_mode->multi_view)
		cur_mode->multi_view->onKeyUp(key_code);
	cur_mode->onKeyUp(key_code);
}

void Edward::onEvent()
{
	string id = hui::GetEvent()->id;
	if (id.num == 0)
		id = hui::GetEvent()->message;
	if (cur_mode->multi_view)
		cur_mode->multi_view->onCommand(id);
	cur_mode->onCommandRecursive(id);
	onCommand(id);
}

void Edward::onAbortCreationMode()
{
	ModeCreationBase *m = dynamic_cast<ModeCreationBase*>(cur_mode);
	if (m)
		m->abort();
}

void Edward::idleFunction()
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
	setBorderWidth(0);
	addGrid("", 0, 0, "vgrid");
	setTarget("vgrid");
	addGrid("", 0, 0, "root-table");
	setTarget("root-table");
	addDrawingArea("!grabfocus,opengl", 0, 0, "nix-area");
	setBorderWidth(5);
	show();

	ed = this;
	no_mode = new ModeNone;
	cur_mode = no_mode;

	progress = new Progress;

	loadKeyCodes();

	possible_sub_dir.add("Fonts");
	possible_sub_dir.add("Maps");
	possible_sub_dir.add("Materials");
	possible_sub_dir.add("Objects");
	possible_sub_dir.add("Scripts");
	possible_sub_dir.add("Sounds");
	possible_sub_dir.add("Textures");

	// configuration
	int w = hui::Config.getInt("Window.Width", 800);
	int h = hui::Config.getInt("Window.Height", 600);
	bool maximized = hui::Config.getBool("Window.Maximized", false);
	setSize(w, h);
	root_dir = hui::Config.getStr("RootDir", "");
	//HuiConfigread_int("Api", api, NIX_API_OPENGL);
	/*bool LocalDocumentation = HuiConfig.getBool("LocalDocumentation", false);
	WorldScriptVarFile = HuiConfig.getStr("WorldScriptVarFile", "");
	ObjectScriptVarFile = HuiConfig.getStr("ObjectScriptVarFile", "");
	ItemScriptVarFile = HuiConfig.getStr("ItemScriptVarFile", "");

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
	setMaximized(maximized);

	// initialize engine
	nix::Init("OpenGL", w, h);
	nix::texture_icon_size = 32;
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

	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;*/

	makeDirs(root_dir,true);

	// subscribe to all data to automatically redraw...
	subscribe(mode_model->data);
	subscribe(mode_material->data);
	subscribe(mode_world->data);
	subscribe(mode_font->data);
	subscribe(multi_view_2d);
	subscribe(multi_view_3d);

	plugins = new PluginManager();

	if (!handleArguments(arg))
		mode_model->_new();


	event("hui:close", std::bind(&Edward::onClose, this));
	event("exit", std::bind(&Edward::onClose, this));
	event("*", std::bind(&Edward::onEvent, this));
	event("what_the_fuck", std::bind(&Edward::onAbout, this));
	event("send_bug_report", std::bind(&Edward::onSendBugReport, this));
	event("execute_plugin", std::bind(&Edward::onExecutePlugin, this));
	event("abort_creation_mode", std::bind(&Edward::onAbortCreationMode, this));
	eventX("nix-area", "hui:draw-gl", std::bind(&Edward::onDrawGL, this));
	setKeyCode("abort_creation_mode", hui::KEY_ESCAPE, "hui:cancel");

	//hui::SetIdleFunction(std::bind(&Edward::idleFunction, this));
	hui::RunLater(0.010f, std::bind(&Edward::forceRedraw, this));
	hui::RunLater(0.100f, std::bind(&Edward::optimizeCurrentView, this));
}

Edward::~Edward()
{
}

void Edward::onDestroy()
{
	delete(plugins);
	delete(multi_view_2d);
	delete(multi_view_3d);
	// saving the configuration data...
	int w, h;
	getSizeDesired(w, h);
	hui::Config.setInt("Window.X", -1);//r.x1);
	hui::Config.setInt("Window.Y", -1);//r.y1);
	hui::Config.setInt("Window.Width", w);
	hui::Config.setInt("Window.Height", h);
	hui::Config.setBool("Window.Maximized", isMaximized());
	hui::Config.setStr("RootDir", root_dir);
	hui::Config.setStr("Language", hui::GetCurLanguage());
	/*HuiConfig.setBool("LocalDocumentation", LocalDocumentation);
	HuiConfig.setStr("WorldScriptVarFile", WorldScriptVarFile);
	HuiConfig.setStr("ObjectScriptVarFile", ObjectScriptVarFile);
	HuiConfig.setStr("ItemScriptVarFile", ItemScriptVarFile);*/
	//HuiConfig.setInt("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));
	hui::Config.save();
}

bool Edward::handleArguments(Array<string> arg)
{
	if (arg.num < 2)
		return false;

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
		makeDirs(param);
		mode_model->data->load(param, true);
		setMode(mode_model);
		/*if (mmodel->Skin[1].Sub[0].Triangle.num==0)
			mmodel->SetEditMode(EditModeVertex);*/
	}else if (ext == "material"){
		makeDirs(param);
		mode_material->data->load(param, true);
		setMode(mode_material);
	/*}else if ((ext == "map") || (ext == "terrain")){
		MakeDirs(param);
		mworld->Terrain.resize(1);
		mworld->LoadFromFileTerrain(0, v0, param, true);
		mworld->OptimizeView();
		SetMode(ModeWorld);*/
	}else if (ext == "world"){
		makeDirs(param);
		mode_world->data->load(param);
		setMode(mode_world);
		multi_view_3d->whole_window = true;
	}else if (ext == "xfont"){
		makeDirs(param);
		mode_font->data->load(param);
		setMode(mode_font);
	}else if (ext == "js"){
		mode_model->importLoadJson(param);
		setMode(mode_model);
	}else if (ext == "ply"){
		mode_model->importLoadPly(param);
		setMode(mode_model);
	/*}else if (ext == "mdl"){
		mmodel->LoadImportFromGameStudioMdl(param);
		SetMode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();
		//mmodel->Changed=false;
	}else if (ext == "wmb"){
		mmodel->LoadImportFromGameStudioWmb(param);
		SetMode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();*/
	}else if (ext == "3ds"){
		mode_model->importLoad3ds(param);
	}else{
		errorBox(_("Unbekannte Dateinamenerweiterung: ") + param);
		app->end();
	}
	}
	return true;
}

void Edward::optimizeCurrentView()
{
	cur_mode->optimizeViewRecursice();
}


// do we change roots?
//  -> data loss?
bool mode_switch_allowed(ModeBase *m)
{
	if (m->equalRoots(ed->cur_mode))
		return true;
	return ed->allowTermination();
}

void Edward::setMode(ModeBase *m)
{
	if (cur_mode == m)
		return;
	if (!mode_switch_allowed(m))
		return;

	// recursive use...
	mode_queue.add(m);
	if (mode_queue.num > 1)
		return;

	cur_mode->onLeave();
	if (cur_mode->getData())
		unsubscribe(cur_mode->getData()->action_manager);

	m = mode_queue[0];
	while(m){

		// close current modes
		while(cur_mode){
			if (cur_mode->isAncestorOf(m))
				break;
			msg_write("end " + cur_mode->name);
			cur_mode->onEnd();
			if (cur_mode->multi_view)
				cur_mode->multi_view->popSettings();
			cur_mode = cur_mode->parent;
		}

		//multi_view_3d->ResetMouseAction();
		//multi_view_2d->ResetMouseAction();

		// start new modes
		while(cur_mode != m){
			cur_mode = cur_mode->getNextChildTo(m);
			msg_write("start " + cur_mode->name);
			if (cur_mode->multi_view)
				cur_mode->multi_view->pushSettings();
			cur_mode->onStart();
		}
		cur_mode->onEnter();
		cur_mode->onSetMultiView();

		// nested set calls?
		mode_queue.erase(0);
		m = NULL;
		if (mode_queue.num > 0)
			m = mode_queue[0];
	}

	setMenu(hui::CreateResourceMenu(cur_mode->menu_id));
	updateMenu();
	cur_mode->onEnter();
	if (cur_mode->getData())
		subscribe(cur_mode->getData()->action_manager);

	forceRedraw();
}

void Edward::onAbout()
{	hui::AboutBox(this);	}

void Edward::onSendBugReport()
{}//	hui::SendBugReport();	}

void Edward::onUpdate(Observable *o, const string &message)
{
	//msg_write(o->getName() + " - " + message);
	if (o->getName() == "MultiView"){
		if (message == multi_view_3d->MESSAGE_SETTINGS_CHANGE){
			updateMenu();
		}else if (message == multi_view_3d->MESSAGE_SELECTION_CHANGE){
			cur_mode->onSelectionChange();
			updateMenu();
		}else if (message == multi_view_3d->MESSAGE_VIEWSTAGE_CHANGE){
			cur_mode->onViewStageChange();
			updateMenu();
		}
		forceRedraw();
	}else if (o->getName() == "ActionManager"){
		ActionManager *am = dynamic_cast<ActionManager*>(o);
		if (message == am->MESSAGE_FAILED){
			errorBox(format(_("Aktion fehlgeschlagen: %s\nGrund: %s"), am->error_location.c_str(), am->error_message.c_str()));
		}else if (message == am->MESSAGE_SAVED){
			setMessage(_("Gespeichert!"));
			updateMenu();
		}
	}else if (dynamic_cast<Data*>(o)){
		if (message != Data::MESSAGE_SELECTION)
			cur_mode->onSetMultiView();
		// data...
		forceRedraw();
		//if (message != o->MESSAGE_CHANGE)
		updateMenu();
	}else{
		updateMenu();
	}
}

void Edward::onExecutePlugin()
{
	string temp = dialog_dir[FD_SCRIPT];
	if (app->installed)
		dialog_dir[FD_SCRIPT] = app->directory_static + "Plugins/";
	else
		dialog_dir[FD_SCRIPT] = app->directory + "Plugins/";

	if (fileDialog(FD_SCRIPT, false, false))
		plugins->execute(dialog_file_complete);
	dialog_dir[FD_SCRIPT] = temp;
}

void Edward::forceRedraw()
{
	redraw("nix-area");
}

void Edward::drawStr(int x, int y, const string &str, AlignType a)
{
	int w = nix::GetStrWidth(str);
	if (a == ALIGN_RIGHT)
		x -= w;
	else if (a == ALIGN_CENTER)
		x -= w / 2;
	nix::SetTexture(NULL);
	nix::SetAlpha(ALPHA_MATERIAL);
	color c = nix::GetColor();
	nix::SetColor(color(0.5f,0.8f,0.8f,0.8f));
	nix::DrawRect(float(x), float(x+w), float(y), float(y+20), 0);
	nix::SetColor(c);
	nix::SetAlpha(ALPHA_SOURCE_ALPHA, ALPHA_SOURCE_INV_ALPHA);
	nix::DrawStr(x, y, str);
	nix::SetAlpha(ALPHA_NONE);
}

void Edward::onDrawGL()
{
	auto e = hui::GetEvent();
	nix::Resize(e->column, e->row);

	if (cur_mode->multi_view)
		cur_mode->multi_view->onDraw();
	cur_mode->onDraw();

	// messages
	nix::SetShader(nix::default_shader_2d);
	foreachi(string &m, message_str, i)
		drawStr(nix::target_width / 2, nix::target_height / 2 - 20 - i * 20, m, ALIGN_CENTER);
}



void Edward::loadKeyCodes()
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
			setKeyCode(id, key_code);
		}
		event("volume_subtract", NULL);
		setKeyCode("volume_subtract", hui::KEY_CONTROL + hui::KEY_J); // TODO ...
		event("invert_selection", NULL);
		setKeyCode("invert_selection", hui::KEY_CONTROL + hui::KEY_TAB); // TODO ...
		event("select_all", NULL);
		setKeyCode("select_all", hui::KEY_CONTROL + hui::KEY_A); // TODO ...

		event("easify_skin", NULL);
		setKeyCode("easify_skin", hui::KEY_CONTROL + hui::KEY_7); // TODO ...
		FileClose(f);
	}catch(...){

	}
}


void Edward::updateDialogDir(int kind)
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


void Edward::setRootDirectory(const string &directory, bool compact_mode)
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
			updateDialogDir(i);
		}
}


void Edward::makeDirs(const string &original_dir, bool as_root_dir)
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
	setRootDirectory(dir, !root_dir_correct);
}


string Edward::getRootDir(int kind)
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

void Edward::removeMessage()
{
	message_str.erase(0);
	forceRedraw();
}

void Edward::setMessage(const string &message)
{
	msg_write(message);
	message_str.add(message);
	forceRedraw();
	hui::RunLater(2.0f, std::bind(&Edward::removeMessage, this));
}


void Edward::errorBox(const string &message)
{
	hui::ErrorBox(this, _("Fehler"), message);
}

void Edward::onCommand(const string &id)
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
		setMode(mode_administration);
	if (id == "opt_view")
		optimizeCurrentView();
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unbenannt");
}


void Edward::updateMenu()
{
	cur_mode->onUpdateMenuRecursive();

	Data *d = cur_mode->getData();
	if (d){
		enable("undo", d->action_manager->undoable());
		enable("redo", d->action_manager->redoable());
		string title = title_filename(d->filename) + " - " + AppName;
		if (!d->action_manager->isSave())
			title = "*" + title;
		setTitle(title);
		if (cur_mode->multi_view)
			enable("view_pop", cur_mode->multi_view->view_stage > 0);
	}else{
		setTitle(AppName);
	}

	// general multiview stuff
	MultiView::MultiView *mv = cur_mode->multi_view;
	if (mv){
		check("whole_window", mv->whole_window);
		check("grid", mv->grid_enabled);
		check("light", mv->light_enabled);
		check("wire", mv->wire_mode);
	}
}

static string NoEnding(const string &filename)
{
	int p = filename.rfind(".");
	if (p >= 0)
		return filename.substr(0, p);
	return filename;
}

bool Edward::fileDialog(int kind,bool save,bool force_in_root_dir)
{
	int done;

	updateDialogDir(kind);
	if (dialog_dir[kind].num < 1)
		dialog_dir[kind] = root_dir_kind[kind];


	string title, show_filter, filter;
	if (kind==FD_MODEL){		title=_("Modell-Datei");	show_filter=_("Modelle (*.model)");			filter="*.model";	}
	if (kind==FD_TEXTURE){	title=_("Textur-Datei");	show_filter=_("Texturen (bmp,jpg,tga,png,avi)");filter="*.jpg;*.bmp;*.tga;*.png;*.avi";	}
	if (kind==FD_SOUND){		title=_("Sound-Datei");		show_filter=_("Sounds (wav,ogg)");			filter="*.wav;*.ogg";	}
	if (kind==FD_MATERIAL){	title=_("Material-Datei");	show_filter=_("Materialien (*.material)");	filter="*.material";	}
	if (kind==FD_TERRAIN){	title=_("Karten-Datei");	show_filter=_("Karten (*.map)");			filter="*.map";	}
	if (kind==FD_WORLD){		title=_("Welt-Datei");		show_filter=_("Welten (*.world)");			filter="*.world";	}
	if (kind==FD_SHADERFILE){title=_("Shader-Datei");	show_filter=_("Shader-Dateien (*.shader)");	filter="*.shader";	}
	if (kind==FD_FONT){		title=_("Font-Datei");		show_filter=_("Font-Dateien (*.xfont)");	filter="*.xfont";	}
	if (kind==FD_SCRIPT){	title=_("Script-Datei");	show_filter=_("Script-Dateien (*.kaba)");	filter="*.kaba";	}
	if (kind==FD_CAMERAFLIGHT){title=_("Kamera-Datei");	show_filter=_("Kamera-Dateien (*.camera)");	filter="*.camera";	}
	if (kind==FD_FILE){		title=_("beliebige Datei");	show_filter=_("Dateien (*.*)");				filter="*";	}

	if (save)	done=hui::FileDialogSave(this,title,dialog_dir[kind],show_filter,filter);
	else		done=hui::FileDialogOpen(this,title,dialog_dir[kind],show_filter,filter);
	if (done){

		bool in_root_dir = (hui::Filename.sys_filename().find(root_dir_kind[kind].sys_filename()) >= 0);

		if (force_in_root_dir){
			if (!in_root_dir){
				errorBox(hui::Filename.sys_filename());
				errorBox(format(_("Datei liegt nicht im vorgesehenen Verzeichnis: \"%s\"\noder in dessen Unterverzeichnis"), root_dir_kind[kind].sys_filename().c_str()));
				return false;
			}
		}//else
			//MakeDirs(HuiFileDialogPath);

		if (in_root_dir){
			updateDialogDir(kind);
			dialog_dir[kind] = hui::Filename.dirname();
		}
		dialog_file_complete = hui::Filename;
		dialog_file = dialog_file_complete.substr(root_dir_kind[kind].num, -1);
		dialog_file_no_ending = NoEnding(dialog_file);

		return true;
	}
	return false;
}

bool Edward::allowTermination()
{
	if (!cur_mode)
		return true;
	Data *d = cur_mode->getData();
	if (!d)
		return true;
	if (d->action_manager->isSave())
		return true;
	string answer = hui::QuestionBox(this,_("Dem&utige aber h&ofliche Frage"),_("Sie haben die Entropie erh&oht. Wollen Sie Ihr Werk speichern?"),true);
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
		img = hui::SetImage(tex->icon);
	}else{
		Image empty;
		empty.create(32, 32, White);
		img = hui::SetImage(empty);
	}
	icon_image.add(tex, img);
	return img;
}
