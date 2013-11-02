/*
 * ModelPropertiesDialog.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ModelPropertiesDialog.h"
#include "../../../Edward.h"
#include "../ModeModel.h"
#include "../Mesh/ModeModelMesh.h"
#include "../Mesh/ModeModelMeshTexture.h"
#include "../../../Action/Model/Data/ActionModelAddMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditData.h"



#define InertiaTensorDec			2
#define DetailDistDec				2

extern matrix3 InertiaTensorTemp;
extern float DetailDistTemp1, DetailDistTemp2, DetailDistTemp3;

ModelPropertiesDialog::ModelPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data):
	HuiWindow("model_dialog", _parent, _allow_parent)
{
	data = _data;

	SetTooltip("material_list", _("- Doppelklick um ein Material zu editieren\n- Auswahl wird f&ur folgende neue Polygone verwendet"));

	EventM("cancel", this, &ModelPropertiesDialog::OnClose);
	EventM("hui:close", this, &ModelPropertiesDialog::OnClose);
	EventM("ok", this, &ModelPropertiesDialog::OnOk);
	EventM("generate_dists_auto", this, &ModelPropertiesDialog::OnGenerateDistsAuto);
	EventM("ph_passive", this, &ModelPropertiesDialog::OnPhysicsPassive);
	EventM("generate_tensor_auto", this, &ModelPropertiesDialog::OnGenerateTensorAuto);
	EventMX("tensor", "hui:change", this, &ModelPropertiesDialog::OnTensorEdit);
	EventM("mass", this, &ModelPropertiesDialog::OnGenerateTensorAuto);
	EventM("num_items", this, &ModelPropertiesDialog::OnNumItems);
	EventM("model_inventary", this, &ModelPropertiesDialog::OnModelInventary);
	EventM("delete_item", this, &ModelPropertiesDialog::OnDeleteItem);
	EventM("max_script_vars", this, &ModelPropertiesDialog::OnMaxScriptVars);
	EventMX("script_vars", "hui:change", this, &ModelPropertiesDialog::OnScriptVarEdit);
	EventM("script_find", this, &ModelPropertiesDialog::OnScriptFind);
	EventM("model_script_var_template", this, &ModelPropertiesDialog::OnModelScriptVarTemplate);

	Subscribe(data);

	temp = data->meta_data;
	LoadData();
}

ModelPropertiesDialog::~ModelPropertiesDialog()
{
	mode_model->PropertiesDialog = NULL;
	Unsubscribe(data);
}

void ModelPropertiesDialog::LoadData()
{
// viewing properties (LOD)
	if (temp.AutoGenerateDists)
		data->GenerateDetailDists(temp.DetailDist);
	SetInt("num_lod", 3);
	Enable("num_lod", false);
	FillDetailList();
	Check("generate_dists_auto", temp.AutoGenerateDists);
	Check("generate_skin_auto", temp.AutoGenerateSkin[1]);
	// physics
	if (temp.AutoGenerateTensor)
		temp.InertiaTensor = data->GenerateInertiaTensor(temp.Mass);
	SetFloat("mass", temp.Mass);
	Check("ph_active", temp.ActivePhysics);
	Check("ph_passive", temp.PassivePhysics);
	Enable("ph_active", temp.PassivePhysics);
	FillTensorList();
	Enable("tensor", !temp.AutoGenerateTensor);
	Check("generate_tensor_auto", temp.AutoGenerateTensor);
	// items
	SetString("model_name", temp.Name);
	SetString("model_description", temp.Description);
	SetInt("num_items", temp.Inventary.num);
	RefillInventaryList();
	// script
	SetString("script", temp.ScriptFile);
	SetInt("max_script_vars", temp.ScriptVar.num);
	RefillScriptVarList();

	/*for (int i=0;i<NumScriptVarFiles;i++)
		SetString("model_script_var_template", data->ScriptVarFile[i]);*/
}

void ModelPropertiesDialog::FillDetailList()
{
	Reset("lod");
	//DetailDistDec
	AddString("lod", format("0\\%.2f\\%d", temp.DetailDist[0], 100));
	AddString("lod", format("1\\%.2f\\%d", temp.DetailDist[1], temp.DetailFactor[1]));
	AddString("lod", format("2\\%.2f\\%d", temp.DetailDist[2], temp.DetailFactor[2]));
}

