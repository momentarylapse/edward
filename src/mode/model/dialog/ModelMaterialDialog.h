/*
 * ModelMaterialDialog.h
 *
 *  Created on: 19.09.2013
 *      Author: michi
 */

#ifndef MODELMATERIALDIALOG_H_
#define MODELMATERIALDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../lib/pattern/Observable.h"
#include "../../../data/model/ModelMaterial.h"
class DataModel;

class ModeModelMesh;
class ModeModelMeshTexture;

class ModelMaterialDialog: public obs::Node<hui::Panel> {
public:
	ModelMaterialDialog(DataModel *_data, bool full=true);
	virtual ~ModelMaterialDialog();

	void load_data();
	void apply_data_color();
	void apply_data_alpha();
	void fill_material_list();

	void on_material_list_select();
	void on_material_list_right_click();
	void on_material_add();
	void on_material_load();
	void on_material_delete();
	void on_material_apply();

	void on_override_colors();

	void on_textures();
	void on_textures_right_click();
	void on_textures_select();
	void on_texture_level_add();
	void on_texture_level_delete();
	void on_texture_level_clear();
	void on_texture_level_load();
	void on_texture_level_save();
	void on_texture_level_scale();
	void on_transparency_mode();

	void fill_texture_list();

private:
	DataModel *data;
	ModeModelMesh *mode_model_mesh();
	ModeModelMeshTexture *mode_model_mesh_texture();
	//ModelMaterial temp;
	int apply_queue_depth;
	hui::Menu *popup_materials;
	hui::Menu *popup_textures;
};

#endif /* MODELMATERIALDIALOG_H_ */
