
#include "Painter.h"
#include "Context.h"
#include "Theme.h"
#include <lib/image/image.h>
#include <lib/math/mat4.h>
#include <lib/base/algo.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/font.h>
#include <lib/ygraphics/Context.h>
#include <cmath>

namespace xhui {

Painter::Painter(Context* c, Window* window, const rect& native_area, const rect& area) :
	ygfx::Painter(c ? c->aux : nullptr, c ? c->font_manager : nullptr, c ? c->text_cache.get() : nullptr, native_area, area)
{
	context = c;

	if (window) {
		context = window->context;

		Painter::set_color(Theme::_default.text);
		Painter::set_font(Theme::_default.font_name /*"CAC Champagne"*/, Theme::_default.font_size, false, false);

#ifdef USING_VULKAN
		cb = context->current_command_buffer();
#endif
	}
}

void Painter::set_font(const string &font, float size, bool bold, bool italic) {
	ygfx::Painter::set_font(font, size, bold, italic);
}


}
