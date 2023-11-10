/*
 * ModeMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ModeMaterial.h"
#include "dialog/MaterialPropertiesDialog.h"
#include "dialog/ShaderGraphDialog.h"
#include "../../EdwardWindow.h"
#include "../../Session.h"
#include "../../storage/Storage.h"
#include "../../data/material/DataMaterial.h"
#include "../../data/material/ShaderGraph.h"
#include "../../data/model/DataModel.h"
#include "../../data/model/geometry/GeometryCube.h"
#include "../../data/model/geometry/GeometryBall.h"
#include "../../data/model/geometry/GeometryPlatonic.h"
#include "../../data/model/geometry/GeometryTorus.h"
#include "../../data/model/geometry/GeometryTorusKnot.h"
#include "../../data/model/geometry/GeometryTeapot.h"
#include "../../multiview/MultiView.h"
#include "../../multiview/Window.h"
#include "../../multiview/ColorScheme.h"
#include "../../multiview/DrawingHelper.h"
#include "../../lib/os/file.h"
#include "../../lib/nix/nix.h"
#include <y/helper/ResourceManager.h>

const int MATERIAL_DETAIL = 32;
const float MATERIAL_RADIUS1 = 80;
const float MATERIAL_RADIUS2 = 50;

#define __MATERIAL_MAX_TEXTURES		4

ModeMaterial::ModeMaterial(Session *_s, MultiView::MultiView *mv) :
	Mode(_s, "Material", nullptr, new DataMaterial(_s), mv, "menu_material"),
	in_data_changed(this, [this] { on_data_update(); })
{
	appearance_dialog = nullptr;
	shader_graph_dialog = nullptr;
	shader_edit_mode = ShaderEditMode::NONE;

	shape_type = hui::config.get_str("MaterialShapeType", "teapot");
	shape_smooth = hui::config.get_bool("MaterialShapeSmooth", true);
}

ModeMaterial::~ModeMaterial() {
}

void ModeMaterial::_new() {
	data->reset();
	optimize_view();
}


void test_save_extras(DataMaterial *data, hui::Callback cb_success) {
	if (data->appearance.passes[0].shader.is_default) {
		cb_success();
		return;
	}

	auto f = [data, cb_success] (const Path &dir) {
		os::fs::write_text(dir | data->appearance.passes[0].shader.file, data->appearance.passes[0].shader.code);

		if (data->appearance.passes[0].shader.from_graph)
			data->appearance.passes[0].shader.graph->save(dir | data->appearance.passes[0].shader.file.with(".graph"));
		cb_success();
	};

	Path dir = data->session->storage->last_dir[FD_SHADERFILE];
	if (data->appearance.passes[0].shader.file) {
		f(dir);
	} else {
		hui::file_dialog_save(data->session->win, "Shader...", dir, {"filter=*.shader", "showfilter=*.shader"}).then([data, dir, f] (const Path &path) {
			data->appearance.passes[0].shader.file = path.relative_to(dir);
			f(dir);
		});
	}
}



void ModeMaterial::save() {
	test_save_extras(data, [this] {
		session->storage->auto_save(data);
	});
}



void ModeMaterial::on_draw() {
}



void ModeMaterial::on_data_update() {
	update_textures();

	shaders.resize(data->appearance.passes.num);
	for (int i=0; i<data->appearance.passes.num; i++) {
		try {
			// dummy
			shaders[i] = session->gl->create_shader("<VertexShader>void main(){gl_Position = vec4(0);}</VertexShader><FragmentShader>void main(){}</FragmentShader>");
		} catch (Exception &e) {
			msg_error(e.message());
		}
	}

	for (int i=0; i<data->appearance.passes.num; i++)
		update_shader(i);
}

void ModeMaterial::update_textures() {
	textures.clear();

	for (auto &tf: data->appearance.texture_files)
		textures.add(session->resource_manager->load_texture(tf));
}

void ModeMaterial::update_shader(int pass_no) {
	msg_write("update shader pass #" + str(pass_no));
	try {
		auto code = session->resource_manager->expand_vertex_shader_source(data->appearance.passes[pass_no].shader.code, "default");
		code = session->resource_manager->expand_fragment_shader_source(code, "forward");
		shaders[pass_no]->update(code);
	} catch(Exception &e) {
		msg_error(e.message());
	}
}



void ModeMaterial::on_command(const string & id) {
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		save_as();

	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();

	if (id == "material_shape_smooth")
		set_shape_smooth(!shape_smooth);
	if (id == "material_shape_cube")
		set_shape_type("cube");
	if (id == "material_shape_ball")
		set_shape_type("ball");
	if (id == "material_shape_torus")
		set_shape_type("torus");
	if (id == "material_shape_torusknot")
		set_shape_type("torusknot");
	if (id == "material_shape_icosahedron")
		set_shape_type("icosahedron");
	if (id == "material_shape_teapot")
		set_shape_type("teapot");

	if (id == "shader-mode-none")
		set_shader_edit_mode(ShaderEditMode::NONE);
	if (id == "shader-mode-graph")
		set_shader_edit_mode(ShaderEditMode::GRAPH);
	if (id == "shader-mode-code")
		set_shader_edit_mode(ShaderEditMode::CODE);
}


void ModeMaterial::on_draw_win(MultiView::Window *win) {
	auto tex = weak(textures);
	while(tex.num < 5)
		tex.add(session->drawing_helper->tex_white.get());
	tex.add(MultiView::cube_map.get());
	nix::set_textures(tex);
	nix::set_fog(nix::FogMode::EXP, 0,10000,0.001f, Blue);

	for (int i=0; i<data->appearance.passes.num; i++) {
		data->apply_for_rendering(i);
		win->set_shader(shaders[i].get());

		//nix::draw_triangles(vertex_buffer[max(data->appearance.texture_files.num, 1)]);
		nix::draw_triangles(vertex_buffer[1].get());
	}


	nix::set_cull(nix::CullMode::BACK);
	nix::enable_fog(false);
	nix::disable_alpha();
	nix::set_z(true,true);
}



void ModeMaterial::open() {
	session->universal_open(FD_MATERIAL);
	/*if (!storage->open(data))
		return false;

	optimize_view();
	session->set_mode(mode_material);
	return true;*/
}



