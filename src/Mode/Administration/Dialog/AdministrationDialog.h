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
typedef Array<AdminFile*> AdminFileList;

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

	AdminFileList file_list_cur; // files to show (not including source/dest-files in detailed viewing mode)
	AdminFileList file_list_all; // the same order as the list view
	AdminFileList file_list_detail_source, file_list_detail_dest;
	AdminFileList file_list_missing;
	AdminFileList file_list_super;
	AdminFile *SelectedAdminFile;
};

#endif /* ADMINISTRATIONDIALOG_H_ */
