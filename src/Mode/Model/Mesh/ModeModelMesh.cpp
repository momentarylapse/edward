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
#include "Creation/ModeModelMeshBevelEdges.h"
#include "Creation/ModeModelMeshBrushExtrudeVertices.h"
#include "Creation/ModeModelMeshExtrudePolygons.h"
#include "../../../Action/Model/Mesh/Skin/ActionModelSkinVerticesFromProjection.h"
#include "../Dialog/ModelMaterialSelectionDialog.h"
#include "../Dialog/ModelEasifyDialog.h"
#include "../Dialog/ModelFXDialog.h"

ModeModelMesh *mode_model_mesh = NULL;

ModeModelMesh::ModeModelMesh(ModeBase *_parent) :
	Mode<DataModel>("ModelMesh", _parent, NULL, "")
{
	Subscribe(data);

	MaterialSelectionDialog = NULL;
	CurrentMaterial = 0;

	right_mouse_function = RMFRotate;

	mode_model_mesh_vertex = new ModeModelMeshVertex(this);
	mode_model_mesh_edge = new ModeModelMeshEdge(this);
	mode_model_mesh_polygon = new ModeModelMeshPolygon(this);
	mode_model_mesh_surface = new ModeModelMeshSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this);
}

ModeModelMesh::~ModeModelMesh()
{
}

void ModeModelMesh::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->Reset();
	t->AddSeparator();
	t->AddItemCheckable(_("Polygon"),dir + "new_triangle.png", "new_tria");
	t->AddItemCheckable(_("Ebene"),dir + "new_plane.png", "new_plane");
	t->AddItemCheckable(_("Quader"),dir + "mode_skin.png", "new_cube");
	t->AddItemCheckable(_("Kugel"), dir + "new_ball.png", "new_ball");
	t->AddItemCheckable(_("Zylinder"), dir + "new_cylinder.png", "new_cylinder");
	t->AddSeparator();
	t->AddItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->AddItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
	t->AddItemCheckable(_("Skalieren (2D)"), dir + "rf_scale2d.png", "scale_2d");
	t->AddItemCheckable(_("Spiegeln"),dir + "rf_mirror.png", "mirror");
	t->Enable(true);
	t->Configure(false,true);
}

void ModeModelMesh::OnEnter()
{
	CurrentMaterial = 0;

	ed->SetMode(mode_model_mesh_vertex);
	//ed->SetMode(mode_model_mesh_skin);
}

void ModeModelMesh::OnEnd()
{
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->Reset();
	t->Enable(false);
}



