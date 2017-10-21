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

class AdministrationDialog: public hui::Dialog, public Observer
{
public:
	AdministrationDialog(hui::Window *_parent, bool _allow_parent, DataAdministration *_data);
	virtual ~AdministrationDialog();

	void LoadData();

	void onUpdate(Observable *o, const string &message);

	void OnClose();
	void OnExit();
	void OnRename();
	void OnDelete();
	void OnEdit();
	void OnFileList();
	void OnRudimentaryConfiguration();
	void OnExportGame();

	Array<AdminFile*> GetSelectedFiles();
	Array<AdminFile*> GetSelectedFilesFromList(const string &lid);
	AdminFile* GetSingleSelectedFile();


	void FillAdminList(int view, const string &lid);
	void ShowDetail(int n, const string &lid);
	AdminFileList *get_list(const string &lid);


private:
	DataAdministration *data;

	Array<AdminFileList> file_list;
	AdminFile *SelectedAdminFile;
};

#endif /* ADMINISTRATIONDIALOG_H_ */
