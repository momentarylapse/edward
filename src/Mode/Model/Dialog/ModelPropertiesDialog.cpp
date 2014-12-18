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
#include "../../../Action/Model/Data/ActionModelAddMaterial.h"
#include "../../../Action/Model/Data/ActionModelEditData.h"
#include "../../../lib/nix/nix.h"
#include "../Mesh/ModeModelMeshTexture.h"



#define InertiaTensorDec			2
#define DetailDistDec				2

extern matrix3 InertiaTensorTemp;
extern float DetailDistTemp1, DetailDistTemp2, DetailDistTemp3;

ModelPropertiesDialog::ModelPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data):
	HuiWindow("model_dialog", _parent, _allow_parent),
	Observer("ModelPropertiesDialog")
{
	data = _data;

	setTooltip("material_list", _("- Doppelklick um ein Material zu editieren\n- Auswahl wird f&ur folgende neue Polygone verwendet"));

	event("cancel", this, &ModelPropertiesDialog::OnClose);
	event("hui:close", this, &ModelPropertiesDialog::OnClose);
	event("ok", this, &ModelPropertiesDialog::OnOk);
	event("generate_dists_auto", this, &ModelPropertiesDialog::OnGenerateDistsAuto);
	event("ph_passive", this, &ModelPropertiesDialog::OnPhysicsPassive);
	event("generate_tensor_auto", this, &ModelPropertiesDialog::OnGenerateTensorAuto);
	eventX("tensor", "hui:change", this, &ModelPropertiesDialog::OnTensorEdit);
	event("mass", this, &ModelPropertiesDialog::OnGenerateTensorAuto);
	event("num_items", this, &ModelPropertiesDialog::OnNumItems);
	event("model_inventary", this, &ModelPropertiesDialog::OnModelInventary);
	event("delete_item", this, &ModelPropertiesDialog::OnDeleteItem);
	event("max_script_vars", this, &ModelPropertiesDialog::OnMaxScriptVars);
	eventX("script_vars", "hui:change", this, &ModelPropertiesDialog::OnScriptVarEdit);
	event("script_find", this, &ModelPropertiesDialog::OnScriptFind);
	event("model_script_var_template", this, &ModelPropertiesDialog::OnModelScriptVarTemplate);

	subscribe(data);

	temp = data->meta_data;
	LoadData();
}

ModelPropertiesDialog::~ModelPropertiesDialog()
{
	mode_model->PropertiesDialog = NULL;
	unsubscribe(data);
}

void ModelPropertiesDialog::LoadData()
{
// viewing properties (LOD)
	if (temp.auto_generate_dists)
		data->GenerateDetailDists(temp.detail_dist);
	setInt("num_lod", 3);
	enable("num_lod", false);
	FillDetailList();
	check("generate_dists_auto", temp.auto_generate_dists);
	check("generate_skin_auto", temp.auto_generate_skin[1]);
	// physics
	if (temp.auto_generate_tensor)
		temp.inertia_tensor = data->GenerateInertiaTensor(temp.mass);
	setFloat("mass", temp.mass);
	check("ph_active", temp.active_physics);
	check("ph_passive", temp.passive_physics);
	enable("ph_active", temp.passive_physics);
	FillTensorList();
	enable("tensor", !temp.auto_generate_tensor);
	check("generate_tensor_auto", temp.auto_generate_tensor);
	// items
	setString("model_name", temp.name);
	setString("model_description", temp.description);
	setInt("num_items", temp.inventary.num);
	RefillInventaryList();
	// script
	setString("script", temp.script_file);
	setInt("max_script_vars", temp.script_var.num);
	RefillScriptVarList();

	/*for (int i=0;i<NumScriptVarFiles;i++)
		SetString("model_script_var_template", data->ScriptVarFile[i]);*/
}

void ModelPropertiesDialog::FillDetailList()
{
	reset("lod");
	//DetailDistDec
	addString("lod", format("0\\%.2f\\%d", temp.detail_dist[0], 100));
	addString("lod", format("1\\%.2f\\%d", temp.detail_dist[1], temp.detail_factor[1]));
	addString("lod", format("2\\%.2f\\%d", temp.detail_dist[2], temp.detail_factor[2]));
}

