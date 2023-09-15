/*
 * ModeModelMesh.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "ModeModelMesh.h"
#include "ModeModelMeshTexture.h"
#include "ModeModelMeshMaterial.h"
#include "ModeModelMeshDeform.h"
#include "ModeModelMeshPaint.h"
#include "creation/ModeModelMeshCreateVertex.h"
#include "creation/ModeModelMeshCreatePolygon.h"
#include "creation/ModeModelMeshCreateBall.h"
#include "creation/ModeModelMeshCreateCube.h"
#include "creation/ModeModelMeshCreateCylinder.h"
#include "creation/ModeModelMeshCreateCylinderSnake.h"
#include "creation/ModeModelMeshCreatePlane.h"
#include "creation/ModeModelMeshCreateTorus.h"
#include "creation/ModeModelMeshCreatePlatonic.h"
#include "creation/ModeModelMeshCutLoop.h"
#include "creation/ModeModelMeshSplitPolygon.h"
#include "creation/ModeModelMeshAutoweld.h"
#include "creation/ModeModelMeshBevelEdges.h"
#include "creation/ModeModelMeshExtrudePolygons.h"
#include "creation/ModeModelMeshPaste.h"
#include "creation/ModeModelMeshDeformFunction.h"
#include "creation/ModeModelMeshDeformCylinder.h"
#include "selection/MeshSelectionModeEdge.h"
#include "selection/MeshSelectionModePolygon.h"
#include "selection/MeshSelectionModeSurface.h"
#include "selection/MeshSelectionModeVertex.h"
#include "../dialog/ModelMaterialSelectionDialog.h"
#include "../dialog/ModelMaterialDialog.h"
#include "../dialog/ModelEasifyDialog.h"
#include "../dialog/ModelFXDialog.h"
#include "../ModeModel.h"
#include "../skeleton/ModeModelSkeleton.h"
#include "../../ModeCreation.h"
#include "../../../data/model/ModelMesh.h"
#include "../../../data/model/geometry/GeometrySphere.h"
#include "../../../data/model/geometry/GeometryCylinder.h"
#include "../../../action/model/mesh/skin/ActionModelSkinVerticesFromProjection.h"
#include "../../../EdwardWindow.h"
#include "../../../Session.h"
#include "../../../stuff/Clipboard.h"
#include "../../../multiview/MultiView.h"
#include "../../../multiview/Window.h"
#include "../../../multiview/DrawingHelper.h"
#include "../../../multiview/ColorScheme.h"
#include "../../../lib/nix/nix.h"


string vb_format(int num_tex) {
	string f = "3f,3fn";
	for (int i=0; i<num_tex; i++)
		f += ",2f";
	return f;
}

ModeModelMesh::ModeModelMesh(ModeModel *_parent, MultiView::MultiView *mv3, MultiView::MultiView *mv2) :
	Mode<ModeModel, DataModel>(_parent->session, "ModelMesh", _parent, mv3, "menu_model") {

	selection_mode = NULL;
	current_material = 0;
	current_skin = MESH_HIGH;

	// vertex buffers
	vb_marked = new nix::VertexBuffer(vb_format(1));
	vb_hover = new nix::VertexBuffer(vb_format(1));
	vb_creation = new nix::VertexBuffer(vb_format(1));

	select_cw = false;
	allow_draw_hover = true;

	choose_mouse_function(MultiView::ACTION_MOVE, false);

	selection_mode_vertex = new MeshSelectionModeVertex(this);
	selection_mode_edge = new MeshSelectionModeEdge(this);
	selection_mode_polygon = new MeshSelectionModePolygon(this);
	selection_mode_surface = new MeshSelectionModeSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this, mv2);
	mode_model_mesh_material = new ModeModelMeshMaterial(this, mv3);
	mode_model_mesh_deform = new ModeModelMeshDeform(this, mv3);
	mode_model_mesh_paint = new ModeModelMeshPaint(this, mv3);

	selection_mode = selection_mode_polygon;
}

ModeModelMesh::~ModeModelMesh() {
}

void ModeModelMesh::on_start() {
	session->win->get_toolbar(hui::TOOLBAR_LEFT)->set_by_id("model-mesh-toolbar");

	data->out_changed >> create_sink([=]{ on_data_update(); });
	data->out_selection >> create_sink([=]{ on_data_update(); });

	update_vertex_buffers(data->mesh->vertex);

	set_selection_mode(selection_mode);
	session->mode_model->allow_selection_modes(true);
	on_data_update();
}

void ModeModelMesh::on_enter() {
	current_material = 0;
	session->mode_model->allow_selection_modes(true);
	multi_view->set_allow_action(true);
	multi_view->set_allow_select(true);
}

void ModeModelMesh::on_end() {
	data->unsubscribe(this);

	auto *t = session->win->get_toolbar(hui::TOOLBAR_LEFT);
	t->reset();
	t->enable(false);
}



void selection_grow_smooth_group(DataModel *data) {
	for (auto &p: data->edit_mesh->polygon)
		if (p.is_selected and p.smooth_group >= 0) {
			for (auto &pp: data->edit_mesh->polygon)
				if (pp.smooth_group == p.smooth_group)
					pp.is_selected = true;

		}
	data->edit_mesh->selection_from_polygons();
}

void ModeModelMesh::on_command(const string &id) {
	if (id == "delete") {
		data->delete_selection(data->edit_mesh->get_selection(), selection_mode == selection_mode_vertex);
		multi_view->selection_changed_manually();
	}
	if (id == "copy")
		copy();
	if (id == "paste")
		session->set_mode(new ModeModelMeshPaste(this));
		//paste();

	if (id == "select_cw")
		toggle_select_cw();
	if (id == "selection-grow-smooth-group")
		selection_grow_smooth_group(data);

	if (id == "volume_subtract")
		data->subtractSelection(multi_view->view_stage);
	if (id == "volume_and")
		data->andSelection();
	if (id == "invert_trias")
		data->invert_polygons(data->get_selection());
	if (id == "extrude_triangles")
		session->set_mode(new ModeModelMeshExtrudePolygons(this, false));
	if (id == "extrude_triangles_independent")
		session->set_mode(new ModeModelMeshExtrudePolygons(this, true));
	if (id == "autoweld_surfaces")
		session->set_mode(new ModeModelMeshAutoweld(this));
	if (id == "convert_to_triangles")
		data->convertSelectionToTriangles();
	if (id == "untriangulate_selection")
		data->mergePolygonsSelection();
	if (id == "cut_out")
		data->cutOutSelection();
	if (id == "nearify")
		data->nearifySelectedVertices();
	if (id == "connect")
		data->collapseSelectedVertices();
	if (id == "align_to_grid")
		data->alignToGridSelection(multi_view->active_win->get_grid_d());
	if (id == "triangulate_selection")
		data->triangulateSelectedVertices();
	if (id == "subdivide_surfaces")
		data->subdivideSelectedSurfaces(data->get_selection());

	if (id == "new_point")
		session->set_mode(new ModeModelMeshCreateVertex(this));
	if (id == "new_tria")
		session->set_mode(new ModeModelMeshCreatePolygon(this));
	if (id == "new_ball")
		session->set_mode(new ModeModelMeshCreateBall(this));
	if (id == "new_cube")
		session->set_mode(new ModeModelMeshCreateCube(this));
	if (id == "new_cylinder")
		session->set_mode(new ModeModelMeshCreateCylinder(this));
	if (id == "new_cylindersnake")
		session->set_mode(new ModeModelMeshCreateCylinderSnake(this));
	if (id == "new_plane")
		session->set_mode(new ModeModelMeshCreatePlane(this));
	if (id == "new_torus")
		session->set_mode(new ModeModelMeshCreateTorus(this));
	if (id == "new_tetrahedron")
		session->set_mode(new ModeModelMeshCreatePlatonic(this, 4));
	if (id == "new_octahedron")
		session->set_mode(new ModeModelMeshCreatePlatonic(this, 8));
	if (id == "new_dodecahedron")
		session->set_mode(new ModeModelMeshCreatePlatonic(this, 12));
	if (id == "new_icosahedron")
		session->set_mode(new ModeModelMeshCreatePlatonic(this, 20));
	if (id == "new_teapot")
		session->set_mode(new ModeModelMeshCreatePlatonic(this, 306));
	if (id == "split_polygon")
		session->set_mode(new ModeModelMeshSplitPolygon(this));
	if (id == "bevel_edges")
		session->set_mode(new ModeModelMeshBevelEdges(this));
	if (id == "cut_loop")
		session->set_mode(new ModeModelMeshCutLoop(this));
	if (id == "deformation_function")
		session->set_mode(new ModeModelMeshDeformFunction(this));
	if (id == "deformation_cylinder")
		session->set_mode(new ModeModelMeshDeformCylinder(this));
	if (id == "flatten_vertices")
		data->flattenSelectedVertices();

	if (id == "select")
		choose_mouse_function(MultiView::ACTION_SELECT, lock_action);
	if (id == "translate")
		choose_mouse_function(MultiView::ACTION_MOVE, lock_action);
	if (id == "rotate")
		choose_mouse_function(MultiView::ACTION_ROTATE, lock_action);
	if (id == "scale")
		choose_mouse_function(MultiView::ACTION_SCALE, lock_action);
	if (id == "mirror")
		choose_mouse_function(MultiView::ACTION_MIRROR, lock_action);
	if (id == "lock_action")
		choose_mouse_function(mouse_action, !lock_action);

	if (id == "create_new_material")
		create_new_material_for_selection();
	if (id == "choose_material")
		choose_material_for_selection();
	if (id == "text_from_bg")
		data->execute(new ActionModelSkinVerticesFromProjection(data, multi_view));
	if (id == "automapping")
		data->automap(current_material, mode_model_mesh_texture->current_texture_level);
	if (id == "easify_skin")
		easify();

	if (id == "normal_this_smooth")
		data->setNormalModeSelection(NORMAL_MODE_SMOOTH);
	if (id == "normal_this_hard")
		data->setNormalModeSelection(NORMAL_MODE_HARD);

	if (id == "fx_new_light")
		add_effects(FX_TYPE_LIGHT);
	if (id == "fx_new_sound")
		add_effects(FX_TYPE_SOUND);
	if (id == "fx_new_script")
		add_effects(FX_TYPE_SCRIPT);
	if (id == "fx_new_field")
		add_effects(FX_TYPE_FORCEFIELD);
	if (id == "fx_none")
		clear_effects();
	if (id == "fx_edit")
		edit_effects();

	if (id == "detail_physical")
		set_current_skin(MESH_PHYSICAL);
	if (id == "detail_high")
		set_current_skin(MESH_HIGH);
}


void ModeModelMesh::on_draw() {
	auto s = data->edit_mesh->get_selection();
	if (s.vertex.num > 0) {
		nix::set_shader(session->gl->default_2d.get());
		string t = format("selected: %d vertices, %d edges, %d polygons", s.vertex.num, s.edge.num, s.polygon.num);
		if (current_skin == MESH_PHYSICAL)
			t += format(", %d balls, %d cylinders", s.ball.num, s.cylinder.num);
				//    %d  %d
		session->drawing_helper->draw_str(10, nix::target_height - 25, t);
	}
}


void ModeModelMesh::on_draw_win(MultiView::Window *win) {

	// visible skin
	draw_mesh(win, data->mesh, data->mesh->vertex, true);

	parent->mode_model_skeleton->draw_skeleton(win, data->bone, true);

	if (current_skin == MESH_PHYSICAL)
		draw_physical(win);

	draw_creation_preview(win);

	if (allow_draw_hover)
		selection_mode->on_draw_win(win);
}


void mesh_check_undef_view_stage(ModelMesh *m, int vs) {
	for (auto &v: m->vertex)
		if (v.view_stage < 0)
			v.view_stage = vs;
	for (auto &e: m->edge)
		if (e.view_stage < 0)
			e.view_stage = vs;
	for (auto &p: m->polygon)
		if (p.view_stage < 0)
			p.view_stage = vs;
}

void ModeModelMesh::on_data_update() {
	// consistency checks
	if (current_material >= data->material.num)
		set_current_material(data->material.num - 1);
	//data->DebugShow();
	//msg_write(o->getName() + " - " + message);

	mesh_check_undef_view_stage(data->edit_mesh, multi_view->view_stage);


	fill_selection_buffer(data->edit_mesh->vertex);
}

void ModeModelMesh::on_view_stage_change() {
	//msg_write("mesh: on view stage change");
	selection_mode->on_view_stage_change();
	update_vertex_buffers(data->mesh->vertex);
}

void ModeModelMesh::on_selection_change() {
	//msg_write("mesh: on sel change");
	selection_mode->on_update_selection();
	fill_selection_buffer(data->edit_mesh->vertex);
}

void ModeModelMesh::on_set_multi_view() {
	//msg_write("mesh: on set mv");
	selection_mode->update_multi_view();
	update_vertex_buffers(data->mesh->vertex);
}



void ModeModelMesh::on_update_menu() {
	session->win->enable("copy", copyable());
	session->win->enable("paste", pasteable());
	session->win->enable("delete", copyable());
	string cm_name = session->cur_mode->name;
	session->win->check("new_point", cm_name == "ModelMeshCreateVertex");
	session->win->check("new_tria", cm_name == "ModelMeshCreatePolygon");
	session->win->check("new_plane", cm_name == "ModelMeshCreatePlane");
	session->win->check("new_cube", cm_name == "ModelMeshCreateCube");
	session->win->check("new_ball", cm_name == "ModelMeshCreateBall");
	session->win->check("new_cylinder", cm_name == "ModelMeshCreateCylinder");
	session->win->check("new_torus", cm_name == "ModelMeshCreateTorus");

	session->win->check("select_cw", select_cw);
	session->win->check("snap_to_grid", multi_view->snap_to_grid);

	session->win->enable("select", multi_view->allow_mouse_actions);
	session->win->enable("translate", multi_view->allow_mouse_actions);
	session->win->enable("rotate", multi_view->allow_mouse_actions);
	session->win->enable("scale", multi_view->allow_mouse_actions);
	session->win->enable("mirror", multi_view->allow_mouse_actions);
	session->win->enable("lock_action", multi_view->allow_mouse_actions);
	session->win->check("select", mouse_action == MultiView::ACTION_SELECT);
	session->win->check("translate", mouse_action == MultiView::ACTION_MOVE);
	session->win->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	session->win->check("scale", mouse_action == MultiView::ACTION_SCALE);
	session->win->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
	session->win->check("lock_action", lock_action);

	session->win->check("detail_physical", current_skin == MESH_PHYSICAL);
	session->win->check("detail_high", current_skin == MESH_HIGH);
	session->win->check("detail_2", current_skin == MESH_MEDIUM);
	session->win->check("detail_3", current_skin == MESH_LOW);
}

bool ModeModelMesh::optimize_view() {
	auto *mv = multi_view;
	bool ww = mv->whole_window;
	mv->reset_view();
	mv->whole_window = ww;
	vec3 min, max;
	data->mesh->get_bounding_box(min, max);
	if (min != max)
		mv->set_view_box(min, max);

	session->multi_view_2d->reset_view();
	session->multi_view_2d->cam.pos = vec3(0.5f, 0.5f, 0);
	/*if ((Bone.num > 0) and (Vertex.num <= 0))
		SetSubMode(SubModeSkeleton);
	if (SubMode == SubModeSkeleton)
		SkeletonOptimizeView();*/
	return true;
}



