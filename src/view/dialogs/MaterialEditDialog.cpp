#include "MaterialEditDialog.h"

#include <cmath>

#include "Session.h"
#include "data/Data.h"
#include "helper/ResourceManager.h"
#include "lib/xhui/xhui.h"
#include "lib/yrenderer/MaterialManager.h"
#include <lib/ygraphics/graphics-impl.h>

#include "lib/mesh/GeometryBall.h"
#include "lib/os/msg.h"
#include "lib/xhui/controls/ListView.h"
#include "lib/yrenderer/scene/SceneRenderer.h"
#include "lib/yrenderer/scene/path/RenderPathForward.h"
#include "lib/yrenderer/target/TextureRenderer.h"
#include "view/EdwardWindow.h"
#include <lib/image/image.h>

#include "lib/yrenderer/TextureManager.h"
#include "lib/yrenderer/post/ThroughShaderRenderer.h"
#include "lib/yrenderer/target/HeadlessRendererVulkan.h"


string file_secure(const Path& filename);

constexpr int PREVIEW_SIZE = 48;

static base::map<yrenderer::Material*, shared<ygfx::Texture>> mat_textures;
static shared<ygfx::DepthBuffer> mat_depth_buffer;

ygfx::Texture* get_mat_texture(yrenderer::Material* m) {
	if (mat_textures.contains(m))
		return mat_textures[m].get();
	auto tex = new ygfx::Texture(PREVIEW_SIZE*2, PREVIEW_SIZE*2, "rgba:i8");
	mat_textures.set(m, tex);
	if (!mat_depth_buffer)
		mat_depth_buffer = new ygfx::DepthBuffer(PREVIEW_SIZE*2, PREVIEW_SIZE*2, "d:f32");
	return tex;
}

class XEmitter : public yrenderer::MeshEmitter {
public:
	yrenderer::Material* material;
	owned<ygfx::VertexBuffer> vb;
	XEmitter(Session* s, yrenderer::Material* m) : yrenderer::MeshEmitter(s->ctx, "x") {
		material = m;
		vb = new ygfx::VertexBuffer("3f,3f,2f");
		auto g = GeometryBall::create({0,0,4}, 1, 32, 64);
		g.smoothen();
		g.build(vb.get());
	}
	void emit(const yrenderer::RenderParams &params, yrenderer::RenderViewData &rvd, bool shadow_pass) override {
		if (!material->cast_shadow and shadow_pass)
			return;

		auto shader = rvd.get_shader(material, 0, "default", "");
		if (shadow_pass)
			material = rvd.material_shadow;

		auto& rd = rvd.start(params, mat4::ID, shader, *material, 0, ygfx::PrimitiveTopology::TRIANGLES, vb.get());

		rd.draw_triangles(params, vb.get());
	}
};

string render_material(Session* session, yrenderer::Material *m) {
	auto tex = get_mat_texture(m);

	yrenderer::Light light;
	light.init(yrenderer::LightType::DIRECTIONAL, White);
	light._ang = quaternion::rotation({0.5f,-0.5f,0});
	light.harshness = 0.5f;
	light.allow_shadow = false;//true;
	light.enabled = true;

	yrenderer::RenderPathForward path(session->ctx, 256);
	path.background_color = color(0,0,0,0);
	path.add_opaque_emitter(new XEmitter(session, m));
	path.set_view({{0,0,0}, quaternion::ID, pi/6, 0.1f, 100});
	path.set_lights({&light});

	Image im(64, 64, Black);
	path.scene_view.cube_map = new ygfx::CubeMap(64, "rgba:i8");
	for (int i=0; i<6; i++)
		path.scene_view.cube_map->write_side(i, im);

	yrenderer::HeadlessRenderer hr(session->ctx, {tex, mat_depth_buffer.get()});
	hr.add_child(&path);
	hr.render({1, false});

	return xhui::texture_to_image(tex);
}

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
}

void MaterialEditPanel::set_material(yrenderer::Material* _material) {
	material = _material;
	update_ui();
}

void MaterialEditPanel::update_ui() {
	int nt = 0;

	set_string("preview", render_material(data->session, material));
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


