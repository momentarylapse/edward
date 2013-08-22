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
#include "Mode/Welcome/ModeWelcome.h"
#include "Mode/ModeCreation.h"
#include "MultiView.h"
#include "x/world.h"
#include "x/camera.h"
#include "meta.h"
#include "x/model_manager.h"
#include "x/font.h"
#include "lib/script/script.h"
#include "lib/nix/nix.h"

Edward *ed = NULL;

#ifndef _X_USE_SOUND_
string SoundDir;
#endif

extern string AppName;


void read_color_4i(CFile *f,int *c)
{
	// argb (file) -> rgba (editor)
	c[3]=f->ReadInt();
	c[0]=f->ReadInt();
	c[1]=f->ReadInt();
	c[2]=f->ReadInt();
}

void write_color_4i(CFile *f,int *c)
{
	// rgba (editor) -> argb (file)
	f->WriteInt(c[3]);
	f->WriteInt(c[0]);
	f->WriteInt(c[1]);
	f->WriteInt(c[2]);
}

void write_color_rgba(CFile *f, const color &c)
{
	f->WriteInt((int)(c.r * 255.0f));
	f->WriteInt((int)(c.g * 255.0f));
	f->WriteInt((int)(c.b * 255.0f));
	f->WriteInt((int)(c.a * 255.0f));
}

void read_color_rgba(CFile *f, color &c)
{
	c.r = (float)f->ReadInt() / 255.0f;
	c.g = (float)f->ReadInt() / 255.0f;
	c.b = (float)f->ReadInt() / 255.0f;
	c.a = (float)f->ReadInt() / 255.0f;
}

void write_color_argb(CFile *f, const color &c)
{
	f->WriteInt((int)(c.a * 255.0f));
	f->WriteInt((int)(c.r * 255.0f));
	f->WriteInt((int)(c.g * 255.0f));
	f->WriteInt((int)(c.b * 255.0f));
}

void read_color_argb(CFile *f, color &c)
{
	c.a = (float)f->ReadInt() / 255.0f;
	c.r = (float)f->ReadInt() / 255.0f;
	c.g = (float)f->ReadInt() / 255.0f;
	c.b = (float)f->ReadInt() / 255.0f;
}

void write_color_3i(CFile *f, const color &c)
{
	f->WriteInt((int)(c.r * 255.0f));
	f->WriteInt((int)(c.g * 255.0f));
	f->WriteInt((int)(c.b * 255.0f));
}

void read_color_3i(CFile *f, color &c)
{
	c.r = (float)f->ReadInt() / 255.0f;
	c.g = (float)f->ReadInt() / 255.0f;
	c.b = (float)f->ReadInt() / 255.0f;
	c.a = 1;
}

color i42c(int *c)
{
	return color(float(c[3])/255.0f,float(c[0])/255.0f,float(c[1])/255.0f,float(c[2])/255.0f);
}

void Edward::OnClose()
{
	if (AllowTermination())
		delete(this);
}

#define IMPLEMENT_EVENT(event, pre_event, param_list, param)	\
void Edward::event() \
{ \
	if (cur_mode) \
		cur_mode->pre_event(); \
	if (force_redraw) \
		OnDraw(); \
}

IMPLEMENT_EVENT(OnKeyDown, OnKeyDownRecursive, , )
IMPLEMENT_EVENT(OnKeyUp, OnKeyUpRecursive, , )
IMPLEMENT_EVENT(OnMouseMove, OnMouseMoveRecursive, , )
IMPLEMENT_EVENT(OnLeftButtonDown, OnLeftButtonDownRecursive, , )
IMPLEMENT_EVENT(OnLeftButtonUp, OnLeftButtonUpRecursive, , )
IMPLEMENT_EVENT(OnMiddleButtonDown, OnMiddleButtonDownRecursive, , )
IMPLEMENT_EVENT(OnMiddleButtonUp, OnMiddleButtonUpRecursive, , )
IMPLEMENT_EVENT(OnRightButtonDown, OnRightButtonDownRecursive, , )
IMPLEMENT_EVENT(OnRightButtonUp, OnRightButtonUpRecursive, , )

void Edward::OnEvent()
{
	string id = HuiGetEvent()->id;
	if (id.num == 0)
		id = HuiGetEvent()->message;
	if (cur_mode)
		cur_mode->OnCommandRecursive(id);
	OnCommand(id);
}

