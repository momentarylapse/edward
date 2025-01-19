/*
 * ModelPropertiesDialog.h
 *
 *  Created on: 13.03.2012
 *      Author: michi
 */

#ifndef MODELPROPERTIESDIALOG_H_
#define MODELPROPERTIESDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../data/model/DataModel.h"

class ModelPropertiesDialog: public hui::Dialog {
public:
	ModelPropertiesDialog(hui::Window *_parent, DataModel *_data);
	virtual ~ModelPropertiesDialog();

	void load_data();
	void apply_data();

	void refill_inventary_list();
	void refill_script_var_list();
	void fill_detail_list();
	void fill_tensor_list();
	void on_generate_dists_auto();
	void on_physics_passive();
	void on_generate_tensor_auto();
	void on_tensor_edit();
	void on_num_items();
	void on_model_inventary();
	void on_delete_item();
	void on_script_var_edit();
	void on_script_find();
	void on_ok();
	void on_close();

private:
	DataModel *data;

	DataModel::MetaData temp;
};

#endif /* MODELPROPERTIESDIALOG_H_ */
