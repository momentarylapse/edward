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
	timer = HuiCreateTimer();
}

Progress::~Progress()
{
}


void Progress::Set(const string &str, float progress)
{
	if (!dlg)
		return;
	time_running += HuiGetTime(timer);
	if (time_running < 5){
		dlg->SetString("progress_bar", str);
	}else{
		float eta = time_running / progress * (1 - progress);
		if (eta < 60)
			dlg->SetString("progress_bar", str + format(_(" (noch %.0d s)"), (int)(eta + 0.5f)));
		else
			dlg->SetString("progress_bar", str + format(_(" (noch %.0d min)"), (int)(eta / 60 + 0.5f)));
	}
	dlg->SetFloat("progress_bar", progress);
	message = str;
	HuiDoSingleMainLoop();
}


void Progress::Set(float progress)
{
	Set(message, progress);
}

void IgnoreEvent(){}

void Progress::Start(const string &str, float progress)
{
	if (!dlg)
		dlg = HuiCreateResourceDialog("progress_dialog", HuiCurWindow);
	dlg->SetString("progress_bar", str);
	dlg->SetFloat("progress_bar", progress);
	dlg->Update();
	dlg->Event("hui:close", &IgnoreEvent);
	HuiDoSingleMainLoop();
	Cancelled = false;
	time_running = 0;
	HuiGetTime(timer);
}

void Progress::Cancel()
{
	Cancelled = true;
}

bool Progress::IsCancelled()
{
	return Cancelled;
}

void OnProgressClose()
{	_progress_->Cancel();	}

void Progress::StartCancelable(const string &str, float progress)
{
	if (!dlg)
		dlg = HuiCreateResourceDialog("progress_cancelable_dialog", HuiCurWindow);
	dlg->SetString("progress_bar", str);
	dlg->SetFloat("progress_bar", progress);
	dlg->Update();
	dlg->Event("hui:close", &OnProgressClose);
	dlg->Event("cancel", &OnProgressClose);
	HuiDoSingleMainLoop();
	Cancelled = false;
	time_running = 0;
	HuiGetTime(timer);
}

void Progress::End()
{
	if (!dlg)
		return;
	delete(dlg);
	dlg = NULL;
	HuiDoSingleMainLoop();
}