void ModeModelMesh::OnCommand(const string & id)
{
	if (id == "delete")
		data->DeleteSelection(ed->cur_mode == mode_model_mesh_vertex);
	if (id == "copy")
		Copy();
	if (id == "paste")
		Paste();

	if (id == "select_cw")
		mode_model_mesh_polygon->ToggleSelectCW();

	if (id == "subtract_surface")
		data->SubtractSelection();
	if (id == "invert_trias")
		data->InvertSelection();
	if (id == "extrude_triangles")
		ed->SetMode(new ModeModelMeshExtrudePolygons(ed->cur_mode));
		//data->ExtrudeSelectedTriangles(40 / mode_model_mesh_vertex->multi_view->zoom);
	if (id == "autoweld_surfaces")
		data->AutoWeldSelectedSurfaces(0.1f / mode_model_mesh_vertex->multi_view->cam.zoom);
	if (id == "triangulate_selection")
		data->TriangulateSelection();
	if (id == "untriangulate_selection")
		data->MergePolygonsSelection();
	if (id == "cut_out")
		data->CutOutSelection();
	if (id == "nearify")
		data->NearifySelectedVertices();
	if (id == "connect")
		data->CollapseSelectedVertices();
	if (id == "align_to_grid")
		data->AlignToGridSelection(mode_model_mesh_vertex->multi_view->GetGridD());
	if (id == "subdivide_surfaces")
		data->SubdivideSelectedSurfaces();

	if (id == "new_point")
		ed->SetMode(new ModeModelMeshCreateVertex(mode_model_mesh_vertex));
	if (id == "new_tria")
		ed->SetMode(new ModeModelMeshCreatePolygon(mode_model_mesh_vertex));
	if (id == "new_ball")
		ed->SetMode(new ModeModelMeshCreateBall(ed->cur_mode));
	if (id == "new_cube")
		ed->SetMode(new ModeModelMeshCreateCube(ed->cur_mode));
	if (id == "new_cylinder")
		ed->SetMode(new ModeModelMeshCreateCylinder(ed->cur_mode));
	if (id == "new_cylindersnake")
		ed->SetMode(new ModeModelMeshCreateCylinderSnake(ed->cur_mode));
	if (id == "new_plane")
		ed->SetMode(new ModeModelMeshCreatePlane(ed->cur_mode));
	if (id == "new_torus")
		ed->SetMode(new ModeModelMeshCreateTorus(ed->cur_mode));
	if (id == "new_tetrahedron")
		ed->SetMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 4));
	if (id == "new_octahedron")
		ed->SetMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 8));
	if (id == "new_dodecahedron")
		ed->SetMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 12));
	if (id == "new_icosahedron")
		ed->SetMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 20));
	if (id == "new_teapot")
		ed->SetMode(new ModeModelMeshCreatePlatonic(ed->cur_mode, 306));
	if (id == "new_extract")
		ed->SetMode(new ModeModelMeshSplitPolygon(mode_model_mesh_polygon));
	if (id == "bevel_edges")
		ed->SetMode(new ModeModelMeshBevelEdges(mode_model_mesh_vertex));
	if (id == "brush_extrude_vertices")
		ed->SetMode(new ModeModelMeshBrushExtrudeVertices(mode_model_mesh_polygon));
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
		MultiView *mv = mode_model_mesh_polygon->multi_view;
		data->Execute(new ActionModelSkinVerticesFromProjection(data, mv));
	}
	if (id == "automapping")
		data->Automap(CurrentMaterial, mode_model_mesh_texture->CurrentTextureLevel);
	if (id == "easify_skin")
		Easify();

	if (id == "normal_this_smooth")
		data->SetNormalModeSelection(NormalModeSmooth);
	if (id == "normal_this_hard")
		data->SetNormalModeSelection(NormalModeHard);
	if (id == "normal_this_angular")
		data->SetNormalModeSelection(NormalModeAngular);

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
		ed->DrawStr(20, 120, format(_("poly: %d"), data->GetNumSelectedPolygons()));
		ed->DrawStr(20, 140, format(_("surf: %d"), data->GetNumSelectedSurfaces()));
	}
}



void ModeModelMesh::OnUpdate(Observable *o)
{
	// consistency checks
	if (CurrentMaterial >= data->Material.num)
		CurrentMaterial = data->Material.num - 1;
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
	ed->Check("new_torus", cm_name == "ModelMeshCreateTorus");

	ed->Check("select_cw", mode_model_mesh_polygon->SelectCW);

	ed->Check("rotate", right_mouse_function == RMFRotate);
	ed->Check("scale", right_mouse_function == RMFScale);
	ed->Check("scale_2d", right_mouse_function == RMFScale2d);
	ed->Check("mirror", right_mouse_function == RMFMirror);
}

bool ModeModelMesh::OptimizeView()
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
		mv->SetViewBox(min, max);
	}

	ed->multi_view_2d->ResetView();
	ed->multi_view_2d->cam.pos = vector(0.5f, 0.5f, 0);
	/*if ((Bone.num > 0) && (Vertex.num <= 0))
		SetSubMode(SubModeSkeleton);
	if (SubMode == SubModeSkeleton)
		SkeletonOptimizeView();*/
	msg_db_l(1);
	return true;
}



void ModeModelMesh::CreateNewMaterialForSelection()
{
#if 0
	msg_db_r("CreateNewMaterialForSelection", 2);
	if (0 == data->GetNumSelectedPolygons()){
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
	if (0 == data->GetNumSelectedPolygons()){
		ed->SetMessage(_("kein Dreieck ausgew&ahlt"));
		msg_db_l(2);
		return;
	}

	int SelectionDialogReturnIndex;


	// dialog
	MaterialSelectionDialog = new ModelMaterialSelectionDialog(ed, false, data);
	MaterialSelectionDialog->PutAnswer(&SelectionDialogReturnIndex);
	//FillMaterialList(MaterialSelectionDialog);
	MaterialSelectionDialog->Run();

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
		mv->SetMouseAction(1, "ActionModelRotate" + suffix, MultiView::ActionRotate);
		mv->SetMouseAction(2, "ActionModelRotate" + suffix, MultiView::ActionRotate2d);
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
	data->PasteGeometry(TempGeo, CurrentMaterial);
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
	dlg->Run();
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
	dlg->Run();
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
	dlg->Run();
}



