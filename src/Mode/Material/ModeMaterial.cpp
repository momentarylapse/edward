/*
 * ModeMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeMaterial.h"
#include "../../Data/Material/DataMaterial.h"
#include "Dialog/MaterialPropertiesDialog.h"
#include "Dialog/MaterialPhysicsDialog.h"
#include "../../lib/x/x.h"

const int MATERIAL_BALL_NUMX = 32;
const int MATERIAL_BALL_NUMY = 64;

ModeMaterial *mode_material = NULL;

ModeMaterial::ModeMaterial() :
	Mode("Material", NULL, new DataMaterial, ed->multi_view_3d, "menu_material")
{
	data = dynamic_cast<DataMaterial*>(data_generic);
	Subscribe(data);

	AppearanceDialog = NULL;

	for (int i=2;i<MATERIAL_MAX_TEXTURE_LEVELS;i++)
		MaterialVB[i] = NixCreateVBM(MATERIAL_BALL_NUMX * MATERIAL_BALL_NUMY * 2, i);
}

ModeMaterial::~ModeMaterial()
{
}

void ModeMaterial::New()
{
	data->Reset();
	multi_view->ResetView();
	multi_view->radius = 500;
	ed->SetMode(mode_material);
}



bool ModeMaterial::Save()
{
	if (data->filename == "")
		return SaveAs();
	return data->Save(data->filename);
}



void ModeMaterial::OnDraw()
{
}



void ModeMaterial::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		data->UpdateTextures();
		NixUnrefShader(data->Shader);
		data->Shader = data->Appearance.GetShader();
	}
}



void ModeMaterial::OnCommand(const string & id)
{
	if (id == "new")
		New();
	if (id == "open")
		Open();
	if (id == "save")
		Save();
	if (id == "save_as")
		SaveAs();

	if (id == "appearance")
		ExecuteAppearanceDialog();
	if (id == "physics")
		ExecutePhysicsDialog();

	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();
}

void ModeMaterial::ExecuteAppearanceDialog()
{
	if (AppearanceDialog)
		return;

	AppearanceDialog = new MaterialPropertiesDialog(ed, true, data);

	AppearanceDialog->Update();
	//HuiWaitTillWindowClosed(AppearanceDialog);
}

void ModeMaterial::ExecutePhysicsDialog()
{
	CHuiWindow *dlg = new MaterialPhysicsDialog(ed, true, data);

	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}

void CreateTorus(int buffer, const vector &pos, const vector dir, float radius1, float radius2, int nx, int ny, int num_tex)
{
	vector dir2 = dir.ortho();
	dir2.normalize();
	vector dir3 = dir ^ dir2;
	for (int x=0;x<nx;x++){
		float fx0 = float(x  )/(float)nx;
		float fx1 = float(x+1)/(float)nx;
		vector rdir0 = dir2 * cos(2 * pi * fx0) + dir3 * sin(2 * pi * fx0);
		vector rdir1 = dir2 * cos(2 * pi * fx1) + dir3 * sin(2 * pi * fx1);
		vector pp0 = pos + rdir0 * radius1;
		vector pp1 = pos + rdir1 * radius1;
		for (int y=0;y<ny;y++){
			float fy0 = float(y  )/(float)ny;
			float fy1 = float(y+1)/(float)ny;
			vector n00 = rdir0 * cos(2 * pi * fy0) + dir * sin(2 * pi * fy0);
			vector n01 = rdir0 * cos(2 * pi * fy1) + dir * sin(2 * pi * fy1);
			vector n10 = rdir1 * cos(2 * pi * fy0) + dir * sin(2 * pi * fy0);
			vector n11 = rdir1 * cos(2 * pi * fy1) + dir * sin(2 * pi * fy1);
			vector p00 = pp0 + n00 * radius2;
			vector p01 = pp0 + n01 * radius2;
			vector p10 = pp1 + n10 * radius2;
			vector p11 = pp1 + n11 * radius2;
			if (num_tex == 1){
				NixVBAddTria(buffer,	p01,n01,fx0,fy1,
										p00,n00,fx0,fy0,
										p10,n10,fx1,fy0);
				NixVBAddTria(buffer,	p01,n01,fx0,fy1,
										p10,n10,fx1,fy0,
										p11,n11,fx1,fy1);
			}else{
				float t00[MODEL_MAX_TEXTURES], t01[MODEL_MAX_TEXTURES], t10[MODEL_MAX_TEXTURES], t11[MODEL_MAX_TEXTURES];
				for (int k=0; k<num_tex;k++){
					t00[k * 2    ] = fx0;	t00[k * 2 + 1] = fy0;
					t01[k * 2    ] = fx0;	t01[k * 2 + 1] = fy1;
					t10[k * 2    ] = fx1;	t10[k * 2 + 1] = fy0;
					t11[k * 2    ] = fx1;	t11[k * 2 + 1] = fy1;
				}
				NixVBAddTriaM(buffer,	p01,n01,t01,
										p00,n00,t00,
										p10,n10,t10);
				NixVBAddTriaM(buffer,	p01,n01,t01,
										p10,n10,t10,
										p11,n11,t11);

			}
		}
	}
}


void ModeMaterial::OnDrawWin(int win)
{
	data->ApplyForRendering();

	int num_tex = data->EffectiveTextureLevels();
	int vb = VBTemp;
	if (num_tex > 1)
		vb = MaterialVB[num_tex];

	if (num_tex > 1){
		NixDraw3DM(vb);
	}else{
		NixDraw3D(vb);
	}

	int tex[] = {-1, -1, -1, -1, -1, -1, -1, -1};
	NixSetTextures(tex, 8);
	NixSetTexture(-1);
	NixSetShader(-1);

	NixSetAlpha(AlphaNone);
	NixSetZ(true,true);
}



bool ModeMaterial::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDMaterial, false, false))
		return false;
	if (!data->Load(ed->DialogFileComplete))
		return false;

	multi_view->ResetView();
	multi_view->radius = 500;
	ed->SetMode(mode_material);
	return true;
}



void ModeMaterial::OnEnd()
{
	if (AppearanceDialog)
		delete(AppearanceDialog);
	AppearanceDialog = NULL;

	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



bool ModeMaterial::SaveAs()
{
	if (ed->FileDialog(FDMaterial, true, false))
		return data->Save(ed->DialogFileComplete);
	return false;
}



void ModeMaterial::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","redo");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Eigenschaften"),_("Eigenschaften"), dir + "configure.png", "appearance");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
	multi_view->MVRectable = false;


	NixVBClear(VBTemp);
	CreateTorus(VBTemp, v_0, e_z, 80, 50, 48, 24, 1);
	for (int i=2;i<MODEL_MAX_TEXTURES;i++){
		int vb = MaterialVB[i];
		NixVBClear(vb);
		CreateTorus(vb, v_0, e_z, 80, 50, 48, 24, i);
	}
}


