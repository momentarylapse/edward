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


class ModeModel: public Mode<DataModel>
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

	void ExecutePropertiesDialog();

	ModelPropertiesDialog *PropertiesDialog;
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
