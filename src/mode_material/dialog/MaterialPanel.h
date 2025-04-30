//
// Created by Michael Ankele on 2025-04-30.
//

#ifndef MATERIALPANEL_H
#define MATERIALPANEL_H


#include "../ModeMaterial.h"
#include <lib/xhui/xhui.h>
#include <lib/pattern/Observable.h>

class DataMaterial;
namespace xhui {
class Menu;
}

class MaterialPanel: public obs::Node<xhui::Panel> {
public:
	explicit MaterialPanel(ModeMaterial *_mode);
	~MaterialPanel() override;

	void load_data();
	void fill_texture_list();

	void on_material_list_select();
	void on_material_list_right_click();
	void on_material_add();
	void on_material_load();
	void on_material_delete();
	void on_material_apply();

private:
	DataMaterial *data;
	ModeMaterial *mode_material();
	//	ModeMeshTexture *mode_mesh_texture();
	//ModelMaterial temp;
	int apply_queue_depth;
	//xhui::Menu *popup_materials;
};



#endif //MATERIALPANEL_H
