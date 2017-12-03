/*
 * ModeModelMesh.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "../ModeModel.h"
#include "ModeModelMesh.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../../ModeCreation.h"
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
#include "Creation/ModeModelMeshBrush.h"
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

ModeModelMesh *mode_model_mesh = NULL;


namespace MultiView{
	void set_wide_lines(float width);
}

ModeModelMesh::ModeModelMesh(ModeBase *_parent) :
	Mode<DataModel>("ModelMesh", _parent, ed->multi_view_3d, "menu_model"),
	Observable("ModelMesh")
{

	selection_mode = NULL;
	material_dialog = NULL;
	current_material = 0;

	// vertex buffers
	vb_marked = new nix::VertexBuffer(1);
	vb_hover = new nix::VertexBuffer(1);
	vb_creation = new nix::VertexBuffer(1);

	select_cw = false;

	chooseMouseFunction(MultiView::ACTION_MOVE, false);

	selection_mode_vertex = new MeshSelectionModeVertex(this);
	selection_mode_edge = new MeshSelectionModeEdge(this);
	selection_mode_polygon = new MeshSelectionModePolygon(this);
	selection_mode_surface = new MeshSelectionModeSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this);

	selection_mode = selection_mode_polygon;

	Observer::subscribe(data);
	//Observer::subscribe(multi_view);
}

ModeModelMesh::~ModeModelMesh()
{
	Observer::unsubscribe(data);
}

void ModeModelMesh::onStart()
{
	string dir = app->directory_static + "icons/toolbar/";
	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->addItemCheckable(_("Polygon"),dir + "new_triangle.png", "new_tria");
	t->addItemCheckable(_("Ebene"),dir + "new_plane.png", "new_plane");
	t->addItemCheckable(_("Quader"),dir + "mode_skin.png", "new_cube");
	t->addItemCheckable(_("Kugel"), dir + "new_ball.png", "new_ball");
	t->addItemCheckable(_("Zylinder"), dir + "new_cylinder.png", "new_cylinder");
	t->addSeparator();
	t->addItemCheckable(_("Materialien"), dir + "mode_textures.png", "mode_model_materials");
	t->addSeparator();
	t->addItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->addItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->addItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->addItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
	t->addItemCheckable(_("Spiegeln"),dir + "rf_mirror.png", "mirror");
	t->addItemCheckable(_("paranoider Modus"), "hui:no", "lock_action");
	t->enable(true);
	t->configure(false,true);


	//subscribe(data);

	updateVertexBuffers(data->vertex);

	setSelectionMode(selection_mode);
	mode_model->allowSelectionModes(true);
}

void ModeModelMesh::onEnter()
{
	current_material = 0;
}

void ModeModelMesh::onEnd()
{
	//unsubscribe(data);

	hui::Toolbar *t = ed->toolbar[hui::TOOLBAR_LEFT];
	t->reset();
	t->enable(false);

	closeMaterialDialog();
}



void ModeModelMesh::onCommand(const string & id)
{
	if (id == "delete")
		data->deleteSelection(selection_mode == selection_mode_vertex);
	if (id == "copy")
		copy();
	if (id == "paste")
		ed->setMode(new ModeModelMeshPaste(this));
		//paste();

	if (id == "select_cw")
		toggleSelectCW();

	if (id == "volume_subtract")
		data->subtractSelection();
	if (id == "volume_and")
		data->andSelection();
	if (id == "invert_trias")
		data->invertSelection();
	if (id == "extrude_triangles")
		ed->setMode(new ModeModelMeshExtrudePolygons(this));
	if (id == "autoweld_surfaces")
		ed->setMode(new ModeModelMeshAutoweld(this));
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
		data->subdivideSelectedSurfaces();

	if (id == "new_point")
		ed->setMode(new ModeModelMeshCreateVertex(this));
	if (id == "new_tria")
		ed->setMode(new ModeModelMeshCreatePolygon(this));
	if (id == "new_ball")
		ed->setMode(new ModeModelMeshCreateBall(this));
	if (id == "new_cube")
		ed->setMode(new ModeModelMeshCreateCube(this));
	if (id == "new_cylinder")
		ed->setMode(new ModeModelMeshCreateCylinder(this));
	if (id == "new_cylindersnake")
		ed->setMode(new ModeModelMeshCreateCylinderSnake(this));
	if (id == "new_plane")
		ed->setMode(new ModeModelMeshCreatePlane(this));
	if (id == "new_torus")
		ed->setMode(new ModeModelMeshCreateTorus(this));
	if (id == "new_tetrahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(this, 4));
	if (id == "new_octahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(this, 8));
	if (id == "new_dodecahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(this, 12));
	if (id == "new_icosahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(this, 20));
	if (id == "new_teapot")
		ed->setMode(new ModeModelMeshCreatePlatonic(this, 306));
	if (id == "new_extract")
		ed->setMode(new ModeModelMeshSplitPolygon(this));
	if (id == "bevel_edges")
		ed->setMode(new ModeModelMeshBevelEdges(this));
	if (id == "deformation_brush")
		ed->setMode(new ModeModelMeshBrush(this));
	if (id == "deformation_function")
		ed->setMode(new ModeModelMeshDeformFunction(this));
	if (id == "deformation_cylinder")
		ed->setMode(new ModeModelMeshDeformCylinder(this));
	if (id == "flatten_vertices")
		data->flattenSelectedVertices();

	if (id == "select")
		chooseMouseFunction(MultiView::ACTION_SELECT, lock_action);
	if (id == "translate")
		chooseMouseFunction(MultiView::ACTION_MOVE, lock_action);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ACTION_ROTATE, lock_action);
	if (id == "scale")
		chooseMouseFunction(MultiView::ACTION_SCALE, lock_action);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ACTION_MIRROR, lock_action);
	if (id == "lock_action")
		chooseMouseFunction(mouse_action, !lock_action);

	if (id == "create_new_material")
		createNewMaterialForSelection();
	if (id == "choose_material")
		chooseMaterialForSelection();
	if (id == "mode_model_materials")
		toggleMaterialDialog();
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
		addEffects(FX_TYPE_LIGHT);
	if (id == "fx_new_sound")
		addEffects(FX_TYPE_SOUND);
	if (id == "fx_new_script")
		addEffects(FX_TYPE_SCRIPT);
	if (id == "fx_new_field")
		addEffects(FX_TYPE_FORCEFIELD);
	if (id == "fx_none")
		clearEffects();
	if (id == "fx_edit")
		editEffects();
}

void ModeModelMesh::showMaterialDialog()
{
	if (!material_dialog){
		material_dialog = new ModelMaterialDialog(data);

		ed->embed(material_dialog, "root-table", 1, 0);
		ed->check("mode_model_materials", true);
	}
}

void ModeModelMesh::closeMaterialDialog()
{
	if (material_dialog){
		delete(material_dialog);
		material_dialog = NULL;
		ed->check("mode_model_materials", false);
	}
}

void ModeModelMesh::toggleMaterialDialog()
{
	if (material_dialog)
		closeMaterialDialog();
	else
		showMaterialDialog();
}



void ModeModelMesh::onDraw()
{
	if (data->getNumSelectedVertices() > 0){
		int nv = data->getNumSelectedVertices();
		int ne = data->getNumSelectedEdges();
		int np = data->getNumSelectedPolygons();
		int ns = data->getNumSelectedSurfaces();
		ed->drawStr(10, nix::target_height - 25, format("selected: %d vertices, %d edges, %d polygons, %d surfaces", nv, ne, np, ns));
	}
}


void ModeModelMesh::onDrawWin(MultiView::Window *win)
{
	drawAll(win, data->vertex);
}

void ModeModelMesh::drawAll(MultiView::Window *win, Array<ModelVertex> &vertex)
{
	drawPolygons(win, vertex);
	mode_model_skeleton->drawSkeleton(win, data->bone, true);
	drawSelection(win);

	drawEdges(win, vertex, !selection_mode_edge->isActive());

	selection_mode->onDrawWin(win);
}



void ModeModelMesh::onUpdate(Observable *o, const string &message)
{
	// consistency checks
	if (current_material >= data->material.num)
		setCurrentMaterial(data->material.num - 1);
	//data->DebugShow();
	//msg_write(o->getName() + " - " + message);


	fillSelectionBuffer(data->vertex);
}

void ModeModelMesh::onViewStageChange()
{
	//msg_write("on view stage change");
	updateVertexBuffers(data->vertex);
}

void ModeModelMesh::onSelectionChange()
{
	//msg_write("on sel change");
	selection_mode->updateSelection();
	fillSelectionBuffer(data->vertex);
}

void ModeModelMesh::onSetMultiView()
{
	//msg_write("on set mv");
	selection_mode->updateMultiView();
	updateVertexBuffers(data->vertex);
}



void ModeModelMesh::onUpdateMenu()
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

	ed->check("mode_model_materials", material_dialog);
}

bool ModeModelMesh::optimizeView()
{
	MultiView::MultiView *mv = multi_view;
	bool ww = mv->whole_window;
	mv->resetView();
	mv->whole_window = ww;
	if (data->vertex.num > 0){
		vector min = data->vertex[0].pos, max = data->vertex[0].pos;
		for (ModelVertex &v: data->vertex){
			min._min(v.pos);
			max._max(v.pos);
		}
		mv->setViewBox(min, max);
	}

	ed->multi_view_2d->resetView();
	ed->multi_view_2d->cam.pos = vector(0.5f, 0.5f, 0);
	/*if ((Bone.num > 0) and (Vertex.num <= 0))
		SetSubMode(SubModeSkeleton);
	if (SubMode == SubModeSkeleton)
		SkeletonOptimizeView();*/
	return true;
}



