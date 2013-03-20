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

void CreateTorus(int buffer, const vector &pos, const vector dir, float radius1, float radius2, int nx, int ny)
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
			NixVBAddTria(buffer,	p01,n01,fx0,fy1,
									p00,n00,fx0,fy0,
									p10,n10,fx1,fy0);
			NixVBAddTria(buffer,	p01,n01,fx0,fy1,
									p10,n10,fx1,fy0,
									p11,n11,fx1,fy1);
		}
	}
}


void ModeMaterial::OnDrawWin(int win)
{
	data->ApplyForRendering();
	if (data->Appearance.NumTextureLevels <= 1){
		NixVBClear(VBTemp);
		//FxCreateBall(VBTemp, v0, 100, 16, 32);
		CreateTorus(VBTemp, v_0, e_z, 80, 50, 64, 32);
		NixSetTexture((data->Appearance.NumTextureLevels == 1) ? data->Appearance.Texture[0] : -1);
		NixDraw3D(VBTemp);
		NixSetTexture(-1);
	}else{
		/*vector p[(MATERIAL_BALL_NUMX + 1) * (MATERIAL_BALL_NUMY + 1)];
		vector n[(MATERIAL_BALL_NUMX + 1) * (MATERIAL_BALL_NUMY + 1)];
		float tc[(MATERIAL_BALL_NUMX + 1) * (MATERIAL_BALL_NUMY + 1) * 2];


		for (int x=0;x<=MATERIAL_BALL_NUMX;x++)
			for (int y=0;y<=MATERIAL_BALL_NUMY;y++){
				int i = x + y * (MATERIAL_BALL_NUMX + 1);
				vector ang = vector(pi*(x  -MATERIAL_BALL_NUMX/2)/nx,pi*2.0f* y   /MATERIAL_BALL_NUMY,0);
				n[i] = VecAng2Dir(ang);
				p[i] = n[i] * 100;
			vector v;
			vector n0=VecAng2Dir(v=vector(pi*(x  -nx/2)/nx,pi*2.0f* y   /ny,0));
			vector n1=VecAng2Dir(v=vector(pi*(x+1-nx/2)/nx,pi*2.0f* y   /ny,0));
			vector n2=VecAng2Dir(v=vector(pi*(x  -nx/2)/nx,pi*2.0f*(y+1)/ny,0));
			vector n3=VecAng2Dir(v=vector(pi*(x+1-nx/2)/nx,pi*2.0f*(y+1)/ny,0));
			vector p0=pos+radius*n0;
			vector p1=pos+radius*n1;
			vector p2=pos+radius*n2;
			vector p3=pos+radius*n3;
			NixVBAddTria(buffer,	p0,n0,float(x  )/(float)nx,float(y  )/(float)ny,
									p1,n1,float(x+1)/(float)nx,float(y  )/(float)ny,
									p2,n2,float(x  )/(float)nx,float(y+1)/(float)ny);
			NixVBAddTria(buffer,	p2,n2,float(x  )/(float)nx,float(y+1)/(float)ny,
									p1,n1,float(x+1)/(float)nx,float(y  )/(float)ny,
									p3,n3,float(x+1)/(float)nx,float(y+1)/(float)ny);
		}*/
		float radius = 100;
		NixVBClear(MaterialVB[data->Appearance.NumTextureLevels]);
		float tc[4][MATERIAL_MAX_TEXTURE_LEVELS * 2];
		for (int x=0;x<MATERIAL_BALL_NUMX;x++)
			for (int y=0;y<MATERIAL_BALL_NUMY;y++){
				vector n0=vector(pi*(x  -MATERIAL_BALL_NUMX/2)/MATERIAL_BALL_NUMX,pi*2.0f* y   /MATERIAL_BALL_NUMY,0).ang2dir();
				vector n1=vector(pi*(x+1-MATERIAL_BALL_NUMX/2)/MATERIAL_BALL_NUMX,pi*2.0f* y   /MATERIAL_BALL_NUMY,0).ang2dir();
				vector n2=vector(pi*(x  -MATERIAL_BALL_NUMX/2)/MATERIAL_BALL_NUMX,pi*2.0f*(y+1)/MATERIAL_BALL_NUMY,0).ang2dir();
				vector n3=vector(pi*(x+1-MATERIAL_BALL_NUMX/2)/MATERIAL_BALL_NUMX,pi*2.0f*(y+1)/MATERIAL_BALL_NUMY,0).ang2dir();
				vector p0=radius*n0;
				vector p1=radius*n1;
				vector p2=radius*n2;
				vector p3=radius*n3;
				for (int i=0;i<data->Appearance.NumTextureLevels;i++){
					tc[0][i*2] = float(x  )/(float)MATERIAL_BALL_NUMX;	tc[0][i*2+1] = float(y  )/(float)MATERIAL_BALL_NUMY;
					tc[1][i*2] = float(x+1)/(float)MATERIAL_BALL_NUMX;	tc[1][i*2+1] = float(y  )/(float)MATERIAL_BALL_NUMY;
					tc[2][i*2] = float(x  )/(float)MATERIAL_BALL_NUMX;	tc[2][i*2+1] = float(y+1)/(float)MATERIAL_BALL_NUMY;
					tc[3][i*2] = float(x+1)/(float)MATERIAL_BALL_NUMX;	tc[3][i*2+1] = float(y+1)/(float)MATERIAL_BALL_NUMY;
				}
				NixVBAddTriaM(MaterialVB[data->Appearance.NumTextureLevels],	p0,n0,tc[0],
															p1,n1,tc[1],
															p2,n2,tc[2]);
				NixVBAddTriaM(MaterialVB[data->Appearance.NumTextureLevels],	p2,n2,tc[2],
															p1,n1,tc[1],
															p3,n3,tc[3]);
			}
		NixSetTextures(data->Appearance.Texture, data->Appearance.NumTextureLevels);
		NixDraw3DM(MaterialVB[data->Appearance.NumTextureLevels]);
		NixSetTexture(-1);
	}

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
}


