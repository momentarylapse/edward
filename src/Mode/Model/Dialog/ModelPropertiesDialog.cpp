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
#include "../../../lib/kaba/kaba.h"
#include "../Mesh/ModeModelMeshTexture.h"



#define InertiaTensorDec			2
#define DetailDistDec				2

extern matrix3 InertiaTensorTemp;
extern float DetailDistTemp1, DetailDistTemp2, DetailDistTemp3;

ModelPropertiesDialog::ModelPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data):
	hui::Dialog("model_dialog", 400, 300, _parent, _allow_parent),
	Observer("ModelPropertiesDialog")
{
	fromResource("model_dialog");
	data = _data;
	active = true;

	setTooltip("material_list", _("- Doppelklick um ein Material zu editieren\n- Auswahl wird f&ur folgende neue Polygone verwendet"));

	event("cancel", std::bind(&ModelPropertiesDialog::OnClose, this));
	event("hui:close", std::bind(&ModelPropertiesDialog::OnClose, this));
	event("ok", std::bind(&ModelPropertiesDialog::OnOk, this));
	event("generate_dists_auto", std::bind(&ModelPropertiesDialog::OnGenerateDistsAuto, this));
	event("ph_passive", std::bind(&ModelPropertiesDialog::OnPhysicsPassive, this));
	event("generate_tensor_auto", std::bind(&ModelPropertiesDialog::OnGenerateTensorAuto, this));
	eventX("tensor", "hui:change", std::bind(&ModelPropertiesDialog::OnTensorEdit, this));
	event("mass", std::bind(&ModelPropertiesDialog::OnGenerateTensorAuto, this));
	event("num_items", std::bind(&ModelPropertiesDialog::OnNumItems, this));
	event("model_inventary", std::bind(&ModelPropertiesDialog::OnModelInventary, this));
	event("delete_item", std::bind(&ModelPropertiesDialog::OnDeleteItem, this));
	eventX("script_vars", "hui:change", std::bind(&ModelPropertiesDialog::OnScriptVarEdit, this));
	event("script_find", std::bind(&ModelPropertiesDialog::OnScriptFind, this));

	restart();
}

ModelPropertiesDialog::~ModelPropertiesDialog()
{
	mode_model->properties_dialog = NULL;
	unsubscribe(data);
}

void ModelPropertiesDialog::restart()
{
	subscribe(data);

	temp = data->meta_data;
	LoadData();
	active = true;
}


void update_model_script_data(DataModel::MetaData &m)
{
	if (m.script_file.num == 0)
		return;
	//m.class_name = "";
	try{
		msg_write(m.script_file);
		auto ss = Kaba::Load(m.script_file, true);

		Array<string> wanted;
		for (auto c:ss->syntax->constants)
			if (c->name == "PARAMETERS" and c->type == Kaba::TypeString)
				wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");

		for (auto *t: ss->syntax->classes){
			if (!t->is_derived_from("Model"))
				continue;
			msg_write(t->name);
			//m.class_name = t->name;
			for (auto &e: t->elements){
				string nn = e.name.replace("_", "").lower();
				if (!sa_contains(wanted, nn))
					continue;
				bool found = false;
				for (auto &v: m.variables)
					if (v.name.lower().replace("_", "") == nn){
						v.name = e.name;
						v.type = e.type->name;
						found = true;
					}
				if (found)
					continue;

				ModelScriptVariable v;
				v.name = e.name;
				v.type = e.type->name;
				m.variables.add(v);
			}
		}
	}catch(Kaba::Exception &e){

		msg_error(e.message());
	}

}

void ModelPropertiesDialog::LoadData()
{
	update_model_script_data(temp);

// viewing properties (LOD)
	if (temp.auto_generate_dists)
		data->generateDetailDists(temp.detail_dist);
	setInt("num_lod", 3);
	enable("num_lod", false);
	FillDetailList();
	check("generate_dists_auto", temp.auto_generate_dists);
	check("generate_skin_auto", temp.auto_generate_skin[1]);
	// physics
	if (temp.auto_generate_tensor)
		temp.inertia_tensor = data->generateInertiaTensor(temp.mass);
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
	nix::Texture *tex = nix::LoadTexture(m->texture_files[0]);
	if (!tex)
		if (m->material->textures.num > 0)
			tex = m->material->textures[0];

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
	return hui::SetImage(img);
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
	reset("variables");
	for (auto &v: temp.variables)
		addString("variables", v.name + "\\" + v.type + "\\" + v.value);
	enable("variables", temp.variables.num > 0);
}


// viewings
void ModelPropertiesDialog::OnGenerateDistsAuto()
{
	bool b = isChecked("");
	/*Enable("detail_dist_1", !b);
	Enable("detail_dist_2", !b);
	Enable("detail_dist_3", !b);*/
	if (b){
		data->generateDetailDists(temp.detail_dist);
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
		temp.inertia_tensor = data->generateInertiaTensor(getFloat("mass"));
		FillTensorList();
	}
}

void ModelPropertiesDialog::OnTensorEdit()
{
	// constraint: symmetric tensor!
	int row = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
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
	if (ed->fileDialog(FD_MODEL, false, true)){
		int n = getInt("");
		temp.inventary[n] = ed->dialog_file_no_ending;
		changeString("model_inventary", n, format("%d\\", n) + ed->dialog_file_no_ending);
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
void ModelPropertiesDialog::OnScriptVarEdit()
{
	int row = hui::GetEvent()->row;
	msg_write(hui::GetEvent()->row);
	temp.variables[row].value = getCell("script_vars", row, 2);
}

void ModelPropertiesDialog::OnScriptFind()
{
	if (ed->fileDialog(FD_SCRIPT, false, true)){
		setString("script", ed->dialog_file);
		temp.script_file = ed->dialog_file;
		update_model_script_data(temp);
	}
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
	unsubscribe(data);
	hide();
	active = false;
}

void ModelPropertiesDialog::OnOk()
{
	ApplyData();
	OnClose();
}