void ModeMaterial::on_end() {

	session->win->set_side_panel(nullptr);
	session->win->set_bottom_panel(nullptr);

	auto *t = session->win->get_toolbar(hui::TOOLBAR_TOP);
	t->reset();
	t->enable(false);

	geo = nullptr;

	for (int i=1; i<=__MATERIAL_MAX_TEXTURES; i++)
		vertex_buffer[i] = nullptr;

	//shader->unref();
	shaders.clear();
}


void ModeMaterial::save_as() {
	test_save_extras(data, [this] {
		session->storage->save_as(data);
	});
}


void ModeMaterial::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_TOP)->set_by_id("material-toolbar");
	auto t = session->win->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);

	data->out_changed >> in_data_changed;
	on_data_update();

	shader_edit_mode = ShaderEditMode::NONE;

	for (int i=1; i<=__MATERIAL_MAX_TEXTURES; i++) {
		vertex_buffer[i] = new nix::VertexBuffer("3f,3fn" + string(",2f").repeat(i));
	}

	shape_type = hui::config.get_str("MaterialShapeType", "teapot");
	shape_smooth = hui::config.get_bool("MaterialShapeSmooth", true);


	multi_view->set_allow_select(false);

	appearance_dialog = new MaterialPropertiesDialog(session->win, this);
	session->win->set_side_panel(appearance_dialog);

	update_shape();
}

void ModeMaterial::set_shape_type(const string &type) {
	shape_type = type;
	hui::config.set_str("MaterialShapeType", shape_type);
	update_shape();
}

