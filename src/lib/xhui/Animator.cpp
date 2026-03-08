//
// Created by michi on 9/8/25.
//

#include "Animator.h"
#include "xhui.h"
#include <lib/math/interpolation.h>

namespace xhui {

Animator::Animator(Control* o) {
	owner = o;
}

Animator::~Animator() {
	if (id_runner >= 0)
		cancel_runner(id_runner);
}


void Animator::start() {
	if (id_runner >= 0)
		stop();
	if (!owner)
		return;

	time = 0;
	t = t0;
	timer.reset();

	if (on_start)
		on_start();

	id_runner = run_repeated(0.015f, [this] {
		time += min(timer.get(), 0.05f);
		if (time >= duration) {
			stop();
			return;
		}
		float tt = time / duration; // uniform
		if (mode == Mode::EASE_IN)
			tt = tt * tt;
		else if (mode == Mode::EASE_OUT)
			tt = tt * (2 - tt);
		else if (mode == Mode::EASE_IN_OUT)
			tt = cubic_spline<float>({0,0,1,0}, tt);
		t = t0 + (t1 - t0) * tt;
		if (on_update)
			on_update(t);
		owner->owner->request_redraw();
	});
}

void Animator::stop() {
	if (id_runner < 0 or !owner)
		return;
	cancel_runner(id_runner);
	id_runner = -1;
	if (on_end)
		on_end();
	owner->owner->request_redraw();
}


}
