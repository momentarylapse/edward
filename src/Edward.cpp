/*
 * Edward.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"

#include "Edward.h"
#include "Mode/Model/ModeModel.h"
#include "Mode/Model/Mesh/ModeModelMesh.h"
#include "Mode/Model/Mesh/ModeModelMeshVertex.h"
#include "Mode/Material/ModeMaterial.h"
#include "Mode/World/ModeWorld.h"
#include "Mode/Font/ModeFont.h"
#include "Mode/Welcome/ModeWelcome.h"
#include "Mode/ModeCreation.h"
#include "MultiView.h"
#include "lib/x/x.h"
#include "lib/script/script.h"
#include "lib/nix/nix.h"

Edward *ed = NULL;

string SoundDir;


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
	ModeCreation *m = dynamic_cast<ModeCreation*>(cur_mode);
	if (m)
		m->Abort();
}

void Edward::IdleFunction()
{
	msg_db_r("Idle", 3);

	if (force_redraw){
		OnDraw();
		force_redraw = false;
	}else
		HuiSleep(10);

	msg_db_l(3);
}

Edward::Edward(Array<string> arg) :
	CHuiWindow(AppName, -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeNix, true)
{
	msg_db_r("Init", 1);

	ed = this;
	cur_mode = NULL;
	force_redraw = false;

	progress = new Progress;


	LoadKeyCodes();

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
	HuiCreateNixWindow(AppName, x, y, w, h);
	SetMaximized(maximized);
	Update();

	// initialize engine
	NixInit("OpenGL", 1024, 768, 32, false, this);
	NixTextureIconSize = 32;

	EventM("hui:close", this, (void(HuiEventHandler::*)())&Edward::OnClose);
	EventM("exit", this, (void(HuiEventHandler::*)())&Edward::OnClose);
	EventM("hui:redraw", this, (void(HuiEventHandler::*)())&Edward::OnDraw);
	EventM("hui:key-down", this, (void(HuiEventHandler::*)())&Edward::OnKeyDown);
	EventM("hui:key-up", this, (void(HuiEventHandler::*)())&Edward::OnKeyUp);
	EventM("hui:mouse-move", this, (void(HuiEventHandler::*)())&Edward::OnMouseMove);
	EventM("hui:left-button-down", this, (void(HuiEventHandler::*)())&Edward::OnLeftButtonDown);
	EventM("hui:left-button-up", this, (void(HuiEventHandler::*)())&Edward::OnLeftButtonUp);
	EventM("hui:middle-button-down", this, (void(HuiEventHandler::*)())&Edward::OnMiddleButtonDown);
	EventM("hui:middle-button-up", this, (void(HuiEventHandler::*)())&Edward::OnMiddleButtonUp);
	EventM("hui:right-button-down", this, (void(HuiEventHandler::*)())&Edward::OnRightButtonDown);
	EventM("hui:right-button-up", this, (void(HuiEventHandler::*)())&Edward::OnRightButtonUp);
	EventM("*", this, (void(HuiEventHandler::*)())&Edward::OnEvent);
	EventM("what_the_fuck", this, (void(HuiEventHandler::*)())&Edward::OnAbout);
	EventM("send_bug_report", this, (void(HuiEventHandler::*)())&Edward::OnSendBugReport);
	HuiAddCommandM("abort_creation_mode", "hui:cancel", KEY_ESCAPE, this, (void(HuiEventHandler::*)())&Edward::OnAbortCreationMode);

	MetaInit();
	FxInit("", "", "");
	FxLightFieldsEnabled = false;
	CameraInit();
	GodInit();

	ScriptInit();
//	ScriptLinkDynamicExternalData();

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
	//madmin = new ModeAdministration;
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


	if (!HandleArguments(arg))
		SetMode(mode_welcome);

	HuiSetIdleFunctionM(this, (void(HuiEventHandler::*)())&Edward::IdleFunction);

	msg_db_l(1);
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
	msg_db_r("LoadParam", 1);
// convert file types...
	/*if (_param == "-cftmodel"){		ConvertFileFormat(FDModel, true);		End();	}
	if (_param == "-cftobject"){	ConvertFileFormat(FDObject, true);		End();	}
	if (_param == "-cftitem"){		ConvertFileFormat(FDItem, true);		End();	}
	if (_param == "-cftmaterial"){	ConvertFileFormat(FDMaterial, true);	End();	}
	if (_param == "-cftmap"){		ConvertFileFormat(FDTerrain, true);		End();	}
	if (_param == "-cftworld"){		ConvertFileFormat(FDWorld, true);		End();	}
// test files
	if (_param == "-tftmodel"){		ConvertFileFormat(FDModel, false);		End();	}
	if (_param == "-tftobject"){	ConvertFileFormat(FDObject, false);		End();	}
	if (_param == "-tftitem"){		ConvertFileFormat(FDItem, false);		End();	}
	if (_param == "-tftmaterial"){	ConvertFileFormat(FDMaterial, false);	End();	}
	if (_param == "-tftmap"){		ConvertFileFormat(FDTerrain, false);	End();	}
	if (_param == "-tftworld"){		ConvertFileFormat(FDWorld, false);		End();	}*/

