/*
 * ModeMaterial.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ModeMaterial.h"
#include "Dialog/MaterialPropertiesDialog.h"
#include "Dialog/ShaderGraphDialog.h"
#include "../../Edward.h"
#include "../../Storage/Storage.h"
#include "../../Data/Material/DataMaterial.h"
#include "../../Data/Material/ShaderGraph.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/Geometry/GeometryCube.h"
#include "../../Data/Model/Geometry/GeometryBall.h"
#include "../../Data/Model/Geometry/GeometryPlatonic.h"
#include "../../Data/Model/Geometry/GeometryTorus.h"
#include "../../Data/Model/Geometry/GeometryTorusKnot.h"
#include "../../Data/Model/Geometry/GeometryTeapot.h"
#include "../../MultiView/MultiView.h"
#include "../../MultiView/Window.h"
#include "../../MultiView/ColorScheme.h"
#include "../../MultiView/DrawingHelper.h"
#include "../../lib/os/file.h"
#include "../../lib/nix/nix.h"
#include "../../y/ResourceManager.h"

const int MATERIAL_DETAIL = 32;
const float MATERIAL_RADIUS1 = 80;
const float MATERIAL_RADIUS2 = 50;

ModeMaterial *mode_material = NULL;

#define __MATERIAL_MAX_TEXTURES		4

ModeMaterial::ModeMaterial(MultiView::MultiView *mv) :
	Mode("Material", nullptr, new DataMaterial, mv, "menu_material")
{
	geo = nullptr;

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
	if (data->shader.is_default) {
		cb_success();
		return;
	}

	auto f = [data, cb_success] (const Path &dir) {
		os::fs::write_text(dir << data->shader.file, data->shader.code);

		if (data->shader.from_graph)
			data->shader.graph->save(dir << data->shader.file.with(".graph"));
		cb_success();
	};

	Path dir = storage->last_dir[FD_SHADERFILE];
	if (data->shader.file) {
		f(dir);
	} else {
		hui::file_dialog_save(ed, "Shader...", dir, {"filter=*.shader", "showfilter=*.shader"}, [data, dir, f] (const Path &path) {
			if (path) {
				data->shader.file = path.relative_to(dir);
				f(dir);
			}
		});
	}
}



void ModeMaterial::save() {
	test_save_extras(data, [this] {
		storage->auto_save(data);
	});
}



void ModeMaterial::on_draw() {
}



void ModeMaterial::on_data_update() {
	update_textures();
	update_shader();
}

void ModeMaterial::update_textures() {
	textures.clear();

	for (auto &tf: data->appearance.texture_files)
		textures.add(ResourceManager::load_texture(tf));
	/*if (appearance.reflection_mode == ReflectionMode::CUBE_MAP_DYNAMIC) {
		create_fake_dynamic_cube_map(cube_map);
		textures.add(cube_map);
	} else if (appearance.reflection_mode == ReflectionMode::CUBE_MAP_STATIC) {
		for (int i=0;i<6;i++)
			temp.cube_map->fill_side(i, nix::LoadTexture(appearance.reflection_texture_file[i]));
		temp.textures.add(temp.cube_map);
	}*/
}

void ModeMaterial::update_shader() {
	msg_write("update shader");
	try {
		auto code = ResourceManager::expand_vertex_shader_source(data->shader.code, "default");
		code = ResourceManager::expand_fragment_shader_source(code, "forward");
		shader->update(code);
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
	data->apply_for_rendering();
	win->set_shader(shader.get());
	auto tex = weak(textures);
	tex.resize(5);
	tex.add(MultiView::cube_map.get());
	nix::set_textures(tex);
	nix::set_fog(nix::FogMode::EXP, 0,10000,0.001f, Blue);

	nix::draw_triangles(MaterialVB[max(data->appearance.texture_files.num, 1)]);


	nix::enable_fog(false);
	nix::disable_alpha();
	nix::set_z(true,true);
}



void ModeMaterial::open() {
	ed->universal_open(FD_MATERIAL);
	/*if (!storage->open(data))
		return false;

	optimize_view();
	ed->set_mode(mode_material);
	return true;*/
}



void ModeMaterial::on_end() {

	ed->set_side_panel(nullptr);
	ed->set_bottom_panel(nullptr);

	auto *t = ed->toolbar[hui::TOOLBAR_TOP];
	t->reset();
	t->enable(false);

	if (geo)
		delete geo;

	for (int i=1; i<=__MATERIAL_MAX_TEXTURES; i++)
		delete MaterialVB[i];

	//shader->unref();
	shader = nullptr;
}


void ModeMaterial::save_as() {
	test_save_extras(data, [this] {
		storage->save_as(data);
	});
}


void ModeMaterial::on_start() {
	msg_write("Material.on start");

	ed->toolbar[hui::TOOLBAR_TOP]->set_by_id("material-toolbar");
	auto t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);

	try {
		shader = nix::Shader::create("<VertexShader>void main(){gl_Position = vec4(0);}</VertexShader><FragmentShader>void main(){}</FragmentShader>");
	} catch (Exception &e) {
		msg_error(e.message());
	}

	data->subscribe(this, [=]{ on_data_update(); });
	on_data_update();

	shader_edit_mode = ShaderEditMode::NONE;

	for (int i=1; i<=__MATERIAL_MAX_TEXTURES; i++) {
		string f = "3f,3fn";
		for (int j=0; j<i; j++)
			f += ",2f";
		MaterialVB[i] = new nix::VertexBuffer(f);
	}

	shape_type = hui::config.get_str("MaterialShapeType", "teapot");
	shape_smooth = hui::config.get_bool("MaterialShapeSmooth", true);


	multi_view->set_allow_select(false);

	appearance_dialog = new MaterialPropertiesDialog(ed, data);
	ed->set_side_panel(appearance_dialog);

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
	if (geo)
		delete(geo);
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
		geo->build(MaterialVB[i]);
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
		ed->set_bottom_panel(nullptr);
	}
	shader_edit_mode = mode;
	if (shader_edit_mode == ShaderEditMode::GRAPH) {
		shader_graph_dialog = new ShaderGraphDialog(data);
		ed->set_bottom_panel(shader_graph_dialog);
	}
	on_update_menu();
}

void ModeMaterial::on_update_menu() {
	ed->check("material_shape_smooth", shape_smooth);
	ed->check("material_shape_cube", shape_type == "cube");
	ed->check("material_shape_ball", shape_type == "ball");
	ed->check("material_shape_torus", shape_type == "torus");
	ed->check("material_shape_torusknot", shape_type == "torusknot");
	ed->check("material_shape_teapot", shape_type == "teapot");
	ed->check("material_shape_icosahedron", shape_type == "icosahedron");

	ed->check("shader-mode-none", shader_edit_mode == ShaderEditMode::NONE);
	ed->check("shader-mode-graph", shader_edit_mode == ShaderEditMode::GRAPH);
	ed->check("shader-mode-code", shader_edit_mode == ShaderEditMode::CODE);
}