void Edward::OnAbortCreationMode()
{
	ModeCreationBase *m = dynamic_cast<ModeCreationBase*>(cur_mode);
	if (m)
		m->Abort();
}

void Edward::IdleFunction()
{
	msg_db_f("Idle", 3);

	if (force_redraw)
		OnDraw();
	else
		HuiSleep(0.010f);
}


Edward::Edward(Array<string> arg) :
	HuiNixWindow(AppName, -1, -1, 800, 600)
{
	msg_db_f("Init", 1);

	ed = this;
	cur_mode = NULL;
	force_redraw = false;

	progress = new Progress;

	LoadKeyCodes();

	PossibleSubDir.add("Fonts");
	PossibleSubDir.add("Maps");
	PossibleSubDir.add("Materials");
	PossibleSubDir.add("Objects");
	PossibleSubDir.add("Scripts");
	PossibleSubDir.add("Sounds");
	PossibleSubDir.add("Textures");

	// configuration
	int x = HuiConfigReadInt("X", -1);
	int y = HuiConfigReadInt("Y", -1);
	int w = HuiConfigReadInt("Width", 800);
	int h = HuiConfigReadInt("Height", 600);
	bool maximized = HuiConfigReadBool("Maximized", false);
	SetSize(w, h);
	RootDir = HuiConfigReadStr("RootDir", "");
	//HuiConfigReadInt("Api", api, NIX_API_OPENGL);
	/*bool LocalDocumentation = HuiConfigReadBool("LocalDocumentation", false);
	WorldScriptVarFile = HuiConfigReadStr("WorldScriptVarFile", "");
	ObjectScriptVarFile = HuiConfigReadStr("ObjectScriptVarFile", "");
	ItemScriptVarFile = HuiConfigReadStr("ItemScriptVarFile", "");

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
	SetMaximized(maximized);
	Show();

	// initialize engine
	NixInit("OpenGL", 1024, 768, 32, false, this);
	NixTextureIconSize = 32;

	EventM("hui:close", this, &Edward::OnClose);
	EventM("exit", this, &Edward::OnClose);
	EventM("hui:redraw", this, &Edward::OnDraw);
	EventM("*", this, &Edward::OnEvent);
	EventM("what_the_fuck", this, &Edward::OnAbout);
	EventM("send_bug_report", this, &Edward::OnSendBugReport);
	EventM("execute_plugin", this, &Edward::OnExecutePlugin);
	HuiAddCommandM("abort_creation_mode", "hui:cancel", KEY_ESCAPE, this, &Edward::OnAbortCreationMode);

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
	multi_view_3d = new MultiView(true);
	multi_view_2d = new MultiView(false);
	mode_welcome = new ModeWelcome;
	mode_model = new ModeModel;
	mode_material = new ModeMaterial;
	mode_world = new ModeWorld;
	mode_font = new ModeFont;
	mode_administration = new ModeAdministration;
	msg_db_m("              \\(^_^)/", 1);

	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;*/

	MakeDirs(RootDir,true);

	// subscribe to all data to automatically redraw...
	Subscribe(mode_model->data);
	Subscribe(mode_material->data);
	Subscribe(mode_world->data);
	Subscribe(mode_font->data);
	Subscribe(multi_view_2d);
	Subscribe(multi_view_3d);

	plugins = new PluginManager();

	if (!HandleArguments(arg))
		SetMode(mode_welcome);

	HuiSetIdleFunctionM(this, &Edward::IdleFunction);
	HuiRunLaterM(0.010f, this, &Edward::ForceRedraw);
	HuiRunLaterM(0.100f, this, &Edward::OptimizeCurrentView);
}

Edward::~Edward()
{
	// saving the configuration data...
	irect r = GetOuteriorDesired();
	HuiConfigWriteInt("X", -1);//r.x1);
	HuiConfigWriteInt("Y", -1);//r.y1);
	HuiConfigWriteInt("Width", r.x2 - r.x1);
	HuiConfigWriteInt("Height", r.y2 - r.y1);
	HuiConfigWriteBool("Maximized", IsMaximized());
	HuiConfigWriteStr("RootDir", RootDir);
	HuiConfigWriteStr("Language", HuiGetCurLanguage());
	/*HuiConfigWriteBool("LocalDocumentation", LocalDocumentation);
	HuiConfigWriteStr("WorldScriptVarFile", WorldScriptVarFile);
	HuiConfigWriteStr("ObjectScriptVarFile", ObjectScriptVarFile);
	HuiConfigWriteStr("ItemScriptVarFile", ItemScriptVarFile);*/
	//HuiConfigWriteInt("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));

	HuiEnd();
}