// loading...
	string param = arg[1];
	if (param[0]=='"')
		param.delete_single(0);
	if (param[param.num-1]=='"')
		param.resize(param.num-1);

	string ext = file_extension(param);

	if (ext == "model"){
		MakeDirs(param);
		mode_model->data->Load(param, true);
		SetMode(mode_model);
		HuiRunLaterM(100, (CHuiWindow*)mode_model_mesh, (void(HuiEventHandler::*)())&ModeModelMesh::OptimizeView);
		/*if (mmodel->Skin[1].Sub[0].Triangle.num==0)
			mmodel->SetEditMode(EditModeVertex);*/
	/*}else if (ext == "object"){
		MakeDirs(param);
		mobject->LoadFromFile(param);
		SetMode(ModeObject);
	}else if (ext == "item"){
		MakeDirs(param);
		mitem->LoadFromFile(param);
		SetMode(ModeItem);*/
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
		/*if (param[0] == '$'){
			JustCreateLightMap = true;
			param.delete_single(0);
		}*/

		MakeDirs(param);
		mode_world->data->Load(param);
		SetMode(mode_world);
		mode_world->OptimizeView();
		multi_view_3d->whole_window = true;
	/*}else if (ext == "xfont"){
		MakeDirs(param);
		mfont->LoadFromFile(param);
		SetMode(ModeFont);
	}else if ((ext == "mdl") || (ext == "MDL")){
		mmodel->LoadImportFromGameStudioMdl(param);
		SetMode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();
		//mmodel->Changed=false;
	}else if ((ext == "wmb") || (ext == "WMB")){
		mmodel->LoadImportFromGameStudioWmb(param);
		SetMode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();
	}else if ((ext == "3ds") || (ext == "3DS")){
		mmodel->LoadImportFrom3DS(param);
		SetMode(ModeModel);
		WholeWindow=true;
		mmodel->OptimizeView();*/
	}else{
		ErrorBox(_("Unbekannte Dateinamenerweiterung: ") + param);
		HuiEnd();
	}
	msg_db_l(1);
	return true;
}


// do we change roots?
//  -> data loss?
bool mode_switch_allowed(Mode *m)
{
	if (m->EqualRoots(ed->cur_mode))
		return true;
	return ed->AllowTermination();
}

void Edward::SetMode(Mode *m)
{
	// ugly redirection
	if (m == mode_model)
		m = mode_model_mesh_vertex;
	if (m == mode_model_mesh)
		m = mode_model_mesh_vertex;


	if (cur_mode == m)
		return;
	if (!mode_switch_allowed(m))
		return;

	msg_db_r("SetMode", 1);

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

	SetMenu(cur_mode->menu);

	UpdateMenu();
	ForceRedraw();

	msg_db_l(1);
}

void Edward::OnAbout()
{	HuiAboutBox(this);	}

void Edward::OnSendBugReport()
{	HuiSendBugReport();	}

void Edward::OnUpdate(Observable *o)
{
	msg_db_r("Edward.OnUpdate", 2);
	//msg_write("ed: " + o->GetName() + " - " + o->GetMessage());
	if (o->GetName() == "MultiView"){
		if (o->GetMessage() == "SettingsChange")
			UpdateMenu();
		else
			ForceRedraw();
	}else{
		// data...
		ForceRedraw();
		UpdateMenu();
	}
	msg_db_l(2);
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
	NixDrawRect(float(x), float(x+w), float(y), float(y+20), color(0.5f,0,0,0), 0);
	NixSetAlpha(AlphaNone);
	NixDrawStr(x, y, str);//SysStr(str));
}

void Edward::OnDraw()
{
	NixStart();
	if (cur_mode){
		if (cur_mode->multi_view)
			cur_mode->multi_view->Draw();
		cur_mode->OnDrawRecursive();
	}else{
		NixResetToColor(Black);
		NixDrawStr(100, 100, "no mode...");
	}

	// messages
	foreachi(message_str, m, i)
		DrawStr(MaxX / 2, MaxY / 2 - 10 - i * 20, m, AlignCenter);

	NixEnd();
}



void Edward::LoadKeyCodes()
{
	msg_db_r("LoadKeyCodes", 1);
	CFile *f = OpenFile(HuiAppDirectory + "Data/keys.txt");
	if (!f)
		f = OpenFile(HuiAppDirectoryStatic + "Data/keys.txt");
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
	msg_db_l(1);
}


