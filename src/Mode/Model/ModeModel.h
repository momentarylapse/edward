/*
 * ModeModel.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODEL_H_
#define MODEMODEL_H_

#include "../Mode.h"
#include "../../Data/Model/DataModel.h"

class ModeModel: public Mode
{
public:
	ModeModel();
	virtual ~ModeModel();

	virtual void Start();
	virtual void End();

	virtual void OnMouseMove();
	virtual void OnLeftButtonDown();
	virtual void OnLeftButtonUp();
	virtual void OnMiddleButtonDown();
	virtual void OnMiddleButtonUp();
	virtual void OnRightButtonDown();
	virtual void OnRightButtonUp();
	virtual void OnKeyDown();
	virtual void OnKeyUp();
	virtual void OnCommand(const string &id);
	virtual void OnDataChange();

	virtual void Draw();

	DataModel *data;
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
