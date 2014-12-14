/*
 * Progress.cpp
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#include "Progress.h"

static Progress *_progress_;

Progress::Progress() :
	dlg(NULL)
{
	_progress_ = this;
}

Progress::~Progress()
{
}


void Progress::set(const string &str, float progress)
{
	if (!dlg)
		return;
	time_running += timer.get();
	if (time_running < 5){
		dlg->setString("progress_bar", str);
	}else{
		float eta = time_running / progress * (1 - progress);
		if (eta < 60)
			dlg->setString("progress_bar", str + format(_(" (noch %.0d s)"), (int)(eta + 0.5f)));
		else
			dlg->setString("progress_bar", str + format(_(" (noch %.0d min)"), (int)(eta / 60 + 0.5f)));
	}
	dlg->setFloat("progress_bar", progress);
	message = str;
	HuiDoSingleMainLoop();
}


void Progress::set(float progress)
{
	set(message, progress);
}

void Progress::start(const string &str, float progress)
{
	if (!dlg)
		dlg = HuiCreateResourceDialog("progress_dialog", HuiCurWindow);
	dlg->setString("progress_bar", str);
	dlg->setFloat("progress_bar", progress);
	dlg->show();
	dlg->event("hui:close", this, &Progress::_ignoreEvent);
	HuiDoSingleMainLoop();
	Cancelled = false;
	time_running = 0;
	timer.reset();
}

void Progress::cancel()
{
	Cancelled = true;
}

bool Progress::isCancelled()
{
	return Cancelled;
}

void Progress::onClose()
{
	cancel();
}

void Progress::startCancelable(const string &str, float progress)
{
	if (!dlg)
		dlg = HuiCreateResourceDialog("progress_cancelable_dialog", HuiCurWindow);
	dlg->setString("progress_bar", str);
	dlg->setFloat("progress_bar", progress);
	dlg->show();
	dlg->event("hui:close", this, &Progress::onClose);
	dlg->event("cancel", this, &Progress::onClose);
	HuiDoSingleMainLoop();
	Cancelled = false;
	time_running = 0;
	timer.get();
}

void Progress::end()
{
	if (!dlg)
		return;
	delete(dlg);
	dlg = NULL;
	HuiDoSingleMainLoop();
}
