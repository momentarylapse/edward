//
// Created by michi on 10/1/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;

class EntityBasePanel : public ComponentContentsPanel {
public:
	explicit EntityBasePanel(DataWorld* _data, int _index);
	void update_ui() override;
	void on_edit();
};
