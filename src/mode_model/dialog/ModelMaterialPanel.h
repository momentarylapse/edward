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
class XMaterialPanel;
//class ModeMeshTexture;
namespace xhui {
	class Menu;
}

class ModelMaterialPanel: public obs::Node<xhui::Panel> {
	friend XMaterialPanel;
public:
	explicit ModelMaterialPanel(DataModel *_data, bool full=true);
	~ModelMaterialPanel() override;

	void load_data();
	void fill_material_list();

	void on_material_list_select();
	void on_material_list_right_click();
	void on_material_add();
	void on_material_load();
	void on_material_delete();
	void on_material_apply();

private:
	DataModel *data;
	ModeMesh *mode_mesh();
//	ModeMeshTexture *mode_mesh_texture();
	//ModelMaterial temp;
	int apply_queue_depth;
	xhui::Menu *popup_materials;
};



#endif //MODELMATERIALPANEL_H
