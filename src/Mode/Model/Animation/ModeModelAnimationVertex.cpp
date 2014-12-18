/*
 * ModeModelAnimationVertex.cpp
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#include "ModeModelAnimationVertex.h"
#include "ModeModelAnimation.h"
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

	multi_view->ClearData(NULL);
	multi_view->allow_rect = true;

	chooseMouseFunction(MultiView::ActionSelect);

	subscribe(data);
	subscribe(multi_view, multi_view->MESSAGE_SELECTION_CHANGE);
	onUpdate(data, "");
}

void ModeModelAnimationVertex::onEnd()
{
	unsubscribe(data);
	unsubscribe(multi_view);
	multi_view->ClearData(NULL);
}

void ModeModelAnimationVertex::onCommand(const string& id)
{
	if (id == "select")
		chooseMouseFunction(MultiView::ActionSelect);
	if (id == "translate")
		chooseMouseFunction(MultiView::ActionMove);
	if (id == "rotate")
		chooseMouseFunction(MultiView::ActionRotate);
	if (id == "scale")
		chooseMouseFunction(MultiView::ActionScale);
	if (id == "mirror")
		chooseMouseFunction(MultiView::ActionMirror);
}

void ModeModelAnimationVertex::chooseMouseFunction(int f)
{
	mouse_action = f;
	ed->updateMenu();

	// mouse action
	if (mouse_action != MultiView::ActionSelect){
		multi_view->SetMouseAction("ActionModelAnimationTransformVertices", mouse_action);
	}else{
		multi_view->SetMouseAction("", MultiView::ActionSelect);
	}
}

void ModeModelAnimationVertex::onUpdate(Observable* o, const string &message)
{
	if (o == data){
		updateVertices();

		multi_view->ClearData(data);
		//CModeAll::SetMultiViewViewStage(&ViewStage, false);

		multi_view->AddData(	MVDModelVertex,
				mode_model_animation->vertex,
				NULL,
				MultiView::FlagDraw | MultiView::FlagIndex | MultiView::FlagSelect);
	}else if (o == multi_view){
		foreachi(ModelVertex &v, data->vertex, i)
			v.is_selected = mode_model_animation->vertex[i].is_selected;
		data->SelectionFromVertices();
	}
}

void ModeModelAnimationVertex::onUpdateMenu()
{
	ed->check("select", mouse_action == MultiView::ActionSelect);
	ed->check("translate", mouse_action == MultiView::ActionMove);
	ed->check("rotate", mouse_action == MultiView::ActionRotate);
	ed->check("scale", mouse_action == MultiView::ActionScale);
	ed->check("mirror", mouse_action == MultiView::ActionMirror);
}

void ModeModelAnimationVertex::onDrawWin(MultiView::Window *win)
{
	msg_db_f("skin.DrawWin",4);

	mode_model_mesh->drawPolygons(win, mode_model_animation->vertex);
	NixSetShader(NULL);
	NixSetWire(false);
	NixSetZ(true,true);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(true);
	msg_db_m("----a",4);

	mode_model_mesh->setMaterialMarked();
	NixDraw3D(mode_model_mesh->vb_marked);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);
}

void ModeModelAnimationVertex::updateVertices()
{
	// deprecated by mode_model_animation->vertex
	/*vertex.resize(data->Vertex.num);
	foreachi(ModelVertex &v, vertex, i)
		v.pos = data->Vertex[i].AnimatedPos;*/
}



