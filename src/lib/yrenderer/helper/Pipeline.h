/*
 * Pipeline.h
 *
 *  Created on: 10 Dec 2022
 *      Author: michi
 */

#pragma once
#include <lib/ygraphics/graphics-fwd.h>

// TODO use...

namespace yrenderer {

class Pipeline {
    Shader *shader = nullptr;
#ifdef USING_VULKAN
    GraphicsPipeline *pipeline = nullptr;
#endif
};

class Bindings {
#ifdef USING_VULKAN
    vulkan::DescriptorSet *dset = nullptr;
#endif
};

}
