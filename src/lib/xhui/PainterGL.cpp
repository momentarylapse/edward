#if HAS_LIB_GL

#include "Painter.h"
#include "Theme.h"
#include "Context.h"
#include <lib/ygraphics/Context.h>
#include <lib/ygraphics/graphics-impl.h>


namespace xhui {

void Painter::draw_ximage(const rect &r, const XImage *image) {
	auto t = image->texture.get();
	const auto mat = mat4::translation(vec3(r.p00() + offset, 0)) * mat4::scale(r.width(), r.height(), 1);
	nix::set_shader(aux->shader);
	nix::set_alpha_split(nix::Alpha::SOURCE_ALPHA, nix::Alpha::SOURCE_INV_ALPHA, nix::Alpha::ZERO, nix::Alpha::ONE);
	aux->shader->set_matrix("matrix", mat_pixel_to_rel * mat);
	aux->shader->set_color("color", _color);
	nix::bind_texture(0, t);
	nix::draw_triangles(aux->vb);
	nix::disable_alpha();
}

void Painter::prepare_2d_drawing() {
	bool gamma_correction = (color_space_display == ColorSpace::SRGB) and (color_space_shaders == ColorSpace::Linear);
	nix::set_srgb(gamma_correction);

	nix::bind_frame_buffer(context->context->ctx->default_framebuffer);
	nix::set_projection_matrix(mat_pixel_to_rel);
	nix::set_view_matrix(mat4::ID);
	nix::set_cull(nix::CullMode::NONE);
	nix::set_z(false, false);

	nix::set_viewport(native_area_window);
	nix::set_scissor(rect::EMPTY, native_area_window);
}


}

#endif
