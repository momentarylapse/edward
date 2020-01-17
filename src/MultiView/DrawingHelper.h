/*
 * DrawingHelper.h
 *
 *  Created on: 17.01.2020
 *      Author: michi
 */

#ifndef SRC_MULTIVIEW_DRAWINGHELPER_H_
#define SRC_MULTIVIEW_DRAWINGHELPER_H_

class vector;
namespace MultiView {
	class Window;
}
class string;
class color;
class Image;

void set_wide_lines(float width);
void draw_helper_line(MultiView::Window *win, const vector &a, const vector &b);
void draw_circle(const vector &pos, const vector &n, float radius);

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
