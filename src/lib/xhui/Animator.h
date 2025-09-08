//
// Created by michi on 9/8/25.
//

#pragma once

#include <functional>
#include <lib/os/time.h>

namespace xhui {

class Control;

struct Animator {
	explicit Animator(Control* owner);
	~Animator();

	Control* owner;
	void start();
	void stop();
	float time = 0; // real time
	float duration = 0.3f;
	float t0 = 0; // relative mapping (0 => t0, duration => t1)
	float t1 = 0;
	float t = 0;
	int id_runner = -1;
	os::Timer timer;

	std::function<void()> on_start;
	std::function<void(float)> on_update;
	std::function<void()> on_end;
};

}

