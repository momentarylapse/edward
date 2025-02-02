//
// Created by Michael Ankele on 2025-01-26.
//

#include "QuestionDialog.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"

namespace xhui {

QuestionDialog::QuestionDialog(Panel* parent, const string& title, const string& question, bool allow_cancel) : Dialog(title, 450, 120, parent) {
	auto g1 = new Grid("");
	add(g1);
	auto label = new Label("", question);
	g1->add(label, 0, 0);
	label->size_mode_y = SizeMode::Expand;
	auto g2 = new Grid("");
	g1->add(g2, 0, 1);
	g2->add(new Button("yes", "Yes"), 0, 0);
	g2->add(new Button("no", "No"), 1, 0);
	if (allow_cancel)
		g2->add(new Button("cancel", "Cancel"), 2, 0);

	event("yes", [this] {
		promise(Answer::Yes);
		request_destroy();
	});
	event("no", [this] {
		promise(Answer::No);
		request_destroy();
	});
	event("cancel", [this] {
		promise(Answer::Cancel);
		request_destroy();
	});
}


base::future<Answer> QuestionDialog::ask(Panel* parent, const string& title, const string& question, bool allow_cancel) {
	auto dlg = new QuestionDialog(parent, title, question, allow_cancel);
	return dlg->promise.get_future();
}



} // xhui