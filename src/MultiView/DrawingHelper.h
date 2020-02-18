/*
 * DrawingHelper.h
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#ifndef SRC_MULTIVIEW_DRAWINGHELPER_H_
#define SRC_MULTIVIEW_DRAWINGHELPER_H_

#include "../lib/base/base.h"

class vector;
namespace MultiView {
	class Window;
}
class string;
class color;
class Image;

void drawing_helper_init();

void set_line_color(const color &c);
void set_wide_lines(float width);
void draw_helper_line(MultiView::Window *win, const vector &a, const vector &b);
void draw_circle(const vector &pos, const vector &n, float radius);


void draw_line_2d(float x1, float y1, float x2, float y2, float depth);
void draw_line(const vector &l1, const vector &l2);
void draw_lines(const Array<vector> &p, bool contiguous);
void draw_lines_colored(const Array<vector> &p, const Array<color> &c, bool contiguous);

enum class TextAlign {
	LEFT,
	CENTER,
	RIGHT
};
void draw_str_bg(int x, int y, const string &str, const color &fg, const color &bg, TextAlign align);
void draw_str(int x, int y, const string &str, TextAlign a = TextAlign::LEFT);
void set_font_size(float size);
void set_font(const string &name, float size);

void render_text(const string &text, Image &im);

#endif /* SRC_MULTIVIEW_DRAWINGHELPER_H_ */
