//
// Created by michi on 6/22/26.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;
class MaterialSelector;

class TemplateRefPanel : public ComponentContentsPanel {
public:
	explicit TemplateRefPanel(DataWorld* _data, int _index);
	void update_ui() override;
};
