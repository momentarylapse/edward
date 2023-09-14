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
#include "../../graphics-fwd.h"

class Model;

class MultiInstance : public Component {
public:
	MultiInstance();
	virtual ~MultiInstance();

	Model *model = nullptr;
	Array<mat4> matrices;
	UniformBuffer *ubo_matrices = nullptr;

	static const kaba::Class *_class;
};