void Edward::UpdateDialogDir(int kind)
{
	if (kind==FDModel)			RootDirKind[kind] = ObjectDir;
	if (kind==FDModel)			RootDirKind[kind] = ObjectDir;
	if (kind==FDObject)			RootDirKind[kind] = ObjectDir;
	if (kind==FDItem)			RootDirKind[kind] = ObjectDir;
	if (kind==FDTexture)		RootDirKind[kind] = NixTextureDir;
	if (kind==FDSound)			RootDirKind[kind] = SoundDir;
	if (kind==FDMaterial)		RootDirKind[kind] = MaterialDir;
	if (kind==FDTerrain)		RootDirKind[kind] = MapDir;
	if (kind==FDWorld)			RootDirKind[kind] = MapDir;
	if (kind==FDShaderFile)		RootDirKind[kind] = MaterialDir;
	if (kind==FDFont)			RootDirKind[kind] = MaterialDir;
	if (kind==FDScript)			RootDirKind[kind] = ScriptDir;
	if (kind==FDCameraFlight)	RootDirKind[kind] = ScriptDir;
	if (kind==FDFile)			RootDirKind[kind] = RootDir;
}


void Edward::SetRootDirectory(const string &directory)
{
	string object_dir, map_dir, texture_dir, sound_dir, script_dir, material_dir;
	bool ufd = (RootDir.find(directory) < 0) && (directory.find(RootDir) < 0);
	RootDir = directory;
	dir_ensure_ending(RootDir, true);
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
	}else{
		map_dir = RootDir;
		object_dir = RootDir;
		texture_dir = RootDir;
		sound_dir = RootDir;
		script_dir = RootDir;
		material_dir = RootDir;
	}
	MetaSetDirs(texture_dir,map_dir,object_dir,sound_dir,script_dir,material_dir);
	if (ufd)
		for (int i=0;i<NumFDs;i++){
			DialogDir[i] = "";
			UpdateDialogDir(i);
		}
}


void Edward::MakeDirs(const string &original_dir, bool as_root_dir)
{
	msg_db_r("MakeDirs", 1);
	string dir = original_dir;
	if (dir.num > 0)
		dir = dirname(dir);
	bool sub_dir=false;
	if (!as_root_dir){
		// we are in a sub dir?
		sub_dir=false;
		foreach(PossibleSubDir, p){
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
	msg_db_l(1);
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
	HuiRunLaterM(2000, this, (void(HuiEventHandler::*)())&Edward::RemoveMessage);
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
	if (id == "exit")
		OnClose();
}


string title_filename(const string &filename)
{
	if (filename.num > 0)
		return basename(filename);// + " (" + dirname(filename) + ")";
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
	if (kind==FDObject){	title=_("Objekte-Datei");	show_filter=_("Objekte (*.object)");		filter="*.object";	}
	if (kind==FDItem){		title=_("Item-Datei");		show_filter=_("Items (*.item)");			filter="*.item";	}
	if (kind==FDTexture){	title=_("Textur-Datei");	show_filter=_("Texturen (bmp,jpg,tga,avi)");filter="*.jpg;*.bmp;*.tga;*.avi";	}
	if (kind==FDSound){		title=_("Sound-Datei");		show_filter=_("Sounds (wav,mp3,midi)");		filter="*.wav;*.mp3;*.midi";	}
	if (kind==FDMaterial){	title=_("Material-Datei");	show_filter=_("Materialien (*.material)");	filter="*.material";	}
	if (kind==FDTerrain){	title=_("Karten-Datei");	show_filter=_("Karten (*.map)");			filter="*.map";	}
	if (kind==FDWorld){		title=_("Welt-Datei");		show_filter=_("Welten (*.world)");			filter="*.world";	}
	if (kind==FDShaderFile){title=_("Shader-Datei");	show_filter=_("Shader-Dateien (*.fx)");		filter="*.fx";	}
	if (kind==FDFont){		title=_("Font-Datei");		show_filter=_("Font-Dateien (*.xfont)");	filter="*.xfont";	}
	if (kind==FDScript){	title=_("Script-Datei");	show_filter=_("Script-Dateien (*.kaba)");	filter="*.kaba";	}
	if (kind==FDCameraFlight){title=_("Kamera-Datei");	show_filter=_("Kamera-Dateien (*.camera)");	filter="*.camera";	}
	if (kind==FDFile){		title=_("beliebige Datei");	show_filter=_("Dateien (*.*)");				filter="*";	}

	if (save)	done=HuiFileDialogSave(this,title,DialogDir[kind],show_filter,filter);
	else		done=HuiFileDialogOpen(this,title,DialogDir[kind],show_filter,filter);
	if (done){

		bool in_root_dir = (SysFileName(HuiFilename).find(SysFileName(RootDirKind[kind])) >= 0);

		if (force_in_root_dir){
			if (!in_root_dir){
				ErrorBox(SysFileName(HuiFilename));
				ErrorBox(format(_("Datei liegt nicht im vorgesehenen Verzeichnis: \"%s\"\noder in dessen Unterverzeichnis"), SysFileName(RootDirKind[kind]).c_str()));
				return false;
			}
		}//else
			//MakeDirs(HuiFileDialogPath);

		if (in_root_dir){
			UpdateDialogDir(kind);
			DialogDir[kind] = dirname(HuiFilename);
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
		img = HuiSetImage(NixTexture[tex].Icon);
	for (int i=icon_image.num;i<=index;i++)
		icon_image.add("");
	icon_image[index] = img;
	return img;
}
