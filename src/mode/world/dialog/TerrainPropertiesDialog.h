/*
 * TerrainPropertiesDialog.h
 *
 *  Created on: 12.06.2012
 *      Author: michi
 */

#ifndef TERRAINPROPERTIESDIALOG_H_
#define TERRAINPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../lib/pattern/Observable.h"
#include "../../../data/world/WorldTerrain.h"

class DataWorld;

class TerrainPropertiesDialog: public obs::Node<hui::Dialog> {
public:
	TerrainPropertiesDialog(DataWorld *_data, int _index);
	virtual ~TerrainPropertiesDialog();

	void load_data();
	void apply_data();

	void fill_texture_list();

	void on_save_as();
	void on_textures();
	void on_textures_edit();
	void on_textures_select();
	void on_add_texture_level();
	void on_delete_texture_level();
	void on_clear_texture_level();
	void on_texture_map_complete();
	void on_default_material();
	void on_material_find();
	void on_ok();
	void on_close();

	void update_data();

private:
	DataWorld *data;
	int index;
	bool applying;

	WorldEditingTerrain temp;
};

#endif /* TERRAINPROPERTIESDIALOG_H_ */
