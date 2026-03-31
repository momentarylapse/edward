/*
 * ParticleManager.h
 *
 *  Created on: Aug 9, 2020
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"
#include <ecs/System.h>

class ParticleManager : public ecs::System {
public:
	void on_iterate(float dt) override;
	static const kaba::Class* _class;
};


