/*
 * System.h
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#pragma once

#include "Entity.h"
#include "EntityManager.h"
#include <lib/pattern/Observable.h>

namespace ecs {

struct MessageParams;

class System : public obs::Node<BaseClass> {
public:
	System();
	~System() override;

	obs::xsink<MessageParams> in_add_component{this, &System::on_add_component};
	obs::xsink<MessageParams> in_remove_component{this, &System::on_remove_component};

	virtual void on_input() {}
	virtual void on_key_down(int k) {}
	virtual void on_key_up(int k) {}
	virtual void on_key(int k) {}
	virtual void on_left_button_down() {}
	virtual void on_left_button_up() {}
	virtual void on_middle_button_down() {}
	virtual void on_middle_button_up() {}
	virtual void on_right_button_down() {}
	virtual void on_right_button_up() {}
	virtual void on_iterate_pre(float dt) {}
	virtual void on_draw_pre() {}

	virtual void on_render_inject() {}
	virtual void on_render_inject2() {}

	virtual void on_add_component(const MessageParams& params) {}
	virtual void on_remove_component(const MessageParams& params) {}

	int channel = -1;
	EntityManager* entity_manager = nullptr;
};

}
