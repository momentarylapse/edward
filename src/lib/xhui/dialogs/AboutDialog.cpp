//
// Created by michi on 10/30/25.
//

#include "AboutDialog.h"
#include <lib/xhui/xhui.h>

#include "../config.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"
#include "lib/xhui/Application.h"

namespace xhui {

AboutDialog::AboutDialog(Panel* parent) : Dialog("About", 400, 400, parent) {
	from_source(R"foo(
Dialog about-dialog 'About' width=400 height=400
	Grid ? '' spacing=-2
		Label ? '' expandy
		---|
		Image logo ''
		---|
		Label ? '' expandy
		---|
		Label program-name '' bold big center expandx
		---|
		Label comment '' center
		---|
		Label version '' center disabled
		---|
		Label copyright '' center disabled
		---|
		Label website '' center
)foo");
	set_string("logo", Application::get_property("logo-big"));
	set_string("program-name", Application::get_property("name"));
	set_string("comment", Application::get_property("comment"));
	set_string("version", Application::get_property("version"));
	set_string("copyright", Application::get_property("copyright"));
	set_string("website", Application::get_property("website"));
	/*auto g1 = new Grid("");
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
	});*/
	event(event_id::Close, [this] {
		request_destroy();
	});
}


void AboutDialog::show(Panel* parent) {
	auto dlg = new AboutDialog(parent);
	parent->open_dialog(dlg).then([dlg] {
	});
}
}
