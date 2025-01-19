/*
 * Mode.cpp
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#include "Mode.h"
#include "../multiview/MultiView.h"
#include "../Session.h"

ModeBase::ModeBase(Session *_session, const string &_name, ModeBase *_parent, MultiView::MultiView *_multi_view, const string &_menu) {
	session = _session;
	parent_untyped = _parent;
	name = _name;
	multi_view = _multi_view;
	menu_id = _menu;
	/*if ((!menu) && (parent))
		menu = parent->menu;*/
}

ModeBase::~ModeBase() {
}

void ModeBase::on_update_menu_recursive() {
	if (parent_untyped)
		parent_untyped->on_update_menu_recursive();
	on_update_menu();
}

void ModeBase::on_command_recursive(const string &id) {
	if (parent_untyped)
		parent_untyped->on_command_recursive(id);
	on_command(id);
}

void ModeBase::optimize_view_recursice() {
	if (optimize_view())
		return;
	if (parent_untyped)
		parent_untyped->optimize_view_recursice();
}



ModeBase *ModeBase::get_root() {
	if (parent_untyped)
		return parent_untyped->get_root();
	return this;
}

bool ModeBase::is_ancestor_of(ModeBase *m) {
	if (m == this)
		return true;
	if (m->parent_untyped)
		return is_ancestor_of(m->parent_untyped);
	return false;
}


ModeBase *ModeBase::get_next_child_to(ModeBase *target) {
	while(target) {
		if (this == target->parent_untyped)
			return target;
		target = target->parent_untyped;
	}
	return nullptr;
}


bool ModeBase::equal_roots(ModeBase *m) {
	if (!m)
		return false;
	return get_root() == m->get_root();
}

ModeBase *ModeBase::find_mode_base(const string &name) {
	return session->find_mode_base(name);
}





