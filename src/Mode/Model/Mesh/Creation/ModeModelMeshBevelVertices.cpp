/*
 * ModeModelMeshBevelVertices.cpp
 *
 *  Created on: 23.09.2012
 *      Author: michi
 */

#include "ModeModelMeshBevelVertices.h"
#include "../../ModeModel.h"
#include "../../../../Edward.h"
#include "../../../../lib/x/x.h"

ModeModelMeshBevelVertices::ModeModelMeshBevelVertices(Mode *_parent) :
	ModeCreation("ModelMeshBevelVertices", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = _("Radius skalieren [Shift + Return]");

	radius = 20 / multi_view->zoom;
}

ModeModelMeshBevelVertices::~ModeModelMeshBevelVertices()
{
}

void ModeModelMeshBevelVertices::OnMouseMove()
{
	radius += (HuiGetEvent()->dx) / multi_view->zoom;
}

void ModeModelMeshBevelVertices::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN)
		if (data->GetNumSelectedVertices() > 0){
			data->BevelSelectedVertices(radius);
			Abort();
		}
}

void ModeModelMeshBevelVertices::OnDrawWin(int win, irect dest)
{
	mode_model->SetMaterialCreation();
	foreach(ModelVertex &v, data->Vertex)
		if (v.is_selected)
			FxDrawBall(v.pos, radius, 16,32);
	NixEnableLighting(false);
	ed->DrawStr(100, 100, f2s(radius, 3));
}


