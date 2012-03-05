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

static void OnClose()
{
	msg_write("beende...");
	HuiEnd();
}

static void OnDraw()
{	ed->Draw();	}

static void OnKeyDown()
{	if (ed->cur_mode)	ed->cur_mode->OnKeyDown();	}

static void OnKeyUp()
{	if (ed->cur_mode)	ed->cur_mode->OnKeyUp();	}

static void OnMouseMove()
{	if (ed->cur_mode)	ed->cur_mode->OnMouseMove();	}

static void OnLeftButtonDown()
{	if (ed->cur_mode)	ed->cur_mode->OnLeftButtonDown();	}

static void OnLeftButtonUp()
{	if (ed->cur_mode)	ed->cur_mode->OnLeftButtonUp();	}

static void OnMiddleButtonDown()
{	if (ed->cur_mode)	ed->cur_mode->OnMiddleButtonDown();	}

static void OnMiddleButtonUp()
{	if (ed->cur_mode)	ed->cur_mode->OnMiddleButtonUp();	}

static void OnRightButtonDown()
{	if (ed->cur_mode)	ed->cur_mode->OnRightButtonDown();	}

static void OnRightButtonUp()
{	if (ed->cur_mode)	ed->cur_mode->OnRightButtonUp();	}

static void OnEvent()
{	if (ed->cur_mode)	ed->cur_mode->OnCommand(HuiGetEvent()->id);	}

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

	// configuration
	int x = HuiConfigReadInt("X", -1);
	int y = HuiConfigReadInt("Y", -1);
	int w = HuiConfigReadInt("Width", 800);
	int h = HuiConfigReadInt("Height", 600);
	bool maximized = HuiConfigReadBool("Maximized", false);
	/*RootDir = HuiConfigReadStr("RootDir", "");
	//HuiConfigReadInt("Api", api, NIX_API_OPENGL);
	bool LocalDocumentation = HuiConfigReadBool("LocalDocumentation", false);
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
	mv3d = new MultiView(true);
	mv2d = new MultiView(false);
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

int Edward::Run()
{
	return HuiRun();
}
