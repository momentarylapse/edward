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
		BACKGROUND = color(1,0.9f,0.9f,0.9f).srgb_to_lin();
		BACKGROUND_SELECTED = color(1,0.96f,0.96f,0.96f).srgb_to_lin();
		GRID = color(1,0.65f,0.65f,0.65f).srgb_to_lin();
		WINDOW_DIVIDER = color(1, 0.4f, 0.4f, 0.75f).srgb_to_lin();
		POINT = color(1, 0.2f, 0.2f, 0.9f).srgb_to_lin();
	} else {
		BACKGROUND = color(1,0.25f,0.25f,0.25f).srgb_to_lin();
		BACKGROUND_SELECTED = color(1,0.29f,0.29f,0.29f).srgb_to_lin();
		GRID = color(1,0.45f,0.45f,0.45f).srgb_to_lin();
		WINDOW_DIVIDER = color(1, 0.35f, 0.35f, 0.65f).srgb_to_lin();
		POINT = color(1, 0.1f, 0.1f, 0.7f).srgb_to_lin();
	}
	SELECTION = color(1, 0.8f,0,0).srgb_to_lin();
	HOVER = White;
	TEXT = color(1, 0.1f, 0.1f, 0.1f).srgb_to_lin();
	TEXT_BG = color(1, 0.7f, 0.7f, 0.7f).srgb_to_lin();
	WINDOW_TITLE = color(1, 0.35f, 0.35f, 0.35f).srgb_to_lin();
	WINDOW_TITLE_BG = TEXT_BG;//color(1, 0.4f, 0.4f, 0.4f).srgb_to_lin();
	//ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	POINT_SELECTED = color(1, 0.9f, 0.2f, 0.2f).srgb_to_lin();
	POINT_SPECIAL = color(1, 0.2f, 0.8f, 0.2f).srgb_to_lin();
	SELECTION_RECT = color(0.2f,0,0,1).srgb_to_lin();
	SELECTION_RECT_BOUNDARY = color(0.7f,0,0,1).srgb_to_lin();
	CREATION = color(0.5f, 0.1f, 0.6f, 0.1f).srgb_to_lin();
	CREATION_LINE = color(1, 0.1f, 0.8f, 0.1f).srgb_to_lin();
	HELPER_LINE = color(1, 0.2f, 0.2f, 0.2f).srgb_to_lin();

	AXIS[0] = color(1, 0.8f, 0.2f, 0.2f).srgb_to_lin();
	AXIS[1] = color(1, 0.2f, 0.8f, 0.2f).srgb_to_lin();
	AXIS[2] = color(1, 0.3f, 0.3f, 1.0f).srgb_to_lin();

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

