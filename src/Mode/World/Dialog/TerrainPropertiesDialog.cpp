/*
 * TerrainPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "TerrainPropertiesDialog.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Data/World/WorldTerrain.h"
#include "../../../Action/World/Terrain/ActionWorldEditTerrain.h"
#include "../../../Edward.h"
#include "../../../Storage/Storage.h"
#include "../../../y/Terrain.h"
#include "../../../y/Material.h"
#include "../../../y/ResourceManager.h"
#include "../../../lib/nix/nix.h"
#include <assert.h>

string file_secure(const Path &filename); // -> ModelPropertiesDialog

TerrainPropertiesDialog::TerrainPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data, int _index) :
	hui::Dialog("terrain_dialog", 400, 300, _parent, _allow_parent)
{
	from_resource("terrain_dialog");
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->terrains.num);

	event("cancel", [=]{ on_close(); });
	event("hui:close", [=]{ on_close(); });
	event("apply", [=]{ apply_data(); });
	event("ok", [=]{ on_ok(); });

	event("add_texture_level", [=]{ on_add_texture_level(); });
	event("delete_texture_level", [=]{ on_delete_texture_level(); });
	event("clear_texture_level", [=]{ on_clear_texture_level(); });
	event("texture_map_complete", [=]{ on_texture_map_complete(); });
	event("textures", [=]{ on_textures(); });
	event_x("textures", "hui:change", [=]{ on_textures_edit(); });
	event_x("textures", "hui:select", [=]{ on_textures_select(); });
	event("material_find", [=]{ on_material_find(); });
	event("default_material", [=]{ on_default_material(); });
	event("terrain_save_as", [=]{ on_save_as(); });

	data->subscribe(this, [=]{ update_data(); });

	update_data();
}

TerrainPropertiesDialog::~TerrainPropertiesDialog() {
	data->unsubscribe(this);
}

void TerrainPropertiesDialog::apply_data() {
	temp.material_file = get_string("material");
	data->execute(new ActionWorldEditTerrain(index, temp));
}



void TerrainPropertiesDialog::on_textures() {
	int n = get_int("textures");
	if (storage->file_dialog(FD_TEXTURE, false, true)) {
		temp.texture_file[n] = storage->dialog_file;
		fill_texture_list();
	}
}


void TerrainPropertiesDialog::fill_texture_list() {
	Material *m = LoadMaterial(get_string("material"));

	reset("textures");
	for (int i=0;i<temp.num_textures;i++) {
		nix::Texture *tex = nullptr;
		if (!temp.texture_file[i].is_empty()) {
			try {
				tex = ResourceManager::load_texture(temp.texture_file[i]);
			} catch (Exception &e) {
			}
		}
		if (!tex)
			if (i < m->textures.num)
				tex = m->textures[i].get();
		string img = ed->get_tex_image(tex);
		add_string("textures", format("%d\\%.5f\\%.5f\\%s\\%s", i, temp.texture_scale[i].x, temp.texture_scale[i].z, img, file_secure(temp.texture_file[i])));
	}
	enable("delete_texture_level", false);
	enable("clear_texture_level", false);
	enable("texture_map_complete", false);
}



void TerrainPropertiesDialog::load_data() {
	set_string("material", temp.material_file.str());
	check("default_material", temp.material_file.is_empty());
	enable("material", false);//(temp.MaterialFile != ""));
	fill_texture_list();

	set_string("filename", temp.filename.str());
	set_int("num_x", temp.num_x);
	set_int("num_z", temp.num_z);
	set_float("pattern_x", temp.pattern.x);
	set_float("pattern_z", temp.pattern.z);
}



void TerrainPropertiesDialog::on_textures_edit() {
	int col = hui::GetEvent()->column;
	int row = hui::GetEvent()->row;
	if (col == 1)
		temp.texture_scale[row].x = s2f(get_cell("textures", row, col));
	else if (col == 2)
		temp.texture_scale[row].z = s2f(get_cell("textures", row, col));
}



void TerrainPropertiesDialog::on_save_as() {
	if (!storage->file_dialog(FD_TERRAIN, true, true))
		return;
	data->terrains[index].save(storage->dialog_file_complete);
	set_string("filename", storage->dialog_file_no_ending.str());
}



void TerrainPropertiesDialog::on_delete_texture_level() {
	int n = get_int("textures");
	if (temp.num_textures <= 1) {
		ed->error_box(_("There has to be at least one texture level!"));
		return;
	}
	for (int i=n;i<temp.num_textures-1;i++) {
		temp.texture_file[i] = temp.texture_file[i+1];
		temp.texture_scale[i] = temp.texture_scale[i+1];
	}
	temp.num_textures --;

	fill_texture_list();
}



void TerrainPropertiesDialog::on_default_material() {
	set_string("material", "");
}



void TerrainPropertiesDialog::on_material_find() {
	if (storage->file_dialog(FD_MATERIAL, false, true))
		set_string("material", storage->dialog_file_no_ending.str());
}



void TerrainPropertiesDialog::on_texture_map_complete() {
	int s = get_int("textures");
	temp.texture_scale[s].x = 1.0f;
	temp.texture_scale[s].z = 1.0f;

	fill_texture_list();
}



void TerrainPropertiesDialog::on_close() {
	request_destroy();
}



void TerrainPropertiesDialog::on_add_texture_level() {
	if (temp.num_textures >= MATERIAL_MAX_TEXTURES) {
		ed->error_box(format(_("No more than %d textures per terrain allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	int l = temp.num_textures;
	temp.texture_file[l] = "";
	temp.texture_scale[l] = vector(0.1f,0,0.1f);
	temp.num_textures ++;

	fill_texture_list();
}

void TerrainPropertiesDialog::on_textures_select() {
	int s = get_int("textures");
	enable("delete_texture_level", s >= 0);
	enable("clear_texture_level", s >= 0);
	enable("texture_map_complete", s >= 0);
}

void TerrainPropertiesDialog::on_clear_texture_level() {
	int s = get_int("textures");
	if (s >= 0)
		temp.texture_file[s] = "";

	fill_texture_list();
}

void TerrainPropertiesDialog::update_data() {
	if (index >= data->terrains.num) {
		request_destroy();
		return;
	}

	Terrain *t = data->terrains[index].terrain;
	assert(t);
	temp.filename = data->terrains[index].filename;
	temp.num_x = t->num_x;
	temp.num_z = t->num_z;
	temp.pattern = t->pattern;
	temp.num_textures = t->material->textures.num;
	for (int i=0; i<temp.num_textures; i++) {
		temp.texture_file[i] = t->texture_file[i];
		temp.texture_scale[i] = t->texture_scale[i];
	}
	temp.material_file = t->material_file;
	load_data();
}



void TerrainPropertiesDialog::on_ok() {
	apply_data();
	request_destroy();
}


