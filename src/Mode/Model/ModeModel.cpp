/*
 * ModeModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeModel.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/Import/Importer3ds.h"
#include "Mesh/ModeModelMesh.h"
#include "Mesh/ModeModelMeshVertex.h"
#include "Mesh/ModeModelMeshEdge.h"
#include "Mesh/ModeModelMeshPolygon.h"
#include "Mesh/ModeModelMeshSurface.h"
#include "Mesh/ModeModelMeshTexture.h"
#include "Skeleton/ModeModelSkeleton.h"
#include "Animation/ModeModelAnimation.h"

ModeModel *mode_model = NULL;

ModeModel::ModeModel() :
	Mode<DataModel>("Model", NULL, new DataModel, NULL, "")
{
	PropertiesDialog = NULL;
	MaterialDialog = NULL;

	mode_model_mesh = new ModeModelMesh(this);
	mode_model_skeleton = new ModeModelSkeleton(this);
	mode_model_animation = new ModeModelAnimation(this);
}

ModeModel::~ModeModel()
{
}



void ModeModel::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->Reset();
	t->AddItem(L("new"),dir + "new.png","new");
	t->AddItem(L("open"),dir + "open.png","open");
	t->AddItem(L("save"),dir + "save.png","save");
	t->AddSeparator();
	t->AddItem(L("undo"),dir + "undo.png","undo");
	t->AddItem(L("redo"),dir + "redo.png","redo");
	t->AddSeparator();
	t->AddItem(_("Push"),dir + "view_push.png","view_push");
	ed->SetTooltip("view_push", _("ViewStage Push"));
	t->AddItem(_("Pop"),dir + "view_pop.png","view_pop");
	ed->SetTooltip("view_pop", _("ViewStage Pop"));
	t->AddSeparator();
	t->AddItemCheckable(_("Vertices"),dir + "model_vertex.svg","mode_model_vertex");
	t->AddItemCheckable(_("Kanten"),dir + "model_edge.svg","mode_model_edge");
	t->AddItemCheckable(_("Polygone"),dir + "model_polygon.svg","mode_model_triangle");
	t->AddItemCheckable(_("Oberfl&achen"),dir + "model_surface.svg","mode_model_surface");
	t->AddItemCheckable(_("Textur-Koordinaten"),dir + "model_skin.svg","mode_model_texture_coord");
	t->AddSeparator();
	t->AddItemCheckable(_("Mesh"), dir + "model_mesh.svg", "mode_model_mesh");
	t->AddItemCheckable(_("Skelett"), dir + "mode_skeletton.png", "mode_model_skeleton");
	t->AddItemCheckable(_("Animation"), dir + "mode_move.png", "mode_model_animation");
	//t->AddItem(_("Texturen"), dir + "mode_textures.png", "mode_model_texture");
	t->AddItem(_("Material"), dir + "mode_textures.png", "edit_current_material");
	//t->AddItem(_("Materialien"), dir + "mode_textures.png", "mode_model_materials");
	t->AddItem(_("Eigenschaften"), dir + "configure.png", "mode_properties");
	t->Enable(true);
	t->Configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
	t->Reset();
	t->Enable(false);

	ed->DeleteControl("table2");
}



void ModeModel::OnEnter()
{
	ed->SetMode(mode_model_mesh);
}


void ModeModel::OnEnd()
{
	if (PropertiesDialog)
		delete(PropertiesDialog);
	PropertiesDialog = NULL;
	if (MaterialDialog)
		delete(MaterialDialog);
	MaterialDialog = NULL;

	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->Reset();
	t->Enable(false);
}



void ModeModel::OnCommand(const string & id)
{
	if (id == "new")
		New();
	if (id == "open")
		Open();
	if (id == "save")
		Save();
	if (id == "save_as")
		SaveAs();

	if (id == "import_from_3ds")
		ImportOpen3ds();

	// TODO -> edward?
	if (id == "undo")
		data->Undo();
	if (id == "redo")
		data->Redo();

	if (id == "mode_model_vertex")
		ed->SetMode(mode_model_mesh_vertex);
	if (id == "mode_model_edge")
		ed->SetMode(mode_model_mesh_edge);
	if (id == "mode_model_triangle")
		ed->SetMode(mode_model_mesh_polygon);
	if (id == "mode_model_surface")
		ed->SetMode(mode_model_mesh_surface);
	if (id == "mode_model_texture_coord")
		ed->SetMode(mode_model_mesh_texture);
	if (id == "mode_model_animation")
		ed->SetMode(mode_model_animation);
	if (id == "mode_model_skeleton")
		ed->SetMode(mode_model_skeleton);
	if (id == "mode_model_texture")
		ExecuteMaterialDialog(1);
		//SetSubMode(SubModeTextures);
	if (id == "mode_properties")
		ExecutePropertiesDialog(0);
	if (id == "edit_current_material")
		ExecuteMaterialDialog(0);
	if (id == "mode_model_materials")
		ExecutePropertiesDialog(1);

	// mainly skin debugging...
	if (id == "detail_1")
		data->ImportFromTriangleSkin(1);
	if (id == "detail_2")
		data->ImportFromTriangleSkin(2);
	if (id == "detail_3")
		data->ImportFromTriangleSkin(3);
}



void ModeModel::OnUpdateMenu()
{
	ed->Check("mode_model_vertex", mode_model_mesh_vertex->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_edge", mode_model_mesh_edge->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_triangle", mode_model_mesh_polygon->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_surface", mode_model_mesh_surface->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_texture_coord", mode_model_mesh_texture->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_mesh", mode_model_mesh->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_skeleton", mode_model_skeleton->IsAncestorOf(ed->cur_mode));
	ed->Check("mode_model_animation", mode_model_animation->IsAncestorOf(ed->cur_mode));
}



void ModeModel::SetMaterialSelected()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
}

void ModeModel::SetMaterialMouseOver()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,White);
}

void ModeModel::SetMaterialCreation()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black, color(0.3f,0.3f,1,0.3f), Black, 0, color(1,0.1f,0.4f,0.1f));
}

void ModeModel::New()
{
	if (!ed->AllowTermination())
		return;
	data->Reset();
	ed->SetMode(this);
	mode_model_mesh->OptimizeView();
}

bool ModeModel::Open()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDModel, false, false))
		return false;
	if (!data->Load(ed->DialogFileComplete))
		return false;

	ed->SetMode(this);
	mode_model_mesh->OptimizeView();
	return true;
}

bool ModeModel::Save()
{
	if (data->filename == "")
		return SaveAs();
	return data->Save(data->filename);
}

bool ModeModel::SaveAs()
{
	if (ed->FileDialog(FDModel, true, false))
		return data->Save(ed->DialogFileComplete);
	return false;
}

bool ModeModel::ImportOpen3ds()
{
	if (!ed->AllowTermination())
		return false;
	if (!ed->FileDialog(FDFile, false, false))
		return false;
	return ImportLoad3ds(ed->DialogFileComplete);
}

bool ModeModel::ImportLoad3ds(const string &filename)
{
	Importer3ds *im = new Importer3ds;
	if (!im->Import(data, filename))
		return false;

	ed->SetMode(this);
	mode_model_mesh->OptimizeView();
	return true;
}

void ModeModel::ExecutePropertiesDialog(int initial_tab_page)
{
	if (PropertiesDialog)
		return;

	PropertiesDialog = new ModelPropertiesDialog(ed, true, data);
	PropertiesDialog->SetInt("model_dialog_tab_control", initial_tab_page);

	PropertiesDialog->Show();
	//PropertiesDialog->Run();
}

void ModeModel::ExecuteMaterialDialog(int initial_tab_page, bool allow_parent)
{
	if (MaterialDialog)
		return;

	MaterialDialog = new ModelMaterialDialog(allow_parent ? ed : HuiCurWindow, allow_parent, data);
	MaterialDialog->SetInt("model_material_dialog_tab_control", initial_tab_page);

	MaterialDialog->Show();

	if (!allow_parent)
		MaterialDialog->Run();
}

