/*
 * ModeModelMesh.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../ModeModel.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshEdge.h"
#include "ModeModelMeshTriangle.h"
#include "ModeModelMeshSurface.h"
#include "ModeModelMeshTexture.h"
#include "../../ModeCreation.h"
#include "Creation/ModeModelMeshCreateVertex.h"
#include "Creation/ModeModelMeshCreateTriangles.h"
#include "Creation/ModeModelMeshCreateBall.h"
#include "Creation/ModeModelMeshCreateCube.h"
#include "Creation/ModeModelMeshCreateCylinder.h"
#include "Creation/ModeModelMeshCreatePlane.h"
#include "Creation/ModeModelMeshSplitTriangle.h"
#include "Creation/ModeModelMeshBevelVertices.h"
#include "../../../Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.h"
#include "../../../Action/Model/Mesh/Triangle/ActionModelTriangulateSelection.h"
#include "../../../Action/Model/Mesh/Triangle/ActionModelCutOutPolygons.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"
#include "../Dialog/ModelEasifyDialog.h"
#include "../Dialog/ModelFXDialog.h"

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(Mode *_parent, DataModel *_data) :
	Mode("ModelMesh", _parent, _data, NULL, "")
{
	data = _data;
	Subscribe(data);

	MaterialSelectionDialog = NULL;

	right_mouse_function = RMFRotate;

	mode_model_mesh_vertex = new ModeModelMeshVertex(this, data);
	mode_model_mesh_edge = new ModeModelMeshEdge(this, data);
	mode_model_mesh_triangle = new ModeModelMeshTriangle(this, data);
	mode_model_mesh_surface = new ModeModelMeshSurface(this, data);
	mode_model_mesh_texture = new ModeModelMeshTexture(this, data);
}

ModeModelMesh::~ModeModelMesh()
{
}

void ModeModelMesh::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItemCheckable(_("Vertexpunkt"),_("Vertexpunkt"), dir + "new_vertex.png", "new_point");
	ed->ToolbarAddItemCheckable(_("Dreieck"),_("Dreieck"), dir + "new_triangle.png", "new_tria");
	ed->ToolbarAddItemCheckable(_("Ebene"),_("Ebene"), dir + "new_plane.png", "new_plane");
	ed->ToolbarAddItemCheckable(_("Quader"),_("Quader"), dir + "mode_skin.png", "new_cube");
	ed->ToolbarAddItemCheckable(_("Kugel"),_("Kugel"), dir + "new_ball.png", "new_ball");
	ed->ToolbarAddItemCheckable(_("Zylinder (-schlange)"),_("Zylinder (-schlange)"), dir + "new_cylinder.png", "new_cylinder");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItemCheckable(_("Rotieren"),_("Rotieren"), dir + "rf_rotate.png", "rotate");
	ed->ToolbarAddItemCheckable(_("Skalieren"),_("Skalieren"), dir + "rf_scale.png", "scale");
	ed->ToolbarAddItemCheckable(_("Skalieren (2D)"),_("Skalieren (2D)"), dir + "rf_scale2d.png", "scale_2d");
	ed->ToolbarAddItemCheckable(_("Spiegeln"),_("Spiegeln"), dir + "rf_mirror.png", "mirror");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
}

void ModeModelMesh::OnEnter()
{
	ed->SetMode(mode_model_mesh_vertex);
	//ed->SetMode(mode_model_mesh_skin);
}

void ModeModelMesh::OnEnd()
{
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
}



void ModeModelMesh::OnCommand(const string & id)
{
	if (id == "opt_view")
		OptimizeView();

	if (id == "delete")
		data->DeleteSelection(ed->cur_mode == mode_model_mesh_vertex);
	if (id == "copy")
		Copy();
	if (id == "paste")
		Paste();

	if (id == "select_cw")
		mode_model_mesh_triangle->ToggleSelectCW();

	if (id == "subtract_surface")
		data->SubtractSelection();
	if (id == "invert_trias")
		data->InvertSelection();
	if (id == "extrude_triangles")
		data->ExtrudeSelectedTriangles(40 / mode_model_mesh_vertex->multi_view->zoom);
	if (id == "autoweld_surfaces")
		data->AutoWeldSelectedSurfaces(0.1f / mode_model_mesh_vertex->multi_view->zoom);
	if (id == "triangulate_selection")
		data->Execute(new ActionModelTriangulateSelection());
	if (id == "cut_out")
		data->Execute(new ActionModelCutOutPolygons());
	if (id == "nearify")
		data->NearifySelectedVertices();
	if (id == "connect")
		data->CollapseSelectedVertices();
	if (id == "align_to_grid")
		data->AlignToGridSelection(mode_model_mesh_vertex->multi_view->GetGridD());

	if (id == "new_point")
		ed->SetMode(new ModeModelMeshCreateVertex(mode_model_mesh_vertex));
	if (id == "new_tria")
		ed->SetMode(new ModeModelMeshCreateTriangles(mode_model_mesh_vertex));
	if (id == "new_ball")
		ed->SetMode(new ModeModelMeshCreateBall(ed->cur_mode));
	if (id == "new_cube")
		ed->SetMode(new ModeModelMeshCreateCube(ed->cur_mode));
	if (id == "new_cylinder")
		ed->SetMode(new ModeModelMeshCreateCylinder(ed->cur_mode));
	if (id == "new_plane")
		ed->SetMode(new ModeModelMeshCreatePlane(ed->cur_mode));
	if (id == "new_extract")
		ed->SetMode(new ModeModelMeshSplitTriangle(mode_model_mesh_triangle));
	if (id == "bevel_vertices")
		ed->SetMode(new ModeModelMeshBevelVertices(mode_model_mesh_vertex));
	if (id == "flatten_vertices")
		data->FlattenSelectedVertices();

	if (id == "rotate")
		ChooseRightMouseFunction(RMFRotate);
	if (id == "scale")
		ChooseRightMouseFunction(RMFScale);
	if (id == "scale_2d")
		ChooseRightMouseFunction(RMFScale2d);
	if (id == "mirror")
		ChooseRightMouseFunction(RMFMirror);

	if (id == "create_new_material")
		CreateNewMaterialForSelection();
	if (id == "choose_material")
		ChooseMaterialForSelection();
	if (id == "text_from_bg"){
		MultiView *mv = mode_model_mesh_triangle->multi_view;
		data->Execute(new ActionModelSkinVerticesFromProjection(data, mv));
	}
	if (id == "easify_skin")
		Easify();

	if (id == "normal_this_smooth")
		data->SetNormalModeSelection(NormalModeSmooth);
	if (id == "normal_this_hard")
		data->SetNormalModeSelection(NormalModeHard);
	if (id == "normal_this_angular")
		data->SetNormalModeSelection(NormalModeAngular);
	if (id == "normal_all_smooth")
		data->SetNormalModeAll(NormalModeSmooth);
	if (id == "normal_all_hard")
		data->SetNormalModeAll(NormalModeHard);
	if (id == "normal_all_angular")
		data->SetNormalModeAll(NormalModeAngular);

	if (id == "fx_new_light")
		AddEffects(FXKindLight);
	if (id == "fx_new_sound")
		AddEffects(FXKindSound);
	if (id == "fx_new_script")
		AddEffects(FXKindScript);
	if (id == "fx_new_field")
		AddEffects(FXKindForceField);
	if (id == "fx_none")
		ClearEffects();
	if (id == "fx_edit")
		EditEffects();
}



void ModeModelMesh::OnDraw()
{
	if (data->GetNumSelectedVertices() > 0){
		ed->DrawStr(20, 100, format(_("vert: %d"), data->GetNumSelectedVertices()));
		ed->DrawStr(20, 120, format(_("tria: %d"), data->GetNumSelectedTriangles()));
		ed->DrawStr(20, 140, format(_("surf: %d"), data->GetNumSelectedSurfaces()));
	}
}



void ModeModelMesh::OnUpdate(Observable *o)
{
	//data->DebugShow();
}



void ModeModelMesh::OnUpdateMenu()
{
	ed->Enable("copy", Copyable());
	ed->Enable("paste", Pasteable());
	ed->Enable("delete", Copyable());
	string cm_name = ed->cur_mode->name;
	ed->Check("new_point", cm_name == "ModelMeshCreateVertex");
	ed->Check("new_tria", cm_name == "ModelMeshCreateTriangles");
	ed->Check("new_plane", cm_name == "ModelMeshCreatePlane");
	ed->Check("new_cube", cm_name == "ModelMeshCreateCube");
	ed->Check("new_ball", cm_name == "ModelMeshCreateBall");
	ed->Check("new_cylinder", cm_name == "ModelMeshCreateCylinder");

	ed->Check("select_cw", mode_model_mesh_triangle->SelectCW);

	ed->Check("rotate", right_mouse_function == RMFRotate);
	ed->Check("scale", right_mouse_function == RMFScale);
	ed->Check("scale_2d", right_mouse_function == RMFScale2d);
	ed->Check("mirror", right_mouse_function == RMFMirror);

	ed->Check("normal_all_hard", data->NormalModeAll == NormalModeHard);
	ed->Check("normal_all_smooth", data->NormalModeAll == NormalModeSmooth);
	ed->Check("normal_all_angular", data->NormalModeAll == NormalModeAngular);
	ed->Check("normal_all_per_vertex", data->NormalModeAll == NormalModePerVertex);
}

void ModeModelMesh::OptimizeView()
{
	msg_db_r("OptimizeView", 1);
	MultiView *mv = ed->multi_view_3d;
	bool ww = mv->whole_window;
	mv->ResetView();
	mv->whole_window = ww;
	if (data->Vertex.num > 0){
		vector min = data->Vertex[0].pos, max = data->Vertex[0].pos;
		foreach(ModelVertex &v, data->Vertex){
			min._min(v.pos);
			max._max(v.pos);
		}
		mv->pos = (min + max) / 2;
		if (data->Vertex.num > 1)
			mv->radius = (max - min).length_fuzzy() * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
	}

	ed->multi_view_2d->ResetView();
	ed->multi_view_2d->pos = vector(0.5f, 0.5f, 0);
	/*if ((Bone.num > 0) && (Vertex.num <= 0))
		SetSubMode(SubModeSkeleton);
	if (SubMode == SubModeSkeleton)
		SkeletonOptimizeView();*/
	msg_db_l(1);
}



