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
#include "data/DataMaterial.h"
#include <y/renderer/Renderer.h>
#include <y/renderer/world/geometry/RenderViewData.h>
#include <y/renderer/world/geometry/SceneView.h>
#include <y/helper/ResourceManager.h>
#include <y/world/Material.h>
#include <y/world/Camera.h>
#include <lib/xhui/Resource.h>
#include <lib/xhui/controls/MenuBar.h>
#include <lib/xhui/controls/Toolbar.h>
#include <lib/xhui/Theme.h>
#include <lib/image/image.h>
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/DrawingHelper.h>
#include <data/mesh/GeometryCube.h>
#include <data/mesh/GeometrySphere.h>
#include <data/mesh/GeometryTorus.h>
#include <data/mesh/GeometryTorusKnot.h>
#include <data/mesh/GeometryTeapot.h>
#include <lib/os/msg.h>
#include <world/Light.h>
#include <y/Entity.h>


ModeMaterial::ModeMaterial(Session* session) :
	Mode(session)
{
	multi_view = new MultiView(session);
	data = new DataMaterial(session);
	generic_data = data;

	data->out_changed >> create_sink([this] {
		//data->lights
		//	world.en
		material = data->to_material();
	});
}

void ModeMaterial::on_enter() {
	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
	multi_view->data_sets = {};
	multi_view->light_mode = MultiView::LightMode::Fixed;

	auto tb = session->win->toolbar;
	tb->set_by_id("material-toolbar");


	auto menu_bar = (xhui::MenuBar*)session->win->get_control("menu");
	auto menu = xhui::create_resource_menu("menu_material");
	menu_bar->set_menu(menu);

	session->win->event("material_shape_cube", [this] {
		set_mesh(PreviewMesh::CUBE);
	});
	session->win->event("material_shape_icosahedron", [this] {
		set_mesh(PreviewMesh::ICOSAHEDRON);
	});
	session->win->event("material_shape_ball", [this] {
		set_mesh(PreviewMesh::SPHERE);
	});
	session->win->event("material_shape_torus", [this] {
		set_mesh(PreviewMesh::TORUS);
	});
	session->win->event("material_shape_torusknot", [this] {
		set_mesh(PreviewMesh::TORUS_KNOT);
	});
	session->win->event("material_shape_teapot", [this] {
		set_mesh(PreviewMesh::TEAPOT);
	});

	session->win->event("properties", [this] {
		//session->win->open_dialog(new PropertiesDialog(session->win, data));
	});

	material = data->to_material();
	set_mesh(PreviewMesh::TEAPOT);

	set_side_panel(new MaterialPanel(this));


	// ground
	vertex_buffer_ground = new VertexBuffer("3f,3f,2f");
	GeometryCube cube({-2,-0.7f,-2}, {4,0,0}, {0,0.01f,0}, {0,0,4}, 1, 1, 1);
	cube.build(vertex_buffer_ground.get());

	Image im(16,16,White);
	for (int i=0; i<16; i++)
		for (int j=0; j<16; j++)
			if (i%2 == j%2)
				im.set_pixel(i, j, Black);
	material_ground = session->resource_manager->load_material("");
	material_ground->roughness = 0.4f;
	material_ground->metal = 0;
	material_ground->textures.add(new Texture(16, 16, "rgba:i8"));
	material_ground->textures[0]->write(im);
	material_ground->textures[0]->set_options("magfilter=nearest");
}


void ModeMaterial::on_leave() {
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);
}


void ModeMaterial::optimize_view() {
	float r = 1.4f;
	multi_view->view_port.suggest_for_box({vec3(-r,-r,-r), vec3(r,r,r)});
}

void ModeMaterial::on_prepare_scene(const RenderParams& params) {
	if (!spot_light) {
		spot_light = new Light(White * 70, -1, -1);
		spot_light->owner = new Entity;
		spot_light->owner->pos = {2, 5, 7};
		spot_light->owner->ang = quaternion::rotation_v((-spot_light->owner->pos).dir2ang());
		spot_light->light.radius = 400;
		spot_light->light.theta = 0.15f;
		spot_light->light.harshness = 1;
		spot_light->enabled = true;
	}

	multi_view->default_light->owner->ang = quaternion::rotation_a(vec3::EX, pi*0.20f);

	multi_view->lights = {multi_view->default_light, spot_light};
}

void ModeMaterial::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd;
	auto dh = win->multi_view->session->drawing_helper;
	dh->clear(params, xhui::Theme::_default.background_low);

	dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer_ground.get(), material_ground.get());

	for (int pass_no=0; pass_no<material->num_passes; pass_no++)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer.get(), material.get(), pass_no);
}

void ModeMaterial::on_draw_shadow(const RenderParams& params, RenderViewData& rvd) {
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
	PolygonMesh mesh;
	switch (m) {
	case PreviewMesh::CUBE:
		mesh = GeometryCube({0,0,0}, vec3::EX, vec3::EY, vec3::EZ, 1, 1, 1);
		break;
	case PreviewMesh::SPHERE:
		mesh = GeometrySphere({0,0,0}, 1, 16);
		mesh.smoothen();
		break;
	case PreviewMesh::TORUS:
		mesh = GeometryTorus({0,0,0}, vec3::EZ, 1, 0.5f, 32, 32);
		mesh.smoothen();
		break;
	case PreviewMesh::TORUS_KNOT:
		mesh = GeometryTorusKnot({0,0,0}, vec3::EZ, 1, 0.5f, 0.3f, 3, 5, 256, 16);
		mesh.smoothen();
		break;
	case PreviewMesh::TEAPOT:
	default:
		mesh = GeometryTeapot({0,0,0}, 1, 8);
		mesh.smoothen();
		break;
	}
	if (!vertex_buffer)
		vertex_buffer = new VertexBuffer("3f,3f,2f");
	mesh.build(vertex_buffer.get());
	session->win->request_redraw();
}
