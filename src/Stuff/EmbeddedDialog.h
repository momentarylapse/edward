/*
 * EmbeddedDialog.h
 *
 *  Created on: 16.09.2013
 *      Author: michi
 */

#ifndef EMBEDDEDDIALOG_H_
#define EMBEDDEDDIALOG_H_

#include "../lib/hui/hui.h"

class EmbeddedDialog : public HuiEventHandler
{
public:
	EmbeddedDialog(HuiWindow *win, const string &target_id, int x, int y, const string &options = "");
	EmbeddedDialog(HuiWindow *win, const string &dialog_id, const string &target_id, int x, int y, const string &options = "");
	void Init(HuiWindow *win, const string &target_id, int x, int y, const string &options);
	virtual ~EmbeddedDialog();

	void enable(const string &id, bool enabled);
	void check(const string &id, bool checked);
	bool isChecked(const string &id);

	void reset(const string &id);
	void setString(const string &id, const string &str);
	void addString(const string &id, const string &str);
	void setFloat(const string &id, float f);
	void setInt(const string &id, int i);
	string getString(const string &id);
	string getCell(const string &id, int row, int col);
	float getFloat(const string &id);
	int getInt(const string &id);

	HuiWindow *win;
	string uid;
};

#endif /* EMBEDDEDDIALOG_H_ */
