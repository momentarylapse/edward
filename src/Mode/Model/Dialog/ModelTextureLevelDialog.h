/*
 * ModelTextureLevelDialog.h
 *
 *  Created on: 22.06.2012
 *      Author: michi
 */

#ifndef MODELTEXTURELEVELDIALOG_H_
#define MODELTEXTURELEVELDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Stuff/Observer.h"

class ModelTextureLevelDialog: public CHuiWindow, public Observer
{
public:
	ModelTextureLevelDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelTextureLevelDialog();

	void LoadData();
	void ApplyData();

	void FillTextureList();
	void OnTextureList();
	void OnTextureListCheck();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataModel *data;
};

#endif /* MODELTEXTURELEVELDIALOG_H_ */
