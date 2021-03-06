/*
 * Progress.h
 *
 *  Created on: 25.03.2012
 *      Author: michi
 */

#ifndef PROGRESS_H_
#define PROGRESS_H_

#include "../lib/hui/hui.h"

class Progress : public hui::EventHandler
{
public:
	Progress();
	virtual ~Progress();
	void set(float progress);
	void set(const string &str, float progress);
	void start(const string &str, float progress);
	void start_cancelable(const string &str, float progress);
	void end();

	void cancel();
	bool is_cancelled();

private:
	void on_close();
	void _ignore_event(){}

	hui::Window *dlg;
	bool Cancelled;
	string message;
	hui::Timer timer;
	float time_running;
};

#endif /* PROGRESS_H_ */
