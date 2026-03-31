/*
 * System.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "System.h"
#include <lib/profiler/Profiler.h>


namespace ecs {

System::System() {
	channel = profiler::create_channel("System", -1);
}

System::~System() {
	profiler::delete_channel(channel);
}

void System::set_profiler_name(const string &name) {
	profiler::set_name(channel, name);
}
}

