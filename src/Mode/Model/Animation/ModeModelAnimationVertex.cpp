/*
 * ModeModelAnimationVertex.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationVertex.h"
#include "ModeModelAnimation.h"
#include "../ModeModel.h"
#include "../Skeleton/ModeModelSkeleton.h"
#include "../../../Edward.h"
#include "../../../MultiView/MultiView.h"
#include "../Mesh/Selection/MeshSelectionModePolygon.h"

ModeModelAnimationVertex *mode_model_animation_vertex = NULL;

ModeModelAnimationVertex::ModeModelAnimationVertex(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationVertex", _parent, ed->multi_view_3d, "menu_move")
{
	mouse_action = -1;
}

ModeModelAnimationVertex::~ModeModelAnimationVertex()
{
}

void ModeModelAnimationVertex::on_start()
{
	ed->toolbar[hui::TOOLBAR_LEFT]->set_by_id("model-animation-vertex-toolbar");

	mode_model->allow_selection_modes(true);

	chooseMouseFunction(MultiView::ACTION_SELECT);

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	on_update(data, "");
}

void ModeModelAnimationVertex::on_end()
{
	unsubscribe(data);
	unsubscribe(multi_view);
	data->showVertices(data->vertex);
	mode_model_mesh->fill_selection_buffer(data->vertex);
}

void ModeModelAnimationVertex::on_command(const string& id)
{
	if (id == "select")
		chooseMouseFunction(MultiView::ACTION_SELECT);
	if (id == "translate")
		chooseMouseFunction(MultiView::ACTION_MOVE);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ACTION_ROTATE);
	if (id == "scale")
		chooseMouseFunction(MultiView::ACTION_SCALE);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ACTION_MIRROR);
}

void ModeModelAnimationVertex::chooseMouseFunction(int f)
{
	mouse_action = f;

	// mouse action
	multi_view->setMouseAction("ActionModelAnimationTransformVertices", mouse_action, false);
}

void ModeModelAnimationVertex::on_update(Observable* o, const string &message)
{
	if (o == data){
		data->showVertices(mode_model_animation->vertex);
		mode_model_mesh->selection_mode->updateMultiView();
	}else if (o == multi_view){
		mode_model_mesh->selection_mode->updateSelection();
	}
}

void ModeModelAnimationVertex::on_update_menu()
{
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	ed->check("scale", mouse_action == MultiView::ACTION_SCALE);
	ed->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}

void ModeModelAnimationVertex::on_draw_win(MultiView::Window *win)
{
	mode_model_mesh->draw_all(win, mode_model_animation->vertex);
}

void ModeModelAnimationVertex::updateVertices()
{
	// deprecated by mode_model_animation->vertex
	/*vertex.resize(data->Vertex.num);
	foreachi(ModelVertex &v, vertex, i)
		v.pos = data->Vertex[i].AnimatedPos;*/
}



