/*
 * ModelPropertiesDialog.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef MODELPROPERTIESDIALOG_H_
#define MODELPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/Model/DataModel.h"
#include "../../../Stuff/Observer.h"

class ModelPropertiesDialog: public HuiWindow, public Observer
{
public:
	ModelPropertiesDialog(HuiWindow *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelPropertiesDialog();

	void LoadData();
	void ApplyData();

	void RefillInventaryList();
	void RefillScriptVarList();
	void FillDetailList();
	void FillTensorList();
	void OnGenerateDistsAuto();
	void OnPhysicsPassive();
	void OnGenerateTensorAuto();
	void OnTensorEdit();
	void OnNumItems();
	void OnModelInventary();
	void OnDeleteItem();
	void OnMaxScriptVars();
	void OnScriptVarEdit();
	void OnScriptFind();
	void OnModelScriptVarTemplate();
	void OnOk();
	void OnClose();

	void onUpdate(Observable *o);

private:
	DataModel *data;

	DataModel::MetaData temp;
};

#endif /* MODELPROPERTIESDIALOG_H_ */
