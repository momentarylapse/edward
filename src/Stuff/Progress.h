/*
 * Progress.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include "../lib/hui/hui.h"

class Progress : public HuiEventHandler
{
public:
	Progress();
	virtual ~Progress();
	void set(float progress);
	void set(const string &str, float progress);
	void start(const string &str, float progress);
	void startCancelable(const string &str, float progress);
	void end();

	void cancel();
	bool isCancelled();

private:
	void onClose();
	void _ignoreEvent(){}

	HuiWindow *dlg;
	bool Cancelled;
	string message;
	HuiTimer timer;
	float time_running;
};

#endif /* PROGRESS_H_ */