void ModelPropertiesDialog::FillTensorList()
{
	//SetDecimals(InertiaTensorDec);
	Reset("tensor");
	AddString("tensor", format("X\\%.2f\\%.2f\\%.2f", temp.InertiaTensor._00, temp.InertiaTensor._01, temp.InertiaTensor._02));
	AddString("tensor", format("Y\\%.2f\\%.2f\\%.2f", temp.InertiaTensor._10, temp.InertiaTensor._11, temp.InertiaTensor._12));
	AddString("tensor", format("Z\\%.2f\\%.2f\\%.2f", temp.InertiaTensor._20, temp.InertiaTensor._21, temp.InertiaTensor._22));
}

void ModelPropertiesDialog::OnUpdate(Observable *o)
{
	//FillMaterialList();
	temp = data->meta_data;
	LoadData();
}

vector img_get_ball_n(int x, int y, int N)
{
	//vector n = vector(x - N/2, y - N/2, 0);
	vector n = vector(x - N/2, y - N/2, 0);
	n.z = - sqrt(N*N/2 - n.x*n.x - n.y*n.y);
	n.normalize();
	return n;
}

string render_material(ModelMaterial *m)
{
	// texture?
	NixTexture *tex = NixLoadTexture(m->TextureFile[0]);
	if (!tex)
		if (m->material->num_textures > 0)
			tex = m->material->texture[0];

	const int N = 32;

	// simulate a lit sphere
	Image img;
	img.create(N, N, Black);
	vector light_dir = vector(-1, -1, -1);
	light_dir.normalize();
	vector cam_dir = - e_z;
	vector light_sp_dir = light_dir + cam_dir;
	light_sp_dir.normalize();
	for (int x=0;x<N;x++)
		for (int y=0;y<N;y++){
			// ambient + diffuse + emission
			vector n = img_get_ball_n(x, y, N);
			float f = clampf(n * light_dir, 0, 1);
			color c = m->Ambient * 0.3f + m->Diffuse * f + m->Emission;

			// texture "mapping"
			if (tex)
				c = c * tex->icon.getPixel(x, y);
			else
				c = c * 0.8f;

			// specular
			f = pow(n * light_sp_dir, m->Shininess) * 0.4f;
			c += m->Specular * f;

			c = c * 0.9f;
			c.clamp();
			c.a = 1;
			img.setPixel(x, y, c);
		}
	return HuiSetImage(img);
}

string file_secure(const string &filename)
{
	if (filename.num > 0)
		return filename;
	return _("   - ohne Datei -");
}

void ModelPropertiesDialog::RefillInventaryList()
{
	Reset("model_inventary");
	foreachi(string &it, temp.Inventary, i)
		AddString("model_inventary", format("%d\\%s", i, it.c_str()));
}

void ModelPropertiesDialog::RefillScriptVarList()
{
	Reset("script_vars");
	foreachi(float v, temp.ScriptVar, i)
		/*if (i<NumObjectScriptVarNames)
			AddString("script_vars", format("%d\\%s\\%.6f", i, ObjectScriptVarName[i].c_str(), v));
		else*/
			AddString("script_vars", format("%d\\\\%.6f", i, v));
}


// viewings
void ModelPropertiesDialog::OnGenerateDistsAuto()
{
	bool b = IsChecked("");
	/*Enable("detail_dist_1", !b);
	Enable("detail_dist_2", !b);
	Enable("detail_dist_3", !b);*/
	if (b){
		data->GenerateDetailDists(temp.DetailDist);
		FillDetailList();
	}
}

// physics
void ModelPropertiesDialog::OnPhysicsPassive()
{
	Enable("ph_active", IsChecked("ph_passive"));
}

void ModelPropertiesDialog::OnGenerateTensorAuto()
{
	//case HMM_MASS:
	//case HMM_GENERATE_TENSOR_AUTO:
	bool b = IsChecked("generate_tensor_auto");
	Enable("tensor", !b);
	if (b){
		temp.InertiaTensor = data->GenerateInertiaTensor(GetFloat("mass"));
		FillTensorList();
	}
}

