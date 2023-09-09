/*
 * WorldPropertiesDialog.h
 *
 *  Created on: 11.06.2012
 *      Author: michi
 */

#ifndef WORLDPROPERTIESDIALOG_H_
#define WORLDPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../lib/pattern/Observable.h"
#include "../../../data/world/DataWorld.h"

class WorldPropertiesDialog: public obs::Node<hui::Dialog> {
public:
	WorldPropertiesDialog(hui::Window *_parent, bool _allow_parent, DataWorld *_data);
	virtual ~WorldPropertiesDialog();

	void load_data();
	void apply_data();

	void fill_skybox_list();
	void fill_script_list();
	void on_fog_mode_none();
	void on_fog_mode_linear();
	void on_fog_mode_exp();
	void on_fog_enabled();
	void on_skybox_right_click();
	void on_skybox_move();
	void on_skybox_add();
	void on_skybox_select();
	void on_skybox_remove();
	void on_physics_enabled();
	void on_script_right_click();
	void on_script_add();
	void on_create_script();
	void on_script_remove();
	void on_edit_script_vars();
	void on_edit_script();
	void on_ok();
	void on_close();

	void restart();

	bool active;

private:
	owned<hui::Menu> popup_skybox;
	owned<hui::Menu> popup_script;
	DataWorld *data;

	DataWorld::MetaData temp;
};

#endif /* WORLDPROPERTIESDIALOG_H_ */
