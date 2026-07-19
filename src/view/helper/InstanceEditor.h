//
// Created by michi on 7/7/26.
//

#pragma once

#include <y/ecs/BaseClass.h>
#include <lib/xhui/Panel.h>
#include <functional>



class Session;

namespace kaba {
	struct Class;
}

class InstanceEditor : public xhui::Panel {
public:
	Session* session;
	const kaba::Class* type;
	ecs::InstanceData description;
	using Func = std::function<void(const ecs::InstanceData&)>;
	Func f_on_edit;

	InstanceEditor(Session* _session, const kaba::Class* _type, const Func& _f_on_edit);
	void build(void* data);
	void update_ui(void* data);
	void on_edit();
};
