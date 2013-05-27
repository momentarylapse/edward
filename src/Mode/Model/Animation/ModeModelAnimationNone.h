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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	CHuiWindow *dialog;
};

extern ModeModelAnimationNone *mode_model_animation_none;

#endif /* MODEMODELANIMATIONNONE_H_ */
