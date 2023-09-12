/*
 * ModeModel.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODEL_H_
#define MODEMODEL_H_

#include "../Mode.h"
#include "../../data/model/DataModel.h"

class ModeModelMesh;
class ModeModelSkeleton;
class ModeModelAnimation;

class ModeModel: public Mode<ModeModel, DataModel> {
public:
	ModeModel(Session *s, MultiView::MultiView *mv3, MultiView::MultiView *mv2);

	void on_start() override;
	void on_end() override;
	void on_enter() override;

	void on_command(const string &id) override;
	void on_update_menu() override;


	void _new();
	void open();
	void save();
	void save_as();

	void import_open_3ds();
	bool import_load_3ds(const Path &filename);

	void import_open_json();
	bool import_load_json(const Path &filename);

	void import_open_ply();
	bool import_load_ply(const Path &filename);

	void export_save_json();
	bool export_write_json(const Path &filename);

	void run_properties_dialog();

	void allow_selection_modes(bool allow);

	ModeModelMesh *mode_model_mesh;
	ModeModelSkeleton *mode_model_skeleton;
	ModeModelAnimation *mode_model_animation;
};

#endif /* MODEMODEL_H_ */
