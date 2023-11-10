/*
 * MaterialPropertiesDialog.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef MATERIALPROPERTIESDIALOG_H_
#define MATERIALPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../data/material/DataMaterial.h"

class ModeMaterial;

class MaterialPropertiesDialog: public obs::Node<hui::Panel> {
public:
	MaterialPropertiesDialog(hui::Window *_parent, ModeMaterial *m);
	virtual ~MaterialPropertiesDialog();

	void load_data();
	void apply_data();
	void apply_data_delayed();
	void apply_phys_data();
	void apply_phys_data_delayed();

	void set_size(int size);

	void on_textures_right_click();
	void on_textures_select();
	void on_texture_level_select_file();
	void on_texture_level_add();
	void on_texture_level_delete();
	void on_texture_level_clear();

	void on_passes_right_click();
	void on_passes_select();
	void on_pass_edit();
	void on_pass_add();
	void on_pass_copy();
	void on_pass_delete();

	void fill_texture_list();
	void fill_passes_list();

	void on_data_update();


private:
	ModeMaterial *mode_material;
	DataMaterial *data;
	DataMaterial::AppearanceData temp;
	DataMaterial::PhysicsData temp_phys;
	int apply_queue_depth;
	int apply_phys_queue_depth;

	owned<hui::Menu> textures_popup;
	owned<hui::Menu> passes_popup;
};

#endif /* MATERIALPROPERTIESDIALOG_H_ */
