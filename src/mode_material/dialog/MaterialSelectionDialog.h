#pragma once

#include <lib/xhui/Dialog.h>

namespace yrenderer {
	class Material;
}

class Session;

class MaterialSelectionDialog : public xhui::Dialog {
public:
	explicit MaterialSelectionDialog(Session* session, const string& title, const Array<yrenderer::Material*>& internal_materials, bool allow_new, bool allow_none);

	Session* session;
	base::promise<yrenderer::Material*> promise;

	Array<yrenderer::Material*> materials;

	static yrenderer::Material* new_material;
	static base::future<yrenderer::Material*> ask(Session* session, const string& title, const Array<yrenderer::Material*>& internal_materials, bool allow_new, bool allow_none);
};

