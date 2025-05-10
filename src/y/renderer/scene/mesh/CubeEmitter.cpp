//
// Created by michi on 10.05.25.
//

#include "CubeEmitter.h"
#include <renderer/base.h>
#include <renderer/scene/RenderViewData.h>
#include <y/EngineData.h>
#include <graphics-impl.h>
#include <lib/math/Box.h>

CubeEmitter::CubeEmitter() : MeshEmitter("cube") {
	vb = new VertexBuffer("3f,3f,2f");
	set_cube(Box::ID_SYM);
	material = new Material(engine.resource_manager);
	material->textures.add(tex_white);
}

void CubeEmitter::set_cube(const Box& box) {
	Array<Vertex1> vertices = {
		{{box.min.x, box.min.y, box.min.z}, {0,0,-1}, 0, 0},
		{{box.max.x, box.min.y, box.min.z}, {0,0,-1}, 0, 0},
		{{box.min.x, box.max.y, box.min.z}, {0,0,-1}, 0, 0},
		{{box.max.x, box.max.y, box.min.z}, {0,0,-1}, 0, 0},
		{{box.max.x, box.min.y, box.max.z}, {0,0,1}, 0, 0},
		{{box.min.x, box.min.y, box.max.z}, {0,0,1}, 0, 0},
		{{box.max.x, box.max.y, box.max.z}, {0,0,1}, 0, 0},
		{{box.min.x, box.max.y, box.max.z}, {0,0,1}, 0, 0},
		{{box.min.x, box.max.y, box.min.z}, {-1,0,0}, 0, 0},
		{{box.min.x, box.max.y, box.max.z}, {-1,0,0}, 0, 0},
		{{box.min.x, box.min.y, box.min.z}, {-1,0,0}, 0, 0},
		{{box.min.x, box.min.y, box.max.z}, {-1,0,0}, 0, 0},
		{{box.max.x, box.min.y, box.min.z}, {1,0,0}, 0, 0},
		{{box.max.x, box.min.y, box.max.z}, {1,0,0}, 0, 0},
		{{box.max.x, box.max.y, box.min.z}, {1,0,0}, 0, 0},
		{{box.max.x, box.max.y, box.max.z}, {1,0,0}, 0, 0},
		{{box.min.x, box.min.y, box.min.z}, {0,-1,0}, 0, 0},
		{{box.min.x, box.min.y, box.max.z}, {0,-1,0}, 0, 0},
		{{box.max.x, box.min.y, box.min.z}, {0,-1,0}, 0, 0},
		{{box.max.x, box.min.y, box.max.z}, {0,-1,0}, 0, 0},
		{{box.max.x, box.max.y, box.min.z}, {0,1,0}, 0, 0},
		{{box.max.x, box.max.y, box.max.z}, {0,1,0}, 0, 0},
		{{box.min.x, box.max.y, box.min.z}, {0,1,0}, 0, 0},
		{{box.min.x, box.max.y, box.max.z}, {0,1,0}, 0, 0},
	};
	vb->update(vertices);
	Array<int> indices = {0,2,1,1,2,3, 4,6,5,5,6,7, 8,10,9,9,10,11, 12,14,13,13,14,15, 16,18,17,17,18,19, 20,22,21,21,22,23};
	vb->update_index(indices);
}

void CubeEmitter::emit(const RenderParams& params, RenderViewData& rvd, bool shadow_pass) {
	auto shader = rvd.get_shader(material.get(), 0, "default", "");
	auto& rd = rvd.start(params, mat4::ID, shader, *material, 0, PrimitiveTopology::TRIANGLES, vb.get());
	rd.draw_triangles(params, vb.get());
}