void ModeModelMesh::create_new_material_for_selection() {
#if 0
	msg_db_f("CreateNewMaterialForSelection", 2);
	if (0 == data->getNumSelectedPolygons()) {
		session->set_message(_("no triangle selected"));
		return;
	}

	//StartChanging();

	data->material.resize(data->material.num + 1);
	data->material.num --; // stupid...
	data->material[data->material.num].reset();
	//Material[Material.num] = cur_mat;
	int cmi = data->current_material;
	data->current_material = data->material.num;

	if (mode_model->ExecuteMaterialDialog(0)) {//, true)) {
		data->material.num ++;

		data->current_texture_level = 0;

		// create new subs
		for (int i=0;i<4;i++) {
			data->Skin[i].Sub.resize(data->material.num);
			data->Skin[i].Sub[data->current_material].NumTextures = data->material[data->current_material].NumTextures;
		}

		// move selected trias to the new sub
//		move_sel_trias_to_mat(CurrentMaterial);
	}else{
		data->material.num ++;
		data->material.resize(data->material.num - 1);
		data->current_material = cmi;
	}

	//EndChanging();
#endif
}

void ModeModelMesh::choose_material_for_selection() {
	if (data->get_selection().polygon.num == 0) {
		session->set_message(_("no polygon selected"));
		return;
	}

	auto dlg = new ModelMaterialSelectionDialog(session->win, false, data);
	hui::run(dlg, [this, dlg] {
		if (dlg->answer >= 0)
			data->setMaterialSelection(dlg->answer);
	});
}

