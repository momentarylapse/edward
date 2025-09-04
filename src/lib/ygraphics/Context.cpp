//
// Created by michi on 9/2/25.
//

#include "Context.h"
#include "graphics-impl.h"
#include <lib/image/image.h>

namespace ygfx {

void Context::_create_default_textures() {
	tex_white = new Texture();
	tex_black = new Texture();
	tex_white->write(Image(16, 16, White));
	tex_black->write(Image(16, 16, Black));
	tex_white->_pointer_ref_counter = 999999999;
}

}
