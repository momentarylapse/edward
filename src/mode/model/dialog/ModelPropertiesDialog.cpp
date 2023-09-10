/*
 * ModelPropertiesDialog.cpp
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#include "ModelPropertiesDialog.h"
#include "../mesh/ModeModelMesh.h"
#include "../mesh/ModeModelMeshTexture.h"
#include "../ModeModel.h"
#include "../../../EdwardWindow.h"
#include "../../../storage/Storage.h"
#include "../../../action/model/data/ActionModelAddMaterial.h"
#include "../../../action/model/data/ActionModelEditData.h"
#include "../../../lib/nix/nix.h"
#include "../../../lib/kaba/kaba.h"



#define InertiaTensorDec			2
#define DetailDistDec				2


ModelPropertiesDialog::ModelPropertiesDialog(hui::Window *_parent, DataModel *_data):
	hui::Dialog("model_dialog", 400, 300, _parent, false)
{
	from_resource("model_dialog");
	data = _data;

	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
	event("ok", [=]{ on_ok(); });
	event("generate_dists_auto", [=]{ on_generate_dists_auto(); });
	event("ph_passive", [=]{ on_physics_passive(); });
	event("generate_tensor_auto", [=]{ on_generate_tensor_auto(); });
	event_x("tensor", "hui:change", [=]{ on_tensor_edit(); });
	event("mass", [=]{ on_generate_tensor_auto(); });
	event("num_items", [=]{ on_num_items(); });
	event("model_inventary", [=]{ on_model_inventary(); });
	event("delete_item", [=]{ on_delete_item(); });
	event_x("variables", "hui:change", [=]{ on_script_var_edit(); });
	event("script_find", [=]{ on_script_find(); });

	temp = data->meta_data;
	load_data();
}

ModelPropertiesDialog::~ModelPropertiesDialog() {
}

shared<const kaba::Class> get_class_by_base(shared<kaba::Module> s, const string &parent) {
	for (auto t: s->tree->base_class->classes)
		if (t->is_derived_from_s(parent))
			return t;
	throw Exception(format(_("script does not contain a class derived from '%s'"), parent));
	return nullptr;
}

void update_model_script_data(EdwardWindow *ed, DataModel::MetaData &m) {

	// remove undefined
	for (int i=m.variables.num-1; i>=0; i--)
		if (m.variables[i].value == "")
			m.variables.erase(i);
	m._script_class = "";

	if (m.script_file.is_empty())
		return;


	//m.class_name = "";
	try {
		msg_write((ed->storage->root_dir_kind[FD_SCRIPT] | m.script_file).str());
		auto c = ownify(kaba::Context::create());
		auto ss = c->load_module(ed->storage->root_dir_kind[FD_SCRIPT] | m.script_file, true);

		auto t = get_class_by_base(ss, "*.Model");
		m._script_class = t->cname(t->owner->base_class);

		Array<string> wanted;
		for (auto c: t->constants)
			if (c->name == "PARAMETERS" and c->type == kaba::TypeString)
				wanted = c->as_string().lower().replace("_", "").replace("\n", "").explode(",");

		//m.class_name = t->name;
		for (auto &e: t->elements) {
			string nn = e.name.replace("_", "").lower();
			if (!sa_contains(wanted, nn))
				continue;
			bool found = false;
			for (auto &v: m.variables)
				if (v.name.lower().replace("_", "") == nn) {
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
	} catch (Exception &e) {

		ed->error_box(e.message());
	}

}

void ModelPropertiesDialog::load_data() {
	update_model_script_data(data->ed, temp);

// viewing properties (LOD)
	if (temp.auto_generate_dists)
		data->generateDetailDists(temp.detail_dist);
	set_int("num_lod", 3);
	enable("num_lod", false);
	fill_detail_list();
	check("generate_dists_auto", temp.auto_generate_dists);
	check("generate_skin_auto", temp.auto_generate_skin[1]);
	// physics
	if (temp.auto_generate_tensor)
		temp.inertia_tensor = data->generateInertiaTensor(temp.mass);
	set_float("mass", temp.mass);
	check("ph_active", temp.active_physics);
	check("ph_passive", temp.passive_physics);
	enable("ph_active", temp.passive_physics);
	fill_tensor_list();
	enable("tensor", !temp.auto_generate_tensor);
	check("generate_tensor_auto", temp.auto_generate_tensor);
	// items
	set_string("model_name", temp.name);
	set_string("model_description", temp.description);
	set_int("num_items", temp.inventary.num);
	refill_inventary_list();
	// script
	set_string("script", temp.script_file.str());
	set_int("max_script_vars", temp.script_var.num);
	refill_script_var_list();
}

void ModelPropertiesDialog::fill_detail_list() {
	reset("lod");
	//DetailDistDec
	add_string("lod", format("0\\%.2f\\%d", temp.detail_dist[0], 100));
	add_string("lod", format("1\\%.2f\\%d", temp.detail_dist[1], temp.detail_factor[1]));
	add_string("lod", format("2\\%.2f\\%d", temp.detail_dist[2], temp.detail_factor[2]));
}

void ModelPropertiesDialog::fill_tensor_list() {
	//SetDecimals(InertiaTensorDec);
	reset("tensor");
	add_string("tensor", format("X\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._00, temp.inertia_tensor._01, temp.inertia_tensor._02));
	add_string("tensor", format("Y\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._10, temp.inertia_tensor._11, temp.inertia_tensor._12));
	add_string("tensor", format("Z\\%.2f\\%.2f\\%.2f", temp.inertia_tensor._20, temp.inertia_tensor._21, temp.inertia_tensor._22));
}


vec3 img_get_ball_n(int x, int y, int N) {
	//vector n = vector(x - N/2, y - N/2, 0);
	vec3 n = vec3(x - N/2, y - N/2, 0);
	n.z = - sqrt(N*N/2 - n.x*n.x - n.y*n.y);
	n.normalize();
	return n;
}

string render_material(ModelMaterial *m) {
	// texture?
	/*nix::Texture *tex = nix::LoadTexture(m->texture_levels[0].texture);
	if (!tex)
		if (m->material->textures.num > 0)
			tex = m->material->textures[0];*/
	static Image default_image;
	if (default_image.width == 0)
		default_image.create(16, 16, White);
	auto tim = &default_image;
	if (m->texture_levels.num > 0)
		tim = m->texture_levels[0]->image;

	const int N = 48;

	// simulate a lit sphere
	Image img;
	img.create(N, N, Black);
	vec3 light_dir = vec3(-1, -1, -1);
	light_dir.normalize();
	vec3 cam_dir = - vec3::EZ;
	vec3 light_sp_dir = light_dir + cam_dir;
	light_sp_dir.normalize();
	for (int x=0;x<N;x++)
		for (int y=0;y<N;y++) {
			// ambient + diffuse + emission
			vec3 n = img_get_ball_n(x, y, N);
			float f = clamp(vec3::dot(n, light_dir), 0.0f, 1.0f);
			color c = m->col.albedo * (m->col.roughness * 0.3f + f) + m->col.emission;

			// texture "mapping"
			c = c * tim->get_pixel((x * tim->width) / N, (y * tim->height) / N);

			// specular
			f = pow(vec3::dot(n, light_sp_dir), 10) * 0.4f;
			c += White * m->col.metal * f;

			c = c * 0.9f;
			c.clamp();
			c.a = 1;
			img.set_pixel(x, y, c);
		}
	return hui::set_image(&img, format("image:material-%s", p2s(m)));
}