void ModeModelMesh::choose_mouse_function(int f, bool _lock_action) {
	mouse_action = f;
	lock_action = _lock_action;
	apply_mouse_function(session->multi_view_3d);
	apply_mouse_function(session->multi_view_2d);
}

void ModeModelMesh::apply_mouse_function(MultiView::MultiView *mv) {
	if (!mv)
		return;

	string name = "ActionModelTransformVertices";
	if (!mv->mode3d)
		name = "ActionModelTransformSkinVertices";

	mv->set_mouse_action(name, mouse_action, lock_action);
}

void ModeModelMesh::copy() {
	clipboard.set_mesh_data(data->edit_mesh->copy_geometry());
	const auto& geo = clipboard.temp_geo;

	//on_update_menu();
	session->set_message(format(_("%d vertices, %d triangles copied"), geo.vertex.num, geo.polygon.num));
}

void ModeModelMesh::paste() {
	const auto& geo = clipboard.temp_geo;
	data->pasteGeometry(geo, current_material);
	session->set_message(format(_("%d vertices, %d triangles pasted"), geo.vertex.num, geo.polygon.num));
}

bool ModeModelMesh::copyable() {
	auto s = data->get_selection();
	return s.vertex.num > 0;
}

void ModeModelMesh::add_effects(int type) {
	if (data->get_selection().vertex.num == 0) {
		session->set_message(_("No vertex point selected"));
		return;
	}
	hui::fly(new ModelFXDialog(session->win, false, data, type, -1));
}

