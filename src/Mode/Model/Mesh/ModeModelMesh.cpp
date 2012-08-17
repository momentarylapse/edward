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
#include "../../../Action/Model/Mesh/ActionModelDeleteSelection.h"
#include "../../../Action/Model/Mesh/Look/ActionModelSetMaterial.h"
#include "../../../Action/Model/Mesh/Surface/ActionModelSurfaceSubtract.h"
#include "../../../Action/Model/Mesh/Surface/ActionModelInvertSelection.h"
#include "../../../Action/Model/Mesh/ActionModelPasteGeometry.h"
#include "../../../Action/Model/Mesh/Look/ActionModelSetNormalModeSelection.h"
#include "../../../Action/Model/Mesh/Look/ActionModelSetNormalModeAll.h"
#include "../../../Action/Model/Mesh/Vertex/ActionModelNearifyVertices.h"
#include "../../../Action/Model/Mesh/Vertex/ActionModelAlignToGrid.h"
#include "../../../Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.h"
#include "../../../Action/Model/Mesh/ActionModelEasify.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(Mode *_parent, DataModel *_data)
{
	name = "ModelMesh";
	parent = _parent;
	data = _data;
	menu = NULL;
	multi_view = NULL;
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
	string dir = HuiAppDirectoryStatic + SysFileName("Data/icons/toolbar/");
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

//	ed->SetMode(mode_model_mesh_vertex);
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
		data->Execute(new ActionModelDeleteSelection(data, (ed->cur_mode == mode_model_mesh_vertex)));
	if (id == "copy")
		Copy();
	if (id == "paste")
		Paste();

	if (id == "select_cw")
		mode_model_mesh_triangle->ToggleSelectCW();

	if (id == "subtract_surface")
		data->Execute(new ActionModelSurfaceSubtract(data));
	if (id == "invert_trias")
		data->Execute(new ActionModelInvertSelection(data));
	if (id == "nearify")
		data->Execute(new ActionModelNearifyVertices(data));
	if (id == "align_to_grid")
		data->Execute(new ActionModelAlignToGrid(data, mode_model_mesh_vertex->multi_view->GetGridD()));

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
		data->Execute(new ActionModelEasify(data, 0.5f));

	if (id == "normal_this_smooth")
		data->Execute(new ActionModelSetNormalModeSelection(data, NormalModeSmooth));
	if (id == "normal_this_hard")
		data->Execute(new ActionModelSetNormalModeSelection(data, NormalModeHard));
	if (id == "normal_this_angular")
		data->Execute(new ActionModelSetNormalModeSelection(data, NormalModeAngular));
	if (id == "normal_all_smooth")
		data->Execute(new ActionModelSetNormalModeAll(NormalModeSmooth));
	if (id == "normal_all_hard")
		data->Execute(new ActionModelSetNormalModeAll(NormalModeHard));
	if (id == "normal_all_angular")
		data->Execute(new ActionModelSetNormalModeAll(NormalModeAngular));
}



void ModeModelMesh::OnDraw()
{
	if (data->GetNumMarkedVertices() > 0){
		ed->DrawStr(20, 100, format(_("vert: %d"), data->GetNumMarkedVertices()));
		ed->DrawStr(20, 120, format(_("tria: %d"), data->GetNumMarkedTriangles()));
		ed->DrawStr(20, 140, format(_("surf: %d"), data->GetNumMarkedSurfaces()));
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
		foreach(data->Vertex, v){
			VecMin(min, v.pos);
			VecMax(max, v.pos);
		}
		mv->pos = (min + max) / 2;
		if (data->Vertex.num > 1)
			mv->radius = VecLengthFuzzy(max - min) * 1.3f * ((float)NixScreenWidth / (float)NixTargetWidth);
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
	if (0 == data->GetNumMarkedTriangles()){
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
	if (0 == data->GetNumMarkedTriangles()){
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
		data->Execute(new ActionModelSetMaterial(data, SelectionDialogReturnIndex));

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
	TempGeo.Vertex.clear();
	TempGeo.Triangle.clear();

	// copy vertices
	Array<int> vert;
	foreachi(data->Vertex, v, vi)
		if (v.is_selected){
			TempGeo.Vertex.add(v);
			vert.add(vi);
		}

	// copy triangles
	foreach(data->Surface, s)
		foreach(s.Triangle, t)
			if (t.is_selected){
				ModeModelTriangle tt = t;
				for (int k=0;k<3;k++)
					foreachi(vert, v, vi)
						if (v == t.Vertex[k])
							tt.Vertex[k] = vi;
				TempGeo.Triangle.add(tt);
			}

	OnUpdateMenu();
	ed->SetMessage(format(_("%d Vertizes, %d Dreiecke kopiert"), TempGeo.Vertex.num, TempGeo.Triangle.num));
}

void ModeModelMesh::Paste()
{
	data->Execute(new ActionModelPasteGeometry(data, TempGeo));
	ed->SetMessage(format(_("%d Vertizes, %d Dreiecke eingef&ugt"), TempGeo.Vertex.num, TempGeo.Triangle.num));
}

bool ModeModelMesh::Copyable()
{
	return data->GetNumMarkedVertices();
}

bool ModeModelMesh::Pasteable()
{
	return TempGeo.Vertex.num > 0;
}



