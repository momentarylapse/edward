/*
 * ModelEasifyDialog.h
 *
 *  Created on: 26.08.2012
 *      Author: michi
 */

#ifndef MODELEASIFYDIALOG_H_
#define MODELEASIFYDIALOG_H_

#include "../../../lib/hui/hui.h"
class DataModel;

class ModelEasifyDialog: public hui::Dialog {
public:
	ModelEasifyDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data);

	void load_data();

	void on_quality_factor();
	void on_quality_slider();
	void on_close();
	void on_ok();

private:
	DataModel *data;
	float factor;
};

#endif /* MODELEASIFYDIALOG_H_ */
