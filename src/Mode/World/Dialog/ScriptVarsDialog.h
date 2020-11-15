/*
 * ScriptVarsDialog.h
 *
 *  Created on: 05.06.2018
 *      Author: michi
 */

#ifndef SRC_MODE_WORLD_DIALOG_SCRIPTVARSDIALOG_H_
#define SRC_MODE_WORLD_DIALOG_SCRIPTVARSDIALOG_H_

#include "../../../lib/hui/hui.h"

class WorldScript;

class ScriptVarsDialog: public hui::Dialog
{
public:
	ScriptVarsDialog(hui::Window *_parent, WorldScript *data);

	void load_data();
	void apply_data();

	void on_ok();
	void on_close();

private:
	WorldScript *data;
};

#endif /* SRC_MODE_WORLD_DIALOG_SCRIPTVARSDIALOG_H_ */