bool Edward::HandleArguments(Array<string> arg)
{
	if (arg.num < 2)
		return false;
	msg_db_f("LoadParam", 1);

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
				plugins->Execute(arg[i]);
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
		MakeDirs(param);
		mode_model->data->Load(param, true);
		SetMode(mode_model);
		/*if (mmodel->Skin[1].Sub[0].Triangle.num==0)
			mmodel->SetEditMode(EditModeVertex);*/
	}else if (ext == "material"){
		MakeDirs(param);
		mode_material->data->Load(param, true);
		SetMode(mode_material);
	/*}else if ((ext == "map") || (ext == "terrain")){
		MakeDirs(param);
		mworld->Terrain.resize(1);
		mworld->LoadFromFileTerrain(0, v0, param, true);
		mworld->OptimizeView();
		SetMode(ModeWorld);*/
	}else if (ext == "world"){
		MakeDirs(param);
		mode_world->data->Load(param);
		SetMode(mode_world);
		multi_view_3d->whole_window = true;
	}else if (ext == "xfont"){
		MakeDirs(param);
		mode_font->data->Load(param);
		SetMode(mode_font);
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
		mode_model->ImportLoad3ds(param);
	}else{
		ErrorBox(_("Unbekannte Dateinamenerweiterung: ") + param);
		HuiEnd();
	}
	}
	return true;
}

void Edward::OptimizeCurrentView()
{
	if (cur_mode)
		cur_mode->OptimizeViewRecursice();
}


// do we change roots?
//  -> data loss?
bool mode_switch_allowed(ModeBase *m)
{
	if (m->EqualRoots(ed->cur_mode))
		return true;
	return ed->AllowTermination();
}

void Edward::SetMode(ModeBase *m)
{
	if (cur_mode == m)
		return;
	if (!mode_switch_allowed(m))
		return;

	// recursive use...
	mode_queue.add(m);
	if (mode_queue.num > 1)
		return;

	msg_db_f("SetMode", 1);
	if (cur_mode){
		cur_mode->OnLeave();
		if (cur_mode->GetData())
			Unsubscribe(cur_mode->GetData()->action_manager);
	}

	m = mode_queue[0];
	while(m){

		// close current modes
		while(cur_mode){
			if (cur_mode->IsAncestorOf(m))
				break;
			msg_write("end " + cur_mode->name);
			cur_mode->OnEnd();
			cur_mode = cur_mode->parent;
		}

		//multi_view_3d->ResetMouseAction();
		//multi_view_2d->ResetMouseAction();

		// start new modes
		while(cur_mode != m){
			cur_mode = cur_mode->GetNextChildTo(m);
			msg_write("start " + cur_mode->name);
			cur_mode->OnStart();
		}
		cur_mode->OnEnter();

		// nested set calls?
		mode_queue.erase(0);
		m = NULL;
		if (mode_queue.num > 0)
			m = mode_queue[0];
	}

	SetMenu(cur_mode->menu);
	UpdateMenu();
	cur_mode->OnEnter();
	if (cur_mode->GetData())
		Subscribe(cur_mode->GetData()->action_manager);

	ForceRedraw();
}

void Edward::OnAbout()
{	HuiAboutBox(this);	}

void Edward::OnSendBugReport()
{	HuiSendBugReport();	}

void Edward::OnUpdate(Observable *o)
{
	msg_db_f("Edward.OnUpdate", 2);
	//msg_write("ed: " + o->GetName() + " - " + o->GetMessage());
	if (o->GetName() == "MultiView"){
		if (o->GetMessage() == "SettingsChange")
			UpdateMenu();
		ForceRedraw();
	}else if (o->GetName() == "ActionManager"){
		ActionManager *am = dynamic_cast<ActionManager*>(o);
		if (o->GetMessage() == "Failed")
			ErrorBox(format(_("Aktion fehlgeschlagen: %s\nGrund: %s"), am->error_location.c_str(), am->error_message.c_str()));
	}else{
		// data...
		ForceRedraw();
		UpdateMenu();
	}
}

