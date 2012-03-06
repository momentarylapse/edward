/*
 * Edward.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "lib/hui/hui.h"

#include "Edward.h"
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

static void OnClose()
{
	msg_write("beende...");
	HuiEnd();
}

static void OnDraw()
{	ed->Draw();	}

static void OnKeyDown()
{	if (ed->cur_mode)	ed->cur_mode->OnPreKeyDown();	}

static void OnKeyUp()
{	if (ed->cur_mode)	ed->cur_mode->OnPreKeyUp();	}

static void OnMouseMove()
{	if (ed->cur_mode)	ed->cur_mode->OnPreMouseMove();	}

static void OnLeftButtonDown()
{	if (ed->cur_mode)	ed->cur_mode->OnPreLeftButtonDown();	}

static void OnLeftButtonUp()
{	if (ed->cur_mode)	ed->cur_mode->OnPreLeftButtonUp();	}

static void OnMiddleButtonDown()
{	if (ed->cur_mode)	ed->cur_mode->OnPreMiddleButtonDown();	}

static void OnMiddleButtonUp()
{	if (ed->cur_mode)	ed->cur_mode->OnPreMiddleButtonUp();	}

static void OnRightButtonDown()
{	if (ed->cur_mode)	ed->cur_mode->OnPreRightButtonDown();	}

static void OnRightButtonUp()
{	if (ed->cur_mode)	ed->cur_mode->OnPreRightButtonUp();	}

static void OnEvent()
{	if (ed->cur_mode)	ed->cur_mode->OnPreCommand(HuiGetEvent()->id);	}

static void OnAbout()
{	ed->About();	}

static void IdleFunction()
{
	msg_db_r("Idle", 3);

	if (ed->force_redraw){
		ed->Draw();
		ed->force_redraw = false;
	}else
		HuiSleep(10);

	msg_db_l(3);
}

Edward::Edward(Array<string> arg)
{
	msg_db_r("Init", 1);

	ed = this;
	cur_mode = NULL;
	force_redraw = false;

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
	win = HuiCreateNixWindow(AppName, x, y, w, h);
	win->SetMaximized(maximized);
	/*if (WelcomeUseDialog)
		MainWin->Hide(true);*/
	win->Update();

	// initialize engine
	NixInit("OpenGL", 1024, 768, 32, false, win);
	NixTextureIconSize = 32;

	win->Event("hui:close", &OnClose);
	win->Event("hui:redraw", &OnDraw);
	win->Event("hui:key-down", &OnKeyDown);
	win->Event("hui:key-up", &OnKeyUp);
	win->Event("hui:mouse-move", &OnMouseMove);
	win->Event("hui:left-button-down", &OnLeftButtonDown);
	win->Event("hui:left-button-up", &OnLeftButtonUp);
	win->Event("hui:middle-button-down", &OnMiddleButtonDown);
	win->Event("hui:middle-button-up", &OnMiddleButtonUp);
	win->Event("hui:right-button-down", &OnRightButtonDown);
	win->Event("hui:right-button-up", &OnRightButtonUp);
	win->Event("*", &OnEvent);
	win->Event("what_the_fuck", &OnAbout);

	MetaInit();
	FxInit("", "", "");
	FxLightFieldsEnabled = false;
	CameraInit();
	GodInit();

	ScriptInit();
//	ScriptLinkDynamicExternalData();

	/*CModeAll();
	RegisterFileTypes();

	// callback functions for CModeAll...
	PostRender = &EdwardPostRender;
	PostGetInput = &EdwardPostGetInput;
	PostDrawMultiView = &EdwardPostDrawMultiView;
	PostDoMultiViewStuff = &EdwardPostDoMultiViewStuff;
	PostEndRect = &EdwardPostEndRect;

	Mode = ModeNone;
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
	/*mobject = new CModeObject();
	mitem = new CModeItem();
	mmaterial = new CModeMaterial();
	mworld = new CModeWorld();
	mfont = new CModeFont();
	madmin = new CModeAdministration();*/
	msg_db_m("              \\(^_^)/", 1);

	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;

	mobject->New();
	mitem->New();
	mmaterial->New();
	mworld->New(false);
	mfont->New();
	mmodel->New();
//	TestHistoryAbility();
	MakeDirs(RootDir,true);
	msg_write("--");*/

	SetMode(mode_welcome);

	LoadKeyCodes();

	HuiSetIdleFunction(&IdleFunction);

	msg_db_l(1);
}

Edward::~Edward()
{
	HuiEnd();
}

void Edward::SetMode(Mode *m)
{
	msg_db_r("SetMode", 1);
	if (cur_mode)
		cur_mode->End();
	cur_mode = m;
	cur_mode->Start();
	win->SetMenu(cur_mode->menu);

	ForceRedraw();
	msg_db_l(1);
}

void Edward::NewWorld()
{
}

void Edward::NewFont()
{
}

bool Edward::OpenModel()
{
	if (mode_model->data->Open()){
		SetMode(mode_model);
		return true;
	}
	return false;
}

void Edward::NewModel()
{
	mode_model->data->Reset();
	SetMode(mode_model);
}

bool Edward::OpenWorld()
{
	return true;
}

bool Edward::OpenMaterial()
{
	return true;
}

void Edward::About()
{	HuiAboutBox(win);	}

void Edward::NewMaterial()
{
}

bool Edward::OpenFont()
{
	return true;
}

void Edward::OnDataChange()
{
	msg_db_r("Edward.OnDataChange", 2);
	if (cur_mode)
		cur_mode->OnDataChange();
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

void Edward::Draw()
{
	NixStart();
	if (cur_mode)
		cur_mode->Draw();
	else{
		NixResetToColor(Black);
		NixDrawStr(100, 100, "no mode...");
	}
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

void Edward::SetMessage(const string &message)
{
	msg_write(message);
}


void Edward::ErrorBox(const string &message)
{
	HuiErrorBox(win, _("Fehler"), message);
}

int Edward::Run()
{
	return HuiRun();
}
