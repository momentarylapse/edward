//
// Created by Michael Ankele on 2025-05-05.
//

#include "MeshEmitter.h"
#include <helper/PerformanceMonitor.h>

MeshEmitter::MeshEmitter(const string &name) {
	channel = PerformanceMonitor::create_channel(name, -1);
}
