#pragma once

#include <lib/xhui/Panel.h>

namespace yrenderer {
	class Material;
}
class Data;

class MaterialEditPanel : public xhui::Panel {
public:
	MaterialEditPanel(Data* data);
	void set_material(yrenderer::Material* material);
	void update_ui();
	void fill_texture_list();

	Data* data;
	yrenderer::Material* material;
};

class MaterialEditDialog {
};

