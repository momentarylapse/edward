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
#include "../../../../MultiView/DrawingHelper.h"


ModeModelMeshCreatePolygon::ModeModelMeshCreatePolygon(ModeBase *_parent) :
	ModeCreation<DataModel>("ModelMeshCreatePolygon", _parent)
{
	message = format(_("Polygon w&ahlen: %d -> Shift + Return"), 0);

	mode_model_mesh->set_selection_mode(mode_model_mesh->selection_mode_vertex);
}


void ModeModelMeshCreatePolygon::on_start()
{
	for (ModelVertex &v: data->vertex)
		v.is_special = false;

	multi_view->set_allow_select(false);
	multi_view->set_allow_action(false);
}



void ModeModelMeshCreatePolygon::on_end()
{
	for (ModelVertex &v: data->vertex)
		v.is_special = false;
}


void ModeModelMeshCreatePolygon::on_draw_win(MultiView::Window *win)
{
	parent->on_draw_win(win);

	nix::SetColor(multi_view->ColorCreationLine);
	set_wide_lines(2);
	for (int i=1;i<selection.num;i++){
		vector pa = data->vertex[selection[i - 1]].pos;
		vector pb = data->vertex[selection[i    ]].pos;
		nix::DrawLine3D(pa, pb);
	}
	if (selection.num > 0){
		if (multi_view->hover.index >= 0)
			nix::DrawLine3D(data->vertex[selection.back()].pos, data->vertex[multi_view->hover.index].pos);
		else
			nix::DrawLine3D(data->vertex[selection.back()].pos, multi_view->get_cursor());
	}
}



void ModeModelMeshCreatePolygon::on_command(const string &id)
{
	if (id == "finish-action"){
		data->execute(new ActionModelAddPolygonAutoSkin(selection, mode_model_mesh->current_material));
		abort();
	}
}


void ModeModelMeshCreatePolygon::on_left_button_down()
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
					ed->set_message(_("keine doppelten Punkte erlaubt!"));
					abort();
					return;
				}

		// all ok -> add
		selection.add(multi_view->hover.index);

	}else{
		data->addVertex(multi_view->get_cursor());
		selection.add(data->vertex.num - 1);
	}
	data->vertex[selection.back()].is_special = true;
	message = format(_("Polygon w&ahlen: %d -> [Ctrl + Return]"), selection.num);
}