void ModeModelMesh::createNewMaterialForSelection()
{
#if 0
	msg_db_f("CreateNewMaterialForSelection", 2);
	if (0 == data->getNumSelectedPolygons()){
		ed->setMessage(_("kein Dreieck ausgew&ahlt"));
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

void ModeModelMesh::chooseMaterialForSelection()
{
	if (0 == data->getNumSelectedPolygons()){
		ed->setMessage(_("kein Dreieck ausgew&ahlt"));
		return;
	}

	int SelectionDialogReturnIndex;


	// dialog
	ModelMaterialSelectionDialog *dlg = new ModelMaterialSelectionDialog(ed, false, data);
	dlg->PutAnswer(&SelectionDialogReturnIndex);
	dlg->run();
	delete dlg;

	if (SelectionDialogReturnIndex >= 0)
		data->setMaterialSelection(SelectionDialogReturnIndex);
}

void ModeModelMesh::chooseMouseFunction(int f, bool _lock_action)
{
	mouse_action = f;
	lock_action = _lock_action;
	applyMouseFunction(ed->multi_view_3d);
	applyMouseFunction(ed->multi_view_2d);
}

void ModeModelMesh::applyMouseFunction(MultiView::MultiView *mv)
{
	if (!mv)
		return;

	string name = "ActionModelTransformVertices";
	if (!mv->mode3d)
		name = "ActionModelTransformSkinVertices";

	mv->setMouseAction(name, mouse_action, lock_action);
}

void ModeModelMesh::copy()
{
	data->copyGeometry(temp_geo);

	onUpdateMenu();
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke kopiert"), temp_geo.vertex.num, temp_geo.polygon.num));
}

void ModeModelMesh::paste()
{
	data->pasteGeometry(temp_geo, current_material);
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), temp_geo.vertex.num, temp_geo.polygon.num));
}

