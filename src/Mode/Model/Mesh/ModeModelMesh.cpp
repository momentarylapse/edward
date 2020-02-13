/*
 * ModeModelMesh.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "../../../MultiView/DrawingHelper.h"
#include "../../../MultiView/ColorScheme.h"
#include "../../../lib/nix/nix.h"
#include "../ModeModel.h"
#include "ModeModelMesh.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../../ModeCreation.h"
#include "../../../Data/Model/Geometry/GeometrySphere.h"
#include "../../../Data/Model/Geometry/GeometryCylinder.h"
#include "Creation/ModeModelMeshCreateVertex.h"
#include "Creation/ModeModelMeshCreatePolygon.h"
#include "Creation/ModeModelMeshCreateBall.h"
#include "Creation/ModeModelMeshCreateCube.h"
#include "Creation/ModeModelMeshCreateCylinder.h"
#include "Creation/ModeModelMeshCreateCylinderSnake.h"
#include "Creation/ModeModelMeshCreatePlane.h"
#include "Creation/ModeModelMeshCreateTorus.h"
#include "Creation/ModeModelMeshCreatePlatonic.h"
#include "Creation/ModeModelMeshSplitPolygon.h"
#include "Creation/ModeModelMeshAutoweld.h"
#include "Creation/ModeModelMeshBevelEdges.h"
#include "Creation/ModeModelMeshExtrudePolygons.h"
#include "Creation/ModeModelMeshPaste.h"
#include "../../../Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"
#include "../Dialog/ModelMaterialDialog.h"
#include "../Dialog/ModelEasifyDialog.h"
#include "../Dialog/ModelFXDialog.h"
#include "Creation/ModeModelMeshDeformFunction.h"
#include "Creation/ModeModelMeshDeformCylinder.h"
#include "Selection/MeshSelectionModeEdge.h"
#include "Selection/MeshSelectionModePolygon.h"
#include "Selection/MeshSelectionModeSurface.h"
#include "Selection/MeshSelectionModeVertex.h"
#include "ModeModelMeshTexture.h"
#include "ModeModelMeshMaterial.h"
#include "ModeModelMeshDeform.h"
#include "ModeModelMeshPaint.h"

ModeModelMesh *mode_model_mesh = NULL;

const string ModeModelMesh::MESSAGE_CURRENT_MATERIAL_CHANGE = "CurrentMaterialChange";

ModeModelMesh::ModeModelMesh(ModeBase *_parent) :
	Mode<DataModel>("ModelMesh", _parent, ed->multi_view_3d, "menu_model"),
	Observable("ModelMesh")
{

	selection_mode = NULL;
	current_material = 0;

	// vertex buffers
	vb_marked = new nix::VertexBuffer(1);
	vb_hover = new nix::VertexBuffer(1);
	vb_creation = new nix::VertexBuffer(1);

	select_cw = false;
	allow_draw_hover = true;

	choose_mouse_function(MultiView::ACTION_MOVE, false);

	selection_mode_vertex = new MeshSelectionModeVertex(this);
	selection_mode_edge = new MeshSelectionModeEdge(this);
	selection_mode_polygon = new MeshSelectionModePolygon(this);
	selection_mode_surface = new MeshSelectionModeSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this);
	mode_model_mesh_material = new ModeModelMeshMaterial(this);
	mode_model_mesh_deform = new ModeModelMeshDeform(this);
	mode_model_mesh_paint = new ModeModelMeshPaint(this);

	selection_mode = selection_mode_polygon;

	Observer::subscribe(data);
	//Observer::subscribe(multi_view);
}

ModeModelMesh::~ModeModelMesh()
{
	Observer::unsubscribe(data);
}

void ModeModelMesh::on_start()
{
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-mesh-toolbar");

	//subscribe(data);

	update_vertex_buffers(data->mesh->vertex);

	set_selection_mode(selection_mode);
	mode_model->allow_selection_modes(true);
}

void ModeModelMesh::on_enter()
{
	current_material = 0;
}

void ModeModelMesh::on_end()
{
	//unsubscribe(data);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);
}



void ModeModelMesh::on_command(const string & id)
{
	if (id == "delete")
		data->delete_selection(data->get_selection(), selection_mode == selection_mode_vertex);
	if (id == "copy")
		copy();
	if (id == "paste")
		ed->set_mode(new ModeModelMeshPaste(this));
		//paste();

	if (id == "select_cw")
		toggle_select_cw();

	if (id == "volume_subtract")
		data->subtractSelection();
	if (id == "volume_and")
		data->andSelection();
	if (id == "invert_trias")
		data->invert_polygons(data->get_selection());
	if (id == "extrude_triangles")
		ed->set_mode(new ModeModelMeshExtrudePolygons(this, false));
	if (id == "extrude_triangles_independent")
		ed->set_mode(new ModeModelMeshExtrudePolygons(this, true));
	if (id == "autoweld_surfaces")
		ed->set_mode(new ModeModelMeshAutoweld(this));
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
		ed->set_mode(new ModeModelMeshCreateVertex(this));
	if (id == "new_tria")
		ed->set_mode(new ModeModelMeshCreatePolygon(this));
	if (id == "new_ball")
		ed->set_mode(new ModeModelMeshCreateBall(this));
	if (id == "new_cube")
		ed->set_mode(new ModeModelMeshCreateCube(this));
	if (id == "new_cylinder")
		ed->set_mode(new ModeModelMeshCreateCylinder(this));
	if (id == "new_cylindersnake")
		ed->set_mode(new ModeModelMeshCreateCylinderSnake(this));
	if (id == "new_plane")
		ed->set_mode(new ModeModelMeshCreatePlane(this));
	if (id == "new_torus")
		ed->set_mode(new ModeModelMeshCreateTorus(this));
	if (id == "new_tetrahedron")
		ed->set_mode(new ModeModelMeshCreatePlatonic(this, 4));
	if (id == "new_octahedron")
		ed->set_mode(new ModeModelMeshCreatePlatonic(this, 8));
	if (id == "new_dodecahedron")
		ed->set_mode(new ModeModelMeshCreatePlatonic(this, 12));
	if (id == "new_icosahedron")
		ed->set_mode(new ModeModelMeshCreatePlatonic(this, 20));
	if (id == "new_teapot")
		ed->set_mode(new ModeModelMeshCreatePlatonic(this, 306));
	if (id == "split_polygon")
		ed->set_mode(new ModeModelMeshSplitPolygon(this));
	if (id == "bevel_edges")
		ed->set_mode(new ModeModelMeshBevelEdges(this));
	if (id == "deformation_function")
		ed->set_mode(new ModeModelMeshDeformFunction(this));
	if (id == "deformation_cylinder")
		ed->set_mode(new ModeModelMeshDeformCylinder(this));
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
	if (id == "normal_this_angular")
		data->setNormalModeSelection(NORMAL_MODE_ANGULAR);

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
}


void ModeModelMesh::on_draw() {
	auto s = data->get_selection();
	if (s.vertex.num > 0){
		draw_str(10, nix::target_height - 25, format("selected: %d vertices, %d edges, %d polygons", s.vertex.num, s.edge.num, s.polygon.num));
	}
}


void ModeModelMesh::on_draw_win(MultiView::Window *win)
{
	draw_all(win, data->mesh->vertex);
}

void ModeModelMesh::draw_all(MultiView::Window *win, Array<ModelVertex> &vertex)
{
	draw_polygons(win, vertex);
	mode_model_skeleton->drawSkeleton(win, data->bone, true);

	draw_selection(win);

	draw_edges(win, vertex, !selection_mode_edge->is_active());

	draw_physical(win);

	if (allow_draw_hover)
		selection_mode->on_draw_win(win);
}



void ModeModelMesh::on_update(Observable *o, const string &message)
{
	// consistency checks
	if (current_material >= data->material.num)
		set_current_material(data->material.num - 1);
	//data->DebugShow();
	//msg_write(o->getName() + " - " + message);


	fill_selection_buffer(data->mesh->vertex);
}

void ModeModelMesh::on_view_stage_change()
{
	//msg_write("mesh: on view stage change");
	update_vertex_buffers(data->mesh->vertex);
}

void ModeModelMesh::on_selection_change()
{
	//msg_write("mesh: on sel change");
	selection_mode->update_selection();
	fill_selection_buffer(data->mesh->vertex);
}

void ModeModelMesh::on_set_multi_view()
{
	//msg_write("mesh: on set mv");
	selection_mode->update_multi_view();
	update_vertex_buffers(data->mesh->vertex);
}



void ModeModelMesh::on_update_menu()
{
	ed->enable("copy", copyable());
	ed->enable("paste", pasteable());
	ed->enable("delete", copyable());
	string cm_name = ed->cur_mode->name;
	ed->check("new_point", cm_name == "ModelMeshCreateVertex");
	ed->check("new_tria", cm_name == "ModelMeshCreatePolygon");
	ed->check("new_plane", cm_name == "ModelMeshCreatePlane");
	ed->check("new_cube", cm_name == "ModelMeshCreateCube");
	ed->check("new_ball", cm_name == "ModelMeshCreateBall");
	ed->check("new_cylinder", cm_name == "ModelMeshCreateCylinder");
	ed->check("new_torus", cm_name == "ModelMeshCreateTorus");

	ed->check("select_cw", select_cw);
	ed->check("snap_to_grid", multi_view->snap_to_grid);

	ed->enable("select", multi_view->allow_mouse_actions);
	ed->enable("translate", multi_view->allow_mouse_actions);
	ed->enable("rotate", multi_view->allow_mouse_actions);
	ed->enable("scale", multi_view->allow_mouse_actions);
	ed->enable("mirror", multi_view->allow_mouse_actions);
	ed->enable("lock_action", multi_view->allow_mouse_actions);
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	ed->check("scale", mouse_action == MultiView::ACTION_SCALE);
	ed->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
	ed->check("lock_action", lock_action);
}

bool ModeModelMesh::optimize_view() {
	auto *mv = multi_view;
	bool ww = mv->whole_window;
	mv->reset_view();
	mv->whole_window = ww;
	vector min, max;
	data->mesh->get_bounding_box(min, max);
	if (min != max)
		mv->set_view_box(min, max);

	ed->multi_view_2d->reset_view();
	ed->multi_view_2d->cam.pos = vector(0.5f, 0.5f, 0);
	/*if ((Bone.num > 0) and (Vertex.num <= 0))
		SetSubMode(SubModeSkeleton);
	if (SubMode == SubModeSkeleton)
		SkeletonOptimizeView();*/
	return true;
}



