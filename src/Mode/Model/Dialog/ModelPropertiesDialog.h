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

class ModelPropertiesDialog: public hui::Dialog, public Observer
{
public:
	ModelPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelPropertiesDialog();

	void LoadData();
	void ApplyData();

	void restart();
	bool active;

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
	void OnScriptVarEdit();
	void OnScriptFind();
	void OnOk();
	void OnClose();

	void on_update(Observable *o, const string &message) override;

private:
	DataModel *data;

	DataModel::MetaData temp;
};

#endif /* MODELPROPERTIESDIALOG_H_ */
