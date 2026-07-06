//
// Created by michi on 10/1/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;
class InstanceEditor;

class UserComponentPanel : public ComponentContentsPanel {
public:
	explicit UserComponentPanel(DataWorld* data, int index, const kaba::Class* type);
	const kaba::Class* type;
	shared<InstanceEditor> editor;

	void update_ui() override;
};

