//
// Created by michi on 8/3/25.
//

#pragma once


#include <y/Component.h>
#include <lib/yrenderer/helper/CubeMapSource.h>


class CubeMapSource : public Component {
public:
	~CubeMapSource() override;

	yrenderer::CubeMapSource source;

	static const kaba::Class *_class;
};

