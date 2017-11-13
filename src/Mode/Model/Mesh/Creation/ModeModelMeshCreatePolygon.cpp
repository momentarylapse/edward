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


namespace MultiView{
	extern nix::Shader *shader_lines_3d;
};


ModeModelMeshCreatePolygon::ModeModelMeshCreatePolygon(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreatePolygon", _parent)
{
	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), 0);

	mode_model_mesh->setSelectionMode(mode_model_mesh->selection_mode_vertex);
}


void ModeModelMeshCreatePolygon::onStart()
{
	for (ModelVertex &v: data->vertex)
		v.is_special = false;
}



void ModeModelMeshCreatePolygon::onEnd()
{
	for (ModelVertex &v: data->vertex)
		v.is_special = false;
}


void ModeModelMeshCreatePolygon::onDrawWin(MultiView::Window *win)
{
	parent->onDrawWin(win);

	nix::SetColor(Green);
	nix::SetShader(MultiView::shader_lines_3d);
	for (int i=1;i<selection.num;i++){
		vector pa = data->vertex[selection[i - 1]].pos;
		vector pb = data->vertex[selection[i    ]].pos;
		nix::DrawLine3D(pa, pb);
	}
	if (selection.num > 0){
		if (multi_view->hover.index >= 0)
			nix::DrawLine3D(data->vertex[selection.back()].pos, data->vertex[multi_view->hover.index].pos);
		else
			nix::DrawLine3D(data->vertex[selection.back()].pos, multi_view->getCursor3d());
	}
	nix::SetColor(White);
}



void ModeModelMeshCreatePolygon::onKeyDown(int key_code)
{
	if (key_code == hui::KEY_SHIFT + hui::KEY_RETURN){
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
		data->addVertex(multi_view->getCursor3d());
		selection.add(data->vertex.num - 1);
	}
	data->vertex[selection.back()].is_special = true;
	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), selection.num);
}

