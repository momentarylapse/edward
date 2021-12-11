/*
 * ModeModel.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODEMODEL_H_
#define MODEMODEL_H_

#include "../Mode.h"
#include "../../Data/Model/DataModel.h"


class ModeModel: public Mode<DataModel> {
public:
	ModeModel(MultiView::MultiView *mv3, MultiView::MultiView *mv2);
	virtual ~ModeModel();

	void on_start() override;
	void on_end() override;
	void on_enter() override;

	void on_command(const string &id) override;
	void on_update_menu() override;


	void _new();
	bool open();
	bool save();
	bool save_as();

	bool import_open_3ds();
	bool import_load_3ds(const Path &filename);

	bool import_open_json();
	bool import_load_json(const Path &filename);

	bool import_open_ply();
	bool import_load_ply(const Path &filename);

	bool export_save_json();
	bool export_write_json(const Path &filename);

	static void set_material_selected();
	static void set_material_hover();
	static void set_material_creation(float intensity = 1.0f);

	void run_properties_dialog();

	void allow_selection_modes(bool allow);
};

extern ModeModel* mode_model;

#endif /* MODEMODEL_H_ */