void Edward::OnExecutePlugin()
{
	string temp = DialogDir[FDScript];
	DialogDir[FDScript] = HuiAppDirectoryStatic + "Plugins/";
	if (FileDialog(FDScript, false, false))
		plugins->Execute(DialogFileComplete);
	DialogDir[FDScript] = temp;
}

void Edward::ForceRedraw()
{
	force_redraw = true;
}

void Edward::DrawStr(int x, int y, const string &str, AlignType a)
{
	int w = NixGetStrWidth(str);
	if (a == AlignRight)
		x -= w;
	else if (a == AlignCenter)
		x -= w / 2;
	NixSetAlpha(AlphaMaterial);
	color c = NixGetColor();
	NixSetColor(color(0.5f,0,0,0));
	NixDrawRect(float(x), float(x+w), float(y), float(y+20), 0);
	NixSetColor(c);
	NixSetAlpha(AlphaNone);
	NixDrawStr(x, y, str);//SysStr(str));
}

void Edward::OnDraw()
{
	NixStart();
	if (cur_mode){
		cur_mode->OnDrawRecursive();
	}else{
		NixResetToColor(Black);
		NixDrawStr(100, 100, "no mode...");
	}

	// messages
	foreachi(string &m, message_str, i)
		DrawStr(MaxX / 2, MaxY / 2 - 10 - i * 20, m, AlignCenter);

	NixEnd();
	force_redraw = false;
}



void Edward::LoadKeyCodes()
{
	msg_db_f("LoadKeyCodes", 1);
	CFile *f = FileOpen(HuiAppDirectory + "Data/keys.txt");
	if (!f)
		f = FileOpen(HuiAppDirectoryStatic + "Data/keys.txt");
	int nk = f->ReadIntC();
	f->ReadComment();
	for (int i=0;i<nk;i++){
		string id = f->ReadStr();
		int key_code = f->ReadInt();
		HuiAddKeyCode(id, key_code);
	}
	HuiAddKeyCode("subtract_surface", KEY_CONTROL + KEY_J); // TODO ...
	HuiAddKeyCode("invert_selection", KEY_CONTROL + KEY_TAB); // TODO ...
	HuiAddKeyCode("select_all", KEY_CONTROL + KEY_A); // TODO ...

	HuiAddKeyCode("easify_skin", KEY_CONTROL + KEY_7); // TODO ...
	FileClose(f);
}


void Edward::UpdateDialogDir(int kind)
{
	if (kind==FDModel)			RootDirKind[kind] = ObjectDir;
	if (kind==FDModel)			RootDirKind[kind] = ObjectDir;
	if (kind==FDTexture)		RootDirKind[kind] = NixTextureDir;
	if (kind==FDSound)			RootDirKind[kind] = SoundDir;
	if (kind==FDMaterial)		RootDirKind[kind] = MaterialDir;
	if (kind==FDTerrain)		RootDirKind[kind] = MapDir;
	if (kind==FDWorld)			RootDirKind[kind] = MapDir;
	if (kind==FDShaderFile)		RootDirKind[kind] = MaterialDir;
	if (kind==FDFont)			RootDirKind[kind] = FontDir;
	if (kind==FDScript)			RootDirKind[kind] = ScriptDir;
	if (kind==FDCameraFlight)	RootDirKind[kind] = ScriptDir;
	if (kind==FDFile)			RootDirKind[kind] = RootDir;
}


void Edward::SetRootDirectory(const string &directory)
{
	string object_dir, map_dir, texture_dir, sound_dir, script_dir, material_dir, font_dir;
	bool ufd = (RootDir.find(directory) < 0) && (directory.find(RootDir) < 0);
	RootDir = directory;
	RootDir.dir_ensure_ending();
	if (RootDirCorrect){
		map_dir = RootDir + "Maps/";
		dir_create(map_dir);
		object_dir = RootDir + "Objects/";
		dir_create(object_dir);
		texture_dir = RootDir + "Textures/";
		dir_create(texture_dir);
		sound_dir = RootDir + "Sounds/";
		dir_create(sound_dir);
		script_dir = RootDir + "Scripts/";
		dir_create(script_dir);
		material_dir = RootDir + "Materials/";
		dir_create(material_dir);
		font_dir = RootDir + "Fonts/";
		dir_create(font_dir);
	}else{
		map_dir = RootDir;
		object_dir = RootDir;
		texture_dir = RootDir;
		sound_dir = RootDir;
		script_dir = RootDir;
		material_dir = RootDir;
		font_dir = RootDir;
	}
	MetaSetDirs(texture_dir, map_dir, object_dir, sound_dir, script_dir, material_dir, font_dir);
#ifndef _X_USE_SOUND_
	SoundDir = sound_dir;
#endif
	if (ufd)
		for (int i=0;i<NumFDs;i++){
			DialogDir[i] = "";
			UpdateDialogDir(i);
		}
}


