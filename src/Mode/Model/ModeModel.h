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

	bool importOpen3ds();
	bool importLoad3ds(const string &filename);

	bool exportSaveJson();
	bool exportWriteJson(const string &filename);

	void setMaterialSelected();
	void setMaterialHover();
	void setMaterialCreation();

	void executePropertiesDialog();

	ModelPropertiesDialog *properties_dialog;

	void allowSelectionModes(bool allow);
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
