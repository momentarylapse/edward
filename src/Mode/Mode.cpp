/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"
#include "../MultiView/MultiView.h"

ModeBase::ModeBase(const string &_name, ModeBase *_parent, MultiView::MultiView *_multi_view, const string &_menu) {
	parent = _parent;
	name = _name;
	multi_view = _multi_view;
	menu_id = _menu;
	/*if ((!menu) && (parent))
		menu = parent->menu;*/
}

ModeBase::~ModeBase() {
}

void ModeBase::on_update_menu_recursive() {
	if (parent)
		parent->on_update_menu_recursive();
	on_update_menu();
}

void ModeBase::on_command_recursive(const string &id) {
	if (parent)
		parent->on_command_recursive(id);
	on_command(id);
}

void ModeBase::optimize_view_recursice() {
	if (optimize_view())
		return;
	if (parent)
		parent->optimize_view_recursice();
}



ModeBase *ModeBase::get_root() {
	if (parent)
		return parent->get_root();
	return this;
}

bool ModeBase::is_ancestor_of(ModeBase *m) {
	if (m == this)
		return true;
	if (m->parent)
		return is_ancestor_of(m->parent);
	return false;
}


ModeBase *ModeBase::get_next_child_to(ModeBase *target) {
	while(target) {
		if (this == target->parent)
			return target;
		target = target->parent;
	}
	return NULL;
}


bool ModeBase::equal_roots(ModeBase *m) {
	if (!m)
		return false;
	return get_root() == m->get_root();
}





