/*
 * NewProjectDialog.h
 *
 *  Created on: 25.11.2020
 *      Author: michi
 */

#ifndef SRC_MODE_ADMINISTRATION_DIALOG_NEWPROJECTDIALOG_H_
#define SRC_MODE_ADMINISTRATION_DIALOG_NEWPROJECTDIALOG_H_

#include "../../../lib/hui/hui.h"

class NewProjectDialog : public hui::Dialog {
public:
	NewProjectDialog(hui::Window *parent);
	virtual ~NewProjectDialog();

	Path directory;
	int _template;
	bool ok;
};

#endif /* SRC_MODE_ADMINISTRATION_DIALOG_NEWPROJECTDIALOG_H_ */