void ModeModelMesh::edit_effects() {
	int index;
	int n = 0;
	foreachi(ModelEffect &fx, data->fx, i)
		if (data->mesh->vertex[fx.vertex].is_selected) {
			index = i;
			n ++;
		}
	if (n != 1) {
		session->set_message(_("One vertex point with effects has to be selected!"));
		return;
	}
	hui::fly(new ModelFXDialog(session->win, false, data, -1, index));
}

void ModeModelMesh::clear_effects() {
	int n = 0;
	for (ModelEffect &fx: data->fx)
		if (data->mesh->vertex[fx.vertex].is_selected)
			n ++;
	if (n == 0) {
		session->set_message(_("No vertex with effects selected!"));
		return;
	}
	data->selectionClearEffects();
}

bool ModeModelMesh::pasteable() {
	return clipboard.has_mesh_data();
}

void ModeModelMesh::easify() {
	hui::fly(new ModelEasifyDialog(session->win, false, data));
}

void ModeModelMesh::set_current_material(int index) {
	if (current_material == index)
		return;
	current_material = index;
	state.out_current_material_changed.notify();
	mode_model_mesh_texture->set_current_texture_level(0);
}

void ModeModelMesh::set_current_skin(int index) {
	if (current_skin == index)
		return;

	data->edit_mesh = data->mesh;
	if (index == MESH_PHYSICAL) {
		data->edit_mesh = data->phys_mesh;
		data->phys_mesh->set_show_vertices(data->phys_mesh->vertex);
		msg_write("PHYSICAL");
	}
	data->reset_history();
	session->set_message(_("changing mesh... cleared history"));

	current_skin = index;

	selection_mode->update_multi_view();
	update_vertex_buffers(data->mesh->vertex);
	state.out_current_skin_changed.notify();
}

