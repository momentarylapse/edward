/*
 * ModelMaterialDialog.h
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#ifndef MODELMATERIALDIALOG_H_
#define MODELMATERIALDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"
#include "../../../Data/Model/ModelMaterial.h"
class DataModel;

class ModelMaterialDialog: public hui::Panel, public Observer
{
public:
	ModelMaterialDialog(DataModel *_data);
	virtual ~ModelMaterialDialog();

	void load_data();
	void apply_data();
	void apply_data_delayed();
	void fill_material_list();

	void on_material_list();
	void on_material_list_select();
	void on_material_list_right_click();
	void on_add_new_material();
	void on_add_material();
	void on_delete_material();
	void on_apply_material();

	void on_default_colors();

	void on_textures();
	void on_textures_right_click();
	void on_textures_select();
	void on_texture_level_add();
	void on_texture_level_delete();
	void on_texture_level_clear();
	void on_texture_level_load();
	void on_transparency_mode();

	void fill_texture_list();

	void on_update(Observable *o, const string &message) override;

private:
	DataModel *data;
	ModelMaterial temp;
	int apply_queue_depth;
	hui::Menu *popup_materials;
	hui::Menu *popup_textures;
};

#endif /* MODELMATERIALDIALOG_H_ */
