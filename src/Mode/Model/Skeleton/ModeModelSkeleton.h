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

class Mode;
class DataModel;

class ModeModelSkeleton: public Mode
{
public:
	ModeModelSkeleton(Mode *_parent, DataModel *_data);
	virtual ~ModeModelSkeleton();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDraw();
	void OnDrawWin(int win, irect dest);

	DataModel *data;
	Data *GetData(){	return data;	}
};

extern ModeModelSkeleton *mode_model_skeleton;

#endif /* MODEMODELSKELETON_H_ */
