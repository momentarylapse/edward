//
// Created by Michael Ankele on 2025-04-26.
//

#include "CommonDialogs.h"
#include <lib/xhui/xhui.h>


IntPairDialog::IntPairDialog(xhui::Panel* parent, const string& question, int a, int b) : Dialog("Question", 320, 120, parent, xhui::DialogFlags::CloseByEscape) {
	from_source(R"foodelim(
Dialog int-pair-dialog 'Question'
	Grid ? ''
		Label question ''
		---|
		Grid ? ''
			SpinButton a '' range=0:9999:1 expandx
			SpinButton b '' range=0:9999:1 expandx
		---|
		Grid ? ''
			Button ok 'Ok'
			Button cancel 'Cancel'
)foodelim");
	set_string("question", question);
	set_int("a", a);
	set_int("b", b);
	event("ok", [this] {
		answer = {get_int("a"), get_int("b")};
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event(xhui::event_id::Close, [this] {
		request_destroy();
	});
}

base::future<base::tuple<int,int>> IntPairDialog::ask(xhui::Panel* parent, const string& question, int a, int b) {
	auto dlg = new IntPairDialog(parent, question, a, b);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}


FloatDialog::FloatDialog(xhui::Panel* parent, const string& question, float f) : Dialog("Question", 320, 120, parent, xhui::DialogFlags::CloseByEscape) {
	from_source(R"foodelim(
Dialog float-dialog 'Question'
	Grid ? ''
		Label question ''
		---|
		SpinButton value '' range=-9999:9999:0.001 expandx
		---|
		Grid ? ''
			Button ok 'Ok'
			Button cancel 'Cancel'
)foodelim");
	set_string("question", question);
	set_float("value", f);
	event("ok", [this] {
		answer = get_float("value");
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event(xhui::event_id::Close, [this] {
		request_destroy();
	});
}

base::future<float> FloatDialog::ask(xhui::Panel* parent, const string& question, float f) {
	auto dlg = new FloatDialog(parent, question, f);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}



TextDialog::TextDialog(xhui::Panel* parent, const string& question, const string& s) : Dialog("Question", 320, 120, parent, xhui::DialogFlags::CloseByEscape) {
	from_source(R"foodelim(
Dialog text-dialog 'Question'
	Grid ? ''
		Label question ''
		---|
		Edit value '' expandx
		---|
		Grid ? ''
			Button ok 'Ok'
			Button cancel 'Cancel'
)foodelim");
	set_string("question", question);
	set_string("value", s);
	event("ok", [this] {
		answer = get_string("value");
		request_destroy();
	});
	event("cancel", [this] {
		request_destroy();
	});
	event(xhui::event_id::Close, [this] {
		request_destroy();
	});
}

base::future<string> TextDialog::ask(xhui::Panel* parent, const string& question, const string& s) {
	auto dlg = new TextDialog(parent, question, s);
	parent->open_dialog(dlg).then([dlg] {
		if (dlg->answer)
			dlg->promise(*dlg->answer);
		else
			dlg->promise.fail();
	});
	return dlg->promise.get_future();
}

