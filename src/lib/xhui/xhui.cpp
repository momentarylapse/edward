
#include "xhui.h"
#include "Resource.h"
#include "Window.h"
#include "Dialog.h"
#include "Theme.h"
#include "draw/font.h"
#include "config.h"
#include "language.h"
#include "../base/algo.h"
#include "../image/image.h"
#include "../os/time.h"
#include "../os/filesystem.h"
#include "../os/msg.h"
#include "../vulkan/Texture.h"
#include "../vulkan/Device.h"
#include "../nix/nix_textures.h"


namespace xhui {
	extern Array<Window*> _windows_;

	float ui_scale = 1;

	Path Application::directory;
	Path Application::directory_static;
	Path Application::initial_working_directory;
	Path Application::filename;
	bool Application::installed;
	bool Application::_end_requested = false;


	font::Face* default_font_regular = nullptr;
	font::Face* default_font_bold = nullptr;
	font::Face* default_font_mono_regular = nullptr;
	font::Face* default_font_mono_bold = nullptr;


	Configuration config;

	Array<string> make_args(int num_args, char *args[]) {
		Array<string> a;
		for (int i=0; i<num_args; i++)
			a.add(args[i]);
		return a;
	}
};

int xhui_main(const Array<string> &);

	// for a system independent usage of this library

#ifdef OS_WINDOWS

	int main(int num_args, char* args[]) {
		return xhui_main(hui::make_args(num_args, args));
	}

#ifdef _CONSOLE

	int _tmain(int num_args, _TCHAR *args[]) {
		return xhui_main(hui::make_args(num_args, args));
	}

#else

	// split by space... but parts might be in quotes "a b"
	Array<string> parse_command_line(const string& s) {
		Array<string> a;
		a.add("-dummy-");

		for (int i=0; i<s.num; i++) {
			if (s[i] == '\"') {
				string t;
				bool escape = false;
				i ++;
				for (int j = i; j<s.num; j++) {
					i = j;
					if (escape) {
						escape = false;
					} else {
						if (s[j] == '\\')
							escape = true;
						else if (s[j] == '\"')
							break;
					}
					t.add(s[j]);
				}
				a.add(t.unescape());
				i ++;
			} else if (s[i] == ' ') {
				continue;
			} else {
				string t;
				for (int j=i; j<s.num; j++) {
					i = j;
					if (s[j] == ' ')
						break;
					t.add(s[j]);
				}
				a.add(t);
			}
		}
		return a;
	}

	int APIENTRY WinMain(HINSTANCE hInstance,
						 HINSTANCE hPrevInstance,
						 LPTSTR    lpCmdLine,
						 int       nCmdShow)
	{
		return xhui_main(parse_command_line(lpCmdLine));
	}

#endif

#endif
#if defined(OS_LINUX) || defined(OS_MAC) || defined(OS_MINGW)

	int main(int num_args, char *args[]) {
		return xhui_main(xhui::make_args(num_args, args));
	}

#endif


namespace xhui {

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

	Array<string> font_names = {"OpenSans", "Helvetica", "NotoSans"};

	for (const string& name: font_names) {
		if (!default_font_regular)
			default_font_regular = font::load_face(name, false, false);
		if (!default_font_bold)
			default_font_bold = font::load_face(name, true, false);
	}
	if (default_font_regular and !default_font_bold)
		default_font_bold = default_font_regular;
	if (!default_font_regular)
		msg_error("no font found...");

	Array<string> font_names_mono = {"Menlo", "Courier New", "FreeMono", "NotoSansMono", "AdwaitaMono"};
	for (const string& name: font_names_mono) {
		if (!default_font_mono_regular)
			default_font_mono_regular = font::load_face(name, false, false);
		if (!default_font_mono_bold)
			default_font_mono_bold = font::load_face(name, true, false);
	}
	// TODO font library!

	if (os::fs::exists(Application::directory | "config.txt"))
		config.load(Application::directory | "config.txt");


	//if ((flags & Flags::DONT_LOAD_RESOURCE) == 0)
	if (os::fs::exists(Application::directory_static | "hui_resources.txt"))
		load_resource(Application::directory_static | "hui_resources.txt");