string file_secure(const Path &filename) {
	if (filename.is_empty())
		return _("   - no file -");
	return filename.str();
}

void ModelPropertiesDialog::refill_inventary_list() {
	reset("model_inventary");
	foreachi(auto &it, temp.inventary, i)
		add_string("model_inventary", format("%d\\%s", i, it));
}

void ModelPropertiesDialog::refill_script_var_list() {
	reset("variables");
	for (auto &v: temp.variables)
		add_string("variables", v.name + "\\" + v.type + "\\" + v.value);
	enable("variables", temp.variables.num > 0);

	if (temp._script_class == "")
		set_string("class", _("\u274C     no class derived from 'Model' found"));
	else
		set_string("class", "\u2714    class: '" + temp._script_class + "'");
}


// viewings
void ModelPropertiesDialog::on_generate_dists_auto() {
	bool b = is_checked("");
	/*Enable("detail_dist_1", !b);
	Enable("detail_dist_2", !b);
	Enable("detail_dist_3", !b);*/
	if (b) {
		data->generateDetailDists(temp.detail_dist);
		fill_detail_list();
	}
}

// physics
void ModelPropertiesDialog::on_physics_passive() {
	enable("ph_active", is_checked("ph_passive"));
}

void ModelPropertiesDialog::on_generate_tensor_auto() {
	//case HMM_MASS:
	//case HMM_GENERATE_TENSOR_AUTO:
	bool b = is_checked("generate_tensor_auto");
	enable("tensor", !b);
	if (b) {
		temp.inertia_tensor = data->generateInertiaTensor(get_float("mass"));
		fill_tensor_list();
	}
}

void ModelPropertiesDialog::on_tensor_edit() {
	// constraint: symmetric tensor!
	int row = hui::get_event()->row;
	int col = hui::get_event()->column;
	if (row != col - 1) {
		set_cell("", col-1, row+1, get_cell("", row, col));
	}
}

// inventary
void ModelPropertiesDialog::on_num_items() {
	temp.inventary.resize(get_int(""));
	refill_inventary_list();
}

void ModelPropertiesDialog::on_model_inventary() {
	data->ed->storage->file_dialog(FD_MODEL, false, true).on([this] (const auto& p) {
		int n = get_int("");
		temp.inventary[n] = p.simple;
		change_string("model_inventary", n, format("%d\\%s", n, p.simple));
	});
}

void ModelPropertiesDialog::on_delete_item() {
	int n = get_int("model_inventary");
	if (n >= 0) {
		temp.inventary[n] = "";
		refill_inventary_list();
	}
}

// script
void ModelPropertiesDialog::on_script_var_edit() {
	int row = hui::get_event()->row;
	temp.variables[row].value = get_cell("variables", row, 2);
}

void ModelPropertiesDialog::on_script_find() {
	data->ed->storage->file_dialog(FD_SCRIPT, false, true).on([this] (const auto& p) {
		set_string("script", p.relative.str());
		temp.script_file = p.relative;
		update_model_script_data(data->ed, temp);
		refill_script_var_list();
	});
}

void ModelPropertiesDialog::apply_data() {
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

void ModelPropertiesDialog::on_close() {
	request_destroy();
}

void ModelPropertiesDialog::on_ok() {
	apply_data();
	on_close();
}
