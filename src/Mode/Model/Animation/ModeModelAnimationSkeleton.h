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

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o);
	virtual void onUpdateMenu();

	virtual void onDrawWin(MultiView::Window *win);
};

extern ModeModelAnimationSkeleton *mode_model_animation_skeleton;

#endif /* MODEMODELANIMATIONSKELETON_H_ */
