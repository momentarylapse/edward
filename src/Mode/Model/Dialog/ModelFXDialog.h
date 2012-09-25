/*
 * ModelFXDialog.h
 *
 *  Created on: 26.09.2012
 *      Author: michi
 */

#ifndef MODELFXDIALOG_H_
#define MODELFXDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"

class ModelFXDialog: public CHuiWindow
{
public:
	ModelFXDialog(CHuiWindow *_parent, bool _allow_parent, DataModel *_data, int _type);
	virtual ~ModelFXDialog();

	void LoadData();
	void ApplyData();

	void OnFindScriptFile();
	void OnFindSoundFile();
	void OnClose();
	void OnOk();

private:
	DataModel *data;
	ModelEffect temp;
};

#endif /* MODELFXDIALOG_H_ */
