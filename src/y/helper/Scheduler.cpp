/*
 * Scheduler.cpp
 *
 *  Created on: Jul 24, 2021
 *      Author: michi
 */

#include "Scheduler.h"
#include "../lib/base/algo.h"
#include "../lib/profiler/Profiler.h"

static int ch_iterate = -1;
static Array<Scheduler*> schedulers;

Scheduler::Scheduler() {
	schedulers.add(this);
}

Scheduler::~Scheduler() {
	base::remove(schedulers, this);
}


void Scheduler::clear() {
	listeners.clear();
}

void Scheduler::repeat(float dt, const Callable<void()> &f) {
	listeners.add({dt, dt, true, &f});
}

void Scheduler::later(float dt, const Callable<void()> &f) {
	listeners.add({dt, dt, false, &f});
}

void Scheduler::iterate(float dt) {
	for (auto &l: listeners) {
		l.t -= dt;
		if (l.t <= 0) {
			(*l.f)();
			if (l.repeated)
				l.t = l.dt;
		}
	}
	base::remove_if(listeners, [] (auto& l) {
		return l.t <= 0 and !l.repeated;
	});
}

void SchedulerManager::init(int ch_iter_parent) {
	ch_iterate = profiler::create_channel("scheduler", ch_iter_parent);
}

void SchedulerManager::reset() {
	schedulers.clear();
}

void SchedulerManager::iterate(float dt) {
	profiler::begin(ch_iterate);
	for (auto s: schedulers)
		s->iterate(dt);
	profiler::end(ch_iterate);
}
