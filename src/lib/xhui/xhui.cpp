#include "xhui.h"
#include "Window.h"
#include "Theme.h"
#include "draw/font.h"
#include "../os/time.h"

namespace xhui {

extern Array<Window*> _windows_;

void init() {
	//msg_init();
	glfwInit();

#if HAS_LIB_GL
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#endif
#if HAS_LIB_VULKAN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

	//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	Theme::load_default();

	font::init();
}

void run() {
	while (true) {
		glfwPollEvents();

		for (auto w: _windows_)
			w->_poll_events();

		for (int i=_windows_.num-1; i>=0; i--)
			if (_windows_[i]->_destroy_requested) {
				delete _windows_[i];
				if (_windows_.num == 0)
					return;
			}
		//usleep(8000);
		os::sleep(0.008f);
	};
}

}
