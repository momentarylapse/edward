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

	virtual void OnStart();
	virtual void OnEnd();

	virtual void OnCommand(const string &id);
	virtual void OnUpdate(Observable *o);
	virtual void OnUpdateMenu();

	virtual void OnDraw();
	virtual void OnDrawWin(MultiView::Window *win);
};

extern ModeModelSkeleton *mode_model_skeleton;

#endif /* MODEMODELSKELETON_H_ */
