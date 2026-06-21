//
// Created by michi on 9/30/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;

class LightPanel : public ComponentContentsPanel {
public:
	explicit LightPanel(DataWorld* _data, int _index);

	void update_ui() override;
	void on_edit();
};
