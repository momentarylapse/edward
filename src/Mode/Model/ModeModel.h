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

class Mode;
class DataModel;

class ModeModel: public Mode
{
public:
	ModeModel();
	virtual ~ModeModel();

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
	void OnDataChange();

	void Draw();
	void DrawWin(int win, irect dest);

	DataModel *data;
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
