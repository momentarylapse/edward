#include "Image.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {

Image::Image(const string &_id, const string &t) : Control(_id) {
	ignore_hover = true;
	if (t != "")
		image = load_image(t);

	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

vec2 Image::get_content_min_size() const {
	vec2 s = {8,8};
	if (image)
		s = image->size();
	if (min_width_user >= 0)
		s.x = min_width_user - padding.x1 - padding.x2;
	if (min_height_user >= 0)
		s.y = min_height_user - padding.y1 - padding.y2;
	return s;
}

void Image::set_string(const string& s) {
	image = load_image(s);
	request_redraw();
}

void Image::_draw(Painter *p) {
	if (image) {
		prepare_image(image);
		p->set_color(White);
		if (!enabled)
			p->set_color(White.with_alpha(0.35f));
		vec2 s = image->size();
		const auto ca = content_area();
		float scale = min(ca.width() / s.x, ca.height() / s.y);
		p->draw_ximage({ca.center() - s * scale/2, ca.center() + s * scale/2}, image);
	}
}

void Image::set_option(const string& key, const string& value) {
	if (key == "image") {
		set_string(value);
	} else {
		Control::set_option(key, value);
	}
}


}
