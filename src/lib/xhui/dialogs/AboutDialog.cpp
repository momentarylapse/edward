//
// Created by michi on 10/30/25.
//

#include "AboutDialog.h"
#include "../controls/Button.h"
#include "../controls/Grid.h"
#include "../Application.h"
#include "../xhui.h"

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
		Button website '' center url
)foo");
	set_string("logo", Application::get_property("logo-big"));
	set_string("program-name", Application::get_property("name"));
	set_string("comment", Application::get_property("comment"));
	set_string("version", Application::get_property("version"));
	set_string("copyright", Application::get_property("copyright"));
	set_string("website", Application::get_property("website"));

	event("website", [] {
#ifdef OS_LINUX
		const string cmd = "xdg-open '" + Application::get_property("website") + "'";
		system(cmd.c_str());
#endif
	});
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
