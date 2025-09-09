//
// Created by michi on 02.02.25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

struct WorldEntity;
class ModeWorld;

class EntityPanel : public obs::Node<xhui::Panel> {
public:
	explicit EntityPanel(ModeWorld* mode);

	ModeWorld* mode_world;
	int cur_index = -1;

	shared<xhui::Panel> add_entity_panel;
	shared<xhui::Panel> entity_list_panel;

	void update(bool force);
};

