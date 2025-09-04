#if HAS_LIB_GL

#include "Painter.h"
#include "Theme.h"
#include "Context.h"
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include "../image/image.h"


namespace xhui {

void Painter::draw_ximage(const rect &r, const XImage *image) {
	auto t = image->texture.get();
	nix::set_model_matrix(mat4::translation(vec3(offset_x + r.x1, offset_y + r.y1, 0)) * mat4::scale(r.width(), r.height(), 1));
	nix::set_shader(aux->shader);
	nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	aux->shader->set_color("_color_", _color);
	aux->shader->set_default_data();
	nix::bind_texture(0, t);
	nix::draw_triangles(aux->vb);
	nix::disable_alpha();
}


}

#endif
