//
// Created by michi on 9/19/25.
//

#include "DocumentSession.h"
#include "Mode.h"
#include "EdwardWindow.h"
#include "MultiView.h"
#include <data/Data.h>
#include <Session.h>
#include <lib/yrenderer/target/XhuiRenderer.h>

DocumentSession::DocumentSession(Session* _session) {
	session = _session;

	mode_none = nullptr;
#if 0
	mode_none = new ModeNone(this);
	cur_mode = mode_none;
	progress = new Progress;
#endif

	mode_model = nullptr;
//	mode_admin = nullptr;
//	mode_font = nullptr;
	mode_material = nullptr;
	mode_world = nullptr;
	mode_coding = nullptr;

	//panel = new MultiViewPanel(this);
	base_panel = new xhui::Panel(p2s(this));
	base_panel->propagate_events = true;
	base_panel->add_control("Grid", "", 0, 0, "base-grid");
}

DocumentSession::~DocumentSession() {
#if 0
	if (mode_world)
		delete mode_world;
	/*delete mode_material;
	delete mode_model;
	delete mode_font;
	delete mode_admin;*/

	// saving the configuration data...
#endif
}

void DocumentSession::set_document_panel(xhui::Panel* panel) {
	document_panel = panel;
	base_panel->embed("base-grid", 0, 0, panel);
}



// do we change roots?
//  -> data loss?
base::future<void> mode_switch_allowed(Mode *m) {
	//	if (!m->session->cur_mode or m->equal_roots(m->session->cur_mode)) {
	base::promise<void> promise;
	promise();
	return promise.get_future();
	//	} else {
	//		return m->session->allow_termination();
	//	}
}


void DocumentSession::set_mode(Mode *m) {
	if (cur_mode == m)
		return;
	mode_switch_allowed(m).then([this, m] {
		set_mode_now(m);
	});
}

Mode* get_next_child_to(Mode* from, Mode* to) {
	if (!from)
		return to->get_root();
	if (from->is_ancestor_of(to))
		return to;
	return get_next_child_to(from, to->get_parent());
}

void DocumentSession::set_mode_now(Mode *m) {
	if (cur_mode == m)
		return;

	auto win = session->win;

	// end current
	if (cur_mode) {
		//msg_write("LEAVE");
		cur_mode->on_leave();
		if (cur_mode->get_data()) {
			cur_mode->get_data()->unsubscribe(win);
			if (cur_mode->multi_view)
				cur_mode->get_data()->unsubscribe(cur_mode->multi_view);
			cur_mode->get_data()->action_manager->unsubscribe(win);
		}
		if (cur_mode->multi_view) {
			cur_mode->multi_view->unsubscribe(win);
//			win->renderer->children.pop();
		}
		cur_mode->unsubscribe(win);
	}

	// close current modes up
	while (cur_mode) {
		if (cur_mode->is_ancestor_of(m))
			break;
		//msg_write("UP");
		cur_mode->on_leave_rec();
		cur_mode = cur_mode->get_parent();
	}

	// start new modes down
	while (cur_mode != m) {
		cur_mode = get_next_child_to(cur_mode, m);
		//msg_write("DOWN");
		cur_mode->on_enter_rec();
	}

	//win->update_menu();


	// start new
	//msg_write("ENTER");
	cur_mode = m;
	cur_mode->on_enter();
//	win->renderer->children.clear();
//	win->renderer->add_child(cur_mode->multi_view->renderer.get());

	cur_mode->out_redraw >> win->in_redraw;
	if (cur_mode->multi_view) {
		cur_mode->multi_view->out_selection_changed >> win->in_redraw;
		cur_mode->multi_view->view_port.out_changed >> win->in_data_selection_changed;
	}
	if (cur_mode->get_data()) {
		cur_mode->get_data()->out_changed >> win->in_data_changed;
		if (cur_mode->multi_view)
			cur_mode->get_data()->out_changed >> cur_mode->multi_view->in_data_changed;
		auto *am = cur_mode->get_data()->action_manager;
		am->out_failed >> win->in_action_failed;
		am->out_saved >> win->in_saved;
	}

	out_changed();
	win->request_redraw();
}

string DocumentSession::title() const {
	if (cur_mode)
		return str(cur_mode->generic_data->filename.absolute().relative_to(session->project_dir));
	return "no file";
}



