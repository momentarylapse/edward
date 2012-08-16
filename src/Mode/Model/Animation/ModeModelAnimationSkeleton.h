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

class Mode;
class DataModel;

class ModeModelAnimationSkeleton: public Mode
{
public:
	ModeModelAnimationSkeleton(Mode *_parent, DataModel *_data);
	virtual ~ModeModelAnimationSkeleton();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}

	CHuiWindow *dialog;
};

extern ModeModelAnimationSkeleton *mode_model_animation_skeleton;

#endif /* MODEMODELANIMATIONSKELETON_H_ */
