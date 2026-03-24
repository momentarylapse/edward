//
// Created by Michael Ankele on 2025-01-26.
//

#include "QuestionDialog.h"

#include <lib/xhui/xhui.h>

#include "../controls/Button.h"
#include "../controls/Grid.h"

namespace xhui {

QuestionDialog::QuestionDialog(Panel* parent, const string& title, const string& question, bool allow_cancel) : Dialog(title, 450, 120, parent) {
	from_source(R"foodelim(
Dialog question-dialog ''
	Grid ? ''
		Label question '' expandx expandy center
		---|
		Grid ? ''
			Label ? '' expandx
			Button yes 'Yes' primary
			Button no 'No' danger
			Button cancel 'Cancel'
)foodelim");
	set_title(title);
	set_string("question", question);
	set_visible("cancel", allow_cancel);

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