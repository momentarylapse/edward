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

class Mode;
class DataModel;

class ModeModelAnimationNone: public Mode
{
public:
	ModeModelAnimationNone(Mode *_parent, DataModel *_data);
	virtual ~ModeModelAnimationNone();

	void OnStart();
	void OnEnd();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void OnDrawWin(MultiViewWindow *win);

	DataModel *data;
	Data *GetData(){	return data;	}

	CHuiWindow *dialog;
};

extern ModeModelAnimationNone *mode_model_animation_none;

#endif /* MODEMODELANIMATIONNONE_H_ */
