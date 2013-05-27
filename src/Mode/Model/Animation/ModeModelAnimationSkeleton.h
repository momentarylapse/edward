/*
 * ModeModelAnimationSkeleton.h
 *
 *  Created on: 16.08.2012
 *      Author: michi
 */

#ifndef MODEMODELANIMATIONSKELETON_H_
#define MODEMODELANIMATIONSKELETON_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelAnimationSkeleton: public Mode<DataModel>
{
public:
	ModeModelAnimationSkeleton(ModeBase *parent);
	virtual ~ModeModelAnimationSkeleton();

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDrawWin(MultiViewWindow *win);

	CHuiWindow *dialog;
};

extern ModeModelAnimationSkeleton *mode_model_animation_skeleton;

#endif /* MODEMODELANIMATIONSKELETON_H_ */
