#include "MaterialEditDialog.h"
#include <Session.h>
#include <data/Data.h>
#include "helper/ResourceManager.h"
#include <lib/xhui/xhui.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/os/msg.h>
#include <lib/xhui/controls/ListView.h>
#include <lib/yrenderer/TextureManager.h>
#include <view/MaterialPreviewManager.h>


string file_secure(const Path& filename);

constexpr int PREVIEW_SIZE = 48;


MaterialEditPanel::MaterialEditPanel(Data* _data) : xhui::Panel("") {
	data = _data;
	//from_resource("model-material-dialog");
	//size_mode_x = SizeMode::Expand;

	//expand("contents", true);


	from_source(R"foodelim(
Dialog model-material-dialog ''
	Grid card '' class=card
		Grid ? '' spacing=16
			Image preview '' width=48 height=48
			Grid ? '' spacing=0
				Label header '' bold expandx
				---|
				Label subheader '' italic small
		---|
		Expander contents ''
			Grid ? ''
				Group grp-color 'Colors'
					Grid ? ""
						Label /t-albedo "Albedo" right disabled
						ColorButton albedo "" alpha linear
						---|
						Label /t-roughness "Roughness" right disabled
						Grid ? ""
							Slider slider-roughness "" range=0:1:0.01 expandx
							SpinButton roughness "" range=0:1:0.01
						---|
						Label /t-reflectivity "Metal" right disabled
						Grid ? ""
							Slider slider-metal "" range=0:1:0.01 expandx
							SpinButton metal "" range=0:1:0.01
						---|
						Label /t-emission "Emission" right disabled
						ColorButton emission "" linear "tooltip=Color in absolute darkness"
				---|
				Group grp-textures "Textures"
					ListView textures "\\Texture" height=130 nobar format=it select-single "tooltip=Mixing texture levels (multitexturing)\n- Actions via right click"
)foodelim");
	expand("contents", true);

	auto tex_list = (xhui::ListView*)get_control("textures");
	tex_list->column_factories[0].f_create = [](const string& id) {
		return xhui::create_control("Image", format("!width=%d,height=%d", PREVIEW_SIZE, PREVIEW_SIZE), id);
	};
	tex_list->column_factories[1].f_create = [](const string& id) {
		return xhui::create_control("Label", "!markup", id);
	};
	event("albedo", [this] {
		material->albedo = get_color("albedo");
		data->session->resource_manager->material_manager->invalidate(material);
	});
}

void MaterialEditPanel::set_material(yrenderer::Material* _material) {
	material = _material;
	update_ui();
}

void MaterialEditPanel::update_ui() {
	int nt = 0;

	set_string("preview", data->session->material_preview_manager->get(material));
	set_string("header", str(data->session->resource_manager->material_manager->get_filename(material)));
	set_string("subheader", "...");
	set_color("albedo", material->albedo);
	set_color("emission", material->emission);
	set_float("roughness", material->roughness);
	set_float("slider-roughness", material->roughness);
	set_float("metal", material->metal);
	set_float("slider-metal", material->metal);
	//set_string("header", file_secure(m->filename));

	fill_texture_list();
}


void MaterialEditPanel::fill_texture_list() {
	reset("textures");
	for (int i=0;i<material->textures.num;i++) {
		string id = format("image:material:%s", p2s(material));
		id = xhui::texture_to_image(material->textures[i]);
		//auto *img = material->textures[i]->image.get();
		/*auto img = xhui::texture_to_image(material->textures[i]);
		auto *icon = img->scale(PREVIEW_SIZE, PREVIEW_SIZE);
		xhui::set_image(id, *icon);
		string space = (img->color_space == ColorSpace::Linear) ? "linear" : "srgb";
		string ext = format("\n<span size='small' alpha='50%%>   %d x %d, %s</span>", img->width, img->height, space);
		if (mat->texture_levels[i]->edited)
			ext += " *";
		add_string("textures", format("Tex[%d]\\%s\\%s", i, id, (file_secure(mat->texture_levels[i]->filename).replace("@linear", "") + ext)));*/
		add_string("textures", format("%s\\%s", id, data->session->resource_manager->texture_manager->texture_file(material->textures[i].get())));
		//delete icon;
	}
	//	set_int("textures", mode_mesh()->current_texture_level);
}


