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
#include "Creation/ModeModelMeshDeform.h"
#include "Creation/ModeModelMeshExtrudePolygons.h"
#include "../../../Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"
#include "../Dialog/ModelMaterialDialog.h"
#include "../Dialog/ModelEasifyDialog.h"
#include "../Dialog/ModelFXDialog.h"
#include "MeshSelectionModeEdge.h"
#include "MeshSelectionModePolygon.h"
#include "MeshSelectionModeSurface.h"
#include "MeshSelectionModeVertex.h"
#include "ModeModelMeshTexture.h"

#include <GL/gl.h>

namespace MultiView
{
	extern color ColorText;
	extern color ColorBackGround2D;
};

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(ModeBase *_parent) :
	Mode<DataModel>("ModelMesh", _parent, ed->multi_view_3d, "menu_model"),
	Observable("ModelMesh")
{
	Observer::subscribe(data);

	selection_mode = NULL;
	material_dialog = NULL;
	current_material = 0;

	// vertex buffers
	vb_marked = new NixVertexBuffer(1);
	vb_model = new NixVertexBuffer(1);
	vb_model2 = NULL;
	vb_model3 = NULL;
	vb_model4 = NULL;
	vb_hover = new NixVertexBuffer(1);
	vb_creation = new NixVertexBuffer(1);

	select_cw = false;

	chooseMouseFunction(MultiView::ActionSelect);

	selection_mode_vertex = new MeshSelectionModeVertex(this);
	selection_mode_edge = new MeshSelectionModeEdge(this);
	selection_mode_polygon = new MeshSelectionModePolygon(this);
	selection_mode_surface = new MeshSelectionModeSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this);

	selection_mode = selection_mode_vertex;
}

ModeModelMesh::~ModeModelMesh()
{
	Observer::unsubscribe(data);
}

void ModeModelMesh::onStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
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
	t->enable(true);
	t->configure(false,true);


	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);

	setSelectionMode(selection_mode_polygon);
}

void ModeModelMesh::onEnter()
{
	current_material = 0;
}

void ModeModelMesh::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);

	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->enable(false);

	closeMaterialDialog();
}



void ModeModelMesh::onCommand(const string & id)
{
	if (id == "delete")
		data->DeleteSelection(selection_mode == selection_mode_vertex);
	if (id == "copy")
		copy();
	if (id == "paste")
		paste();

	if (id == "select_cw")
		toggleSelectCW();

	if (id == "volume_subtract")
		data->SubtractSelection();
	if (id == "volume_and")
		data->AndSelection();
	if (id == "invert_trias")
		data->InvertSelection();
	if (id == "extrude_triangles")
		ed->setMode(new ModeModelMeshExtrudePolygons(ed->cur_mode));
	if (id == "autoweld_surfaces")
		ed->setMode(new ModeModelMeshAutoweld(ed->cur_mode));
	if (id == "convert_to_triangles")
		data->ConvertSelectionToTriangles();
	if (id == "untriangulate_selection")
		data->MergePolygonsSelection();
	if (id == "cut_out")
		data->CutOutSelection();
	if (id == "nearify")
		data->NearifySelectedVertices();
	if (id == "connect")
		data->CollapseSelectedVertices();
	if (id == "align_to_grid")
		data->AlignToGridSelection(multi_view->GetGridD());
	if (id == "triangulate_selection")
		data->TriangulateSelectedVertices();
	if (id == "subdivide_surfaces")
		data->SubdivideSelectedSurfaces();

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
		ed->setMode(new ModeModelMeshDeform(this));
	if (id == "flatten_vertices")
		data->FlattenSelectedVertices();

	if (id == "select")
		chooseMouseFunction(MultiView::ActionSelect);
	if (id == "translate")
		chooseMouseFunction(MultiView::ActionMove);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ActionRotate);
	if (id == "scale")
		chooseMouseFunction(MultiView::ActionScale);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ActionMirror);

	if (id == "create_new_material")
		createNewMaterialForSelection();
	if (id == "choose_material")
		chooseMaterialForSelection();
	if (id == "mode_model_materials")
		toggleMaterialDialog();
	if (id == "text_from_bg")
		data->execute(new ActionModelSkinVerticesFromProjection(data, multi_view));
	if (id == "automapping")
		data->Automap(current_material, mode_model_mesh_texture->current_texture_level);
	if (id == "easify_skin")
		easify();

	if (id == "normal_this_smooth")
		data->SetNormalModeSelection(NORMAL_MODE_SMOOTH);
	if (id == "normal_this_hard")
		data->SetNormalModeSelection(NORMAL_MODE_HARD);
	if (id == "normal_this_angular")
		data->SetNormalModeSelection(NORMAL_MODE_ANGULAR);

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
		material_dialog = new ModelMaterialDialog(ed, data);
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
	fillSelectionBuffers(data->vertex);

	if (data->GetNumSelectedVertices() > 0){
		ed->drawStr(20, 100, format(_("vert: %d"), data->GetNumSelectedVertices()));
		ed->drawStr(20, 120, format(_("poly: %d"), data->GetNumSelectedPolygons()));
		ed->drawStr(20, 140, format(_("surf: %d"), data->GetNumSelectedSurfaces()));
	}
}


