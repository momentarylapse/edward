//
// Created by michi on 10/1/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;
class MaterialSelector;

class TerrainRefPanel : public ComponentContentsPanel {
public:
	explicit TerrainRefPanel(DataWorld* _data, int _index);
	MaterialSelector* material_selector;

	void update_ui() override;
};
