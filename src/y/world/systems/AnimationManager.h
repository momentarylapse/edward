//
// Created by michi on 3/28/26.
//

#pragma once

#include "../../ecs/System.h"


class AnimationManager : public System {
public:
	void on_iterate(float dt) override;

	static const kaba::Class* _class;
};

