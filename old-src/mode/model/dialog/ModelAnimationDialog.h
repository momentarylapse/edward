/*
 * ModelAnimationDialog.h
 *
 *  Created on: 13.08.2012
 *      Author: michi
 */

#ifndef MODELANIMATIONDIALOG_H_
#define MODELANIMATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../lib/pattern/Observable.h"

class DataModel;
class ModeModelAnimation;

class ModelAnimationDialog: public obs::Node<hui::Panel> {
public:
	ModelAnimationDialog(DataModel *_data);
	virtual ~ModelAnimationDialog();

	void load_data();

	void fill_animation();
	void on_animation_list();
	void on_animation_list_select();
	void on_animation_list_right_click();
	void on_add_animation();
	void on_copy_animation();
	void on_delete_animation();
	void on_frame();
	void on_add_frame();
	void on_delete_frame();
	void on_name();
	void on_fps_const();
	void on_fps_factor();
	void on_speed();
	void on_parameter();
	void on_simulation_play();
	void on_simulation_stop();

	int get_selected_animation();

	int get_first_free_index();

private:
	ModeModelAnimation *mode_model_animation;
	DataModel *data;
	hui::Menu *popup;
};

#endif /* MODELANIMATIONDIALOG_H_ */
