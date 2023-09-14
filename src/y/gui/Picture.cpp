/*
 * Picture.cpp
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#include "Picture.h"
#include "../lib/math/rect.h"
#include "../lib/math/vec3.h"
#include "../lib/math/mat4.h"
#include "../graphics-impl.h"
#include <iostream>


/*vulkan::Shader *Picture::shader = nullptr;
vulkan::Pipeline *Picture::pipeline = nullptr;
vulkan::VertexBuffer *Picture::vertex_buffer = nullptr;
vulkan::RenderPass *Picture::render_pass = nullptr;*/



struct UBOMatrices {
	alignas(16) mat4 model;
	alignas(16) mat4 view;
	alignas(16) mat4 proj;
};


namespace gui {

Picture::Picture(const rect &r, shared<Texture> tex, const rect &s, shared<Shader> _shader) :
	Node(r)
{
	type = Type::PICTURE;
	source = s;
	texture = tex;
	bg_blur = 0;
	angle = 0;
	visible = true;
	shader = nullptr;
}

Picture::Picture(const rect &r, shared<Texture> tex, const rect &s) : Picture(r, tex, s, nullptr) {
}

Picture::~Picture() {
	/*
	delete ubo;
	delete dset;
//	if (user_shader)
//		delete user_shader;
	if (user_pipeline)
		delete user_pipeline;*/
}

//void Picture::rebuild() {
	//dset->set({ubo}, {texture});
//}

void Picture::__init__(const rect &r, shared<Texture> tex, const rect &s) {
	new(this) Picture(r, tex, s);
	source = s;
}

void Picture::__delete__() {
	this->Picture::~Picture();
}


}
