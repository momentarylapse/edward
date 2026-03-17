//
// Created by Michael Ankele on 2025-04-18.
//

#ifndef MODELMATERIALSELECTIONDIALOG_H
#define MODELMATERIALSELECTIONDIALOG_H

#include <lib/xhui/Dialog.h>

namespace yrenderer {
	class Material;
}

class Session;

class ModelMaterialSelectionDialog : public xhui::Dialog {
public:
	explicit ModelMaterialSelectionDialog(Session* session, const Array<yrenderer::Material*>& internal_materials);

	Session* session;
	base::promise<yrenderer::Material*> promise;

	Array<yrenderer::Material*> materials;

	static base::future<yrenderer::Material*> ask(Session* session, const Array<yrenderer::Material*>& internal_materials);
};



#endif //MODELMATERIALSELECTIONDIALOG_H
