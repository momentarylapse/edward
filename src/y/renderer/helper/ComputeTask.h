#ifndef COMPUTETASK_H
#define COMPUTETASK_H


#include "../Renderer.h"
#include "Bindable.h"
#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/any/any.h>
#include "../../graphics-fwd.h"

class ComputeTask : public RenderTask {
public:
    explicit ComputeTask(const string& name, const shared<Shader>& shader, int nx, int ny, int nz);
    shared<Shader> shader;

    IMPLEMENT_BINDABLE_INTERFACE

    int nx, ny, nz;

#ifdef USING_VULKAN
    owned<vulkan::ComputePipeline> pipeline;
#endif

    void render(const RenderParams &params) override;
};


#endif //COMPUTETASK_H
