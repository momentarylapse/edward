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

	void Start();
	void End();

	void OnMouseMove();
	void OnLeftButtonDown();
	void OnLeftButtonUp();
	void OnMiddleButtonDown();
	void OnMiddleButtonUp();
	void OnRightButtonDown();
	void OnRightButtonUp();
	void OnKeyDown();
	void OnKeyUp();
	void OnCommand(const string &id);
	void OnUpdate(Observable *o);
	void OnUpdateMenu();

	void Draw();
	void DrawWin(int win, irect dest);


	void New();
	bool Open();
	bool Save();
	bool SaveAs();

	void SetMaterialSelected();
	void SetMaterialMouseOver();
	void SetMaterialCreation();

	void ExecutePropertiesDialog(int initial_tab_page = 0);
	void ExecuteMaterialDialog(int initial_tab_page = 0);

	DataModel *data;
	ModelPropertiesDialog *PropertiesDialog;
	ModelMaterialDialog *MaterialDialog;
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
