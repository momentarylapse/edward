#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

namespace history {
	class Data;
}
class Session;
class MaterialParameterPanel;
namespace yrenderer {
	class Material;
}


class MaterialSelector : public obs::Node<xhui::Panel> {
public:
	explicit MaterialSelector(Session* s, history::Data* _data);
	void set_material(yrenderer::Material* m);
	obs::xsource<yrenderer::Material*> out_selected{this, "selected"};
	history::Data* data;
	Session* session;
	yrenderer::Material* material = nullptr;
	MaterialParameterPanel* edit_panel;
	Array<yrenderer::Material*> internal_materials;
};