	string def_lang = "English";
	//if (def_lang.num > 0)
		set_language(config.get_str("Language", def_lang));
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

void Application::end() {
	_end_requested = true;
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
			r->t = 0;
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
	while (!Application::_end_requested) {
		glfwPollEvents();

		for (auto w: _windows_)
			w->_handle_events();

		for (auto w: _windows_)
			if (w->dialogs.num > 0)
				if (w->dialogs.back()->_destroy_requested)
					w->close_dialog(w->dialogs.back().get());

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

namespace clipboard {
	void copy(const string& text) {
		glfwSetClipboardString(nullptr, text.c_str());
	}
	string paste() {
		if (const char* text = glfwGetClipboardString(nullptr))
			return text;
		return "";
	}
}

namespace event_id {
	const string Activate = "hui:activate";
	const string Close = "hui:close";
	const string Click = "hui:click";
	const string Changed = "hui:changed";
	const string MouseMove = "hui:mouse-move";
	const string MouseEnter = "hui:mouse-enter";
	const string MouseLeave = "hui:mouse-leave";
	const string MouseWheel = "hui:mouse-wheel";
	const string Draw = "hui:draw";
	const string Initialize = "hui:initialize";
	const string LeftButtonDown = "hui:left-button-down";
	const string LeftButtonUp = "hui:left-button-up";
	const string MiddleButtonDown = "hui:middle-button-down";
	const string MiddleButtonUp = "hui:middle-button-up";
	const string RightButtonDown = "hui:right-button-down";
	const string RightButtonUp = "hui:right-button-up";
	const string KeyDown = "hui:key-down";
	const string KeyUp = "hui:key-up";
	const string Select = "hui:select";
	const string DragStart = "hui:drag-start";
	const string DragDrop = "hui:drag-drop";
	const string Scroll = "hui:scroll";
};

static owned_array<XImage> _images_;

Path find_image(const string& name) {
	Array<Path> paths;
	paths.add(Application::directory_static | "icons" | "hicolor" | "64x64" | "actions" | (name + ".png"));
	paths.add(Application::directory_static | "icons" | "hicolor" | "64x64" | "actions" | (name + ".symbolic.png"));
	paths.add(Application::directory_static | "icons" | "hicolor" | "24x24" | "actions" | (name + ".png"));
	paths.add(Application::directory_static | "icons" | "hicolor" | "24x24" | "actions" | (name + ".symbolic.png"));
	for (const Path& p: paths)
	if (os::fs::exists(p))
		return p;
	return "";
}

XImage* load_image(const string& name) {
	for (auto* im: weak(_images_))
		if (im->uid == name)
			return im;

	const auto path = find_image(name);
	if (path.is_empty())
		return nullptr;

	auto im = new XImage;
	im->filename = path;
	im->uid = name;
#if HAS_LIB_VULKAN
	if (vulkan::default_device)
		im->texture = vulkan::Texture::load(path);
#endif
	_images_.add(im);
	return im;
}

string create_image(const Image& _im) {
	auto im = new XImage;
	im->image = new Image(8,8,White);
	*im->image = _im;
	im->uid = format("image:%d", randi(100000000));
#if HAS_LIB_VULKAN
	if (vulkan::default_device) {
		im->texture = new vulkan::Texture();
		im->texture->write(*im->image);
	}
#else
	im->texture = new Texture();
	im->texture->write(*im->image);
#endif
	_images_.add(im);
	return im->uid;
}


string texture_to_image(const shared<Texture>& texture) {
	for (auto* im: weak(_images_))
		if (im->texture == texture.get())
			return im->uid;

	auto im = new XImage;
	im->uid = format("image:texture:%s", p2s(texture.get()));
	im->texture = texture;
	_images_.add(im);
	return im->uid;
}

void delete_image(const string& name) {
	base::remove_if(_images_, [name] (XImage* im) {
		return im->uid == name;
	});
}

void prepare_image(XImage* image) {
#if HAS_LIB_VULKAN
	if (!image->texture)
		if (vulkan::default_device) {
			if (image->image) {
				image->texture = new Texture();
				image->texture->write(*image->image);
			} else {
				image->texture = vulkan::Texture::load(image->filename);
			}
		}
#else
	if (!image->texture) {
		if (image->image) {
			image->texture = new Texture();
			image->texture->write(*image->image);
		} else {
			image->texture = nix::Texture::load(image->filename);
		}
	}
#endif
}

vec2 XImage::size() const {
	if (texture)
		return {(float)texture->width, (float)texture->height};
	return {20,20};
}

}

