/*
 * ConfigurationDialog.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef CONFIGURATIONDIALOG_H_
#define CONFIGURATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"
class DataAdministration;

class ConfigurationDialog: public hui::Dialog, public Observer
{
public:
	ConfigurationDialog(hui::Window *_parent, bool _allow_parent, DataAdministration *_data, bool _exporting);
	virtual ~ConfigurationDialog();

	void LoadData();

	void on_update(Observable *o, const string &message) override;

	void OnClose();
	void OnFindRootdir();
	void OnFindDefaultWorld();
	void OnFindDefaultSecondWorld();
	void OnFindDefaultScript();
	void OnFindDefaultMaterial();
	void OnFindDefaultFont();
	void OnOk();


private:
	DataAdministration *data;
	bool exporting;
};

#endif /* CONFIGURATIONDIALOG_H_ */
