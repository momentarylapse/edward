//
// Created by michi on 10.05.25.
//

#ifndef CUBEEMITTER_H
#define CUBEEMITTER_H

#include "../MeshEmitter.h"
#include <graphics-fwd.h>

class Material;
struct Box;

class CubeEmitter : public MeshEmitter {
public:
	owned<VertexBuffer> vb;
	owned<Material> material;

	explicit CubeEmitter();
	void set_cube(const Box& box);
	void emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) override;
};

#endif //CUBEEMITTER_H
