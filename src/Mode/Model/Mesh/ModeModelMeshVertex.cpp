/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../../../MultiView/Window.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshPolygon.h"
#include "../../../lib/nix/nix.h"

ModeModelMeshVertex *mode_model_mesh_vertex = NULL;

ModeModelMeshVertex::ModeModelMeshVertex(ModeBase *_parent) :
	Mode<DataModel>("ModelMeshVertex", _parent, ed->multi_view_3d, "menu_model")
{
}

ModeModelMeshVertex::~ModeModelMeshVertex()
{
}

void ModeModelMeshVertex::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->allow_rect = true;
	OnUpdate(data);
}

void ModeModelMeshVertex::OnEnd()
{
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelMeshVertex::OnDrawWin(MultiView::Window *win)
{
	mode_model_mesh_polygon->OnDrawWin(win);
	DrawEffects(win);
}



void ModeModelMeshVertex::OnDraw()
{
}

void ModeModelMeshVertex::OnUpdate(Observable *o)
{
	if (o->GetName() == "Data"){
		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->AddData(	MVDModelVertex,
				data->Vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove);
	}else if (o->GetName() == "MultiView"){
		data->SelectionFromVertices();
	}
	mode_model_mesh_polygon->FillSelectionBuffers(data->Vertex);
}

void ModeModelMeshVertex::DrawEffects(MultiView::Window *win)
{
	NixEnableLighting(false);
	foreach(ModelEffect &fx, data->Fx){
		vector p = win->Project(data->Vertex[fx.Vertex].pos);
		if ((p.z > 0) && (p.z < 1))
			ed->drawStr(p.x, p.y, fx.get_type());
	}
	NixEnableLighting(multi_view->light_enabled);
}

