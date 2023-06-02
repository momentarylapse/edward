/*
 * ModelDuplicateAnimationDialog.h
 *
 *  Created on: 26.12.2014
 *      Author: michi
 */

#ifndef MODELDUPLICATEANIMATIONDIALOG_H_
#define MODELDUPLICATEANIMATIONDIALOG_H_

#include "../../../lib/hui/hui.h"
class DataModel;

class ModelDuplicateAnimationDialog: public hui::Dialog {
public:
	ModelDuplicateAnimationDialog(hui::Window *_parent, bool _allow_parent, DataModel *_data, int index, int source);

	void on_close();
	void on_ok();

private:
	DataModel *data;
	int source;
};

#endif /* MODELDUPLICATEANIMATIONDIALOG_H_ */
