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

	void on_start() override;
	void on_end() override;
	void on_enter() override;

	void on_command(const string &id) override;
	void on_update_menu() override;


	void _new() override;
	bool open() override;
	bool save() override;
	bool save_as() override;

	bool importOpen3ds();
	bool importLoad3ds(const string &filename);

	bool importOpenJson();
	bool importLoadJson(const string &filename);

	bool importOpenPly();
	bool importLoadPly(const string &filename);

	bool exportSaveJson();
	bool exportWriteJson(const string &filename);

	static void setMaterialSelected();
	static void setMaterialHover();
	static void setMaterialCreation(float intensity = 1.0f);

	void executePropertiesDialog();

	ModelPropertiesDialog *properties_dialog;

	void allowSelectionModes(bool allow);
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
