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
#include "ModeModelMeshTriangle.h"
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



void ModeModelMeshVertex::OnDrawWin(int win, irect dest)
{
	mode_model_mesh_triangle->OnDrawWin(win, dest);
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
		data->SelectionTrianglesFromVertices();
		data->SelectionSurfacesFromTriangles();
	}
	mode_model_mesh_triangle->FillSelectionBuffers();
}


