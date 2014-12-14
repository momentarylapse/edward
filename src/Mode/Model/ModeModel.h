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

	virtual void onStart();
	virtual void onEnd();
	virtual void onEnter();

	virtual void onCommand(const string &id);
	virtual void onUpdateMenu();


	virtual void _new();
	virtual bool open();
	virtual bool save();
	virtual bool saveAs();

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
