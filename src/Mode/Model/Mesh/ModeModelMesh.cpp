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
#include "../../../Action/Model/ActionModelDeleteSelection.h"
#include "../../../Action/Model/ActionModelSetMaterial.h"
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

void ModeModelMesh::Start()
{
	string dir = HuiAppDirectoryStatic + SysFileName("Data/icons/toolbar/");
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItemCheckable(_("Vertexpunkt"),_("Vertexpunkt"), dir + "new_vertex.png", "new_point");
	ed->ToolbarAddItemCheckable(_("Dreieck"),_("Dreieck"), dir + "new_triangle.png", "new_tria");
	ed->ToolbarAddItemCheckable(_("Dreieck \"U\""),_("Dreieck \"U\""), dir + "new_triangles_u.png", "new_tria_u");
	ed->ToolbarAddItemCheckable(_("Ebene"),_("Ebene"), dir + "new_plane.png", "new_plane");
	ed->ToolbarAddItemCheckable(_("Quader"),_("Quader"), dir + "mode_skin.png", "new_cube");
	ed->ToolbarAddItemCheckable(_("Kugel"),_("Kugel"), dir + "new_ball.png", "new_ball");
	ed->ToolbarAddItemCheckable(_("Zylinder"),_("Zylinder"), dir + "new_cylinder.png", "new_cylinder");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItemCheckable(_("Rotieren"),_("Rotieren"), dir + "rf_rotate.png", "rotate");
	ed->ToolbarAddItemCheckable(_("Skalieren"),_("Skalieren"), dir + "rf_scale.png", "scale");
	ed->ToolbarAddItemCheckable(_("Skalieren (2D)"),_("Skalieren (2D)"), dir + "rf_scale2d.png", "scale_2d");
	ed->ToolbarAddItemCheckable(_("Spiegeln"),_("Spiegeln"), dir + "rf_mirror.png", "mirror");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);

	ed->SetMode(mode_model_mesh_vertex);
	//ed->SetMode(mode_model_mesh_skin);
}

void ModeModelMesh::End()
{
}

void ModeModelMesh::OnMiddleButtonUp()
{
}



void ModeModelMesh::OnRightButtonUp()
{
}



void ModeModelMesh::OnLeftButtonUp()
{
}



void ModeModelMesh::OnLeftButtonDown()
{
}



void ModeModelMesh::OnKeyDown()
{
}



void ModeModelMesh::OnKeyUp()
{
}



void ModeModelMesh::OnCommand(const string & id)
{
	if (id == "opt_view")
		OptimizeView();

	if (id == "delete")
		data->Execute(new ActionModelDeleteSelection(data, (ed->cur_mode == mode_model_mesh_vertex)));

	if (id == "new_point"){
		ed->SetMode(mode_model_mesh_vertex);
		ed->SetCreationMode(new ModeModelMeshCreateVertex(ed->cur_mode, data));
	}
	if (id == "new_tria"){
		ed->SetMode(mode_model_mesh_vertex);
		ed->SetCreationMode(new ModeModelMeshCreateTriangles(ed->cur_mode, data));
	}
	if (id == "new_ball")
		ed->SetCreationMode(new ModeModelMeshCreateBall(ed->cur_mode, data));
	if (id == "new_cube")
		ed->SetCreationMode(new ModeModelMeshCreateCube(ed->cur_mode, data));
	if (id == "new_cylinder")
		ed->SetCreationMode(new ModeModelMeshCreateCylinder(ed->cur_mode, data));
	if (id == "new_plane")
		ed->SetCreationMode(new ModeModelMeshCreatePlane(ed->cur_mode, data));

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
}



void ModeModelMesh::OnRightButtonDown()
{
}



void ModeModelMesh::DrawWin(int win, irect dest)
{
}



void ModeModelMesh::Draw()
{
}



void ModeModelMesh::OnMouseMove()
{
}



void ModeModelMesh::OnMiddleButtonDown()
{
}



void ModeModelMesh::OnUpdate(Observable *o)
{
	data->DebugShow();
}



void ModeModelMesh::OnUpdateMenu()
{
	/*ed->Check("new_point", );
	ed->Check("new_tria", ); // "new_tria_u"
	ed->Check("new_plane", );
	ed->Check("new_cube", );
	ed->Check("new_ball", );
	ed->Check("new_cylinder", );*/

	ed->Check("rotate", right_mouse_function == RMFRotate);
	ed->Check("scale", right_mouse_function == RMFScale);
	ed->Check("scale_2d", right_mouse_function == RMFScale2d);
	ed->Check("mirror", right_mouse_function == RMFMirror);
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
}

void ModeModelMesh::ApplyRightMouseFunction(MultiView *mv)
{
	if (!mv)
		return;

	// left -> translate
	mv->SetMouseAction(0, "ActionModelMVMoveVertices", MultiView::ActionMove);

	// right...
	if (right_mouse_function == RMFRotate){
		mv->SetMouseAction(1, "ActionModelMVRotateVertices", MultiView::ActionRotate2d);
		mv->SetMouseAction(2, "ActionModelMVRotateVertices", MultiView::ActionRotate);
	}else if (right_mouse_function == RMFScale){
		mv->SetMouseAction(2, "ActionModelMVScaleVertices", MultiView::ActionScale);
	}else if (right_mouse_function == RMFScale2d){
		mv->SetMouseAction(2, "ActionModelMVScaleVertices", MultiView::ActionScale2d);
	}else if (right_mouse_function == RMFMirror){
		mv->SetMouseAction(2, "ActionModelMVMirrorVertices", MultiView::ActionOnce);
	}
}



