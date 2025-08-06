//
// Created by Michael Ankele on 2025-05-05.
//

#include "MeshEmitter.h"
#include <lib/profiler/Profiler.h>

namespace yrenderer {

MeshEmitter::MeshEmitter(Context* _ctx, const string &name) {
	ctx = _ctx;
	channel = profiler::create_channel(name, -1);
}

}
