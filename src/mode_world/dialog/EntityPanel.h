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

	void update(bool force);
	void update_xxx(int next, bool force);
};

