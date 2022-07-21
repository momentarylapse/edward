/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

static Progress *_progress_;

Progress::Progress() :
	dlg(nullptr)
{
	_progress_ = this;
}

Progress::~Progress() {
}


void Progress::set(const string &str, float progress) {
	if (!dlg)
		return;
	time_running += timer.get();
	if (time_running < 5) {
		dlg->set_string("progress_bar", str);
	} else {
		float eta = time_running / progress * (1 - progress);
		if (eta < 60)
			dlg->set_string("progress_bar", str + format(_(" (%.0d s remaining)"), (int)(eta + 0.5f)));
		else
			dlg->set_string("progress_bar", str + format(_(" (%.0d min remaining)"), (int)(eta / 60 + 0.5f)));
	}
	dlg->set_float("progress_bar", progress);
	message = str;
	//HuiDoSingleMainLoop();
	os::sleep(0.001f);
}


void Progress::set(float progress) {
	set(message, progress);
}

void Progress::start(const string &str, float progress) {
	if (!dlg)
		dlg = hui::create_resource_dialog("progress_dialog", hui::CurWindow);
	dlg->set_string("progress_bar", str);
	dlg->set_float("progress_bar", progress);
	dlg->show();
	dlg->event("hui:close", [this] { }); // ignore event!
	os::sleep(0.001f);
	Cancelled = false;
	time_running = 0;
	timer.reset();
}

void Progress::cancel() {
	Cancelled = true;
}

bool Progress::is_cancelled() {
	return Cancelled;
}

void Progress::on_close() {
	cancel();
}

void Progress::start_cancelable(const string &str, float progress) {
	if (!dlg)
		dlg = hui::create_resource_dialog("progress_cancelable_dialog", hui::CurWindow);
	dlg->set_string("progress_bar", str);
	dlg->set_float("progress_bar", progress);
	dlg->show();
	dlg->event("hui:close", [this] {
		on_close();
	});
	dlg->event("cancel", [this] {
		on_close();
	});
	os::sleep(0.001f);
	Cancelled = false;
	time_running = 0;
	timer.get();
}

void Progress::end() {
	if (!dlg)
		return;
	delete(dlg);
	dlg = NULL;
	os::sleep(0.001f);
}
