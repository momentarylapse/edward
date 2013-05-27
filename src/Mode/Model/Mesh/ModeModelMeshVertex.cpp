/*
 * ModeModelMeshVertex.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMesh.h"
#include "ModeModelMeshVertex.h"
#include "ModeModelMeshPolygon.h"
#include "../../../lib/nix/nix.h"

ModeModelMeshVertex *mode_model_mesh_vertex = NULL;

ModeModelMeshVertex::ModeModelMeshVertex(Mode *_parent, DataModel *_data) :
	Mode("ModelMeshVertex", _parent, _data, ed->multi_view_3d, "menu_model")
{
	data = _data;
}

ModeModelMeshVertex::~ModeModelMeshVertex()
{
}

void ModeModelMeshVertex::OnStart()
{
	Subscribe(data);
	Subscribe(multi_view, "SelectionChange");
	mode_model_mesh->ApplyRightMouseFunction(multi_view);
	multi_view->MVRectable = true;
	OnUpdate(data);
}

void ModeModelMeshVertex::OnEnd()
{
	multi_view->ResetData(NULL);
	Unsubscribe(data);
	Unsubscribe(multi_view);
}



void ModeModelMeshVertex::OnDrawWin(MultiViewWindow *win)
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
		multi_view->ResetData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);
		multi_view->SetData(	MVDModelVertex,
				data->Vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect | MultiView::FlagMove,
				NULL, NULL);
	}else if (o->GetName() == "MultiView"){
		data->SelectionFromVertices();
	}
	mode_model_mesh_polygon->FillSelectionBuffers();
}

void ModeModelMeshVertex::DrawEffects(MultiViewWindow *win)
{
	NixEnableLighting(false);
	foreach(ModelEffect &fx, data->Fx){
		vector p = win->Project(data->Vertex[fx.Vertex].pos);
		if ((p.z > 0) && (p.z < 1))
			ed->DrawStr(p.x, p.y, fx.get_type());
	}
	NixEnableLighting(multi_view->light_enabled);
}

