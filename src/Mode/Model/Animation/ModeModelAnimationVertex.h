/*
 * ModeModelAnimationVertex.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONVERTEX_H_
#define MODEMODELANIMATIONVERTEX_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelAnimationVertex: public Mode<DataModel>
{
public:
	ModeModelAnimationVertex(ModeBase *parent);
	virtual ~ModeModelAnimationVertex();

	void onStart() override;
	void onEnd() override;

	void onCommand(const string &id) override;
	void onUpdate(Observable *o, const string &message) override;
	void onUpdateMenu() override;

	void onDrawWin(MultiView::Window *win) override;

	void updateVertices();
	void chooseMouseFunction(int f);

	int mouse_action;
};

extern ModeModelAnimationVertex *mode_model_animation_vertex;

#endif /* MODEMODELANIMATIONVERTEX_H_ */
