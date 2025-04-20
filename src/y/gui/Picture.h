/*
 * Picture.h
 *
 *  Created on: 04.01.2020
 *      Author: michi
 */

#pragma once

#include "../graphics-fwd.h"
#include "Node.h"
#include "../lib/any/any.h"

namespace gui {

class Picture : public Node {
public:
	Picture(const rect &r, shared<Texture> tex, const rect &source, shared<Shader> shader);
	Picture(const rect &r, shared<Texture> tex, const rect &source = rect::ID);
	~Picture() override;

	void __init__(const rect &r, shared<Texture> tex, const rect &source);
	virtual void __delete__();

	rect source;

	float bg_blur;
	float angle;

	shared<Shader> shader;
	shared<Texture> texture;
	Any shader_data;
};

}
