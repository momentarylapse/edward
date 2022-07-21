/*
 * ColorScheme.cpp
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#include "ColorScheme.h"
#include "../lib/hui/hui.h"


ColorScheme scheme;


ColorScheme::ColorScheme() {
	string name = hui::config.get_str("view.color-scheme", "dark");
	if (name == "bright") {
		BACKGROUND = color(1,0.9f,0.9f,0.9f);
		BACKGROUND_SELECTED = color(1,0.96f,0.96f,0.96f);
		GRID = color(1,0.65f,0.65f,0.65f);
		WINDOW_DIVIDER = color(1, 0.4f, 0.4f, 0.75f);
		POINT = color(1, 0.2f, 0.2f, 0.9f);
	} else {
		BACKGROUND = color(1,0.25f,0.25f,0.25f);
		BACKGROUND_SELECTED = color(1,0.29f,0.29f,0.29f);
		GRID = color(1,0.45f,0.45f,0.45f);
		WINDOW_DIVIDER = color(1, 0.35f, 0.35f, 0.65f);
		POINT = color(1, 0.1f, 0.1f, 0.7f);
	}
	SELECTION = color(1, 0.8f,0,0);
	HOVER = White;
	TEXT = color(1, 0.1f, 0.1f, 0.1f);
	TEXT_BG = color(1, 0.7f, 0.7f, 0.7f);
	WINDOW_TITLE = color(1, 0.35f, 0.35f, 0.35f);
	WINDOW_TITLE_BG = TEXT_BG;//color(1, 0.4f, 0.4f, 0.4f);
	//ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	POINT_SELECTED = color(1, 0.9f, 0.2f, 0.2f);
	POINT_SPECIAL = color(1, 0.2f, 0.8f, 0.2f);
	SELECTION_RECT = color(0.2f,0,0,1);
	SELECTION_RECT_BOUNDARY = color(0.7f,0,0,1);
	CREATION = color(0.5f, 0.1f, 0.6f, 0.1f);
	CREATION_LINE = color(1, 0.1f, 0.8f, 0.1f);
	HELPER_LINE = color(1, 0.2f, 0.2f, 0.2f);

	AXIS[0] = color(1, 0.8f, 0.2f, 0.2f);
	AXIS[1] = color(1, 0.2f, 0.8f, 0.2f);
	AXIS[2] = color(1, 0.3f, 0.3f, 1.0f);

	FONT_NAME = "Sans Semi-Bold";
	FONT_SIZE = 12;
	TEXT_LINE_HEIGHT = FONT_SIZE * 1.4f;

	LINE_WIDTH_THICK = 5;
	LINE_WIDTH_MEDIUM = 2.5f;
	LINE_WIDTH_THIN = 2.0f;
	LINE_WIDTH_HELPER = 3;
	LINE_WIDTH_BONE = 5;
	WINDOW_DIVIDER_THICKNESS = 5;

	POINT_RADIUS = 3;
	POINT_RADIUS_HOVER = 5;

	BOX_PADDING = 8;
	BOX_ROUNDNESS = 10;
}

color ColorScheme::hoverify(const color &c) const {
	return color::interpolate(c, HOVER, 0.3f);
}

