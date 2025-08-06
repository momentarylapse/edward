/*
 * MultiInstance.cpp
 *
 *  Created on: Dec 15, 2022
 *      Author: michi
 */

#include "MultiInstance.h"
#include <lib/ygraphics/graphics-impl.h>

const kaba::Class *MultiInstance::_class = nullptr;

MultiInstance::MultiInstance() {
}

MultiInstance::~MultiInstance() {
    if (ubo_matrices)
        delete ubo_matrices;
}
