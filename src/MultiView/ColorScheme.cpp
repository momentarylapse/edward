/*
 * ColorScheme.cpp
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#include "ColorScheme.h"


ColorScheme scheme;


ColorScheme::ColorScheme() {
	BACKGROUND = color(1,0.9f,0.9f,0.9f);
	BACKGROUND_SELECTED = color(1,0.96f,0.96f,0.96f);
	SELECTION = color(1, 0.8f,0,0);
	HOVER = White;
	GRID = color(1,0.65f,0.65f,0.65f);
	TEXT = color(1, 0.1f, 0.1f, 0.1f);
	TEXT_BG = color(1, 0.7f, 0.7f, 0.7f);
	WINDOW_TITLE = color(1, 0.35f, 0.35f, 0.35f);
	WINDOW_TITLE_BG = TEXT_BG;//color(1, 0.4f, 0.4f, 0.4f);
	//ColorWindowType = color(1, 0.5f, 0.5f, 0.5f);
	POINT = color(1, 0.2f, 0.2f, 0.9f);
	POINT_SELECTED = color(1, 0.9f, 0.2f, 0.2f);
	POINT_SPECIAL = color(1, 0.2f, 0.8f, 0.2f);
	WINDOW_DIVIDER = color(1, 0.4f, 0.4f, 0.75f);
	SELECTION_RECT = color(0.2f,0,0,1);
	SELECTION_RECT_BOUNDARY = color(0.7f,0,0,1);
	CREATION = color(0.5f, 0.1f, 0.6f, 0.1f);
	CREATION_LINE = color(1, 0.1f, 0.8f, 0.1f);
	HELPER_LINE = color(1, 0.2f, 0.2f, 0.2f);

	AXIS_X = color(1, 0.8f, 0.2f, 0.2f);
	AXIS_Y = color(1, 0.2f, 0.8f, 0.2f);
	AXIS_Z = color(1, 0.2f, 0.2f, 0.8f);

	FONT_NAME = "Sans Semi-Bold";
	FONT_SIZE = 12;
	TEXT_LINE_HEIGHT = FONT_SIZE * 1.4f;

	LINE_WIDTH_THICK = 5;
	LINE_WIDTH_MEDIUM = 2.5f;
	LINE_WIDTH_THIN = 1.5f;
	LINE_WIDTH_HELPER = 3;
	LINE_WIDTH_BONE = 5;
	WINDOW_DIVIDER_THICKNESS = 5;

	POINT_RADIUS = 3;
	POINT_RADIUS_HOVER = 5;
}

color ColorScheme::hoverify(const color &c) const {
	return ColorInterpolate(c, HOVER, 0.3f);
}

