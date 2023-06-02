/*
 * LightmapDialog.h
 *
 *  Created on: 19.05.2013
 *      Author: michi
 */

#ifndef LIGHTMAPDIALOG_H_
#define LIGHTMAPDIALOG_H_

#include "../../../lib/hui/hui.h"
class DataWorld;
class LightmapData;

class LightmapDialog: public hui::Dialog
{
public:
	LightmapDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~LightmapDialog();

	void SetData();
	void FillList();

	void OnType();
	void OnResolution();
	void OnFindNewWorld();
	void OnOk();
	void OnPreview();
	void OnClose();

private:
	DataWorld *data;
	LightmapData *lmd;
};

#endif /* LIGHTMAPDIALOG_H_ */
