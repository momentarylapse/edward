//
// Created by Michael Ankele on 2025-04-18.
//

#ifndef MODELMATERIALSELECTIONDIALOG_H
#define MODELMATERIALSELECTIONDIALOG_H

#include <lib/xhui/Dialog.h>


class ModeMesh;
class DataModel;

class ModelMaterialSelectionDialog : public xhui::Dialog {
public:
	explicit ModelMaterialSelectionDialog(ModeMesh* mode_mesh);

	ModeMesh* mode_mesh;
	base::promise<int> promise;

	static base::future<int> ask(ModeMesh* mode_mesh);
};



#endif //MODELMATERIALSELECTIONDIALOG_H