void ModeMaterial::set_shape_smooth(bool smooth) {
	shape_smooth = smooth;
	hui::config.set_bool("MaterialShapeSmooth", shape_smooth);
	update_shape();
}

void ModeMaterial::update_shape() {
	bool needs_opt_view = !geo;
	if (shape_type == "torus")
		geo = new GeometryTorus(v_0, vec3::EZ, MATERIAL_RADIUS1, MATERIAL_RADIUS2, MATERIAL_DETAIL*2, MATERIAL_DETAIL);
	else if (shape_type == "torusknot")
		geo = new GeometryTorusKnot(v_0, vec3::EZ, MATERIAL_RADIUS1, 40, 22, 2, 5, MATERIAL_DETAIL*10, MATERIAL_DETAIL*2);
	else if (shape_type == "teapot")
		geo = new GeometryTeapot(v_0, MATERIAL_RADIUS1, MATERIAL_DETAIL/3);
	else if (shape_type == "cube")
		geo = new GeometryCube(-vec3(1,1,1) * MATERIAL_RADIUS1/2, vec3::EX * MATERIAL_RADIUS1, vec3::EY * MATERIAL_RADIUS1, vec3::EZ * MATERIAL_RADIUS1, 1, 1, 1);
		//geo = new ModelGeometryPlatonic(v_0, MATERIAL_RADIUS1, 6);
	else if (shape_type == "icosahedron")
		geo = new GeometryPlatonic(v_0, MATERIAL_RADIUS1, 20);
	else //if (shape_type == "ball")
		geo = new GeometryBall(v_0, MATERIAL_RADIUS1, MATERIAL_DETAIL, MATERIAL_DETAIL*2);
	if (shape_smooth)
		geo->smoothen();

	for (int i=1; i<=__MATERIAL_MAX_TEXTURES; i++) {
		geo->build(vertex_buffer[i].get());
	}
	on_update_menu();
	if (needs_opt_view)
		optimize_view();
}

bool ModeMaterial::optimize_view() {
	multi_view->reset_view();
	if (geo) {
		vec3 min, max;
		geo->get_bounding_box(min, max);
		multi_view->set_view_box(min, max);
	}
	return true;
}

void ModeMaterial::set_shader_edit_mode(ShaderEditMode mode) {
	if (mode == shader_edit_mode)
		return;

	if (shader_edit_mode == ShaderEditMode::GRAPH) {
		session->win->set_bottom_panel(nullptr);
	}
	shader_edit_mode = mode;
	if (shader_edit_mode == ShaderEditMode::GRAPH) {
		appearance_dialog->set_size(0);
		shader_graph_dialog = new ShaderGraphDialog(data);
		session->win->set_bottom_panel(shader_graph_dialog);
	} else if (shader_edit_mode == ShaderEditMode::NONE) {
		appearance_dialog->set_size(1);
	}
	out_shader_edit_mode_changed();
	on_update_menu();
}

void ModeMaterial::select_render_pass(int p) {
	current_render_pass = p;
	shader_graph_dialog->set_current_render_pass(p);
	out_current_render_pass_changed();
}

void ModeMaterial::on_update_menu() {
	session->win->check("material_shape_smooth", shape_smooth);
	session->win->check("material_shape_cube", shape_type == "cube");
	session->win->check("material_shape_ball", shape_type == "ball");
	session->win->check("material_shape_torus", shape_type == "torus");
	session->win->check("material_shape_torusknot", shape_type == "torusknot");
	session->win->check("material_shape_teapot", shape_type == "teapot");
	session->win->check("material_shape_icosahedron", shape_type == "icosahedron");

	session->win->check("shader-mode-none", shader_edit_mode == ShaderEditMode::NONE);
	session->win->check("shader-mode-graph", shader_edit_mode == ShaderEditMode::GRAPH);
	session->win->check("shader-mode-code", shader_edit_mode == ShaderEditMode::CODE);
}



