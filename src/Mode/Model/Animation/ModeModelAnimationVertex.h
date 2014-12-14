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

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	virtual void onDrawWin(MultiView::Window *win);

	void UpdateVertices();
};

extern ModeModelAnimationVertex *mode_model_animation_vertex;

#endif /* MODEMODELANIMATIONVERTEX_H_ */
