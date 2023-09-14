/*
 * UserMesh.h
 *
 *  Created on: Dec 12, 2022
 *      Author: michi
 */

#ifndef WORLD_COMPONENTS_USERMESH_H
#define WORLD_COMPONENTS_USERMESH_H

#include "../../y/Component.h"
#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/vec3.h>
#include <lib/image/color.h>
#include "../../graphics-impl.h"


class Material;

class UserMesh : public Component {
public:
	UserMesh() {
	}

	Material *material = nullptr;
	string vertex_shader_module;
	string geometry_shader_module;
	Shader *shader_cache[2] = {nullptr, nullptr};
	Shader *shader_cache_shadow[2] = {nullptr, nullptr};

	VertexBuffer *vertex_buffer = nullptr;
	PrimitiveTopology topology = PrimitiveTopology::TRIANGLES;

	static const kaba::Class *_class;
};



#endif //WORLD_COMPONENTS_USERMESH_H
