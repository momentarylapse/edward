/*
 * Font.h
 *
 *  Created on: Feb 2, 2021
 *      Author: michi
 */

#pragma once

//#include "../lib/base/base.h"
//#include "gui.h"
#include "Node.h"

class Image;

namespace gui {

class Font {
public:
	void *face = nullptr;
	string name;
	int line_height;
	float line_y_offset;

	static Font *load(const string &name);
	static void init_fonts();

	void render_text(const string &str, Node::Align align, Image &im);
	int get_width(const string &str);
	int get_height(const string &str);
	float get_height_rel(const string &str);

	static const float FONT_SIZE;
	//static const float LINE_GAP;
	//static const float LINE_Y_OFFSET;
	static Font *_default;
};

}
