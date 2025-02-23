#include "Image.h"
#include "../Painter.h"
#include "../Theme.h"

namespace xhui {

Image::Image(const string &_id, const string &t) : Control(_id) {
	margin = {0,0,0,0};
	ignore_hover = true;
	if (t != "")
		image = load_image(t);

	size_mode_x = SizeMode::Shrink;
	size_mode_y = SizeMode::Shrink;
}

vec2 Image::get_content_min_size() const {
	if (image)
		return image->size();
	return {8,8};
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
		float scale = min(_area.width() / s.x, _area.height() / s.y);
		p->draw_ximage({_area.center() - s * scale/2, _area.center() + s * scale/2}, image);
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
