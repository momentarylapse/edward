#if HAS_LIB_GL

#include "Context.h"
#include "Window.h"
#include "Painter.h"
#include "../os/msg.h"
#include "../image/image.h"

namespace xhui {

Context::Context(Window* w) {
	window = w;

	glfwMakeContextCurrent(w->window);
	api_init();
}

bool Context::start() {
	return true;
}



void Context::api_init() {
}


void Context::rebuild_default_stuff() {
}

void Context::resize(int w, int h) {
	rebuild_default_stuff();
}

}

#endif
