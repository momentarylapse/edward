//
// Created by michi on 3/30/26.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;

class LinkPanel : public ComponentContentsPanel {
public:
	explicit LinkPanel(DataWorld* _data, int _index);

	void update_ui() override;
	void on_edit();
};
