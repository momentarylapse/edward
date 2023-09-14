/*
 * TerrainPropertiesDialog.cpp
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#include "TerrainPropertiesDialog.h"
#include "../../../data/world/DataWorld.h"
#include "../../../data/world/WorldTerrain.h"
#include "../../../action/world/terrain/ActionWorldEditTerrain.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../storage/Storage.h"
#include <y/world/Terrain.h>
#include <y/world/Material.h>
#include <y/helper/ResourceManager.h>
#include "../../../lib/nix/nix.h"
#include <assert.h>

string file_secure(const Path &filename); // -> ModelPropertiesDialog

TerrainPropertiesDialog::TerrainPropertiesDialog(DataWorld *_data, int _index) :
	obs::Node<hui::Dialog>("terrain_dialog", 400, 300, _data->session->win, false)
{
	from_resource("terrain_dialog");
	data = _data;
	index = _index;
	assert(index >= 0);
	assert(index < data->terrains.num);

	event("cancel", [this]{ on_close(); });
	event("hui:close", [this]{ on_close(); });
	event("apply", [this]{ apply_data(); });
	event("ok", [this]{ on_ok(); });

	event("add_texture_level", [this]{ on_add_texture_level(); });
	event("delete_texture_level", [this]{ on_delete_texture_level(); });
	event("clear_texture_level", [this]{ on_clear_texture_level(); });
	event("texture_map_complete", [this]{ on_texture_map_complete(); });
	event("textures", [this]{ on_textures(); });
	event_x("textures", "hui:change", [this]{ on_textures_edit(); });
	event_x("textures", "hui:select", [this]{ on_textures_select(); });
	event("material_find", [this]{ on_material_find(); });
	event("default_material", [this]{ on_default_material(); });
	event("terrain_save_as", [this]{ on_save_as(); });

	data->out_changed >> create_sink([this]{ update_data(); });

	update_data();
}

TerrainPropertiesDialog::~TerrainPropertiesDialog() {
	data->unsubscribe(this);
}

void TerrainPropertiesDialog::apply_data() {
	temp.pattern.x = get_float("pattern_x");
	temp.pattern.z = get_float("pattern_z");
	data->execute(new ActionWorldEditTerrain(index, temp));
}



void TerrainPropertiesDialog::on_textures() {
	int n = get_int("textures");
	data->session->storage->file_dialog(FD_TEXTURE, false, true).on([this, n] (const auto& p) {
		temp.texture_file[n] = p.relative;
		fill_texture_list();
	});
}


void TerrainPropertiesDialog::fill_texture_list() {
	Material *m = data->session->resource_manager->load_material(get_string("material"));

	reset("textures");
	for (int i=0;i<temp.num_textures;i++) {
		shared<nix::Texture> tex;
		if (!temp.texture_file[i].is_empty()) {
			try {
				tex = data->session->resource_manager->load_texture(temp.texture_file[i]);
			} catch (Exception &e) {
			}
		}
		if (!tex)
			if (i < m->textures.num)
				tex = m->textures[i];
		string img = data->session->get_tex_image(tex.get());
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

	Terrain *t = data->terrains[index].terrain;

	set_string("filename", temp.filename.str());
	set_int("num_x",  t->num_x);
	set_int("num_z", t->num_z);
	enable("num_x", false);
	enable("num_z", false);
	set_float("pattern_x", temp.pattern.x);
	set_float("pattern_z", temp.pattern.z);
}



void TerrainPropertiesDialog::on_textures_edit() {
	int col = hui::get_event()->column;
	int row = hui::get_event()->row;
	if (col == 1)
		temp.texture_scale[row].x = s2f(get_cell("textures", row, col));
	else if (col == 2)
		temp.texture_scale[row].z = s2f(get_cell("textures", row, col));
}



void TerrainPropertiesDialog::on_save_as() {
	data->session->storage->file_dialog(FD_TERRAIN, true, true).on([this] (const auto& p) {
		data->terrains[index].save(p.complete);
		set_string("filename", p.simple.str());
	});
}



void TerrainPropertiesDialog::on_delete_texture_level() {
	int n = get_int("textures");
	if (temp.num_textures <= 1) {
		data->session->error(_("There has to be at least one texture level!"));
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
	temp.material_file = "";
	set_string("material", "");
}



void TerrainPropertiesDialog::on_material_find() {
	data->session->storage->file_dialog(FD_MATERIAL, false, true).on([this] (const auto& p) {
		temp.material_file = p.simple;
		set_string("material", p.simple.str());
		check("default_material", false);
	});
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
		data->session->error(format(_("No more than %d textures per terrain allowed!"), MATERIAL_MAX_TEXTURES));
		return;
	}
	int l = temp.num_textures;
	temp.texture_file[l] = "";
	temp.texture_scale[l] = vec3(0.1f,0,0.1f);
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


