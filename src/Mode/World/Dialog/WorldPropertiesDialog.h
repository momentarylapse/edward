/*
 * WorldPropertiesDialog.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef WORLDPROPERTIESDIALOG_H_
#define WORLDPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Stuff/Observer.h"

class WorldPropertiesDialog: public hui::Dialog, public Observer
{
public:
	WorldPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~WorldPropertiesDialog();

	void LoadData();
	void ApplyData();

	void FillSkyboxList();
	void FillScriptList();
	void OnSunEnabled();
	void OnSunAngFromCamera();
	void OnFogModeNone();
	void OnFogModeLinear();
	void OnFogModeExp();
	void OnFogEnabled();
	void on_skybox_right_click();
	void on_skybox_select();
	void on_skybox_remove();
	void OnPhysicsEnabled();
	void on_script_right_click();
	void OnAddScript();
	void OnRemoveScript();
	void OnEditScriptVars();
	void OnOk();
	void OnClose();

	void on_update(Observable *o, const string &message) override;
	void restart();

	bool active;

private:
	hui::Menu *popup_skybox;
	hui::Menu *popup_script;
	DataWorld *data;

	DataWorld::MetaData temp;
};

#endif /* WORLDPROPERTIESDIALOG_H_ */
