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

class Mode;
class DataModel;

class ModeModelAnimation: public Mode
{
public:
	ModeModelAnimation(Mode *_parent, DataModel *_data);
	virtual ~ModeModelAnimation();

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void Draw();
	void DrawWin(int win, irect dest);

	DataModel *data;
};

extern ModeModelAnimation *mode_model_animation;

#endif /* MODEMODELANIMATION_H_ */
