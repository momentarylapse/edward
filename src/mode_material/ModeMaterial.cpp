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
#include <storage/Storage.h>
#include <view/EdwardWindow.h>
#include <view/DrawingHelper.h>
#include <data/mesh/GeometryCube.h>
#include <data/mesh/GeometrySphere.h>
#include <data/mesh/GeometryTorus.h>
#include <data/mesh/GeometryTorusKnot.h>
#include <data/mesh/GeometryTeapot.h>
#include <lib/xhui/Theme.h>
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
	multi_view->default_light->owner->ang = quaternion::rotation_a(vec3::EX, 0.5f);

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
}


void ModeMaterial::on_leave() {
	//session->win->unembed(dialog);
	set_side_panel(nullptr);

	data->out_changed.unsubscribe(this);
}


void ModeMaterial::optimize_view() {
	multi_view->view_port.suggest_for_box({vec3(-1,-1,-1), vec3(1,1,1)});
}

void ModeMaterial::on_prepare_scene(const RenderParams& params) {
	// user lights?
}


void ModeMaterial::on_draw_win(const RenderParams& params, MultiViewWindow* win) {

	auto& rvd = win->rvd;
	auto dh = win->multi_view->session->drawing_helper;
	dh->clear(params, xhui::Theme::_default.background_low);

	for (int pass_no=0; pass_no<material->num_passes; pass_no++)
		dh->draw_mesh(params, rvd, mat4::ID, vertex_buffer.get(), material.get(), pass_no);
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
