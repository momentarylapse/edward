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
	from_resource("model_dialog");
	data = _data;
	active = true;

	event("cancel", std::bind(&ModelPropertiesDialog::OnClose, this));
	event("hui:close", std::bind(&ModelPropertiesDialog::OnClose, this));
	event("ok", std::bind(&ModelPropertiesDialog::OnOk, this));
	event("generate_dists_auto", std::bind(&ModelPropertiesDialog::OnGenerateDistsAuto, this));
	event("ph_passive", std::bind(&ModelPropertiesDialog::OnPhysicsPassive, this));
	event("generate_tensor_auto", std::bind(&ModelPropertiesDialog::OnGenerateTensorAuto, this));
	event_x("tensor", "hui:change", std::bind(&ModelPropertiesDialog::OnTensorEdit, this));
	event("mass", std::bind(&ModelPropertiesDialog::OnGenerateTensorAuto, this));
	event("num_items", std::bind(&ModelPropertiesDialog::OnNumItems, this));
	event("model_inventary", std::bind(&ModelPropertiesDialog::OnModelInventary, this));
	event("delete_item", std::bind(&ModelPropertiesDialog::OnDeleteItem, this));
	event_x("script_vars", "hui:change", std::bind(&ModelPropertiesDialog::OnScriptVarEdit, this));
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
		for (auto c:ss->syntax->base_class->constants)
			if (c->name == "PARAMETERS" and c->type == Kaba::TypeString)
				wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");

		for (auto *t: ss->syntax->base_class->classes){
			if (!t->is_derived_from_s("Model"))
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
	set_int("num_lod", 3);
	enable("num_lod", false);
	FillDetailList();
	check("generate_dists_auto", temp.auto_generate_dists);
	check("generate_skin_auto", temp.auto_generate_skin[1]);
	// physics
	if (temp.auto_generate_tensor)
		temp.inertia_tensor = data->generateInertiaTensor(temp.mass);
	set_float("mass", temp.mass);
	check("ph_active", temp.active_physics);
	check("ph_passive", temp.passive_physics);
	enable("ph_active", temp.passive_physics);
	FillTensorList();
	enable("tensor", !temp.auto_generate_tensor);
	check("generate_tensor_auto", temp.auto_generate_tensor);
	// items
	set_string("model_name", temp.name);
	set_string("model_description", temp.description);
	set_int("num_items", temp.inventary.num);
	RefillInventaryList();
	// script
	set_string("script", temp.script_file);
	set_int("max_script_vars", temp.script_var.num);
	RefillScriptVarList();
}

void ModelPropertiesDialog::FillDetailList()
{
	reset("lod");
	//DetailDistDec
	add_string("lod", format("0\\%.2f\\%d", temp.detail_dist[0], 100));
	add_string("lod", format("1\\%.2f\\%d", temp.detail_dist[1], temp.detail_factor[1]));
	add_string("lod", format("2\\%.2f\\%d", temp.detail_dist[2], temp.detail_factor[2]));
}

