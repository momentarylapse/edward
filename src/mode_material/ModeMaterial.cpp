//
// Created by Michael Ankele on 2025-04-30.
//

#include "ModeMaterial.h"
#include "dialog/MaterialPanel.h"
#include <Session.h>
#include <lib/base/iter.h>
#include <lib/kaba/kaba.h>
#include <lib/os/filesystem.h>
#include <view/MultiView.h>
#include <view/MultiViewPanel.h>
#include <view/DocumentSession.h>
#include "data/DataMaterial.h"
#include <y/helper/ResourceManager.h>
#include <lib/yrenderer/Material.h>
#include <world/components/Camera.h>
#include <world/components/Light.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/Theme.h>
#include <lib/image/image.h>
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/DrawingHelper.h>
#include <lib/polymesh/create/Cube.h>
#include <lib/polymesh/create/Platonic.h>
#include <lib/polymesh/create/Sphere.h>
#include <lib/polymesh/create/Torus.h>
#include <lib/polymesh/create/TorusKnot.h>
#include <lib/polymesh/create/Teapot.h>
#include <lib/os/msg.h>


ModeMaterial::ModeMaterial(DocumentSession* doc) :
	Mode(doc)
{
	auto mvp = new MultiViewPanel(doc, new MultiView(doc));
	multi_view = mvp->multi_view;
	doc->set_document_panel(mvp);
	data = new DataMaterial(doc);
	generic_data = data;
	toolbar_id = "material-toolbar";
	menu_id = "menu_material";
}


void ModeMaterial::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
	multi_view->data_sets = {};
	multi_view->light_mode = MultiView::LightMode::Fixed;

	material = data->to_material();
	set_mesh(PreviewMesh::TEAPOT);

	set_side_panel(new MaterialPanel(this));


	// ground
	vertex_buffer_ground = new ygfx::VertexBuffer("3f,3f,2f");
	auto cube = polymesh::create_cube({{-4,-1.2f,-4}, {4,-1.0f,4}}, {1, 1, 1});
	cube.build(vertex_buffer_ground.get());

	Image im(16,16,White);
	for (int i=0; i<16; i++)
		for (int j=0; j<16; j++)
			if (i%2 == j%2)
				im.set_pixel(i, j, Black);
	auto tex = new ygfx::Texture(16, 16, "rgba:i8");
	tex->write(im);
	tex->set_options("magfilter=nearest");
	material_ground = session->resource_manager->create_material();
	material_ground->roughness = 0.4f;
	material_ground->metal = 0;
	material_ground->albedo = color(1, 0.3f, 0.3f, 0.3f);
	material_ground->textures[0] = tex;
}

void ModeMaterial::on_connect_events() {
	doc->event("material_shape_cube", [this] {
		set_mesh(PreviewMesh::CUBE);
	});
	doc->event("material_shape_icosahedron", [this] {
		set_mesh(PreviewMesh::ICOSAHEDRON);
	});
	doc->event("material_shape_ball", [this] {
		set_mesh(PreviewMesh::SPHERE);
	});
	doc->event("material_shape_torus", [this] {
		set_mesh(PreviewMesh::TORUS);
	});
	doc->event("material_shape_torusknot", [this] {
		set_mesh(PreviewMesh::TORUS_KNOT);
	});
	doc->event("material_shape_teapot", [this] {
		set_mesh(PreviewMesh::TEAPOT);
	});

	doc->event("properties", [this] {
		//session->win->open_dialog(new PropertiesDialog(session->win, data));
	});
}

void ModeMaterial::on_leave() {
	set_side_panel(nullptr);
}

void ModeMaterial::on_enter_rec() {
	data->out_changed >> create_sink([this] {
		material = data->to_material();
	});
}

void ModeMaterial::on_leave_rec() {
	data->out_changed.unsubscribe(this);
}


void ModeMaterial::optimize_view() {
	float r = 1.8f;
	multi_view->view_port.suggest_for_box({vec3(-r,-r,-r), vec3(r,r,r)});
	multi_view->view_port.ang = quaternion::rotation(vec3(0.4f,-0.8f,0));
}

void ModeMaterial::on_prepare_scene(const yrenderer::RenderParams& params) {
	if (!spot_light) {
		spot_light = new yrenderer::Light;
		spot_light->init(yrenderer::LightType::CONE, White, 0.15f);
		spot_light->pos = {3, 6, 12};
		spot_light->_ang = quaternion::rotation_v((-spot_light->pos).normalized().dir2ang());
		spot_light->power = 170;
		spot_light->harshness = 1;
		spot_light->enabled = true;
	}

	multi_view->default_light->_ang = quaternion::rotation_a(vec3::EX, pi*0.23f);
	multi_view->default_light->allow_shadow = true;
	multi_view->default_light->power = 0.3f;
	multi_view->default_light->harshness = 0.7f;

	multi_view->lights = {multi_view->default_light, spot_light.get()};
}

void ModeMaterial::on_draw_background(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	DrawingHelper::clear(params, xhui::Theme::_default.background_low.srgb_to_linear());

	auto dh = multi_view->session->drawing_helper;
	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_ground.get(), material_ground.get());
}

void ModeMaterial::on_draw_win(const yrenderer::RenderParams& params, MultiViewWindow* win) {
	auto& rvd = win->rvd();
	auto dh = win->multi_view->session->drawing_helper;

	for (int pass_no=0; pass_no<material->num_passes; pass_no++)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer.get(), const_cast<yrenderer::Material*>(material), pass_no);
}

void ModeMaterial::on_draw_shadow(const yrenderer::RenderParams& params, yrenderer::RenderViewData& rvd) {
	auto dh = multi_view->session->drawing_helper;

	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_ground.get(), dh->material_shadow);

	if (material->cast_shadow)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer.get(), dh->material_shadow);
}


void ModeMaterial::on_draw_post(Painter* p) {
}

void ModeMaterial::on_command(const string& id) {
	if (id == "new")
		session->universal_new(FD_MATERIAL);
	if (id == "open")
		session->universal_open(FD_MATERIAL);
	if (id == "save")
		session->storage->save(data->filename, data);
	if (id == "save-as")
		session->storage->save_as(data);
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();
}

void ModeMaterial::on_key_down(int key) {
}

void ModeMaterial::set_mesh(PreviewMesh m) {
	polymesh::Mesh mesh;
	switch (m) {
	case PreviewMesh::CUBE:
		mesh = polymesh::create_cube(Box::ID_SYM, {1, 1, 1});
		break;
	case PreviewMesh::ICOSAHEDRON:
		mesh = polymesh::create_icosahedron({0,0,0}, 1);
		break;
	case PreviewMesh::SPHERE:
		mesh = polymesh::create_sphere({0,0,0}, 1, 16);
		mesh.smoothen();
		break;
	case PreviewMesh::TORUS:
		mesh = polymesh::create_torus({0,0,0}, vec3::EZ, 1, 0.5f, 32, 32);
		mesh.smoothen();
		break;
	case PreviewMesh::TORUS_KNOT:
		mesh = polymesh::create_torus_knot({0,0,0}, vec3::EZ, 1, 0.5f, 0.3f, 3, 5, 256, 16);
		mesh.smoothen();
		break;
	case PreviewMesh::TEAPOT:
	default:
		mesh = polymesh::create_teapot({0,0,0}, 1.5f, 8);
		mesh.smoothen();
		break;
	}
	if (!vertex_buffer)
		vertex_buffer = new ygfx::VertexBuffer("3f,3f,2f");
	mesh.build(vertex_buffer.get());
	session->win->request_redraw();
}
