
#include "xhui.h"
#include "Application.h"
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
#include "../os/app.h"
#include "../ygraphics/graphics-impl.h"


namespace xhui {
	extern Array<Window*> _windows_;

	float global_ui_scale = 1;
	string separator = "\\";


	font::Face* default_font_regular = nullptr;
	font::Face* default_font_bold = nullptr;
	font::Face* default_font_mono_regular = nullptr;
	font::Face* default_font_mono_bold = nullptr;


	Configuration config;

	Flags operator|(Flags a, Flags b) {
		return (Flags)((int)a | (int)b);
	}
	int operator&(Flags a, Flags b) {
		return (int)a & (int)b;
	}

	void create_default_images();

void init(const Array<string> &arg, const string& app_name) {
	os::app::detect(arg, app_name);
	//msg_init();
	glfwInit();

#if HAS_LIB_GL
	glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
#endif
#if HAS_LIB_VULKAN
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif

	//glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

	global_ui_scale = 1.0f;
	glfwGetMonitorContentScale(glfwGetPrimaryMonitor(), &global_ui_scale, nullptr);

	Theme::load_default();

	font::init();

	Array<string> font_names = {"FreeSans", "Cantarell", "OpenSans", "Helvetica", "NotoSans"};

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

	if (os::fs::exists(os::app::directory_dynamic | "config.txt"))
		config.load(os::app::directory_dynamic | "config.txt");


	if ((Application::flags & Flags::DONT_LOAD_RESOURCE) == 0)
		if (os::fs::exists(os::app::directory_static | "hui_resources.txt"))
			load_resource(os::app::directory_static | "hui_resources.txt");

	string def_lang = "English";
	//if (def_lang.num > 0)
		set_language(config.get_str("Language", def_lang));

	create_default_images();
}


void create_default_images() {
	int size = 32;
	float r = 6;
	vec2 m = vec2{(float)size/2, (float)size/2};
	{
		Image im(size, size, color(0,0,0,0));
		auto p = im.start_draw();
		p->set_color(White);
		p->set_line_width(2);
		p->draw_line(m - vec2(r, 0), m + vec2(r, 0));
		p->draw_line(m - vec2(0, r), m + vec2(0, r));
		delete p;
		set_image("hui:plus", im);
	}
	{
		Image im(size, size, color(0,0,0,0));
		auto p = im.start_draw();
		p->set_color(White);
		p->set_line_width(2);
		p->draw_line(m - vec2(r, 0), m + vec2(r, 0));
		delete p;
		set_image("hui:minus", im);
	}
	{
		Image im(size, size, color(0,0,0,0));
		auto p = im.start_draw();
		p->set_color(White);
		p->set_line_width(2);
		p->draw_line(m - vec2(r, 0), m + vec2(r, 0));
		p->draw_line(m - vec2(r, 0), m + vec2(0, r));
		p->draw_line(m + vec2(0, r), m + vec2(r, 0));
		p->draw_line(m - vec2(r/2, 0), m + vec2(r/2, 0));
		p->draw_line(m - vec2(r/2, 0), m + vec2(0, r/2));
		p->draw_line(m + vec2(0, r/2), m + vec2(r/2, 0));
		delete p;
		set_image("hui:triangle-down", im);
	}
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
	const string ActivateDialogDefault = "hui:activate-dialog-default";
	const string Close = "hui:close";
	const string Click = "hui:click";
	const string Changed = "hui:changed";
	const string MouseMove = "hui:mouse-move";
	const string MouseEnter = "hui:mouse-enter";
	const string MouseLeave = "hui:mouse-leave";
	const string MouseWheel = "hui:mouse-wheel";
	const string Draw = "hui:draw";
	const string Initialize = "hui:initialize";
	const string JustBeforeDraw = "hui:just-before-draw";
	const string LeftButtonDown = "hui:left-button-down";
	const string LeftButtonUp = "hui:left-button-up";
	const string MiddleButtonDown = "hui:middle-button-down";
	const string MiddleButtonUp = "hui:middle-button-up";
	const string RightButtonDown = "hui:right-button-down";
	const string RightButtonUp = "hui:right-button-up";
	const string KeyDown = "hui:key-down";
	const string KeyUp = "hui:key-up";
	const string KeyChar = "hui:key-char";
	const string Select = "hui:select";
	const string DragStart = "hui:drag-start";
	const string DragDrop = "hui:drag-drop";
	const string Scroll = "hui:scroll";
};

static owned_array<XImage> _images_;

Path find_image(const string& name) {
	Array<Path> paths;
	paths.add(os::app::directory_static | "icons" | "hicolor" | "64x64" | "actions" | (name + ".png"));
	paths.add(os::app::directory_static | "icons" | "hicolor" | "64x64" | "actions" | (name + ".symbolic.png"));
	paths.add(os::app::directory_static | "icons" | "hicolor" | "24x24" | "actions" | (name + ".png"));
	paths.add(os::app::directory_static | "icons" | "hicolor" | "24x24" | "actions" | (name + ".symbolic.png"));
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
	string uid = format("image:%d", randi(100000000));
	set_image(uid, _im);
	return uid;
}

void set_image(const string& uid, const Image& _im) {
	XImage* im = nullptr;
	if (auto p = base::find_if(weak(_images_), [uid] (XImage* i) {
		return i->uid == uid;
	})) {
		im = *p;
	}
	if (!im) {
		im = new XImage;
		im->uid = uid;
		_images_.add(im);
	}
	if (!im->image)
		im->image = new Image(8,8,White);
	*im->image = _im;
}


string texture_to_image(const shared<ygfx::Texture>& texture) {
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
	if (!image->texture)
#ifdef USING_VULKAN
		if (vulkan::default_device) {
#else
		{
#endif
			if (image->image) {
				image->texture = new ygfx::Texture();
				image->texture->write(*image->image);
			} else {
				image->texture = ygfx::Texture::load(image->filename);
			}
		}
}

vec2 XImage::size() const {
	if (texture)
		return {(float)texture->width, (float)texture->height};
	return {20,20};
}

}

