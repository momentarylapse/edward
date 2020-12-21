/*
 * ColorScheme.h
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#ifndef SRC_MULTIVIEW_COLORSCHEME_H_
#define SRC_MULTIVIEW_COLORSCHEME_H_

#include "../lib/image/color.h"

class ColorScheme {
public:
	ColorScheme();

	color hoverify(const color &c) const;

	color BACKGROUND;
	color BACKGROUND_SELECTED;
	color GRID;
	color HOVER;
	color SELECTION;
	color TEXT;
	color TEXT_BG;
	color WINDOW_TITLE;
	color WINDOW_TITLE_BG;
	color POINT;
	color POINT_SELECTED;
	color POINT_SPECIAL;
	color WINDOW_DIVIDER;
	color SELECTION_RECT;
	color SELECTION_RECT_BOUNDARY;
	color CREATION;
	color CREATION_LINE;
	color HELPER_LINE;

	color AXIS[3];

	string FONT_NAME;
	float FONT_SIZE;
	float TEXT_LINE_HEIGHT;
	float BOX_PADDING;
	float BOX_ROUNDNESS;

	float WINDOW_DIVIDER_THICKNESS;

	float LINE_WIDTH_THICK;
	float LINE_WIDTH_MEDIUM;
	float LINE_WIDTH_THIN;
	float LINE_WIDTH_HELPER;
	float LINE_WIDTH_BONE;

	float POINT_RADIUS;
	float POINT_RADIUS_HOVER;
};

extern ColorScheme scheme;

#endif /* SRC_MULTIVIEW_COLORSCHEME_H_ */