void ModelPropertiesDialog::FillTensorList()
{
	//SetDecimals(InertiaTensorDec);
	reset("tensor");
	addString("tensor", format("X\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._00, temp.inertia_tensor._01, temp.inertia_tensor._02));
	addString("tensor", format("Y\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._10, temp.inertia_tensor._11, temp.inertia_tensor._12));
	addString("tensor", format("Z\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._20, temp.inertia_tensor._21, temp.inertia_tensor._22));
}

void ModelPropertiesDialog::onUpdate(Observable *o, const string &message)
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
	NixTexture *tex = NixLoadTexture(m->texture_file[0]);
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
			color c = m->ambient * 0.3f + m->diffuse * f + m->emission;

			// texture "mapping"
			if (tex)
				c = c * tex->icon.getPixel(x, y);
			else
				c = c * 0.8f;

			// specular
			f = pow(n * light_sp_dir, m->shininess) * 0.4f;
			c += m->specular * f;

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
	reset("model_inventary");
	foreachi(string &it, temp.inventary, i)
		addString("model_inventary", format("%d\\%s", i, it.c_str()));
}

void ModelPropertiesDialog::RefillScriptVarList()
{
	reset("script_vars");
	foreachi(float v, temp.script_var, i)
		/*if (i<NumObjectScriptVarNames)
			addString("script_vars", format("%d\\%s\\%.6f", i, ObjectScriptVarName[i].c_str(), v));
		else*/
			addString("script_vars", format("%d\\\\%.6f", i, v));
}


// viewings
void ModelPropertiesDialog::OnGenerateDistsAuto()
{
	bool b = isChecked("");
	/*Enable("detail_dist_1", !b);
	Enable("detail_dist_2", !b);
	Enable("detail_dist_3", !b);*/
	if (b){
		data->GenerateDetailDists(temp.detail_dist);
		FillDetailList();
	}
}

// physics
void ModelPropertiesDialog::OnPhysicsPassive()
{
	enable("ph_active", isChecked("ph_passive"));
}

void ModelPropertiesDialog::OnGenerateTensorAuto()
{
	//case HMM_MASS:
	//case HMM_GENERATE_TENSOR_AUTO:
	bool b = isChecked("generate_tensor_auto");
	enable("tensor", !b);
	if (b){
		temp.inertia_tensor = data->GenerateInertiaTensor(getFloat("mass"));
		FillTensorList();
	}
}

void ModelPropertiesDialog::OnTensorEdit()
{
	// constraint: symmetric tensor!
	int row = HuiGetEvent()->row;
	int col = HuiGetEvent()->column;
	if (row != col - 1){
		setCell("", col-1, row+1, getCell("", row, col));
	}
}

// inventary
void ModelPropertiesDialog::OnNumItems()
{
	temp.inventary.resize(getInt(""));
	RefillInventaryList();
}

void ModelPropertiesDialog::OnModelInventary()
{
	if (ed->fileDialog(FDModel, false, true)){
		int n = getInt("");
		temp.inventary[n] = ed->DialogFileNoEnding;
		changeString("model_inventary", n, format("%d\\", n) + ed->DialogFileNoEnding);
	}
}

void ModelPropertiesDialog::OnDeleteItem()
{
	int n = getInt("model_inventary");
	if (n >= 0){
		temp.inventary[n] = "";
		RefillInventaryList();
	}
}

// script
void ModelPropertiesDialog::OnMaxScriptVars()
{
	temp.script_var.resize(getInt("max_script_vars"));
	RefillScriptVarList();
}

void ModelPropertiesDialog::OnScriptVarEdit()
{
	int row = HuiGetEvent()->row;
	msg_write(HuiGetEvent()->row);
	temp.script_var[row] = s2f(getCell("script_vars", row, 2));
}

void ModelPropertiesDialog::OnScriptFind()
{
	if (ed->fileDialog(FDScript, false, true))
		setString("script", ed->DialogFile);
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
	temp.passive_physics = isChecked("ph_passive");
	temp.active_physics = isChecked("ph_active");
	temp.mass = getFloat("mass");
	temp.auto_generate_tensor = isChecked("generate_tensor_auto");
	temp.inertia_tensor._00 = getCell("tensor", 0, 1)._float();
	temp.inertia_tensor._01 = getCell("tensor", 0, 2)._float();
	temp.inertia_tensor._02 = getCell("tensor", 0, 3)._float();
	temp.inertia_tensor._10 = getCell("tensor", 1, 1)._float();
	temp.inertia_tensor._11 = getCell("tensor", 1, 2)._float();
	temp.inertia_tensor._12 = getCell("tensor", 1, 3)._float();
	temp.inertia_tensor._20 = getCell("tensor", 2, 1)._float();
	temp.inertia_tensor._21 = getCell("tensor", 2, 2)._float();
	temp.inertia_tensor._22 = getCell("tensor", 2, 3)._float();
// inventary
	temp.name = getString("model_name");
	temp.description = getString("model_description");
// script
	temp.script_file = getString("script");

	data->execute(new ActionModelEditData(temp));

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