bool ModeModelMesh::copyable()
{
	return data->getNumSelectedVertices() > 0;
}

void ModeModelMesh::addEffects(int type)
{
	if (data->getNumSelectedVertices() == 0){
		ed->setMessage(_("Kein Punkt markiert!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, type, -1);
	dlg->run();
	delete dlg;
}

void ModeModelMesh::editEffects()
{
	int index;
	int n = 0;
	foreachi(ModelEffect &fx, data->fx, i)
		if (data->vertex[fx.vertex].is_selected){
			index = i;
			n ++;
		}
	if (n != 1){
		ed->setMessage(_("Es muss genau ein Punkt mit Effekt markiert sein!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, -1, index);
	dlg->run();
	delete dlg;
}

void ModeModelMesh::clearEffects()
{
	int n = 0;
	for (ModelEffect &fx: data->fx)
		if (data->vertex[fx.vertex].is_selected)
			n ++;
	if (n == 0){
		ed->setMessage(_("Kein Punkt mit Effekt markiert!"));
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

void ModeModelMesh::setCurrentMaterial(int index)
{
	if (current_material == index)
		return;
	current_material = index;
	notify();
	mode_model_mesh_texture->setCurrentTextureLevel(0);
}

void ModeModelMesh::drawEffects(MultiView::Window *win)
{
	nix::EnableLighting(false);
	for (ModelEffect &fx: data->fx){
		vector p = win->project(data->vertex[fx.vertex].pos);
		if ((p.z > 0) and (p.z < 1))
			ed->drawStr(p.x, p.y, fx.get_type());
	}
	nix::EnableLighting(multi_view->light_enabled);
}

void draw_edges(DataModel *data, MultiView::Window *win, Array<ModelVertex> &vertex, bool selection_filter)
{
	color bg = win->getBackgroundColor();
	auto *multi_view = win->multi_view;

	nix::SetWire(false);
	MultiView::set_wide_lines(selection_filter ? 2.0f : 1.0f);
	Array<vector> line_pos;
	Array<color> line_color;

	vector dir = win->getDirection();
	for (ModelSurface &s: data->surface){
		for (ModelEdge &e: s.edge){
			if (e.is_selected != selection_filter)
				continue;
			if (min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage) < multi_view->view_stage)
				continue;
			float w = min(s.polygon[e.polygon[0]].temp_normal * dir, s.polygon[e.polygon[1]].temp_normal * dir);
			float f = 0.5f - 0.4f*w;//0.7f - 0.3f * w;
			if (e.is_selected){
				//nix::SetColor(color(1, f, 0, 0));
				line_color.add(color(1, f, 0, 0));
				line_color.add(color(1, f, 0, 0));
			}else{
				//nix::SetColor(f * multi_view->ColorText + (1 - f) * bg);
				line_color.add(f * multi_view->ColorText + (1 - f) * bg);
				line_color.add(f * multi_view->ColorText + (1 - f) * bg);
			}
			//nix::DrawLine3D(vertex[e.vertex[0]].pos, vertex[e.vertex[1]].pos);
			line_pos.add(vertex[e.vertex[0]].pos);
			line_pos.add(vertex[e.vertex[1]].pos);
		}
	}
	nix::DrawLinesColored(line_pos, line_color, false);
	nix::SetColor(White);
	nix::SetWire(win->multi_view->wire_mode);
}

void ModeModelMesh::drawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected)
{
	if (!only_selected)
		draw_edges(data, win, vertex, false);
	draw_edges(data, win, vertex, true);
}


void ModeModelMesh::drawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex)
{
	if (multi_view->wire_mode){
		drawEdges(win, vertex, false);
		return;
	}

	// draw all materials separately
	foreachi(ModelMaterial &m, data->material, mi){
		if (!m.vb)
			continue;

		// draw
		m.applyForRendering();
		nix::SetOffset(1.0f);
		nix::Draw3D(m.vb);
		nix::SetOffset(0);
		//nix::SetShader(NULL);
		//nix::SetTexture(NULL);
	}
}



void ModeModelMesh::updateVertexBuffers(Array<ModelVertex> &vertex)
{
	//msg_write("update vertex buffers!!!!!!!!!!");
	// draw all materials separately
	foreachi(ModelMaterial &m, data->material, mi){
		int num_tex = min(m.textures.num, 4);
		if (!m.vb)
			m.vb = new nix::VertexBuffer(num_tex);
		if (m.vb->num_textures != num_tex){
			delete(m.vb);
			m.vb = new nix::VertexBuffer(num_tex);
		}

		m.vb->clear();

		for (ModelSurface &surf: data->surface){
			if (!surf.is_visible)
				continue;
			for (ModelPolygon &t: surf.polygon)
				if ((t.view_stage >= multi_view->view_stage) and (t.material == mi))
					t.addToVertexBuffer(vertex, m.vb, m.textures.num);
		}

		//m.vb->optimize();
	}
}



void ModeModelMesh::fillSelectionBuffer(Array<ModelVertex> &vertex)
{
	vb_marked->clear();

	// create selection buffers
	for (ModelSurface &s: data->surface){
		for (ModelPolygon &t: s.polygon)
			/*if (t.view_stage >= ViewStage)*/{
			if (t.is_selected)
				t.addToVertexBuffer(vertex, vb_marked, 1);
		}
	}
}

void ModeModelMesh::drawSelection(MultiView::Window *win)
{
	nix::SetWire(false);
	nix::SetZ(true,true);
	nix::SetAlpha(ALPHA_NONE);
	nix::EnableLighting(true);

	nix::SetOffset(1.0f);
	ModeModel::setMaterialSelected();
	nix::Draw3D(vb_marked);
	ModeModel::setMaterialCreation();
	nix::Draw3D(vb_creation);
	nix::SetMaterial(White,White,Black,0,Black);
	nix::SetAlpha(ALPHA_NONE);
	nix::SetOffset(0);
}

void ModeModelMesh::setSelectionMode(MeshSelectionMode *mode)
{
	if (selection_mode)
		selection_mode->onEnd();
	selection_mode = mode;
	mode->onStart();
	mode->updateMultiView();
	notify();
	ed->updateMenu(); // TODO
}

void ModeModelMesh::toggleSelectCW()
{
	select_cw = !select_cw;
	notify();
	ed->updateMenu();
}
