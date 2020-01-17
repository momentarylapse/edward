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

void set_wide_lines(float width);
void draw_helper_line(MultiView::Window *win, const vector &a, const vector &b);
void draw_circle(const vector &pos, const vector &n, float radius);


#endif /* SRC_MULTIVIEW_DRAWINGHELPER_H_ */
