#include "Theme.h"
#include "Application.h"
#include "../os/config.h"
#include "../os/path.h"
#include "../os/app.h"
#include "../os/msg.h"

namespace xhui {

Theme Theme::_default;

void Theme::load_default() {
	Configuration c;
	if (!c.load(os::app::directory_dynamic | "default.theme"))
		if (!c.load(os::app::directory_static | "default.theme"))
			if (!c.load(os::app::directory_dynamic | "packages/xhui/default.theme")) // kaba package workaround...
				msg_error("failed to load default.theme");

	_default.font_size = c.get_float("font.size", 12);
	_default.font_name = c.get_str("font.name", "Sans");

	_default.spacing = c.get_float("spacing", 8);
	_default.focus_frame_width = c.get_float("focus-frame-width", 1);
	_default.window_margin = c.get_float("window.margin", 8);
	_default.window_radius = c.get_float("window.radius", 10);
	_default.headerbar_height = c.get_float("headerbar.height", 40);
	_default.label_margin_x = c.get_float("label.margin-horizontal", 8);
	_default.label_margin_y = c.get_float("label.margin-vertical", 8);
	_default.button_margin_x = c.get_float("button.margin-horizontal", 8);
	_default.button_margin_y = c.get_float("button.margin-vertical", 8);
	_default.button_radius = c.get_float("button.radius", 8);
	_default.edit_margin_x = c.get_float("edit.margin-horizontal", 8);

	_default.background = color::parse(c.get_str("colors.background", "#000000"));
	_default.background_button = color::parse(c.get_str("colors.background-button", "#000000"));
	_default.background_button_primary = color::parse(c.get_str("colors.background-button-primary", "#0000ff"));
	_default.background_button_primary_hover = color::parse(c.get_str("colors.background-button-primary-hover", "#0000ff"));
	_default.background_button_primary_active = color::parse(c.get_str("colors.background-button-primary-active", "#0000ff"));
	_default.background_header = color::parse(c.get_str("colors.background-header", "#000000"));
	_default.background_header_button = color::parse(c.get_str("colors.background-header-button", "#000000"));
	_default.background_hover = color::parse(c.get_str("colors.background-hover", "#404040"));
	_default.background_active = color::parse(c.get_str("colors.background-active", "#707070"));
	_default.background_low = color::parse(c.get_str("colors.background-low", "#000000"));
	_default.background_low_hover = color::parse(c.get_str("colors.background-low-hover", "#000000"));
	_default.background_low_selected = color::parse(c.get_str("colors.background-low-selected", "#000080"));
	_default.text = color::parse(c.get_str("colors.text", "#ffffff"));
	_default.text_label = color::parse(c.get_str("colors.text-label", "#ffffff"));
	_default.text_disabled = color::parse(c.get_str("colors.text-disabled", "#808080"));
	_default.border = color::parse(c.get_str("colors.border", "#ffffff"));
}

color Theme::background_raised() const {
	return color::interpolate(background, background_button, 0.5f);
}



}