void ModeModelMesh::CreateNewMaterialForSelection()
{
#if 0
	msg_db_r("CreateNewMaterialForSelection", 2);
	if (0 == data->GetNumSelectedTriangles()){
		ed->SetMessage(_("kein Dreieck ausgew&ahlt"));
		msg_db_l(2);
		return;
	}

	//StartChanging();

	data->Material.resize(data->Material.num + 1);
	data->Material.num --; // stupid...
	data->Material[data->Material.num].reset();
	//Material[Material.num] = cur_mat;
	int cmi = data->CurrentMaterial;
	data->CurrentMaterial = data->Material.num;

	if (mode_model->ExecuteMaterialDialog(0)){//, true)){
		data->Material.num ++;

		data->CurrentTextureLevel = 0;

		// create new subs
		for (int i=0;i<4;i++){
			data->Skin[i].Sub.resize(data->Material.num);
			data->Skin[i].Sub[data->CurrentMaterial].NumTextures = data->Material[data->CurrentMaterial].NumTextures;
		}

		// move selected trias to the new sub
//		move_sel_trias_to_mat(CurrentMaterial);
	}else{
		data->Material.num ++;
		data->Material.resize(data->Material.num - 1);
		data->CurrentMaterial = cmi;
	}

	//EndChanging();
	msg_db_l(2);
#endif
}

