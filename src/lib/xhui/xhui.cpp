#include "xhui.h"
#include "Window.h"
#include "Theme.h"
#include "draw/font.h"
#include "../os/time.h"
#include "../os/filesystem.h"

namespace xhui {

extern Array<Window*> _windows_;

float ui_scale = 1;

Path Application::directory;
Path Application::directory_static;
Path Application::initial_working_directory;
Path Application::filename;
bool Application::installed;

void init(const Array<string> &arg, const string& app_name) {
	//msg_init();
	glfwInit();

#if HAS_LIB_GL
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#endif
#if HAS_LIB_VULKAN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

	//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	ui_scale = 1.0f;
	glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &ui_scale, nullptr);


	Application::guess_directories(arg, app_name);

	Theme::load_default();

	font::init();
}

//   filename -> executable file
//   directory ->
//      NONINSTALLED:  binary dir
//      INSTALLED:     ~/.MY_APP/      <<< now always this
//   directory_static ->
//      NONINSTALLED:  binary dir/static/
//      INSTALLED:     /usr/local/share/MY_APP/
//   initial_working_directory -> working dir before running this program
void Application::guess_directories(const Array<string> &arg, const string &app_name) {

	initial_working_directory = os::fs::current_directory();
	installed = false;


	// executable file
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
	if (arg.num > 0)
		filename = arg[0];
#else // OS_WINDOWS
	char *ttt = nullptr;
	int r = _get_pgmptr(&ttt);
	filename = ttt;
	hui_win_instance = (void*)GetModuleHandle(nullptr);
#endif


	// first, assume a local/non-installed version
	directory = initial_working_directory; //strip_dev_dirs(filename.parent());
	directory_static = directory | "static";

#ifdef INSTALL_PREFIX
	// our build system should define this:
	Path prefix = INSTALL_PREFIX;
#else
	// oh no... fall-back
	Path prefix = "/usr/local";
#endif

#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW) //defined(__GNUC__) || defined(OS_LINUX)
	// installed version?
	if (filename.is_in(prefix) or (filename.str().find("/") < 0)) {
		installed = true;
		directory_static = prefix | "share" | app_name;
	}

	// inside an AppImage?
	if (getenv("APPIMAGE")) {
		installed = true;
		directory_static = Path(getenv("APPDIR")) | "usr" | "share" | app_name;
	}

	// inside MacOS bundle?
	if (str(filename).find(".app/Contents/MacOS/") >= 0) {
		installed = true;
		directory_static = filename.parent().parent() | "Resources";
	}

	directory = format("%s/.%s/", getenv("HOME"), app_name);
	os::fs::create_directory(directory);
#endif
}

struct Runner {
	bool used, repeat;
	float dt;
	float t = 0;
	Callback f;
	int id;
};
static Array<Runner*> runners;

Runner* create_runner() {
	for (auto r: runners)
		if (!r->used) {
			r->used = true;
			return r;
		}
	auto r = new Runner;
	r->used = true;
	r->id = runners.num;
	runners.add(r);
	return r;
}

int run_repeated(float dt, Callback f) {
	auto r = create_runner();
	r->f = f;
	r->dt = dt;
	r->repeat = true;
	return r->id;
}
int run_later(float dt, Callback f) {
	auto r = create_runner();
	r->f = f;
	r->dt = dt;
	r->repeat = false;
	return r->id;

}
void cancel_runner(int id) {
	if (id >= 0 and id < runners.num)
		runners[id]->used = false;
}

void iterate_runners(float dt) {
	for (auto r: runners)
		if (r->used) {
			r->t += dt;
			if (r->t >= r->dt) {
				r->f();
				if (r->repeat)
					r->t = 0;
				else
					r->used = false;
			}
		}
}

void run() {
	os::Timer timer;
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

		iterate_runners(timer.get());

		//usleep(8000);
		os::sleep(0.008f);
	};
}

}
