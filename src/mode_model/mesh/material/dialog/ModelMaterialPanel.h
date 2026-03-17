//
// Created by Michael Ankele on 2025-04-16.
//

#ifndef MODELMATERIALPANEL_H
#define MODELMATERIALPANEL_H


#include "../ModeMeshMaterial.h"
#include <lib/xhui/xhui.h>
#include <lib/pattern/Observable.h>

class DataModel;
class ModeMesh;
class InlineMaterialEditorPanel;
//class ModeMeshTexture;
namespace xhui {
	class Menu;
}

class ModelMaterialPanel: public obs::Node<xhui::Panel> {
	friend InlineMaterialEditorPanel;
public:
	explicit ModelMaterialPanel(DataModel *_data, bool full=true);
	~ModelMaterialPanel() override;

	void load_data();
	void fill_material_list();

	void on_material_list_select();
	void on_material_add();
	void on_material_load();

private:
	DataModel *data;
	ModeMesh *mode_mesh();
//	ModeMeshTexture *mode_mesh_texture();
	//ModelMaterial temp;
	int apply_queue_depth;
};



#endif //MODELMATERIALPANEL_H
