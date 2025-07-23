//
// Created by michi on 08.06.25.
//

#include "Application.h"
#include "xhui.h"
#include <lib/os/app.h>
#include <lib/os/filesystem.h>

namespace xhui {

bool Application::_end_requested = false;
Array<string> Application::_args;
base::map<string, string> Application::_properties_;
Flags Application::flags;


Application::Application(const string &app_name, const string &def_lang, Flags _flags) {
	flags = _flags;
	set_property("name", app_name);

#if 0

#ifdef HUI_API_GTK
	g_set_prgname(app_name.c_str());
#endif

	os::app::detect(_args, app_name);


	separator = "\\";
	_using_language_ = false;
	//	if ((flags & Flags::NO_ERROR_HANDLER) == 0)
	//		SetDefaultErrorHandler(nullptr);

	if (os::fs::exists(os::app::directory_dynamic | "config.txt"))
		config.load(os::app::directory_dynamic | "config.txt");


	if ((flags & Flags::DONT_LOAD_RESOURCE) == 0)
		load_resource(os::app::directory_static | "hui_resources.txt");

	if (def_lang.num > 0)
		set_language(config.get_str("Language", def_lang));


	// default "logo" used for "about" dialog (full path)
#if defined(OS_LINUX) || defined(OS_MAC)
	if (os::fs::exists(directory_static | "icons" | "hicolor" | "scalable" | "apps" | (app_name + ".svg")))
		set_property("logo", str(directory_static | "icons" | "hicolor" | "scalable" | "apps" | (app_name + ".svg")));
	else if (os::fs::exists(directory_static | "icon.svg"))
		set_property("logo", str(directory_static | "icon.svg"));
	else
#endif
		if (os::fs::exists(os::app::directory_static | "icon.png"))
			set_property("logo", str(os::app::directory_static | "icon.png"));
		else if (os::fs::exists(os::app::directory_static | "icon.ico"))
			set_property("logo", str(os::app::directory_static | "icon.ico"));

	// default "icon" used for windows (just name)
	set_property("icon", app_name);
#endif
}

Application::~Application() = default;



void Application::end() {
	//set_idle_function(nullptr);
	_end_requested = true;
}

static std::function<void()> _run_after_gui_init_func_;

AppStatus Application::on_startup_before_gui_init(const Array<string> &args) {
	return AppStatus::RUN;
}

void Application::run_after_gui_init(std::function<void()> f) {
	_run_after_gui_init_func_ = f;
}

int Application::run() {
	xhui::run();
	on_end();
	return 0;
}

int Application::try_execute(const Array<string> &args) {

	// before init
	auto status = on_startup_before_gui_init(args);
	if (status == AppStatus::END)
		return 0;
	if (status == AppStatus::AUTO and !_run_after_gui_init_func_)
		return 0;

	init(args, get_property("name"));

	// after init
	if (_run_after_gui_init_func_) {
		_run_after_gui_init_func_();
		return run();
	}
	if (on_startup(args) == AppStatus::RUN)
		return run();
	return 0;
}

//void Application::do_single_main_loop() {
//}


void Application::set_property(const string &name, const string &value) {
	_properties_.set(name, value);
}

string Application::get_property(const string &name) {
	try {
		return _properties_[name];
	} catch(...) {
		return "";
	}
}

} // xhui