void ModeModelMesh::onDrawWin(MultiView::Window *win)
{
	if (multi_view->wire_mode)
		drawEdges(win, data->vertex, false);
	else
		drawPolygons(win, data->vertex);
	mode_model_skeleton->drawSkeleton(win, data->bone, true);
	drawSelection(win);

	selection_mode->onDrawWin(win);
}



void ModeModelMesh::onUpdate(Observable *o, const string &message)
{
	// consistency checks
	if (current_material >= data->material.num)
		setCurrentMaterial(data->material.num - 1);
	//data->DebugShow();

	if (o == data){
		selection_mode->updateMultiView();
	}else if (o == multi_view){
		selection_mode->updateSelection();
	}

	fillSelectionBuffers(data->vertex);
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
	ed->check("select", mouse_action == MultiView::ActionSelect);
	ed->check("translate", mouse_action == MultiView::ActionMove);
	ed->check("rotate", mouse_action == MultiView::ActionRotate);
	ed->check("scale", mouse_action == MultiView::ActionScale);
	ed->check("mirror", mouse_action == MultiView::ActionMirror);

	ed->check("mode_model_materials", material_dialog);
}

bool ModeModelMesh::optimizeView()
{
	msg_db_f("OptimizeView", 1);
	MultiView::MultiView *mv = multi_view;
	bool ww = mv->whole_window;
	mv->ResetView();
	mv->whole_window = ww;
	if (data->vertex.num > 0){
		vector min = data->vertex[0].pos, max = data->vertex[0].pos;
		foreach(ModelVertex &v, data->vertex){
			min._min(v.pos);
			max._max(v.pos);
		}
		mv->SetViewBox(min, max);
	}

	ed->multi_view_2d->ResetView();
	ed->multi_view_2d->cam.pos = vector(0.5f, 0.5f, 0);
	/*if ((Bone.num > 0) && (Vertex.num <= 0))
		SetSubMode(SubModeSkeleton);
	if (SubMode == SubModeSkeleton)
		SkeletonOptimizeView();*/
	return true;
}



void ModeModelMesh::createNewMaterialForSelection()
{
#if 0
	msg_db_f("CreateNewMaterialForSelection", 2);
	if (0 == data->GetNumSelectedPolygons()){
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
	msg_db_f("ChooseMaterialForSelection", 2);
	if (0 == data->GetNumSelectedPolygons()){
		ed->setMessage(_("kein Dreieck ausgew&ahlt"));
		return;
	}

	int SelectionDialogReturnIndex;


	// dialog
	ModelMaterialSelectionDialog *dlg = new ModelMaterialSelectionDialog(ed, false, data);
	dlg->PutAnswer(&SelectionDialogReturnIndex);
	dlg->run();

	if (SelectionDialogReturnIndex >= 0)
		data->SetMaterialSelection(SelectionDialogReturnIndex);
}

void ModeModelMesh::chooseMouseFunction(int f)
{
	mouse_action = f;
	ed->updateMenu();
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

	// left mouse action
	if (mouse_action != MultiView::ActionSelect){
		mv->SetMouseAction(name, mouse_action);
	}else{
		mv->SetMouseAction("", MultiView::ActionSelect);
	}
}

void ModeModelMesh::copy()
{
	data->CopyGeometry(temp_geo);

	onUpdateMenu();
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke kopiert"), temp_geo.vertex.num, temp_geo.polygon.num));
}

void ModeModelMesh::paste()
{
	data->PasteGeometry(temp_geo, current_material);
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), temp_geo.vertex.num, temp_geo.polygon.num));
}

bool ModeModelMesh::copyable()
{
	return data->GetNumSelectedVertices() > 0;
}

