/*
 * ParticleManager.h
 *
 *  Created on: Aug 9, 2020
 *      Author: michi
 */

#pragma once

#include "../lib/base/base.h"

class EntityManager;

class ParticleManager {
public:
	explicit ParticleManager(EntityManager* entity_manager);
	void iterate(float dt);
	EntityManager* entity_manager;
};


