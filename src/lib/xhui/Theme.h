#pragma once

#include "../image/color.h"

namespace xhui {

class Theme {
public:
	string font_name;
	float font_size;
	float font_size_small;
	float font_size_big;

	float spacing, window_margin;
	float label_margin_x;
	float label_margin_y;
	float button_margin_x;
	float button_margin_y;
	float button_radius;
	float edit_margin_x;

	float focus_frame_width;
	float headerbar_height;
	float window_radius;

	color background, background_button;
	color background_button_primary, background_button_primary_hover, background_button_primary_active;
	color background_button_danger, background_button_danger_hover, background_button_danger_active;
	color background_header, background_header_button;
	color background_hover, background_active;
	color background_low, background_low_hover, background_low_selected;
	color text, text_label, text_disabled;
	color text_link;
	color border;

	color background_raised() const;

	static void load_default();
	static Theme _default;
};

}