void Edward::MakeDirs(const string &original_dir, bool as_root_dir)
{
	msg_db_f("MakeDirs", 1);
	string dir = original_dir;
	if (dir.num > 0)
		dir = dir.dirname();
	bool sub_dir=false;
	if (!as_root_dir){
		// we are in a sub dir?
		sub_dir=false;
		foreach(string &p, PossibleSubDir){
			if (dir.find(p) >= 0){
				dir = dir.substr(0, dir.find(p));
				sub_dir=true;
				break;
			}
		}
		RootDirCorrect = sub_dir;
		RootDirCorrect &= file_test_existence(dir + "game.ini");
	}else{
		RootDirCorrect = file_test_existence(dir + "game.ini");
	}
	SetRootDirectory(dir);
}


string Edward::GetRootDir(int kind)
{
	if (kind==-1)				return RootDir;
	if (kind==FDModel)			return ObjectDir;
	if (kind==FDTexture)		return NixTextureDir;
	if (kind==FDSound)			return SoundDir;
	if (kind==FDMaterial)		return MaterialDir;
	if (kind==FDTerrain)		return MapDir;
	if (kind==FDWorld)			return MapDir;
	if (kind==FDShaderFile)		return MaterialDir;
	if (kind==FDFont)			return FontDir;
	if (kind==FDScript)			return ScriptDir;
	if (kind==FDCameraFlight)	return ScriptDir;
	if (kind==FDFile)			return RootDir;
	return RootDir;
}

void Edward::RemoveMessage()
{
	message_str.erase(0);
	ForceRedraw();
}

void Edward::SetMessage(const string &message)
{
	msg_write(message);
	message_str.add(message);
	ForceRedraw();
	HuiRunLaterM(2.0f, this, &Edward::RemoveMessage);
}


void Edward::ErrorBox(const string &message)
{
	HuiErrorBox(this, _("Fehler"), message);
}

void Edward::OnCommand(const string &id)
{
	if (id == "model_new")
		mode_model->New();
	if (id == "model_open")
		mode_model->Open();
	if (id == "material_new")
		mode_material->New();
	if (id == "material_open")
		mode_material->Open();
	if (id == "world_new")
		mode_world->New();
	if (id == "world_open")
		mode_world->Open();
	if (id == "font_new")
		mode_font->New();
	if (id == "font_open")
		mode_font->Open();
	if (id == "administrate")
		SetMode(mode_administration);
	if (id == "exit")
		OnClose();
	if (id == "opt_view")
		OptimizeCurrentView();
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return filename.basename();// + " (" + filename.dirname() + ")";
	return _("Unbenannt");
}


void Edward::UpdateMenu()
{
	if (!cur_mode)
		return;
	cur_mode->OnUpdateMenuRecursive();

	Data *d = cur_mode->GetData();
	if (d){
		Enable("undo", d->action_manager->Undoable());
		Enable("redo", d->action_manager->Redoable());
		string title = title_filename(d->filename) + " - " + AppName;
		if (!d->action_manager->IsSave())
			title = "*" + title;
		SetTitle(title);
		if (cur_mode->multi_view)
			Enable("view_pop", cur_mode->multi_view->view_stage > 0);
	}else{
		SetTitle(AppName);
	}

	// general multiview stuff
	MultiView *mv = cur_mode->multi_view;
	if (mv){
		Check("whole_window", mv->whole_window);
		Check("grid", mv->grid_enabled);
		Check("light", mv->light_enabled);
		Check("wire", mv->wire_mode);
	}
}

