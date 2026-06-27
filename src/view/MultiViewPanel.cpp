//
// Created by michi on 6/15/26.
//

#include "MultiViewPanel.h"
#include "MultiView.h"
#include "Mode.h"
#include "DocumentSession.h"
#include "EdwardWindow.h"
#include <Session.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/controls/DrawingArea.h>
#include <lib/yrenderer/target/XhuiRenderer.h>


MultiViewPanel::MultiViewPanel(DocumentSession* _doc, MultiView* mv) : xhui::Panel("multi-view-panel") {
	doc = _doc;
	from_source(R"foodelim(
Dialog multi-view-panel ''
	Grid ? ''
		Overlay ? ''
			DrawingArea area '' grabfocus
			Grid overlay-main-grid '' padding=25
				Grid overlay-button-grid-left '' spacing=20
				.
				Label ? '' ignorehover expandx
				Grid overlay-button-grid-right '' spacing=20
					Button cam-rotate 'R' 'tooltip=Drag button to rotate camera' image=rf-rotate height=50 width=50 padding=7 noexpandx ignorefocus
					---|
					Button cam-move 'M' 'tooltip=Drag button to move camera' image=rf-translate height=50 width=50 padding=7 noexpandx ignorefocus
)foodelim");
	propagate_events = true;

	win = doc->session->win;
	multi_view = nullptr;

	renderer = new yrenderer::XhuiRenderer(doc->session->ctx);
	set_multi_view(mv);

	win_events.add(win->event_xp(win->id, xhui::event_id::JustBeforeDraw, [this] (Painter* p) {
		if (!doc->cur_mode or !multi_view)
			return;
		if (doc != doc->session->cur_doc)
			return;
		if (auto da = static_cast<xhui::DrawingArea*>(get_control("area")))
			da->for_painter_do(static_cast<xhui::Painter*>(p), [this] (Painter* p) {
				multi_view->set_area(p->area());
				renderer->before_draw(p);
			});
	}));
	events.add(event_xp("area", xhui::event_id::Draw, [this] (Painter* p) {
		if (!doc->cur_mode or !multi_view)
			return;
		multi_view->set_area(p->area());
		renderer->draw(p);
		multi_view->on_draw(p);
		doc->cur_mode->on_draw_post(p);
	}));
	events.add(event_x("area", xhui::event_id::MouseMove, [this] {
		if (!doc->cur_mode or !multi_view)
			return;
		multi_view->on_mouse_move(win->state.m, win->state.m - win->state_prev.m);
		doc->cur_mode->on_mouse_move(win->state.m, win->state.m - win->state_prev.m);
		doc->cur_mode->out_redraw();
	}));
	events.add(event_x("area", xhui::event_id::MouseWheel, [this] {
		if (!doc->cur_mode or !multi_view)
			return;
		multi_view->on_mouse_wheel(win->state.m, win->state.scroll);
	}));
	events.add(event_x("area", xhui::event_id::MouseLeave, [this] {
		if (!doc->cur_mode or !multi_view)
			return;
		multi_view->on_mouse_leave();
		doc->cur_mode->on_mouse_leave(win->state.m);
	}));
	events.add(event_x("area", xhui::event_id::LeftButtonDown, [this] {
		if (!doc->cur_mode or !multi_view)
			return;
		multi_view->on_left_button_down(win->state.m);
		doc->cur_mode->on_left_button_down(win->state.m);
	}));
	events.add(event_x("area", xhui::event_id::LeftButtonUp, [this] {
		if (!doc->cur_mode or !multi_view)
			return;
		multi_view->on_left_button_up(win->state.m);
		doc->cur_mode->on_left_button_up(win->state.m);
	}));
	events.add(event_x("area", xhui::event_id::KeyDown, [this] {
		multi_view->on_key_down(win->state.key_code);
		doc->cur_mode->on_key_down(win->state.key_code);
	}));
	events.add(event_x("cam-move", xhui::event_id::LeftButtonDown, [this] {
		win->set_mouse_mode(0);
	}));
	events.add(event_x("cam-move", xhui::event_id::LeftButtonUp, [this] {
		win->set_mouse_mode(1);
	}));
	events.add(event_x("cam-move", xhui::event_id::MouseMove, [this] {
		vec2 d = win->state.m - win->state_prev.m;
		if (win->state.lbut) {
			if (win->is_key_pressed(xhui::KEY_SHIFT))
				multi_view->view_port.move(vec3(0,0,d.y) / 800.0f);
			else
				multi_view->view_port.move(vec3(-d.x, d.y, 0) / 800.0f);
		}
	}));
	events.add(event_x("cam-rotate", xhui::event_id::LeftButtonDown, [this] {
		win->set_mouse_mode(0);
	}));
	events.add(event_x("cam-rotate", xhui::event_id::LeftButtonUp, [this] {
		win->set_mouse_mode(1);
	}));
	events.add(event_x("cam-rotate", xhui::event_id::MouseMove, [this] {
		vec2 d = win->state.m - win->state_prev.m;
		if (win->state.lbut)
			multi_view->view_port.rotate(quaternion::rotation({d.y*0.003f, d.x*0.003f, 0}));
	}));
}

MultiViewPanel::~MultiViewPanel() {
	for (int eid: events)
		remove_event_handler(eid);
	for (int eid: win_events)
		win->remove_event_handler(eid);
}

void MultiViewPanel::set_multi_view(MultiView *mv) {
	if (multi_view)
		renderer->remove_child(multi_view->renderer.get());
	multi_view = mv;
	renderer->add_child(multi_view->renderer.get());
}

