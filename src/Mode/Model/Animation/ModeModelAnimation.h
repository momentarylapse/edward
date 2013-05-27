/*
 * ModeModelAnimation.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATION_H_
#define MODEMODELANIMATION_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelAnimation: public Mode<DataModel>
{
public:
	ModeModelAnimation(ModeBase *parent);
	virtual ~ModeModelAnimation();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	CHuiWindow *dialog;
};

extern ModeModelAnimation *mode_model_animation;

#endif /* MODEMODELANIMATION_H_ */