void ModeModelMesh::ChooseMaterialForSelection()
{
	msg_db_r("ChooseMaterialForSelection", 2);
	if (0 == data->GetNumSelectedTriangles()){
		ed->SetMessage(_("kein Dreieck ausgew&ahlt"));
		msg_db_l(2);
		return;
	}

	int SelectionDialogReturnIndex;


	// dialog
	MaterialSelectionDialog = new ModelMaterialSelectionDialog(ed, false, data);
	MaterialSelectionDialog->PutAnswer(&SelectionDialogReturnIndex);
	//FillMaterialList(MaterialSelectionDialog);
	MaterialSelectionDialog->Update();
	HuiWaitTillWindowClosed(MaterialSelectionDialog);

	if (SelectionDialogReturnIndex >= 0)
		data->SetMaterialSelection(SelectionDialogReturnIndex);

	msg_db_l(2);
}

void ModeModelMesh::ChooseRightMouseFunction(int f)
{
	right_mouse_function = f;
	ed->UpdateMenu();
	ApplyRightMouseFunction(ed->multi_view_3d);
	ApplyRightMouseFunction(ed->multi_view_2d);
}

void ModeModelMesh::ApplyRightMouseFunction(MultiView *mv)
{
	if (!mv)
		return;

	string suffix = "Vertices";
	if (!mv->mode3d)
		suffix = "SkinVertices";

	// left -> translate
	mv->SetMouseAction(0, "ActionModelMove" + suffix, MultiView::ActionMove);

	// right...
	if (right_mouse_function == RMFRotate){
		mv->SetMouseAction(1, "ActionModelRotate" + suffix, MultiView::ActionRotate2d);
		mv->SetMouseAction(2, "ActionModelRotate" + suffix, MultiView::ActionRotate);
	}else if (right_mouse_function == RMFScale){
		mv->SetMouseAction(2, "ActionModelScale" + suffix, MultiView::ActionScale);
	}else if (right_mouse_function == RMFScale2d){
		mv->SetMouseAction(2, "ActionModelScale" + suffix, MultiView::ActionScale2d);
	}else if (right_mouse_function == RMFMirror){
		mv->SetMouseAction(2, "ActionModelMirror" + suffix, MultiView::ActionOnce);
	}
}

