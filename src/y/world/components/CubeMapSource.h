//
// Created by michi on 8/3/25.
//

#pragma once


#include <ecs/Component.h>
#include <lib/yrenderer/helper/CubeMapSource.h>


struct CubeMapSource : ecs::Component {
	~CubeMapSource() override;

	yrenderer::CubeMapSource source;

	static const kaba::Class *_class;
};

