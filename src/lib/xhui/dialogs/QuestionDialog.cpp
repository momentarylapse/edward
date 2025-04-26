//
// Created by Michael Ankele on 2025-01-26.
//

#include "QuestionDialog.h"

#include <lib/xhui/xhui.h>

#include "../controls/Button.h"
#include "../controls/Grid.h"

namespace xhui {

QuestionDialog::QuestionDialog(Panel* parent, const string& title, const string& question, bool allow_cancel) : Dialog(title, 450, 120, parent) {
	auto g1 = new Grid("");
	add_child(g1);
	auto label = new Label("", question);
	label->align = Label::Align::Center;
	g1->add_child(label, 0, 0);
	label->size_mode_y = SizeMode::Expand;
	auto g2 = new Grid("");
	g1->add_child(g2, 0, 1);
	g2->add_child(new Button("yes", "Yes"), 0, 0);
	g2->add_child(new Button("no", "No"), 1, 0);
	if (allow_cancel)
		g2->add_child(new Button("cancel", "Cancel"), 2, 0);

	event("yes", [this] {
		answer = Answer::Yes;
		request_destroy();
	});
	event("no", [this] {
		answer = Answer::No;
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event(event_id::Close, [this] {
		request_destroy();
	});
}


base::future<Answer> QuestionDialog::ask(Panel* parent, const string& title, const string& question, bool allow_cancel) {
	auto dlg = new QuestionDialog(parent, title, question, allow_cancel);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}



} // xhui