void ModelPropertiesDialog::FillTensorList()
{
	//SetDecimals(InertiaTensorDec);
	reset("tensor");
	add_string("tensor", format("X\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._00, temp.inertia_tensor._01, temp.inertia_tensor._02));
	add_string("tensor", format("Y\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._10, temp.inertia_tensor._11, temp.inertia_tensor._12));
	add_string("tensor", format("Z\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._20, temp.inertia_tensor._21, temp.inertia_tensor._22));
}

void ModelPropertiesDialog::on_update(Observable *o, const string &message)
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
	/*nix::Texture *tex = nix::LoadTexture(m->texture_levels[0].texture);
	if (!tex)
		if (m->material->textures.num > 0)
			tex = m->material->textures[0];*/
	auto tim = m->texture_levels[0]->image;

	const int N = 48;

	// simulate a lit sphere
	Image img;
	img.create(N, N, Black);
	vector light_dir = vector(-1, -1, -1);
	light_dir.normalize();
	vector cam_dir = - vector::EZ;
	vector light_sp_dir = light_dir + cam_dir;
	light_sp_dir.normalize();
	for (int x=0;x<N;x++)
		for (int y=0;y<N;y++){
			// ambient + diffuse + emission
			vector n = img_get_ball_n(x, y, N);
			float f = clampf(n * light_dir, 0, 1);
			color c = m->col.ambient * 0.3f + m->col.diffuse * f + m->col.emission;

			// texture "mapping"
			c = c * tim->get_pixel((x * tim->width) / N, (y * tim->height) / N);

			// specular
			f = pow(n * light_sp_dir, m->col.shininess) * 0.4f;
			c += m->col.specular * f;

			c = c * 0.9f;
			c.clamp();
			c.a = 1;
			img.set_pixel(x, y, c);
		}
	return hui::SetImage(&img, format("image:material-%p", m));
}

string file_secure(const string &filename)
{
	if (filename.num > 0)
		return filename;
	return _("   - no file -");
}

void ModelPropertiesDialog::RefillInventaryList()
{
	reset("model_inventary");
	foreachi(string &it, temp.inventary, i)
		add_string("model_inventary", format("%d\\%s", i, it.c_str()));
}

void ModelPropertiesDialog::RefillScriptVarList()
{
	reset("variables");
	for (auto &v: temp.variables)
		add_string("variables", v.name + "\\" + v.type + "\\" + v.value);
	enable("variables", temp.variables.num > 0);
}


// viewings
void ModelPropertiesDialog::OnGenerateDistsAuto()
{
	bool b = is_checked("");
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
	enable("ph_active", is_checked("ph_passive"));
}

void ModelPropertiesDialog::OnGenerateTensorAuto()
{
	//case HMM_MASS:
	//case HMM_GENERATE_TENSOR_AUTO:
	bool b = is_checked("generate_tensor_auto");
	enable("tensor", !b);
	if (b){
		temp.inertia_tensor = data->generateInertiaTensor(get_float("mass"));
		FillTensorList();
	}
}

void ModelPropertiesDialog::OnTensorEdit()
{
	// constraint: symmetric tensor!
	int row = hui::GetEvent()->row;
	int col = hui::GetEvent()->column;
	if (row != col - 1){
		set_cell("", col-1, row+1, get_cell("", row, col));
	}
}

// inventary
void ModelPropertiesDialog::OnNumItems()
{
	temp.inventary.resize(get_int(""));
	RefillInventaryList();
}

void ModelPropertiesDialog::OnModelInventary()
{
	if (ed->file_dialog(FD_MODEL, false, true)){
		int n = get_int("");
		temp.inventary[n] = ed->dialog_file_no_ending;
		change_string("model_inventary", n, format("%d\\", n) + ed->dialog_file_no_ending);
	}
}

void ModelPropertiesDialog::OnDeleteItem()
{
	int n = get_int("model_inventary");
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
	temp.variables[row].value = get_cell("script_vars", row, 2);
}

void ModelPropertiesDialog::OnScriptFind()
{
	if (ed->file_dialog(FD_SCRIPT, false, true)){
		set_string("script", ed->dialog_file);
		temp.script_file = ed->dialog_file;
		update_model_script_data(temp);
	}
}

void ModelPropertiesDialog::ApplyData()
{
// visual properties
	/*temp.DetailDist[0]	= get_float("detail_dist_1");
	temp.DetailDist[1]	= get_float("detail_dist_2");
	temp.DetailDist[2]	= get_float("detail_dist_3");
	temp.DetailFactor[1]	= get_int("detail_factor_2");
	temp.DetailFactor[2]	= get_int("detail_factor_3");
	temp.AutoGenerateDists	= is_checked("generate_dists_auto");
	temp.AutoGenerateSkin[1]	= is_checked("generate_skin_auto");
	temp.AutoGenerateSkin[2]	= is_checked("generate_skin_auto");*/
// physics
	temp.passive_physics = is_checked("ph_passive");
	temp.active_physics = is_checked("ph_active");
	temp.mass = get_float("mass");
	temp.auto_generate_tensor = is_checked("generate_tensor_auto");
	temp.inertia_tensor._00 = get_cell("tensor", 0, 1)._float();
	temp.inertia_tensor._01 = get_cell("tensor", 0, 2)._float();
	temp.inertia_tensor._02 = get_cell("tensor", 0, 3)._float();
	temp.inertia_tensor._10 = get_cell("tensor", 1, 1)._float();
	temp.inertia_tensor._11 = get_cell("tensor", 1, 2)._float();
	temp.inertia_tensor._12 = get_cell("tensor", 1, 3)._float();
	temp.inertia_tensor._20 = get_cell("tensor", 2, 1)._float();
	temp.inertia_tensor._21 = get_cell("tensor", 2, 2)._float();
	temp.inertia_tensor._22 = get_cell("tensor", 2, 3)._float();
// inventary
	temp.name = get_string("model_name");
	temp.description = get_string("model_description");
// script
	temp.script_file = get_string("script");

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
