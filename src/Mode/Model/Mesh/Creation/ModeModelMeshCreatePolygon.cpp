/*
 * ModeModelMeshCreatePolygon.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreatePolygon.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../lib/nix/nix.h"

ModeModelMeshCreatePolygon::ModeModelMeshCreatePolygon(Mode *_parent) :
	ModeCreation("ModelMeshCreatePolygon", _parent)
{
	data = (DataModel*)_parent->GetData();

	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), 0);
}

ModeModelMeshCreatePolygon::~ModeModelMeshCreatePolygon()
{
}


void ModeModelMeshCreatePolygon::OnStart()
{
	foreach(ModelVertex &v, data->Vertex)
		v.is_special = false;
}



void ModeModelMeshCreatePolygon::OnEnd()
{
	foreach(ModelVertex &v, data->Vertex)
		v.is_special = false;
}


void ModeModelMeshCreatePolygon::OnDrawWin(int win, irect dest)
{
	for (int i=1;i<selection.num;i++){
		NixEnableLighting(false);
		vector pa = multi_view->VecProject(data->Vertex[selection[i - 1]].pos, win);
		vector pb = multi_view->VecProject(data->Vertex[selection[i    ]].pos, win);
		NixSetColor(Green);
		if ((pa.z >= 0) and (pa.z < 1) and (pb.z >= 0) and (pb.z <= 1))
			NixDrawLine(pa.x, pa.y, pb.x, pb.y, 0);
		NixSetColor(White);
		NixEnableLighting(true);
	}
}



void ModeModelMeshCreatePolygon::OnKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		data->Execute(new ActionModelAddPolygonAutoSkin(selection));
		Abort();
	}
}


void ModeModelMeshCreatePolygon::OnLeftButtonDown()
{
	if (multi_view->Selected >= 0){
		// closed loop -> done
		if (selection.num > 0)
			if (multi_view->Selected == selection[0]){
				data->Execute(new ActionModelAddPolygonAutoSkin(selection));
				Abort();
				return;
			}

		// consistency?
		foreachi(int s, selection, i)
			if (s == multi_view->Selected)
				if (i > 0){
					ed->SetMessage(_("keine doppelten Punkte erlaubt!"));
					Abort();
					return;
				}

		// all ok -> add
		selection.add(multi_view->Selected);

	}else{
		data->AddVertex(multi_view->GetCursor3d());
		selection.add(data->Vertex.num - 1);
	}
	data->Vertex[selection.back()].is_special = true;
	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), selection.num);
}

