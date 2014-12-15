/*
 * ModeModelMesh.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../ModeModel.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshEdge.h"
#include "ModeModelMeshPolygon.h"
#include "ModeModelMeshSurface.h"
#include "ModeModelMeshTexture.h"
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
#include "../../../Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"
#include "../Dialog/ModelMaterialDialog.h"
#include "../Dialog/ModelEasifyDialog.h"
#include "../Dialog/ModelFXDialog.h"

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(ModeBase *_parent) :
	Mode<DataModel>("ModelMesh", _parent, ed->multi_view_3d, ""),
	Observable("ModelMesh")
{
	Observer::subscribe(data);

	material_dialog = NULL;
	current_material = 0;

	chooseMouseFunction(MultiView::ActionSelect);

	mode_model_mesh_vertex = new ModeModelMeshVertex(this);
	mode_model_mesh_edge = new ModeModelMeshEdge(this);
	mode_model_mesh_polygon = new ModeModelMeshPolygon(this);
	mode_model_mesh_surface = new ModeModelMeshSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this);
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
	t->addItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->addItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->addItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->addItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
	t->addItemCheckable(_("Spiegeln"),dir + "rf_mirror.png", "mirror");
	t->enable(true);
	t->configure(false,true);
}

void ModeModelMesh::onEnter()
{
	current_material = 0;

	ed->setMode(mode_model_mesh_vertex);
	//ed->setMode(mode_model_mesh_skin);
}

void ModeModelMesh::onEnd()
{
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->enable(false);

	CloseMaterialDialog();
}



void ModeModelMesh::onCommand(const string & id)
{
	if (id == "delete")
		data->DeleteSelection(ed->cur_mode == mode_model_mesh_vertex);
	if (id == "copy")
		Copy();
	if (id == "paste")
		Paste();

	if (id == "select_cw")
		mode_model_mesh_polygon->ToggleSelectCW();

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
		ed->setMode(new ModeModelMeshCreateVertex(mode_model_mesh_vertex));
	if (id == "new_tria")
		ed->setMode(new ModeModelMeshCreatePolygon(mode_model_mesh_vertex));
	if (id == "new_ball")
		ed->setMode(new ModeModelMeshCreateBall(ed->cur_mode));
	if (id == "new_cube")
		ed->setMode(new ModeModelMeshCreateCube(ed->cur_mode));
	if (id == "new_cylinder")
		ed->setMode(new ModeModelMeshCreateCylinder(ed->cur_mode));
	if (id == "new_cylindersnake")
		ed->setMode(new ModeModelMeshCreateCylinderSnake(ed->cur_mode));
	if (id == "new_plane")
		ed->setMode(new ModeModelMeshCreatePlane(ed->cur_mode));
	if (id == "new_torus")
		ed->setMode(new ModeModelMeshCreateTorus(ed->cur_mode));
	if (id == "new_tetrahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 4));
	if (id == "new_octahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 8));
	if (id == "new_dodecahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 12));
	if (id == "new_icosahedron")
		ed->setMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 20));
	if (id == "new_teapot")
		ed->setMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 306));
	if (id == "new_extract")
		ed->setMode(new ModeModelMeshSplitPolygon(mode_model_mesh_polygon));
	if (id == "bevel_edges")
		ed->setMode(new ModeModelMeshBevelEdges(mode_model_mesh_vertex));
	if (id == "deformation_brush")
		ed->setMode(new ModeModelMeshBrush(mode_model_mesh_polygon));
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
		CreateNewMaterialForSelection();
	if (id == "choose_material")
		ChooseMaterialForSelection();
	if (id == "mode_model_materials")
		ToggleMaterialDialog();
	if (id == "text_from_bg")
		data->execute(new ActionModelSkinVerticesFromProjection(data, multi_view));
	if (id == "automapping")
		data->Automap(current_material, mode_model_mesh_texture->current_texture_level);
	if (id == "easify_skin")
		Easify();

	if (id == "normal_this_smooth")
		data->SetNormalModeSelection(NormalModeSmooth);
	if (id == "normal_this_hard")
		data->SetNormalModeSelection(NormalModeHard);
	if (id == "normal_this_angular")
		data->SetNormalModeSelection(NormalModeAngular);

	if (id == "fx_new_light")
		AddEffects(FX_KIND_LIGHT);
	if (id == "fx_new_sound")
		AddEffects(FX_KIND_SOUND);
	if (id == "fx_new_script")
		AddEffects(FX_KIND_SCRIPT);
	if (id == "fx_new_field")
		AddEffects(FX_KIND_FORCEFIELD);
	if (id == "fx_none")
		ClearEffects();
	if (id == "fx_edit")
		EditEffects();
}

void ModeModelMesh::ShowMaterialDialog()
{
	if (!material_dialog){
		material_dialog = new ModelMaterialDialog(ed, data);
		ed->check("mode_model_materials", true);
	}
}

void ModeModelMesh::CloseMaterialDialog()
{
	if (material_dialog){
		delete(material_dialog);
		material_dialog = NULL;
		ed->check("mode_model_materials", false);
	}
}

void ModeModelMesh::ToggleMaterialDialog()
{
	if (material_dialog)
		CloseMaterialDialog();
	else
		ShowMaterialDialog();
}



void ModeModelMesh::onDraw()
{
	if (data->GetNumSelectedVertices() > 0){
		ed->drawStr(20, 100, format(_("vert: %d"), data->GetNumSelectedVertices()));
		ed->drawStr(20, 120, format(_("poly: %d"), data->GetNumSelectedPolygons()));
		ed->drawStr(20, 140, format(_("surf: %d"), data->GetNumSelectedSurfaces()));
	}
}



void ModeModelMesh::onUpdate(Observable *o, const string &message)
{
	// consistency checks
	if (current_material >= data->material.num)
		SetCurrentMaterial(data->material.num - 1);
	//data->DebugShow();
}



void ModeModelMesh::onUpdateMenu()
{
	ed->enable("copy", Copyable());
	ed->enable("paste", Pasteable());
	ed->enable("delete", Copyable());
	string cm_name = ed->cur_mode->name;
	ed->check("new_point", cm_name == "ModelMeshCreateVertex");
	ed->check("new_tria", cm_name == "ModelMeshCreateTriangles");
	ed->check("new_plane", cm_name == "ModelMeshCreatePlane");
	ed->check("new_cube", cm_name == "ModelMeshCreateCube");
	ed->check("new_ball", cm_name == "ModelMeshCreateBall");
	ed->check("new_cylinder", cm_name == "ModelMeshCreateCylinder");
	ed->check("new_torus", cm_name == "ModelMeshCreateTorus");

	ed->check("select_cw", mode_model_mesh_polygon->SelectCW);

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



void ModeModelMesh::CreateNewMaterialForSelection()
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

void ModeModelMesh::ChooseMaterialForSelection()
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

void ModeModelMesh::Copy()
{
	data->CopyGeometry(temp_geo);

	onUpdateMenu();
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke kopiert"), temp_geo.vertex.num, temp_geo.polygon.num));
}

void ModeModelMesh::Paste()
{
	data->PasteGeometry(temp_geo, current_material);
	ed->setMessage(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), temp_geo.vertex.num, temp_geo.polygon.num));
}

bool ModeModelMesh::Copyable()
{
	return data->GetNumSelectedVertices() > 0;
}

void ModeModelMesh::AddEffects(int type)
{
	if (data->GetNumSelectedVertices() == 0){
		ed->setMessage(_("Kein Punkt markiert!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, type, -1);
	dlg->run();
}

void ModeModelMesh::EditEffects()
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

void ModeModelMesh::ClearEffects()
{
	int n = 0;
	foreach(ModelEffect &fx, data->fx)
		if (data->vertex[fx.vertex].is_selected)
			n ++;
	if (n == 0){
		ed->setMessage(_("Kein Punkt mit Effekt markiert!"));
		return;
	}
	data->SelectionClearEffects();
}

bool ModeModelMesh::Pasteable()
{
	return temp_geo.vertex.num > 0;
}

void ModeModelMesh::Easify()
{
	ModelEasifyDialog *dlg = new ModelEasifyDialog(ed, false, data);
	dlg->run();
}

void ModeModelMesh::SetCurrentMaterial(int index)
{
	if (current_material == index)
		return;
	current_material = index;
	notify();
	mode_model_mesh_texture->SetCurrentTextureLevel(0);
}
