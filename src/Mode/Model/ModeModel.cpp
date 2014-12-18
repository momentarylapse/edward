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
#include "Skeleton/ModeModelSkeleton.h"
#include "Animation/ModeModelAnimation.h"
#include "../../lib/nix/nix.h"
#include "Mesh/MeshSelectionModeEdge.h"
#include "Mesh/MeshSelectionModePolygon.h"
#include "Mesh/MeshSelectionModeSurface.h"
#include "Mesh/MeshSelectionModeVertex.h"
#include "Mesh/ModeModelMeshTexture.h"

ModeModel *mode_model = NULL;

ModeModel::ModeModel() :
	Mode<DataModel>("Model", NULL, new DataModel, NULL, "")
{
	PropertiesDialog = NULL;

	mode_model_mesh = new ModeModelMesh(this);
	mode_model_skeleton = new ModeModelSkeleton(this);
	mode_model_animation = new ModeModelAnimation(this);
}

ModeModel::~ModeModel()
{
}



void ModeModel::onStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->reset();
	t->addItem(L("new"),dir + "new.png","new");
	t->addItem(L("open"),dir + "open.png","open");
	t->addItem(L("save"),dir + "save.png","save");
	t->addSeparator();
	t->addItem(L("undo"),dir + "undo.png","undo");
	t->addItem(L("redo"),dir + "redo.png","redo");
	t->addSeparator();
	t->addItem(_("Push"),dir + "view_push.png","view_push");
	ed->setTooltip("view_push", _("ViewStage Push"));
	t->addItem(_("Pop"),dir + "view_pop.png","view_pop");
	ed->setTooltip("view_pop", _("ViewStage Pop"));
	t->addSeparator();
	t->addItemCheckable(_("Vertices"),dir + "model_vertex.svg","mode_model_vertex");
	t->addItemCheckable(_("Kanten"),dir + "model_edge.svg","mode_model_edge");
	t->addItemCheckable(_("Polygone"),dir + "model_polygon.svg","mode_model_triangle");
	t->addItemCheckable(_("Oberfl&achen"),dir + "model_surface.svg","mode_model_surface");
	t->addItemCheckable(_("Textur-Koordinaten"),dir + "model_skin.svg","mode_model_texture_coord");
	t->addSeparator();
	t->addItemCheckable(_("Mesh"), dir + "model_mesh.svg", "mode_model_mesh");
	t->addItemCheckable(_("Skelett"), dir + "mode_skeletton.png", "mode_model_skeleton");
	t->addItemCheckable(_("Animation"), dir + "mode_move.png", "mode_model_animation");
	t->addItem(_("Eigenschaften"), dir + "configure.png", "mode_properties");
	t->enable(true);
	t->configure(false,true);
	t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->enable(false);
}



void ModeModel::onEnter()
{
	ed->setMode(mode_model_mesh);
}


void ModeModel::onEnd()
{
	if (PropertiesDialog)
		delete(PropertiesDialog);
	PropertiesDialog = NULL;

	HuiToolbar *t = ed->toolbar[HuiToolbarTop];
	t->reset();
	t->enable(false);
}



void ModeModel::onCommand(const string & id)
{
	if (id == "new")
		_new();
	if (id == "open")
		open();
	if (id == "save")
		save();
	if (id == "save_as")
		saveAs();

	if (id == "import_from_3ds")
		ImportOpen3ds();

	// TODO -> edward?
	if (id == "undo")
		data->undo();
	if (id == "redo")
		data->redo();

	if (id == "mode_model_mesh")
		ed->setMode(mode_model_mesh);
	if (id == "mode_model_vertex")
		mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);
	if (id == "mode_model_edge")
		mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_edge);
	if (id == "mode_model_triangle")
		mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_polygon);
	if (id == "mode_model_surface")
		mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_surface);
	if (id == "mode_model_texture_coord")
		ed->setMode(mode_model_mesh_texture);
	if (id == "mode_model_animation")
		ed->setMode(mode_model_animation);
	if (id == "mode_model_skeleton")
		ed->setMode(mode_model_skeleton);
		//SetSubMode(SubModeTextures);
	if (id == "mode_properties")
		ExecutePropertiesDialog();

	// mainly skin debugging...
	if (id == "detail_1")
		data->ImportFromTriangleSkin(1);
	if (id == "detail_2")
		data->ImportFromTriangleSkin(2);
	if (id == "detail_3")
		data->ImportFromTriangleSkin(3);
}



void ModeModel::onUpdateMenu()
{
	ed->check("mode_model_vertex", mode_model_mesh->selection_mode_vertex->isActive());
	ed->check("mode_model_edge", mode_model_mesh->selection_mode_edge->isActive());
	ed->check("mode_model_triangle", mode_model_mesh->selection_mode_polygon->isActive());
	ed->check("mode_model_surface", mode_model_mesh->selection_mode_surface->isActive());
	ed->check("mode_model_texture_coord", mode_model_mesh_texture->isAncestorOf(ed->cur_mode));
	ed->check("mode_model_mesh", mode_model_mesh->isAncestorOf(ed->cur_mode));
	ed->check("mode_model_skeleton", mode_model_skeleton->isAncestorOf(ed->cur_mode));
	ed->check("mode_model_animation", mode_model_animation->isAncestorOf(ed->cur_mode));
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

void ModeModel::_new()
{
	if (!ed->allowTermination())
		return;
	data->reset();
	ed->setMode(this);
	mode_model_mesh->optimizeView();
}

bool ModeModel::open()
{
	if (!ed->allowTermination())
		return false;
	if (!ed->fileDialog(FDModel, false, false))
		return false;
	if (!data->load(ed->DialogFileComplete))
		return false;

	ed->setMode(this);
	mode_model_mesh->optimizeView();
	return true;
}

bool ModeModel::save()
{
	if (data->filename == "")
		return saveAs();
	return data->save(data->filename);
}

bool ModeModel::saveAs()
{
	if (ed->fileDialog(FDModel, true, false))
		return data->save(ed->DialogFileComplete);
	return false;
}

bool ModeModel::ImportOpen3ds()
{
	if (!ed->allowTermination())
		return false;
	if (!ed->fileDialog(FDFile, false, false))
		return false;
	return ImportLoad3ds(ed->DialogFileComplete);
}

bool ModeModel::ImportLoad3ds(const string &filename)
{
	Importer3ds *im = new Importer3ds;
	if (!im->Import(data, filename))
		return false;

	ed->setMode(this);
	mode_model_mesh->optimizeView();
	return true;
}

void ModeModel::ExecutePropertiesDialog()
{
	if (PropertiesDialog)
		return;

	PropertiesDialog = new ModelPropertiesDialog(ed, true, data);

	PropertiesDialog->show();
	//PropertiesDialog->run();
}

