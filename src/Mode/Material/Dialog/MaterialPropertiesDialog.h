/*
 * MaterialPropertiesDialog.h
 *
 *  Created on: 09.03.2012
 *      Author: michi
 */

#ifndef MATERIALPROPERTIESDIALOG_H_
#define MATERIALPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Material/DataMaterial.h"

class MaterialPropertiesDialog: public hui::Panel {
public:
	MaterialPropertiesDialog(hui::Window *_parent, DataMaterial *_data);
	virtual ~MaterialPropertiesDialog();

	void load_data();
	void apply_data();
	void apply_data_delayed();
	void apply_phys_data();
	void apply_phys_data_delayed();

	void on_textures();
	void on_textures_select();
	void on_add_texture_level();
	void on_delete_texture_level();
	void on_clear_texture_level();
	void on_transparency_mode();
	void on_reflection_mode();
	void on_reflection_textures();

	void refill_refl_tex_view();
	void fill_texture_list();

	void on_data_update();


private:
	DataMaterial *data;
	DataMaterial::AppearanceData temp;
	DataMaterial::PhysicsData temp_phys;
	int apply_queue_depth;
	int apply_phys_queue_depth;
};

#endif /* MATERIALPROPERTIESDIALOG_H_ */
