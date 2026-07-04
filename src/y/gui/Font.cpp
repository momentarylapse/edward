/*
 * Font.cpp
 *
 *  Created on: Feb 2, 2021
 *      Author: michi
 */


#include "Font.h"
#include "../Config.h"
#include <lib/ygraphics/Context.h>
#include <lib/os/app.h>


namespace gui {
	ygfx::FontManager* font_manager = nullptr;

	void init_fonts() {
		font_manager = new ygfx::FontManager();
		font_manager->directories = {config.game_dir | "Fonts", os::app::directory_static};
		font_manager->try_load_defaults({config.default_font, "FreeSans", "Verdana", "arial"}, {});
		if (!font_manager->default_font_regular)
			throw Exception("no font found...");

	}
}

