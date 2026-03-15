//
// Created by michi on 10/1/25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

class DataWorld;
class MaterialEditPanel;

class TerrainRefPanel : public obs::Node<xhui::Panel> {
public:
	explicit TerrainRefPanel(DataWorld* _data, int _index);
	DataWorld* data;
	int index;
	MaterialEditPanel* material_edit_panel;

	void update_ui();
};
