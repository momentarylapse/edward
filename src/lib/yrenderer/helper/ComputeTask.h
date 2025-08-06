#ifndef COMPUTETASK_H
#define COMPUTETASK_H


#include "../Renderer.h"
#include "Bindable.h"
#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/any/any.h>
#include <lib/ygraphics/graphics-fwd.h>

namespace yrenderer {

class ComputeTask : public RenderTask {
public:
    explicit ComputeTask(Context* ctx, const string& name, const shared<ygfx::Shader>& shader, int nx, int ny, int nz);
    shared<ygfx::Shader> shader;

    IMPLEMENT_BINDABLE_INTERFACE

    int nx, ny, nz;

#ifdef USING_VULKAN
    owned<vulkan::ComputePipeline> pipeline;
#endif

    void render(const RenderParams &params) override;
};

}


#endif //COMPUTETASK_H
