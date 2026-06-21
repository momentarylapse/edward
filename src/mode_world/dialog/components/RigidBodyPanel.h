//
// Created by michi on 9/30/25.
//

#pragma once

#include "ComponentPanel.h"

class DataWorld;


class RigidBodyPanel : public ComponentContentsPanel {
public:
	explicit RigidBodyPanel(DataWorld* _data, int _index);
	void update_ui() override;
	void on_edit();
};
