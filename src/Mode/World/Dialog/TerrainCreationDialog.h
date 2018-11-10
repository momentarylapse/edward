/*
 * TerrainCreationDialog.h
 *
 *  Created on: 14.06.2012
 *      Author: michi
 */

#ifndef TERRAINCREATIONDIALOG_H_
#define TERRAINCREATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Data/World/DataWorld.h"
#include "../../../Stuff/Observer.h"

class TerrainCreationDialog: public hui::Dialog, public Observer
{
public:
	TerrainCreationDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~TerrainCreationDialog();

	void LoadData();
	void ApplyData();

	void OnFindHeightmap();
	void OnSizeChange();
	void OnOk();
	void OnClose();

	void on_update(Observable *o, const string &message) override;

private:
	DataWorld *data;
};

#endif /* TERRAINCREATIONDIALOG_H_ */
