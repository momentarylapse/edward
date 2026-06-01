/*
 * ActionMultiView.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "ActionMultiView.h"

ActionMultiView::ActionMultiView() {
	mat = mat4::ID;
}

void ActionMultiView::abort(history::Data *d) {
	undo(d);
}

void ActionMultiView::abort_and_notify(history::Data *d) {
	abort(d);
	d->out_changed.notify();
}

void ActionMultiView::update_and_notify(history::Data *d, const mat4 &m) {
	abort(d);
	mat = m;
	execute(d);
	d->out_changed.notify();
}



