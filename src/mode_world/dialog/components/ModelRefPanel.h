//
// Created by michi on 10/1/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;
class MaterialSelector;

class ModelRefPanel : public ComponentContentsPanel {
public:
	explicit ModelRefPanel(DataWorld* _data, int _index);
	void update_ui() override;
	MaterialSelector* material_selector;
};
