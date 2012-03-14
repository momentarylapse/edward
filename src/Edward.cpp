/*
 * Edward.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"

#include "Edward.h"
#include "Mode/Model/ModeModel.h"
#include "Mode/Material/ModeMaterial.h"
#include "Mode/World/ModeWorld.h"
#include "Mode/Font/ModeFont.h"
#include "Mode/Welcome/ModeWelcome.h"
#include "MultiView.h"
#include "lib/x/x.h"
#include "lib/script/script.h"
#include "lib/nix/nix.h"

Edward *ed = NULL;

string SoundDir;


void read_color_4(CFile *f,int *c)
{
	// argb (file) -> rgba (editor)
	c[3]=f->ReadInt();
	c[0]=f->ReadInt();
	c[1]=f->ReadInt();
	c[2]=f->ReadInt();
}

void write_color_4(CFile *f,int *c)
{
	// rgba (editor) -> argb (file)
	f->WriteInt(c[3]);
	f->WriteInt(c[0]);
	f->WriteInt(c[1]);
	f->WriteInt(c[2]);
}

color i42c(int *c)
{
	return color(float(c[3])/255.0f,float(c[0])/255.0f,float(c[1])/255.0f,float(c[2])/255.0f);
}

void Edward::OnClose()
{
	if (AllowTermination())
		HuiEnd();
}

#define IMPLEMENT_EVENT(event, pre_event, param_list, param)	\
void Edward::event() \
{ \
	if (cur_mode) \
		cur_mode->pre_event(); \
	if (creation_mode) \
		creation_mode->event(); \
}

IMPLEMENT_EVENT(OnKeyDown, OnPreKeyDown, , )
IMPLEMENT_EVENT(OnKeyUp, OnPreKeyUp, , )
IMPLEMENT_EVENT(OnMouseMove, OnPreMouseMove, , )
IMPLEMENT_EVENT(OnLeftButtonDown, OnPreLeftButtonDown, , )
IMPLEMENT_EVENT(OnLeftButtonUp, OnPreLeftButtonUp, , )
IMPLEMENT_EVENT(OnMiddleButtonDown, OnPreMiddleButtonDown, , )
IMPLEMENT_EVENT(OnMiddleButtonUp, OnPreMiddleButtonUp, , )
IMPLEMENT_EVENT(OnRightButtonDown, OnPreRightButtonDown, , )
IMPLEMENT_EVENT(OnRightButtonUp, OnPreRightButtonUp, , )

void Edward::OnEvent()
{
	string id = HuiGetEvent()->id;
	if (id.num == 0)
		id = HuiGetEvent()->message;
	if (cur_mode)
		cur_mode->OnPreCommand(id);
	if (creation_mode)
		creation_mode->OnCommand(id);
	OnCommand(id);
}

static void OnAbortCreationMode()
{	ed->SetCreationMode(NULL);	}

static void IdleFunction()
{
	msg_db_r("Idle", 3);

	if (ed->force_redraw){
		ed->OnDraw();
		ed->force_redraw = false;
	}else
		HuiSleep(10);

	msg_db_l(3);
}

Edward::Edward(Array<string> arg) :
	CHuiWindow("", -1, -1, 800, 600, NULL, false, HuiWinModeResizable | HuiWinModeNix, true)
{
	msg_db_r("Init", 1);

	ed = this;
	cur_mode = NULL;
	creation_mode = NULL;
	force_redraw = false;


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

	EventM("hui:close", this, (void(CHuiWindow::*)())&Edward::OnClose);
	EventM("exit", this, (void(CHuiWindow::*)())&Edward::OnClose);
	EventM("hui:redraw", this, (void(CHuiWindow::*)())&Edward::OnDraw);
	EventM("hui:key-down", this, (void(CHuiWindow::*)())&Edward::OnKeyDown);
	EventM("hui:key-up", this, (void(CHuiWindow::*)())&Edward::OnKeyUp);
	EventM("hui:mouse-move", this, (void(CHuiWindow::*)())&Edward::OnMouseMove);
	EventM("hui:left-button-down", this, (void(CHuiWindow::*)())&Edward::OnLeftButtonDown);
	EventM("hui:left-button-up", this, (void(CHuiWindow::*)())&Edward::OnLeftButtonUp);
	EventM("hui:middle-button-down", this, (void(CHuiWindow::*)())&Edward::OnMiddleButtonDown);
	EventM("hui:middle-button-up", this, (void(CHuiWindow::*)())&Edward::OnMiddleButtonUp);
	EventM("hui:right-button-down", this, (void(CHuiWindow::*)())&Edward::OnRightButtonDown);
	EventM("hui:right-button-up", this, (void(CHuiWindow::*)())&Edward::OnRightButtonUp);
	EventM("*", this, (void(CHuiWindow::*)())&Edward::OnEvent);
	EventM("what_the_fuck", this, (void(CHuiWindow::*)())&Edward::OnAbout);
	EventM("send_bug_report", this, (void(CHuiWindow::*)())&Edward::OnSendBugReport);
	HuiAddCommand("abort_creation_mode", "hui:cancel", KEY_ESCAPE, &OnAbortCreationMode);

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
	Subscribe(multi_view_2d, "SelectionChange");
	Subscribe(multi_view_3d, "SelectionChange");


	SetMode(mode_welcome);

	HuiSetIdleFunction(&IdleFunction);

	msg_db_l(1);
}

Edward::~Edward()
{
	HuiEnd();
}

bool mode_switch_allowed(Mode *m)
{
	Mode *root_cur = NULL;
	if (ed->cur_mode)
		root_cur = ed->cur_mode->GetRootMode();
	Mode *root_new = m->GetRootMode();

	if (root_cur == root_new)
		return true;
	return ed->AllowTermination();
}

void Edward::SetMode(Mode *m)
{
	if (cur_mode == m)
		return;
	if (!mode_switch_allowed(m))
		return;

	msg_db_r("SetMode", 1);

	// close current creation_mode
	SetCreationMode(NULL);

	// close current mode
	if (cur_mode){
		msg_write("end " + cur_mode->name);
		cur_mode->End();
	}
	multi_view_3d->ResetMouseAction();
	multi_view_2d->ResetMouseAction();

	// start new mode
	cur_mode = m;
	msg_write("start " + cur_mode->name);
	cur_mode->Start();
	SetMenu(cur_mode->menu);

	ForceRedraw();
	msg_db_l(1);
}

void Edward::SetCreationMode(ModeCreation *m)
{
	msg_db_r("SetCreationMode", 1);

	// close current creation_mode
	if (creation_mode){
		msg_write("end (creation) " + creation_mode->name);
		creation_mode->End();
		delete(creation_mode);
	}

	// start new creation mode
	creation_mode = m;
	if (creation_mode){
		msg_write("start (creation) " + creation_mode->name);
		creation_mode->Start();
	}

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
	ForceRedraw();
	msg_db_l(2);
}

void Edward::ForceRedraw()
{
	force_redraw = true;
}

void Edward::DrawStr(int x, int y, const string &str)
{
	int w = NixGetStrWidth(str, -1, -1);
	NixSetAlpha(AlphaMaterial);
	NixDrawRect(float(x),float(x+w),float(y),float(y+20),color(0.5f,0,0,0),0);
	NixSetAlpha(AlphaNone);
	NixDrawStr(x,y,str);//SysStr(str));
}

void Edward::OnDraw()
{
	NixStart();
	if (cur_mode){
		if (cur_mode->multi_view)
			cur_mode->multi_view->Draw();
		cur_mode->Draw();
	}else{
		NixResetToColor(Black);
		NixDrawStr(100, 100, "no mode...");
	}

	if (creation_mode){
		creation_mode->PostDraw();
		DrawStr(MaxX / 2, MaxY - 20, creation_mode->message);
	}

	// messages
	foreachi(message_str, m, i)
		DrawStr(MaxX / 2 - NixGetStrWidth(m, -1, -1) / 2, MaxY / 2 - 10 - i * 20, m);

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

void RemoveMessage()
{
	ed->message_str.erase(0);
	ed->ForceRedraw();
}

void Edward::SetMessage(const string &message)
{
	msg_write(message);
	message_str.add(message);
	ForceRedraw();
	HuiRunLater(2000, &RemoveMessage);
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
			//CModeAll::MakeDirs(HuiFileDialogPath);

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

Mode *get_cur_root_mode()
{
	Mode *m;
	Mode *p = ed->cur_mode;
	while(p){
		m = p;
		p = m->parent;
	}
	return m;
}

Data *get_mode_data(Mode *m)
{
	if (m == mode_model)
		return mode_model->data;
	if (m == mode_material)
		return mode_material->data;
	if (m == mode_world)
		return mode_world->data;
	if (m == mode_font)
		return mode_font->data;
	return NULL;
}

bool Edward::AllowTermination()
{
	if (!cur_mode)
		return true;
	Mode *root = cur_mode->GetRootMode();
	Data *d = get_mode_data(root);
	if (!d)
		return true;
	if (d->action_manager->IsSave())
		return true;
	string answer = HuiQuestionBox(this,_("Dem&utige aber h&ofliche Frage"),_("Sie haben die Entropie erh&oht. Wollen Sie Ihr Werk speichern?"),true);
	if (answer == "hui:cancel")
		return false;
	if (answer == "hui:no")
		return true;
	//if (answer == "hui:no")
	bool saved = true;
	if (root == mode_model)		saved = mode_model->Save();
	/*if (Mode==ModeObject)		saved=mobject->Save();
	if (Mode==ModeItem)			saved=mitem->Save();
	if (Mode==ModeMaterial)		saved=mmaterial->Save();
	if (Mode==ModeWorld)		saved=mworld->Save();*/
	return saved;
}

int Edward::Run()
{
	return HuiRun();
}

string Edward::get_tex_image(int tex)
{
	if (tex < 0)
		return "";
	if (tex < icon_image.num)
		if (icon_image[tex] != "")
			return icon_image[tex];

	string img = HuiSetImage(NixTexture[tex].Icon);
	for (int i=icon_image.num;i<=tex;i++)
		icon_image.add("");
	icon_image[tex] = img;
	return img;
}
