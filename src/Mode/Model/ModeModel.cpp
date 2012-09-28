/*
 * ModeModel.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../Edward.h"
#include "ModeModel.h"
#include "../../Data/Model/DataModel.h"
#include "Mesh/ModeModelMesh.h"
#include "Mesh/ModeModelMeshVertex.h"
#include "Mesh/ModeModelMeshEdge.h"
#include "Mesh/ModeModelMeshPolygon.h".h"
#include "Mesh/ModeModelMeshSurface.h"
#include "Mesh/ModeModelMeshTexture.h"
#include "Skeleton/ModeModelSkeleton.h"
#include "Animation/ModeModelAnimation.h"

ModeModel *mode_model = NULL;

ModeModel::ModeModel() :
	Mode("Model", NULL, new DataModel, NULL, "")
{
	data = dynamic_cast<DataModel*>(data_generic);
	PropertiesDialog = NULL;
	MaterialDialog = NULL;

	mode_model_mesh = new ModeModelMesh(this, data);
	mode_model_skeleton = new ModeModelSkeleton(this, data);
	mode_model_animation = new ModeModelAnimation(this, data);
	Subscribe(data);
}

ModeModel::~ModeModel()
{
}



void ModeModel::OnStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->ToolbarAddItem(L("new"),L("new"),dir + "new.png","new");
	ed->ToolbarAddItem(L("open"),L("open"),dir + "open.png","open");
	ed->ToolbarAddItem(L("save"),L("save"),dir + "save.png","save");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(L("undo"),L("undo"),dir + "undo.png","undo");
	ed->ToolbarAddItem(L("redo"),L("redo"),dir + "redo.png","redo");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItem(_("Push"),_("ViewStage Push"),dir + "view_push.png","view_push");
	ed->ToolbarAddItem(_("Pop"),_("ViewStage Pop"),dir + "view_pop.png","view_pop");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItemCheckable(_("Vertices"),_("Vertices"),dir + "mode_vertices.png","mode_model_vertex");
	ed->ToolbarAddItemCheckable(_("Kanten"),_("Kanten"),dir + "mode_vertices.png","mode_model_edge");
	ed->ToolbarAddItemCheckable(_("Polygone"),_("Polygone"),dir + "mode_skin.png","mode_model_triangle");
	ed->ToolbarAddItemCheckable(_("Oberfl&achen"),_("Oberfl&achen"),dir + "mode_skin.png","mode_model_surface");
	ed->ToolbarAddItemCheckable(_("Textur-Koordinaten"),_("Textur-Koordinaten"),dir + "mode_texturecoordinates.png","mode_model_texture_coord");
	ed->ToolbarAddSeparator();
	ed->ToolbarAddItemCheckable(_("Mesh"),_("Mesh"), dir + "mode_skin.png", "mode_model_mesh");
	ed->ToolbarAddItemCheckable(_("Skelett"),_("Skelett"), dir + "mode_skeletton.png", "mode_model_skeleton");
	ed->ToolbarAddItemCheckable(_("Animation"),_("Animation"), dir + "mode_move.png", "mode_model_animation");
	ed->ToolbarAddItem(_("Texturen"),_("Texturen"), dir + "mode_textures.png", "mode_model_texture");
	ed->ToolbarAddItem(_("Material"),_("Material"), dir + "mode_textures.png", "mode_model_material");
	ed->ToolbarAddItem(_("Materialien"),_("Materialien"), dir + "mode_textures.png", "mode_model_materials");
	ed->ToolbarAddItem(_("Eigenschaften"),_("Eigenschaften"), dir + "configure.png", "mode_properties");
	ed->EnableToolbar(true);
	ed->ToolbarConfigure(false,true);
	ed->ToolbarSetCurrent(HuiToolbarLeft);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
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

	ed->ToolbarSetCurrent(HuiToolbarTop);
	ed->ToolbarReset();
	ed->EnableToolbar(false);
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
	if (id == "mode_model_material")
		ExecuteMaterialDialog(0);
	if (id == "mode_model_materials")
		ExecutePropertiesDialog(1);
}



void ModeModel::OnUpdate(Observable *o)
{
	data->UpdateNormals();
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
	ed->SetMode(mode_model);
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

	ed->SetMode(mode_model);
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

void ModeModel::ExecutePropertiesDialog(int initial_tab_page)
{
	if (PropertiesDialog)
		return;

	PropertiesDialog = new ModelPropertiesDialog(ed, true, data);
	PropertiesDialog->SetInt("model_dialog_tab_control", initial_tab_page);

	PropertiesDialog->Update();
	//HuiWaitTillWindowClosed(PropertiesDialog);
}

void ModeModel::ExecuteMaterialDialog(int initial_tab_page)
{
	if (MaterialDialog)
		return;

	MaterialDialog = new ModelMaterialDialog(ed, true, data);
	MaterialDialog->SetInt("model_material_dialog_tab_control", initial_tab_page);

	MaterialDialog->Update();
	//HuiWaitTillWindowClosed(MaterialDialog);
}

