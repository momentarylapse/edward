//
// Created by Michael Ankele on 2025-05-05.
//

#include "MeshEmitter.h"
#include <lib/profiler/Profiler.h>

MeshEmitter::MeshEmitter(const string &name) {
	channel = profiler::create_channel(name, -1);
}
