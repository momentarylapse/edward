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
class DataAdministration;
class AdminFile;
class AdminFileList;

class AdministrationDialog: public CHuiWindow, public Observer
{
public:
	AdministrationDialog(CHuiWindow *_parent, bool _allow_parent, DataAdministration *_data);
	virtual ~AdministrationDialog();

	void LoadData();

	void OnUpdate(Observable *o);

	void OnClose();


	void FillAdminList(int view, const string &lid);
	void ShowDetail(int n, const string &lid);
	AdminFileList *get_list(const string &lid);


private:
	DataAdministration *data;

	Array<AdminFileList> file_list;
	AdminFile *SelectedAdminFile;
};

#endif /* ADMINISTRATIONDIALOG_H_ */
