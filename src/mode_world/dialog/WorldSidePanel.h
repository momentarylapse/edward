//
// Created by michi on 04/10/2025.
//

#pragma once


#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

struct WorldEntity;
class ModeWorld;

class WorldSidePanel : public obs::Node<xhui::Panel> {
public:
	explicit WorldSidePanel(ModeWorld* mode);

	ModeWorld* mode_world;

	xhui::Panel* current_panel = nullptr;
	shared<xhui::Panel> add_entity_panel;
	shared<xhui::Panel> entity_list_panel;
	shared<xhui::Panel> entity_panel;

	void update(bool force);
};

