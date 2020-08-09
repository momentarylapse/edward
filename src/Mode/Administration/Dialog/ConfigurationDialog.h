/*
 * ConfigurationDialog.h
 *
 *  Created on: 23.08.2012
 *      Author: michi
 */

#ifndef CONFIGURATIONDIALOG_H_
#define CONFIGURATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
class DataAdministration;
class GameIniData;

class ConfigurationDialog: public hui::Dialog {
public:
	ConfigurationDialog(hui::Window *_parent, DataAdministration *_data, bool _exporting);

	void load_data();
	void into_game_init(GameIniData &g);

	void on_close();
	void on_find_root_dir();
	void on_find_world();
	void on_find_second_world();
	void on_find_script();
	void on_find_material();
	void on_find_font();
	void on_ok();


private:
	DataAdministration *data;
	bool exporting;
};

#endif /* CONFIGURATIONDIALOG_H_ */