void ModelPropertiesDialog::OnTensorEdit()
{
	// constraint: symmetric tensor!
	int row = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (row != col - 1){
		SetCell("", col-1, row+1, GetCell("", row, col));
	}
}

// inventary
void ModelPropertiesDialog::OnNumItems()
{
	temp.Inventary.resize(GetInt(""));
	RefillInventaryList();
}

void ModelPropertiesDialog::OnModelInventary()
{
	if (ed->FileDialog(FDModel, false, true)){
		int n = GetInt("");
		temp.Inventary[n] = ed->DialogFileNoEnding;
		ChangeString("model_inventary", n, format("%d\\", n) + ed->DialogFileNoEnding);
	}
}

void ModelPropertiesDialog::OnDeleteItem()
{
	int n = GetInt("model_inventary");
	if (n >= 0){
		temp.Inventary[n] = "";
		RefillInventaryList();
	}
}

// script
void ModelPropertiesDialog::OnMaxScriptVars()
{
	temp.ScriptVar.resize(GetInt("max_script_vars"));
	RefillScriptVarList();
}

void ModelPropertiesDialog::OnScriptVarEdit()
{
	int row = HuiGetEvent()->row;
	msg_write(HuiGetEvent()->row);
	temp.ScriptVar[row] = s2f(GetCell("script_vars", row, 2));
}

void ModelPropertiesDialog::OnScriptFind()
{
	if (ed->FileDialog(FDScript, false, true))
		SetString("script", ed->DialogFile);
}

void ModelPropertiesDialog::OnModelScriptVarTemplate()
{
//	int n = GetInt("");
	//strcpy(ObjectScriptVarFile, ScriptVarFile[n]);
//	LoadScriptVarNames(1, ScriptVarFile[n]);
	RefillScriptVarList();
}

void ModelPropertiesDialog::ApplyData()
{
// visual properties
	/*temp.DetailDist[0]	= GetFloat("detail_dist_1");
	temp.DetailDist[1]	= GetFloat("detail_dist_2");
	temp.DetailDist[2]	= GetFloat("detail_dist_3");
	temp.DetailFactor[1]	= GetInt("detail_factor_2");
	temp.DetailFactor[2]	= GetInt("detail_factor_3");
	temp.AutoGenerateDists	= IsChecked("generate_dists_auto");
	temp.AutoGenerateSkin[1]	= IsChecked("generate_skin_auto");
	temp.AutoGenerateSkin[2]	= IsChecked("generate_skin_auto");*/
// physics
	temp.PassivePhysics = IsChecked("ph_passive");
	temp.ActivePhysics = IsChecked("ph_active");
	temp.Mass = GetFloat("mass");
	temp.AutoGenerateTensor = IsChecked("generate_tensor_auto");
	temp.InertiaTensor._00 = GetCell("tensor", 0, 1)._float();
	temp.InertiaTensor._01 = GetCell("tensor", 0, 2)._float();
	temp.InertiaTensor._02 = GetCell("tensor", 0, 3)._float();
	temp.InertiaTensor._10 = GetCell("tensor", 1, 1)._float();
	temp.InertiaTensor._11 = GetCell("tensor", 1, 2)._float();
	temp.InertiaTensor._12 = GetCell("tensor", 1, 3)._float();
	temp.InertiaTensor._20 = GetCell("tensor", 2, 1)._float();
	temp.InertiaTensor._21 = GetCell("tensor", 2, 2)._float();
	temp.InertiaTensor._22 = GetCell("tensor", 2, 3)._float();
// inventary
	temp.Name = GetString("model_name");
	temp.Description = GetString("model_description");
// script
	temp.ScriptFile = GetString("script");

	data->Execute(new ActionModelEditData(temp));

			//Change(true,true);
			//Change();
}

void ModelPropertiesDialog::OnClose()
{
	delete(this);
}

void ModelPropertiesDialog::OnOk()
{
	ApplyData();
	delete(this);
}
