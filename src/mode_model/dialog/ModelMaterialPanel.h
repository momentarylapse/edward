//
// Created by Michael Ankele on 2025-04-16.
//

#ifndef MODELMATERIALPANEL_H
#define MODELMATERIALPANEL_H


#include "../mesh/ModeMeshMaterial.h"
#include <lib/xhui/xhui.h>
#include <lib/pattern/Observable.h>

class DataModel;
class ModeMesh;
//class ModeMeshTexture;
namespace xhui {
	class Menu;
}

class ModelMaterialPanel: public obs::Node<xhui::Panel> {
public:
	explicit ModelMaterialPanel(DataModel *_data, bool full=true);
	~ModelMaterialPanel() override;

	void load_data();
	void apply_data_color();
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

	void fill_texture_list();

private:
	DataModel *data;
	ModeMesh *mode_mesh();
//	ModeMeshTexture *mode_mesh_texture();
	//ModelMaterial temp;
	int apply_queue_depth;
	xhui::Menu *popup_materials;
	xhui::Menu *popup_textures;
};



#endif //MODELMATERIALPANEL_H
