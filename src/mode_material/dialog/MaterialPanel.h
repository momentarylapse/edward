//
// Created by Michael Ankele on 2025-04-30.
//

#ifndef MATERIALPANEL_H
#define MATERIALPANEL_H


#include "../ModeMaterial.h"
#include <lib/xhui/xhui.h>
#include <lib/pattern/Observable.h>

class DataMaterial;
class MaterialParameterPanel;
namespace xhui {
class Menu;
}

class MaterialPanel: public obs::Node<xhui::Panel> {
	friend class MaterialPassPanel;
public:
	explicit MaterialPanel(ModeMaterial* mode);
	~MaterialPanel() override;

	void load_data();


private:
	DataMaterial* data;
	ModeMaterial* mode_material();
	MaterialParameterPanel* material_parameter_panel;
	int apply_queue_depth;
};



#endif //MATERIALPANEL_H
