#pragma once

#include <lib/pattern/Observable.h>
#include <lib/xhui/Panel.h>

#include "lib/ygraphics/graphics-impl.h"

class Data;
class Session;
namespace yrenderer {
	class Material;
}
namespace xhui {
	class Menu;
}

class MaterialParameterPanel : public obs::Node<xhui::Panel> {
public:
	Session* session;
	Data* data;
	yrenderer::Material* material;
	shared<xhui::Menu> popup_textures;
	int apply_queue_depth = 0;

	std::function<void(shared<ygfx::Texture>)> f_add_texture;
	std::function<void(int index)> f_delete_texture;

	MaterialParameterPanel(Data* _data, yrenderer::Material* _material);
	void set_material(yrenderer::Material* m);
	string material_name() const;
	void update_ui();
	void update_reset_buttons();
	void apply_data_color();
	void fill_texture_list();
	void on_texture_level_add();
	void on_textures();
	void on_texture_level_load();
	void on_texture_level_save();
	void on_texture_level_scale();
	void on_textures_select();
	void on_texture_level_delete();
	void on_texture_level_clear();
	void on_texture_level_linear();
	void on_texture_level_srgb();
	void on_textures_right_click();
};