void ModeModelMesh::draw_effects(MultiView::Window *win) {
	nix::set_shader(win->gl->default_2d.get());
	for (ModelEffect &fx: data->fx) {
		vec3 p = win->project(data->mesh->vertex[fx.vertex].pos);
		if ((p.z > 0) and (p.z < 1))
			win->drawing_helper->draw_str(p.x, p.y, fx.get_type());
	}
}

void _draw_edges(DataModel *data, MultiView::Window *win, ModelMesh *m, const Array<ModelVertex> &vertex, bool selection_filter) {
}

void ModeModelMesh::draw_edges(MultiView::Window *win, ModelMesh *m, const Array<ModelVertex> &vertex, bool selected, bool non_selected, bool as_selected) {

	color bg = win->get_background_color();
	auto *multi_view = win->multi_view;

	nix::set_offset(-2);
	win->drawing_helper->set_line_width(as_selected ? scheme.LINE_WIDTH_MEDIUM : scheme.LINE_WIDTH_THIN);
	Array<vec3> line_pos;
	Array<color> line_color;

	vec3 dir = win->get_direction();
	for (auto &e: m->edge) {
		if (!(e.is_selected and selected) and !(!e.is_selected and non_selected))
			continue;
		if (min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage) < multi_view->view_stage)
			continue;

		float w = 1;
		if (e.polygon[0] >= 0 and e.polygon[1] >= 0)
			w = min(vec3::dot(m->polygon[e.polygon[0]].temp_normal, dir), vec3::dot(m->polygon[e.polygon[1]].temp_normal, dir));
		else if (e.polygon[0] >= 0)
			w = vec3::dot(m->polygon[e.polygon[0]].temp_normal, dir);
		else if (e.polygon[1] >= 0)
			w = vec3::dot(m->polygon[e.polygon[1]].temp_normal, dir);
		float f = 0.5f - 0.4f*w;//0.7f - 0.3f * w;
		color cc;
		if (as_selected) {
			cc = color::interpolate(scheme.SELECTION, bg, 1-f);
		} else {
			cc = color::interpolate(scheme.TEXT, bg, 1-f);
		}
		line_color.add(cc);
		line_color.add(cc);
		line_pos.add(vertex[e.vertex[0]].pos);
		line_pos.add(vertex[e.vertex[1]].pos);
	}
	win->drawing_helper->draw_lines_colored(line_pos, line_color, false);
	nix::set_offset(0);
}


