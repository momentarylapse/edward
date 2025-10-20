/*
 * UserMesh.h
 *
 *  Created on: Dec 12, 2022
 *      Author: michi
 */

#ifndef WORLD_COMPONENTS_USERMESH_H
#define WORLD_COMPONENTS_USERMESH_H

#include <ecs/Component.h>
#include <lib/yrenderer/Material.h>
#include <lib/base/base.h>
#include <lib/base/pointer.h>
#include <lib/math/vec3.h>
#include <lib/image/color.h>
#include <lib/ygraphics/graphics-impl.h>


namespace yrenderer {
	class Material;
}

class UserMesh : public Component {
public:
	UserMesh() = default;

	owned<yrenderer::Material> material;
	string vertex_shader_module = "default";
	string geometry_shader_module;

	owned<ygfx::VertexBuffer> vertex_buffer;
	ygfx::PrimitiveTopology topology = ygfx::PrimitiveTopology::TRIANGLES;

	static const kaba::Class *_class;
};



#endif //WORLD_COMPONENTS_USERMESH_H
