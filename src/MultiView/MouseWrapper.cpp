/*
 * MouseWrapper.cpp
 *
 *  Created on: Jan 15, 2021
 *      Author: michi
 */


#include "MouseWrapper.h"
#include "../lib/math/math.h"
#include "MultiView.h"

#define HAS_LIB_X11 1

#if HAS_LIB_X11

#include <X11/Xlib.h>
#include <X11/extensions/XInput2.h>
#include <X11/Xmd.h>

#include <gdk/gdkx.h>
#endif


namespace MouseWrapper {

#if HAS_LIB_X11
static Display* x11_display = nullptr;
static int x11_runner = -1;
static ::Window x11_window;
#endif

static int x0, y0;
static int dx, dy;
static int x_hold, y_hold;
static bool holding = false;

#if HAS_LIB_X11
static auto get_x11_window(hui::Window *w) {
	auto ww = gtk_widget_get_window(w->window);
	if (!ww)
		return 0uL;
	return GDK_WINDOW_XID(ww);
	//return gdk_x11_window_get_xid(ww);
}


void x11_set_cursor(int x, int y) {
	XWarpPointer(x11_display, None, x11_window, 0,0,0,0, x, y);
	XFlush(x11_display);
}

void x11_get_cursor(int &x, int &y) {
	::Window root, child;
	int root_x, root_y;
	unsigned int mask;

	XQueryPointer(x11_display, x11_window,
		&root, &child,
		&root_x, &root_y, &x, &y,
		&mask);
}
#endif

void start(hui::Window *w) {
#if HAS_LIB_X11

	if (!x11_display)
		x11_display = XOpenDisplay(nullptr);

	x11_window = get_x11_window(w);
	if (x11_window == 0)
		return;

	dx = dy = 0;
	int width, height;
	w->get_size(width, height);
	// argh, ignoring display scaling...
	x_hold = width/2;
	y_hold = height/2;
	holding = true;

	w->show_cursor(false);
	x11_get_cursor(x0, y0);
	x11_set_cursor(x_hold, y_hold);

	x11_runner = hui::RunRepeated(0.01f, [=] {
		int xx, yy;
		x11_get_cursor(xx, yy);

		dx += (xx - x_hold);
		dy += (yy - y_hold);

		x11_set_cursor(x_hold, y_hold);
	});
#endif
}

void stop(hui::Window *w) {
	if (!holding)
		return;

#if HAS_LIB_X11
	hui::CancelRunner(x11_runner);
	x11_runner = -1;

	x11_set_cursor(x0, y0);
	w->show_cursor(true);
#endif
	holding = false;
}

void update(MultiView::MultiView *view) {
#if HAS_LIB_X11
	view->v = vector(dx, dy, 0);
#endif
	dx = dy = 0;
}

};