// always visible mesh!
void ModeModelMesh::draw_mesh(MultiView::Window *win, ModelMesh *mesh, const Array<ModelVertex> &vertex, bool selectable) {
	if (multi_view->wire_mode) {
		draw_edges(win, mesh, vertex, true, false, selectable);
		draw_edges(win, mesh, vertex, false, true, false);
	} else {

		draw_polygons(win, mesh, vertex);

		if (selection_mode_surface->is_active()) {
			draw_edges(win, mesh, vertex, true, false, selectable);
		} else {
			draw_edges(win, mesh, vertex, true, false, selectable);
			draw_edges(win, mesh, vertex, false, true, false);
		}
	}

	if (selectable)
		draw_selection(win);
}

// always visible mesh!
void ModeModelMesh::draw_polygons(MultiView::Window *win, ModelMesh *mesh, const Array<ModelVertex> &vertex) {

	// draw all materials separately
	for (auto *m: data->material) {
		if (!m->vb)
			continue;

		// draw
		m->apply_for_rendering(win);
		nix::set_offset(0);
		nix::draw_triangles(m->vb);
	}
	nix::disable_alpha();
	nix::set_z(true, true);
}


void ModeModelMesh::draw_physical(MultiView::Window *win) {

	for (auto &b: data->phys_mesh->ball) {
		auto geo = GeometrySphere(data->phys_mesh->vertex[b.index].pos, b.radius, 6);
		geo.build(win->gl->vb_temp);
		win->drawing_helper->set_material_creation(1.5f);
		if (b.is_selected)
			win->drawing_helper->set_material_selected();
		if (multi_view->hover.data == &b)
			win->drawing_helper->set_material_hover();
		nix::draw_triangles(win->gl->vb_temp);
	}

	for (auto &c: data->phys_mesh->cylinder) {
		auto geo = GeometryCylinder(data->phys_mesh->vertex[c.index[0]].pos, data->phys_mesh->vertex[c.index[1]].pos, c.radius, 1, 24, c.round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT);
		geo.build(win->gl->vb_temp);
		win->drawing_helper->set_material_creation(1.5f);
		if (c.is_selected)
			win->drawing_helper->set_material_selected();
		if (multi_view->hover.data == &c)
			win->drawing_helper->set_material_hover();
		nix::draw_triangles(win->gl->vb_temp);
	}


	win->drawing_helper->set_material_creation(1.5f);
	VertexStagingBuffer vbs;
	for (auto &t: data->phys_mesh->polygon)
		if (t.view_stage >= multi_view->view_stage)
			t.add_to_vertex_buffer(data->phys_mesh->vertex, vbs, 1);
	vbs.build(win->gl->vb_temp, 1);
	nix::set_offset(-0.5f);
	nix::draw_triangles(win->gl->vb_temp);
	nix::set_offset(0);
	draw_edges(win, data->phys_mesh, data->phys_mesh->vertex, false, true, false);
}