void ModeModelMesh::Copy()
{
	data->CopyGeometry(TempGeo);

	OnUpdateMenu();
	ed->SetMessage(format(_("%d Vertizes, %d Dreiecke kopiert"), TempGeo.Vertex.num, TempGeo.Polygon.num));
}

void ModeModelMesh::Paste()
{
	data->PasteGeometry(TempGeo);
	ed->SetMessage(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), TempGeo.Vertex.num, TempGeo.Polygon.num));
}

bool ModeModelMesh::Copyable()
{
	return data->GetNumSelectedVertices() > 0;
}

void ModeModelMesh::AddEffects(int type)
{
	if (data->GetNumSelectedVertices() == 0){
		ed->SetMessage(_("Kein Punkt markiert!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, type, -1);
	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}

void ModeModelMesh::EditEffects()
{
	int index;
	int n = 0;
	foreachi(ModelEffect &fx, data->Fx, i)
		if (data->Vertex[fx.Vertex].is_selected){
			index = i;
			n ++;
		}
	if (n != 1){
		ed->SetMessage(_("Es muss genau ein Punkt mit Effekt markiert sein!"));
		return;
	}
	ModelFXDialog *dlg = new ModelFXDialog(ed, false, data, -1, index);
	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}

void ModeModelMesh::ClearEffects()
{
	int n = 0;
	foreach(ModelEffect &fx, data->Fx)
		if (data->Vertex[fx.Vertex].is_selected)
			n ++;
	if (n == 0){
		ed->SetMessage(_("Kein Punkt mit Effekt markiert!"));
		return;
	}
	data->SelectionClearEffects();
}

bool ModeModelMesh::Pasteable()
{
	return TempGeo.Vertex.num > 0;
}

void ModeModelMesh::Easify()
{
	ModelEasifyDialog *dlg = new ModelEasifyDialog(ed, false, data);
	dlg->Update();
	HuiWaitTillWindowClosed(dlg);
}



