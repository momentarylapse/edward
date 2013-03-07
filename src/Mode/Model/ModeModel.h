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
#include "Dialog/ModelPropertiesDialog.h"
#include "Dialog/ModelMaterialDialog.h"

class Mode;
class DataModel;

class ModeModel: public Mode
{
public:
	ModeModel();
	virtual ~ModeModel();

	void OnStart();
	void OnEnd();
	void OnEnter();

	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	bool Import3ds();

	void SetMaterialSelected();
	void SetMaterialMouseOver();
	void SetMaterialCreation();

	void ExecutePropertiesDialog(int initial_tab_page = 0);
	void ExecuteMaterialDialog(int initial_tab_page = 0, bool allow_parent = true);

	DataModel *data;
	Data *GetData(){	return data;	}

	ModelPropertiesDialog *PropertiesDialog;
	ModelMaterialDialog *MaterialDialog;
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
