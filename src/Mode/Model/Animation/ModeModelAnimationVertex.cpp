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
#include "../Mesh/MeshSelectionModePolygon.h"

ModeModelAnimationVertex *mode_model_animation_vertex = NULL;

ModeModelAnimationVertex::ModeModelAnimationVertex(ModeBase* _parent) :
	Mode<DataModel>("ModelAnimationVertex", _parent, ed->multi_view_3d, "menu_move")
{
}

ModeModelAnimationVertex::~ModeModelAnimationVertex()
{
}

void ModeModelAnimationVertex::onStart()
{
	string dir = (HuiAppDirectoryStatic + "Data/icons/toolbar/").sys_filename();
	HuiToolbar *t = ed->toolbar[HuiToolbarLeft];
	t->reset();
	t->addSeparator();
	t->addItemCheckable(_("Selektieren"), dir + "rf_select.png", "select");
	t->addItemCheckable(_("Verschieben"), dir + "rf_translate.png", "translate");
	t->addItemCheckable(_("Rotieren"), dir + "rf_rotate.png", "rotate");
	t->addItemCheckable(_("Skalieren"), dir + "rf_scale.png", "scale");
	t->addItemCheckable(_("Spiegeln"),dir + "rf_mirror.png", "mirror");
	t->enable(true);
	t->configure(false,true);
	mode_model->allowSelectionModes(true);

	multi_view->allow_rect = true;

	chooseMouseFunction(MultiView::ACTION_SELECT);

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	onUpdate(data, "");
}

void ModeModelAnimationVertex::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
	data->showVertices(data->vertex);
	mode_model_mesh->fillSelectionBuffer(data->vertex);
}

void ModeModelAnimationVertex::onCommand(const string& id)
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
	if (mouse_action != MultiView::ACTION_SELECT){
		multi_view->setMouseAction("ActionModelAnimationTransformVertices", mouse_action);
	}else{
		multi_view->setMouseAction("", MultiView::ACTION_SELECT);
	}
}

void ModeModelAnimationVertex::onUpdate(Observable* o, const string &message)
{
	if (o == data){
		updateVertices();
		data->showVertices(mode_model_animation->vertex);

		/*multi_view->clearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->addData(	MVD_MODEL_VERTEX,
				mode_model_animation->vertex,
				NULL,
				MultiView::FLAG_DRAW | MultiView::FLAG_INDEX | MultiView::FLAG_SELECT);*/

		mode_model_mesh->selection_mode->updateMultiView();
	}else if (o == multi_view){
		mode_model_mesh->selection_mode->updateSelection();
		/*foreachi(ModelVertex &v, data->vertex, i)
			v.is_selected = mode_model_animation->vertex[i].is_selected;
		data->SelectionFromVertices();*/
	}
}

void ModeModelAnimationVertex::onUpdateMenu()
{
	ed->check("select", mouse_action == MultiView::ACTION_SELECT);
	ed->check("translate", mouse_action == MultiView::ACTION_MOVE);
	ed->check("rotate", mouse_action == MultiView::ACTION_ROTATE);
	ed->check("scale", mouse_action == MultiView::ACTION_SCALE);
	ed->check("mirror", mouse_action == MultiView::ACTION_MIRROR);
}

void ModeModelAnimationVertex::onDrawWin(MultiView::Window *win)
{
	mode_model_mesh->drawAll(win, mode_model_animation->vertex);
}

void ModeModelAnimationVertex::updateVertices()
{
	// deprecated by mode_model_animation->vertex
	/*vertex.resize(data->Vertex.num);
	foreachi(ModelVertex &v, vertex, i)
		v.pos = data->Vertex[i].AnimatedPos;*/
}