void ModeModelMesh::update_vertex_buffers(const Array<ModelVertex> &vertex) {
	//msg_write("update vertex buffers!!!!!!!!!!");
	// draw all materials separately
	foreachi(ModelMaterial *m, data->material, mi) {
		int num_tex = m->texture_levels.num;
		if (!m->vb)
			m->vb = new nix::VertexBuffer(vb_format(num_tex));
		if (m->vb->num_attributes-2 != num_tex) {
			delete m->vb;
			m->vb = new nix::VertexBuffer(vb_format(num_tex));
		}

		VertexStagingBuffer vbs;

		for (ModelPolygon &t: data->mesh->polygon)
			if ((t.view_stage >= multi_view->view_stage) and (t.material == mi))
				t.add_to_vertex_buffer(vertex, vbs, m->texture_levels.num);
		vbs.build(m->vb, m->texture_levels.num);

		//m.vb->optimize();
	}
}



void ModeModelMesh::fill_selection_buffer(const Array<ModelVertex> &vertex) {

	// create selection buffers
	VertexStagingBuffer vbs;
	for (auto &t: data->edit_mesh->polygon)
		/*if (t.view_stage >= ViewStage)*/{
		if (t.is_selected)
			t.add_to_vertex_buffer(vertex, vbs, 1);
	}
	vbs.build(vb_marked, 1);
}

void ModeModelMesh::draw_selection(MultiView::Window *win) {
	nix::set_z(true, true);
	nix::set_offset(-1.0f);

	win->drawing_helper->set_material_selected();
	nix::draw_triangles(vb_marked);

	nix::set_material(White, 0.5f, 0, Black);
	nix::disable_alpha();
	nix::set_offset(0);
}

void ModeModelMesh::draw_creation_preview(MultiView::Window *win) {
	nix::set_z(true, true);
	nix::set_offset(-1.0f);

	win->drawing_helper->set_material_creation();
	nix::draw_triangles(vb_creation);

	nix::set_material(White, 0.5f, 0, Black);
	nix::disable_alpha();
	nix::set_offset(0);
}

void ModeModelMesh::set_selection_mode(MeshSelectionMode *mode) {
	if (selection_mode)
		selection_mode->on_end();
	selection_mode = mode;
	mode->on_start();
	mode->update_multi_view();
	state.out_changed.notify();
	multi_view->force_redraw();
	session->win->update_menu(); // TODO
}

void ModeModelMesh::toggle_select_cw() {
	select_cw = !select_cw;
	state.out_changed.notify();
	session->win->update_menu();
}

void ModeModelMesh::set_allow_draw_hover(bool allow) {
	allow_draw_hover = allow;
}
