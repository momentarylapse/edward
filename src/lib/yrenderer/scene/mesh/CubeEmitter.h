//
// Created by michi on 10.05.25.
//

#ifndef CUBEEMITTER_H
#define CUBEEMITTER_H

#include "../MeshEmitter.h"
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/math/mat4.h>

struct Box;

namespace yrenderer {

class Material;

class CubeEmitter : public MeshEmitter {
public:
	owned<ygfx::VertexBuffer> vb;
	owned<Material> material;
	mat4 matrix;

	explicit CubeEmitter(Context* ctx);
	void set_cube(const Box& box);
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};

}

#endif //CUBEEMITTER_H
