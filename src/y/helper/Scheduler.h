/*
 * Scheduler.h
 *
 *  Created on: Jul 24, 2021
 *      Author: michi
 */

#pragma once

#include "../lib/base/callable.h"

class Scheduler {
public:
	Scheduler();
	~Scheduler();

	void clear();
	void repeat(float dt, const Callable<void()> &f);
	void later(float dt, const Callable<void()> &f);

	void iterate(float dt);

	struct ScheduleListener {
		float dt;
		float t;
		bool repeated;
		const Callable<void()> *f;
	};
	Array<ScheduleListener> listeners;
};

class SchedulerManager {
public:
	static void init(int ch_iter);
	static void reset();

	static void iterate(float dt);
};
