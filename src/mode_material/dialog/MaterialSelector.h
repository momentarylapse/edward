#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class Data;
class Session;
class MaterialParameterPanel;
namespace yrenderer {
	class Material;
}


class MaterialSelector : public obs::Node<xhui::Panel> {
public:
	MaterialSelector(Data* _data);
	void set_material(yrenderer::Material* m);
	obs::xsource<yrenderer::Material*> out_selected{this, "selected"};
	Data* data;
	Session* session;
	yrenderer::Material* material = nullptr;
	MaterialParameterPanel* edit_panel;
};

