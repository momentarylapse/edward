/*
 * base.cpp
 *
 *  Created on: 21 Nov 2021
 *      Author: michi
 */

#include "base.h"
#include "../graphics-impl.h"
#include "../lib/image/image.h"
#include "../Config.h"

Texture *tex_white = nullptr;
Texture *tex_black = nullptr;

Array<int> gpu_timestamp_queries;

void _create_default_textures() {
	tex_white = new Texture();
	tex_black = new Texture();
	tex_white->write(Image(16, 16, White));
	tex_black->write(Image(16, 16, Black));
}



