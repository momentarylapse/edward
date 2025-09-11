/*
 * MultiInstance.h
 *
 *  Created on: Dec 15, 2022
 *      Author: michi
 */

#pragma once

#include "../../y/Component.h"
#include "../../y/Entity.h"
#include <lib/base/pointer.h>
#include <lib/math/mat4.h>
#include <lib/ygraphics/graphics-fwd.h>

class Model;

class MultiInstance : public Component {
public:
	MultiInstance();
	~MultiInstance() override;

	Model *model = nullptr;
	Array<mat4> matrices;
	ygfx::UniformBuffer *ubo_matrices = nullptr;

	static const kaba::Class *_class;
};
