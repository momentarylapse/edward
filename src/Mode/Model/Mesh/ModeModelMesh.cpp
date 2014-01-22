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
	Mode<DataModel>("ModelMesh", _parent, NULL, ""),
	Observable("ModelMesh")
{
	Observer::Subscribe(data);

	MaterialDialog = NULL;
	CurrentMaterial = 0;

	ChooseRightMouseFunction(MultiView::ActionSelect);

	mode_model_mesh_vertex = new ModeModelMeshVertex(this);
	mode_model_mesh_edge = new ModeModelMeshEdge(this);
	mode_model_mesh_polygon = new ModeModelMeshPolygon(this);
	mode_model_mesh_surface = new ModeModelMeshSurface(this);
	mode_model_mesh_texture = new ModeModelMeshTexture(this);
}

ModeModelMesh::~ModeModelMesh()
{
	Observer::Unsubscribe(data);
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
	t->AddItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->AddItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->AddItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->AddItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
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

	CloseMaterialDialog();
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

	if (id == "volume_subtract")
		data->SubtractSelection();
	if (id == "volume_and")
		data->AndSelection();
	if (id == "invert_trias")
		data->InvertSelection();
	if (id == "extrude_triangles")
		ed->SetMode(new ModeModelMeshExtrudePolygons(ed->cur_mode));
	if (id == "autoweld_surfaces")
		ed->SetMode(new ModeModelMeshAutoweld(ed->cur_mode));
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
	if (id == "deformation_brush")
		ed->SetMode(new ModeModelMeshBrush(mode_model_mesh_polygon));
	if (id == "flatten_vertices")
		data->FlattenSelectedVertices();

	if (id == "select")
		ChooseRightMouseFunction(MultiView::ActionSelect);
	if (id == "translate")
		ChooseRightMouseFunction(MultiView::ActionMove);
	if (id == "rotate")
		ChooseRightMouseFunction(MultiView::ActionRotate);
	if (id == "scale")
		ChooseRightMouseFunction(MultiView::ActionScale);
	if (id == "mirror")
		ChooseRightMouseFunction(MultiView::ActionMirror);

	if (id == "create_new_material")
		CreateNewMaterialForSelection();
	if (id == "choose_material")
		ChooseMaterialForSelection();
	if (id == "mode_model_materials")
		ToggleMaterialDialog();
	if (id == "text_from_bg"){
		MultiView::MultiView *mv = mode_model_mesh_polygon->multi_view;
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

void ModeModelMesh::ShowMaterialDialog()
{
	if (!MaterialDialog){
		MaterialDialog = new ModelMaterialDialog(ed, data);
		ed->Check("mode_model_materials", true);
	}
}

void ModeModelMesh::CloseMaterialDialog()
{
	if (MaterialDialog){
		delete(MaterialDialog);
		MaterialDialog = NULL;
		ed->Check("mode_model_materials", false);
	}
}

void ModeModelMesh::ToggleMaterialDialog()
{
	if (MaterialDialog)
		CloseMaterialDialog();
	else
		ShowMaterialDialog();
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
		SetCurrentMaterial(data->Material.num - 1);
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

	ed->Check("select", mouse_action == MultiView::ActionSelect);
	ed->Check("translate", mouse_action == MultiView::ActionMove);
	ed->Check("rotate", mouse_action == MultiView::ActionRotate);
	ed->Check("scale", mouse_action == MultiView::ActionScale);
	ed->Check("mirror", mouse_action == MultiView::ActionMirror);

	ed->Check("mode_model_materials", MaterialDialog);
}

bool ModeModelMesh::OptimizeView()
{
	msg_db_f("OptimizeView", 1);
	MultiView::MultiView *mv = ed->multi_view_3d;
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
	return true;
}



void ModeModelMesh::CreateNewMaterialForSelection()
{
#if 0
	msg_db_f("CreateNewMaterialForSelection", 2);
	if (0 == data->GetNumSelectedPolygons()){
		ed->SetMessage(_("kein Dreieck ausgew&ahlt"));
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
#endif
}

void ModeModelMesh::ChooseMaterialForSelection()
{
	msg_db_f("ChooseMaterialForSelection", 2);
	if (0 == data->GetNumSelectedPolygons()){
		ed->SetMessage(_("kein Dreieck ausgew&ahlt"));
		return;
	}

	int SelectionDialogReturnIndex;


	// dialog
	ModelMaterialSelectionDialog *dlg = new ModelMaterialSelectionDialog(ed, false, data);
	dlg->PutAnswer(&SelectionDialogReturnIndex);
	dlg->Run();

	if (SelectionDialogReturnIndex >= 0)
		data->SetMaterialSelection(SelectionDialogReturnIndex);
}

void ModeModelMesh::ChooseRightMouseFunction(int f)
{
	mouse_action = f;
	ed->UpdateMenu();
	ApplyRightMouseFunction(ed->multi_view_3d);
	ApplyRightMouseFunction(ed->multi_view_2d);
}

void ModeModelMesh::ApplyRightMouseFunction(MultiView::MultiView *mv)
{
	if (!mv)
		return;

	string suffix = "Vertices";
	if (!mv->mode3d)
		suffix = "SkinVertices";

	// left mouse action
	if (mouse_action != MultiView::ActionSelect){
		mv->SetMouseAction("ActionModelTransform" + suffix, mouse_action);
	}else{
		mv->SetMouseAction("", MultiView::ActionSelect);
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

void ModeModelMesh::SetCurrentMaterial(int index)
{
	if (CurrentMaterial == index)
		return;
	CurrentMaterial = index;
	Notify("Change");
	mode_model_mesh_texture->SetCurrentTextureLevel(0);
}
