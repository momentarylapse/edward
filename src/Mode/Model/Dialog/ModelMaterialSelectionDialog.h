/*
 * ModelMaterialSelectionDialog.h
 *
 *  Created on: 14.03.2012
 *      Author: michi
 */

#ifndef MODELMATERIALSELECTIONDIALOG_H_
#define MODELMATERIALSELECTIONDIALOG_H_

#include "../../../lib/hui/hui.h"
#include "../../../Stuff/Observer.h"
class DataModel;

class ModelMaterialSelectionDialog: public hui::Dialog, public Observer {
public:
	ModelMaterialSelectionDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data);
	virtual ~ModelMaterialSelectionDialog();

	void on_close();
	void on_apply();
	void on_material_list_right_click();
	void on_material_add();
	void on_material_load();
	void on_material_edit();
	void on_material_delete();

	void on_update(Observable *o, const string &message) override;

	void fill_material_list();

	void put_answer(int *_answer);
private:
	int *answer;
	DataModel *data;
	hui::Menu *popup_materials;
};

#endif /* MODELMATERIALSELECTIONDIALOG_H_ */
