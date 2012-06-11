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

class WorldPropertiesDialog: public CHuiWindow, public Observer
{
public:
	WorldPropertiesDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~WorldPropertiesDialog();

	void LoadData();
	void ApplyData();

	void FillSkyboxList();
	void FillScriptVarList();
	void OnSunEnabled();
	void OnFogEnabled();
	void OnSkybox();
	void OnRemoveSkybox();
	void OnMaxScriptVars();
	void OnScriptVarEdit();
	void OnScriptVarTemplate();
	void OnOk();
	void OnClose();

	void OnUpdate(Observable *o);

private:
	DataWorld *data;

	DataWorld::MetaData temp;
};

#endif /* WORLDPROPERTIESDIALOG_H_ */
