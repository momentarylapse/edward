/*
 * ModeModelAnimationNone.h
 *
 *  Created on: 18.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONNONE_H_
#define MODEMODELANIMATIONNONE_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelAnimationNone: public Mode<DataModel>
{
public:
	ModeModelAnimationNone(ModeBase *parent);
	virtual ~ModeModelAnimationNone();

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o, const string &message);
	virtual void onUpdateMenu();

	virtual void onDrawWin(MultiView::Window *win);
};

extern ModeModelAnimationNone *mode_model_animation_none;

#endif /* MODEMODELANIMATIONNONE_H_ */