static string NoEnding(const string &filename)
{
	int p = filename.rfind(".");
	if (p >= 0)
		return filename.substr(0, p);
	return filename;
}

bool Edward::FileDialog(int kind,bool save,bool force_in_root_dir)
{
	int done;

	UpdateDialogDir(kind);
	if (DialogDir[kind].num < 1)
		DialogDir[kind] = RootDirKind[kind];


	string title, show_filter, filter;
	if (kind==FDModel){		title=_("Modell-Datei");	show_filter=_("Modelle (*.model)");			filter="*.model";	}
	if (kind==FDTexture){	title=_("Textur-Datei");	show_filter=_("Texturen (bmp,jpg,tga,avi)");filter="*.jpg;*.bmp;*.tga;*.avi";	}
	if (kind==FDSound){		title=_("Sound-Datei");		show_filter=_("Sounds (wav,mp3,midi)");		filter="*.wav;*.mp3;*.midi";	}
	if (kind==FDMaterial){	title=_("Material-Datei");	show_filter=_("Materialien (*.material)");	filter="*.material";	}
	if (kind==FDTerrain){	title=_("Karten-Datei");	show_filter=_("Karten (*.map)");			filter="*.map";	}
	if (kind==FDWorld){		title=_("Welt-Datei");		show_filter=_("Welten (*.world)");			filter="*.world";	}
	if (kind==FDShaderFile){title=_("Shader-Datei");	show_filter=_("Shader-Dateien (*.glsl)");	filter="*.glsl";	}
	if (kind==FDFont){		title=_("Font-Datei");		show_filter=_("Font-Dateien (*.xfont)");	filter="*.xfont";	}
	if (kind==FDScript){	title=_("Script-Datei");	show_filter=_("Script-Dateien (*.kaba)");	filter="*.kaba";	}
	if (kind==FDCameraFlight){title=_("Kamera-Datei");	show_filter=_("Kamera-Dateien (*.camera)");	filter="*.camera";	}
	if (kind==FDFile){		title=_("beliebige Datei");	show_filter=_("Dateien (*.*)");				filter="*";	}

	if (save)	done=HuiFileDialogSave(this,title,DialogDir[kind],show_filter,filter);
	else		done=HuiFileDialogOpen(this,title,DialogDir[kind],show_filter,filter);
	if (done){

		bool in_root_dir = (HuiFilename.sys_filename().find(RootDirKind[kind].sys_filename()) >= 0);

		if (force_in_root_dir){
			if (!in_root_dir){
				ErrorBox(HuiFilename.sys_filename());
				ErrorBox(format(_("Datei liegt nicht im vorgesehenen Verzeichnis: \"%s\"\noder in dessen Unterverzeichnis"), RootDirKind[kind].sys_filename().c_str()));
				return false;
			}
		}//else
			//MakeDirs(HuiFileDialogPath);

		if (in_root_dir){
			UpdateDialogDir(kind);
			DialogDir[kind] = HuiFilename.dirname();
		}
		DialogFileComplete = HuiFilename;
		DialogFile = DialogFileComplete.substr(RootDirKind[kind].num, -1);
		DialogFileNoEnding = NoEnding(DialogFile);

		return true;
	}
	return false;
}

bool Edward::AllowTermination()
{
	if (!cur_mode)
		return true;
	Data *d = cur_mode->GetData();
	if (!d)
		return true;
	if (d->action_manager->IsSave())
		return true;
	string answer = HuiQuestionBox(this,_("Dem&utige aber h&ofliche Frage"),_("Sie haben die Entropie erh&oht. Wollen Sie Ihr Werk speichern?"),true);
	if (answer == "hui:cancel")
		return false;
	if (answer == "hui:no")
		return true;
	bool saved = cur_mode->Save();
	return saved;
}

int Edward::Run()
{
	return HuiRun();
}

string Edward::get_tex_image(int tex)
{
	int index = max(tex + 1, 0);
	if (index < icon_image.num)
		if (icon_image[index] != "")
			return icon_image[index];

	string img;
	if (tex < 0){
		Image empty;
		empty.Create(32, 32, White);
		img = HuiSetImage(empty);
	}else
		img = HuiSetImage(NixTextures[tex].icon);
	for (int i=icon_image.num;i<=index;i++)
		icon_image.add("");
	icon_image[index] = img;
	return img;
}