void ModeModelMesh::create_new_material_for_selection()
{
#if 0
	msg_db_f("CreateNewMaterialForSelection", 2);
	if (0 == data->getNumSelectedPolygons()){
		ed->set_message(_("no triangle selected"));
		return;
	}

	//StartChanging();

	data->material.resize(data->material.num + 1);
	data->material.num --; // stupid...
	data->material[data->material.num].reset();
	//Material[Material.num] = cur_mat;
	int cmi = data->current_material;
	data->current_material = data->material.num;

	if (mode_model->ExecuteMaterialDialog(0)){//, true)){
		data->material.num ++;

		data->current_texture_level = 0;

		// create new subs
		for (int i=0;i<4;i++){
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

void ModeModelMesh::choose_material_for_selection()
{
	if (data->get_selection().polygon.num == 0){
		ed->set_message(_("no polygon selected"));
		return;
	}

	int SelectionDialogReturnIndex;


	// dialog
	ModelMaterialSelectionDialog *dlg = new ModelMaterialSelectionDialog(ed, false, data);
	dlg->put_answer(&SelectionDialogReturnIndex);
	dlg->run();
	delete dlg;

	if (SelectionDialogReturnIndex >= 0)
		data->setMaterialSelection(SelectionDialogReturnIndex);
}

void ModeModelMesh::choose_mouse_function(int f, bool _lock_action)
{
	mouse_action = f;
	lock_action = _lock_action;
	apply_mouse_function(ed->multi_view_3d);
	apply_mouse_function(ed->multi_view_2d);
}

void ModeModelMesh::apply_mouse_function(MultiView::MultiView *mv)
{
	if (!mv)
		return;

	string name = "ActionModelTransformVertices";
	if (!mv->mode3d)
		name = "ActionModelTransformSkinVertices";

	mv->set_mouse_action(name, mouse_action, lock_action);
}

void ModeModelMesh::copy()
{
	data->copyGeometry(temp_geo);

	on_update_menu();
	ed->set_message(format(_("%d vertices, %d triangles copied"), temp_geo.vertex.num, temp_geo.polygon.num));
}

void ModeModelMesh::paste()
{
	data->pasteGeometry(temp_geo, current_material);
	ed->set_message(format(_("%d vertices, %d triangles pasted"), temp_geo.vertex.num, temp_geo.polygon.num));
}

bool ModeModelMesh::copyable() {
	auto s = data->get_selection();
	return s.vertex.num > 0;
}

void ModeModelMesh::add_effects(int type)
{
	if (data->get_selection().vertex.num == 0){
		ed->set_message(_("No vertex point selected"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, type, -1);
	dlg->run();
	delete dlg;
}

void ModeModelMesh::edit_effects()
{
	int index;
	int n = 0;
	foreachi(ModelEffect &fx, data->fx, i)
		if (data->mesh->vertex[fx.vertex].is_selected){
			index = i;
			n ++;
		}
	if (n != 1){
		ed->set_message(_("One vertex point with effects has to be selected!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, -1, index);
	dlg->run();
	delete dlg;
}

void ModeModelMesh::clear_effects()
{
	int n = 0;
	for (ModelEffect &fx: data->fx)
		if (data->mesh->vertex[fx.vertex].is_selected)
			n ++;
	if (n == 0){
		ed->set_message(_("No vertex with effects selected!"));
		return;
	}
	data->selectionClearEffects();
}

bool ModeModelMesh::pasteable()
{
	return temp_geo.vertex.num > 0;
}

void ModeModelMesh::easify()
{
	ModelEasifyDialog *dlg = new ModelEasifyDialog(ed, false, data);
	dlg->run();
	delete dlg;
}

void ModeModelMesh::set_current_material(int index)
{
	if (current_material == index)
		return;
	current_material = index;
	notify(MESSAGE_CURRENT_MATERIAL_CHANGE);
	mode_model_mesh_texture->setCurrentTextureLevel(0);
}

void ModeModelMesh::draw_effects(MultiView::Window *win)
{
	for (ModelEffect &fx: data->fx){
		vector p = win->project(data->mesh->vertex[fx.vertex].pos);
		if ((p.z > 0) and (p.z < 1))
			draw_str(p.x, p.y, fx.get_type());
	}
}

void _draw_edges(DataModel *data, MultiView::Window *win, Array<ModelVertex> &vertex, bool selection_filter)
{
	color bg = win->getBackgroundColor();
	auto *multi_view = win->multi_view;

	nix::SetWire(false);
	set_wide_lines(selection_filter ? 2.0f : 1.0f);
	Array<vector> line_pos;
	Array<color> line_color;

	vector dir = win->getDirection();
	for (auto &e: data->mesh->edge){
		if (e.is_selected != selection_filter)
			continue;
		if (min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage) < multi_view->view_stage)
			continue;
		float w = min(data->mesh->polygon[e.polygon[0]].temp_normal * dir, data->mesh->polygon[e.polygon[1]].temp_normal * dir);
		float f = 0.5f - 0.4f*w;//0.7f - 0.3f * w;
		color cc;
		if (e.is_selected){
			cc = color(1, f, 0, 0);
		}else{
			cc = ColorInterpolate(scheme.TEXT, bg, 1-f);
		}
		line_color.add(cc);
		line_color.add(cc);
		//nix::DrawLine3D(vertex[e.vertex[0]].pos, vertex[e.vertex[1]].pos);
		line_pos.add(vertex[e.vertex[0]].pos);
		line_pos.add(vertex[e.vertex[1]].pos);
	}
	nix::DrawLinesColored(line_pos, line_color, false);
	nix::SetColor(White);
	nix::SetWire(win->multi_view->wire_mode);
}

void ModeModelMesh::draw_edges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected)
{
	if (!only_selected)
		_draw_edges(data, win, vertex, false);
	_draw_edges(data, win, vertex, true);
}


void ModeModelMesh::draw_polygons(MultiView::Window *win, Array<ModelVertex> &vertex)
{
	if (multi_view->wire_mode){
		draw_edges(win, vertex, false);
		return;
	}

	// draw all materials separately
	foreachi(ModelMaterial *m, data->material, mi){
		if (!m->vb)
			continue;

		// draw
		m->applyForRendering();
		nix::SetOffset(1.0f);
		nix::Draw3D(m->vb);
		nix::SetOffset(0);
		//nix::SetShader(NULL);
		//nix::SetTexture(NULL);
	}
}


void ModeModelMesh::draw_physical(MultiView::Window *win)
{
	nix::SetWire(false);
	mode_model->set_material_creation(0.3f);

	for (auto &b: data->phys_mesh->ball){
		Geometry *geo = new GeometrySphere(data->phys_mesh->vertex[b.index].pos, b.radius, 6);

		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);

		delete geo;
	}

	for (auto &c: data->phys_mesh->cylinder){
		Geometry *geo = new GeometryCylinder(data->phys_mesh->vertex[c.index[0]].pos, data->skin[0].vertex[c.index[1]].pos, c.radius, 1, 24, c.round ? GeometryCylinder::END_ROUND : GeometryCylinder::END_FLAT);

		geo->build(nix::vb_temp);
		nix::Draw3D(nix::vb_temp);

		delete geo;
	}

	nix::SetWire(multi_view->wire_mode);
}



void ModeModelMesh::update_vertex_buffers(Array<ModelVertex> &vertex)
{
	//msg_write("update vertex buffers!!!!!!!!!!");
	// draw all materials separately
	foreachi(ModelMaterial *m, data->material, mi){
		int num_tex = m->texture_levels.num;
		if (!m->vb)
			m->vb = new nix::VertexBuffer(num_tex);
		if (m->vb->num_textures != num_tex){
			delete(m->vb);
			m->vb = new nix::VertexBuffer(num_tex);
		}

		m->vb->clear();

		for (ModelPolygon &t: data->mesh->polygon)
			if ((t.view_stage >= multi_view->view_stage) and (t.material == mi))
				t.addToVertexBuffer(vertex, m->vb, m->texture_levels.num);

		//m.vb->optimize();
	}
}



void ModeModelMesh::fill_selection_buffer(Array<ModelVertex> &vertex)
{
	vb_marked->clear();

	// create selection buffers
	for (ModelPolygon &t: data->mesh->polygon)
		/*if (t.view_stage >= ViewStage)*/{
		if (t.is_selected)
			t.addToVertexBuffer(vertex, vb_marked, 1);
	}
}

void ModeModelMesh::draw_selection(MultiView::Window *win)
{
	nix::SetWire(false);
	nix::SetZ(true,true);
	nix::SetAlpha(ALPHA_NONE);

	nix::SetOffset(1.0f);
	ModeModel::set_material_selected();
	nix::Draw3D(vb_marked);
	ModeModel::set_material_creation();
	nix::Draw3D(vb_creation);
	nix::SetMaterial(White,White,Black,0,Black);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetOffset(0);
}

void ModeModelMesh::set_selection_mode(MeshSelectionMode *mode)
{
	if (selection_mode)
		selection_mode->on_end();
	selection_mode = mode;
	mode->on_start();
	mode->update_multi_view();
	notify();
	multi_view->force_redraw();
	ed->update_menu(); // TODO
}

void ModeModelMesh::toggle_select_cw()
{
	select_cw = !select_cw;
	notify();
	ed->update_menu();
}

void ModeModelMesh::set_allow_draw_hover(bool allow) {
	allow_draw_hover = allow;
}