void ModeModelMesh::addEffects(int type)
{
	if (data->GetNumSelectedVertices() == 0){
		ed->setMessage(_("Kein Punkt markiert!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, type, -1);
	dlg->run();
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
}

void ModeModelMesh::clearEffects()
{
	int n = 0;
	foreach(ModelEffect &fx, data->fx)
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
	NixEnableLighting(false);
	foreach(ModelEffect &fx, data->fx){
		vector p = win->project(data->vertex[fx.vertex].pos);
		if ((p.z > 0) && (p.z < 1))
			ed->drawStr(p.x, p.y, fx.get_type());
	}
	NixEnableLighting(multi_view->light_enabled);
}


void ModeModelMesh::drawEdges(MultiView::Window *win, Array<ModelVertex> &vertex, bool only_selected)
{
	NixSetWire(false);
	NixEnableLighting(false);
	vector dir = win->getDirection();
	foreach(ModelSurface &s, data->surface){
		foreach(ModelEdge &e, s.edge){
			if (min(vertex[e.vertex[0]].view_stage, vertex[e.vertex[1]].view_stage) < multi_view->view_stage)
				continue;
			if (!e.is_selected && only_selected)
				continue;
			float w = max(s.polygon[e.polygon[0]].temp_normal * dir, s.polygon[e.polygon[1]].temp_normal * dir);
			float f = 0.5f - 0.4f*w;//0.7f - 0.3f * w;
			if (e.is_selected)
				NixSetColor(color(1, f, 0, 0));
			else
				NixSetColor(f * MultiView::ColorText + (1 - f) * MultiView::ColorBackGround2D);
			NixDrawLine3D(vertex[e.vertex[0]].pos, vertex[e.vertex[1]].pos);
		}
	}
	NixSetColor(White);
	NixSetWire(win->multi_view->wire_mode);
	NixEnableLighting(multi_view->light_enabled);
}


void ModeModelMesh::drawPolygons(MultiView::Window *win, Array<ModelVertex> &vertex)
{
	msg_db_f("ModelSkin.DrawPolys",2);

	if (multi_view->wire_mode){
		drawEdges(win, vertex, false);
		return;
	}

	// draw all materials separately
	foreachi(ModelMaterial &m, data->material, mi){
		NixVertexBuffer **vb = &vb_model;
		int num_tex = min(m.num_textures, 4);
		if (num_tex == 2)
			vb = &vb_model2;
		else if (num_tex == 3)
			vb = &vb_model3;
		else if (num_tex == 4)
			vb = &vb_model4;
		if (!*vb)
			*vb = new NixVertexBuffer(num_tex);

		(*vb)->clear();

		foreach(ModelSurface &surf, data->surface){
			if (!surf.is_visible)
				continue;
			foreach(ModelPolygon &t, surf.polygon)
				if ((t.view_stage >= multi_view->view_stage) && (t.material == mi))
					t.AddToVertexBuffer(vertex, *vb, m.num_textures);
		}

		// draw
		m.ApplyForRendering();
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(1.0f, 1.0f);
		NixDraw3D(*vb);
		glDisable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(0, 0);
		NixSetShader(NULL);
		NixSetTexture(NULL);
	}

	drawEdges(win, vertex, true);
}



void ModeModelMesh::fillSelectionBuffers(Array<ModelVertex> &vertex)
{
	msg_db_f("SkinFillSelBuf", 4);
	vb_marked->clear();
	vb_hover->clear();

	// create selection buffers
	msg_db_m("a",4);
	ModelPolygon *mmo = NULL;
	if ((multi_view->hover.index >= 0) && (multi_view->hover.set < data->surface.num) && (multi_view->hover.type == MVDModelPolygon))
		mmo = &data->surface[multi_view->hover.set].polygon[multi_view->hover.index];
	foreachi(ModelSurface &s, data->surface, si){
		bool s_mo = false;
		if ((multi_view->hover.index >= 0) && (multi_view->hover.type == MVDModelSurface))
			s_mo = (multi_view->hover.index == si);
		foreach(ModelPolygon &t, s.polygon)
			/*if (t.view_stage >= ViewStage)*/{
			if (t.is_selected)
				t.AddToVertexBuffer(vertex, vb_marked, 1);
			if ((&t == mmo) || (s_mo))
				t.AddToVertexBuffer(vertex, vb_hover, 1);
		}
	}
}

void ModeModelMesh::setMaterialMarked()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
}

void ModeModelMesh::setMaterialMouseOver()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,White);
}

void ModeModelMesh::setMaterialCreation()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0.3f,1,0.3f),Black,0,color(1,0.1f,0.4f,0.1f));
}

void ModeModelMesh::drawSelection(MultiView::Window *win)
{
	NixSetWire(false);
	NixSetZ(true,true);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(true);

	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(1.0f, 1.0f);
	setMaterialMarked();
	NixDraw3D(vb_marked);
	setMaterialMouseOver();
	NixDraw3D(vb_hover);
	setMaterialCreation();
	NixDraw3D(vb_creation);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(0, 0);
}

void ModeModelMesh::setSelectionMode(MeshSelectionMode *mode)
{
	if ((!this->isAncestorOf(ed->cur_mode)) || (mode_model_mesh_texture->isAncestorOf(ed->cur_mode)))
		ed->setMode(this);
	if (selection_mode)
		selection_mode->onEnd();
	selection_mode = mode;
	mode->onStart();
	mode->updateMultiView();
	fillSelectionBuffers(data->vertex);
	ed->updateMenu();
}

void ModeModelMesh::toggleSelectCW()
{
	select_cw = !select_cw;
	ed->updateMenu();
}
