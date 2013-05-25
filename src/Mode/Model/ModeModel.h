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

	virtual void OnStart();
	virtual void OnEnd();
	virtual void OnEnter();

	virtual void OnCommand(const string &id);
	virtual void OnUpdateMenu();


	virtual void New();
	virtual bool Open();
	virtual bool Save();
	virtual bool SaveAs();

	bool ImportOpen3ds();
	bool ImportLoad3ds(const string &filename);

	void SetMaterialSelected();
	void SetMaterialMouseOver();
	void SetMaterialCreation();

	void ExecutePropertiesDialog(int initial_tab_page = 0);
	void ExecuteMaterialDialog(int initial_tab_page = 0, bool allow_parent = true);

	DataModel *data;
	virtual Data *GetData(){	return data;	}

	ModelPropertiesDialog *PropertiesDialog;
	ModelMaterialDialog *MaterialDialog;
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
