//
// Created by michi on 02.02.25.
//

#pragma once

#include <lib/xhui/Panel.h>
#include <lib/pattern/Observable.h>

namespace kaba {
	class Class;
}

struct WorldEntity;
class ModeWorld;

class EntityPanel : public obs::Node<xhui::Panel> {
public:
	explicit EntityPanel(ModeWorld* mode);

	ModeWorld* mode_world;
	int cur_entity_index = -1;
	const kaba::Class* preferred_type = nullptr;
	Array<const kaba::Class*> component_type_list;

	void update(bool force);
	void update_to_entity(int index, bool force);
};

