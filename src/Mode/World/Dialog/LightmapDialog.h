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

class LightmapDialog: public CHuiWindow
{
public:
	LightmapDialog(CHuiWindow *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~LightmapDialog();

	void OnOk();
	void OnClose();

private:
	DataWorld *data;
};

#endif /* LIGHTMAPDIALOG_H_ */
