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
	friend class MaterialPassPanel;
public:
	explicit MaterialPanel(ModeMaterial* mode);
	~MaterialPanel() override;

	void load_data();
	void fill_texture_list();

	void on_texture_level_add();
	void on_texture_level_load();
	void on_texture_level_delete();
	void on_texture_level_clear();
	void on_textures_right_click();


private:
	DataMaterial* data;
	ModeMaterial* mode_material();
	int apply_queue_depth;
	owned<xhui::Menu> popup_textures;
};



#endif //MATERIALPANEL_H
