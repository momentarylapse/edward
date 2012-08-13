/*
 * ModelAnimationDialog.h
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#ifndef MODELANIMATIONDIALOG_H_
#define MODELANIMATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Stuff/Observer.h"

class ModelAnimationDialog: public CHuiWindow, public Observer
{
public:
	ModelAnimationDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelAnimationDialog();

	void LoadData();
	void ApplyData();

	void FillAnimation();
	void OnAnimationList();
	void OnAddAnimation();
	void OnDeleteAnimation();
	void OnFrameInc();
	void OnFrameDec();
	void OnFrame();
	void OnAddFrame();
	void OnDeleteFrame();
	void OnName();
	void OnFpsConst();
	void OnFpsFactor();
	void OnSpeed();
	void OnParameter();
	void OnTabControl();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataModel *data;
};

#endif /* MODELANIMATIONDIALOG_H_ */
