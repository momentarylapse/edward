/*
 * ModeModelSkeleton.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODEMODELSKELETON_H_
#define MODEMODELSKELETON_H_

#include "../../Mode.h"
#include "../../../Data/Model/DataModel.h"

class DataModel;

class ModeModelSkeleton: public Mode<DataModel>
{
public:
	ModeModelSkeleton(ModeBase *_parent);
	virtual ~ModeModelSkeleton();

	virtual void onStart();
	virtual void onEnd();

	virtual void onCommand(const string &id);
	virtual void onUpdate(Observable *o);
	virtual void onUpdateMenu();

	virtual void onDraw();
	virtual void onDrawWin(MultiView::Window *win);
};

extern ModeModelSkeleton *mode_model_skeleton;

#endif /* MODEMODELSKELETON_H_ */
