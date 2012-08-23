/*
 * AdministrationDialog.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef ADMINISTRATIONDIALOG_H_
#define ADMINISTRATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"

class AdministrationDialog: public CHuiWindow, public Observer
{
public:
	AdministrationDialog(CHuiWindow *_parent, bool _allow_parent);
	virtual ~AdministrationDialog();

	void LoadData();

	void OnUpdate(Observable *o);


private:
};

#endif /* ADMINISTRATIONDIALOG_H_ */
