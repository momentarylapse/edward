//
// Created by michi on 10/1/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;

class UserComponentPanel : public ComponentContentsPanel {
public:
	explicit UserComponentPanel(DataWorld* data, int index, const kaba::Class* type);
	const kaba::Class* type;

	void update_ui() override;
	void on_edit();
};

