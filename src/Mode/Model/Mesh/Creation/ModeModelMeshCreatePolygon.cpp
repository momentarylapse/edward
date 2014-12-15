/*
 * ModeModelMeshCreatePolygon.cpp
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#include "ModeModelMeshCreatePolygon.h"
#include "../ModeModelMesh.h"
#include "../../../../Edward.h"
#include "../../../../Action/Model/Mesh/Polygon/ActionModelAddPolygonAutoSkin.h"
#include "../../../../lib/nix/nix.h"
#include "../../../../MultiView/MultiView.h"
#include "../../../../MultiView/Window.h"

ModeModelMeshCreatePolygon::ModeModelMeshCreatePolygon(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreatePolygon", _parent)
{
	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), 0);
}

ModeModelMeshCreatePolygon::~ModeModelMeshCreatePolygon()
{
}


void ModeModelMeshCreatePolygon::onStart()
{
	foreach(ModelVertex &v, data->vertex)
		v.is_special = false;
}



void ModeModelMeshCreatePolygon::onEnd()
{
	foreach(ModelVertex &v, data->vertex)
		v.is_special = false;
}


void ModeModelMeshCreatePolygon::onDrawWin(MultiView::Window *win)
{
	for (int i=1;i<selection.num;i++){
		NixEnableLighting(false);
		vector pa = win->project(data->vertex[selection[i - 1]].pos);
		vector pb = win->project(data->vertex[selection[i    ]].pos);
		NixSetColor(Green);
		if ((pa.z >= 0) and (pa.z < 1) and (pb.z >= 0) and (pb.z <= 1))
			NixDrawLine(pa.x, pa.y, pb.x, pb.y, 0);
		NixSetColor(White);
		NixEnableLighting(true);
	}
}



void ModeModelMeshCreatePolygon::onKeyDown()
{
	if (HuiGetEvent()->key_code == KEY_SHIFT + KEY_RETURN){
		data->execute(new ActionModelAddPolygonAutoSkin(selection, mode_model_mesh->current_material));
		abort();
	}
}


void ModeModelMeshCreatePolygon::onLeftButtonDown()
{
	if (multi_view->hover.index >= 0){
		// closed loop -> done
		if (selection.num > 0)
			if (multi_view->hover.index == selection[0]){
				data->execute(new ActionModelAddPolygonAutoSkin(selection, mode_model_mesh->current_material));
				abort();
				return;
			}

		// consistency?
		foreachi(int s, selection, i)
			if (s == multi_view->hover.index)
				if (i > 0){
					ed->setMessage(_("keine doppelten Punkte erlaubt!"));
					abort();
					return;
				}

		// all ok -> add
		selection.add(multi_view->hover.index);

	}else{
		data->AddVertex(multi_view->GetCursor3d());
		selection.add(data->vertex.num - 1);
	}
	data->vertex[selection.back()].is_special = true;
	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), selection.num);
}

