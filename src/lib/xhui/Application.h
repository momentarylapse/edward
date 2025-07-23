#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <functional>

class Path;

namespace xhui {

enum class Flags;

enum class AppStatus {
	END,
	RUN,
	AUTO
};

class Application : public VirtualBase {
public:
	Application(const string &app_name, const string &def_lang, Flags flags);
	~Application() override;

	virtual AppStatus on_startup_before_gui_init(const Array<string> &arg);
	void run_after_gui_init(std::function<void()> f);

	virtual AppStatus on_startup(const Array<string> &arg) = 0;
	virtual void on_end() {}
	int run();

	static void end();
	static void _cdecl set_property(const string &name, const string &value);
	static string get_property(const string &name);
	static Flags flags;

	static base::map<string, string> _properties_;

	static bool _end_requested;

	static Array<string> _args;

	int try_execute(const Array<string> &args);
};

} // xhui



#define XHUI_EXECUTE(APP_CLASS) \
namespace os::app { \
int main(const Array<string> &args) { \
	APP_CLASS::_args = args; \
	APP_CLASS *app = new APP_CLASS; \
	int r = app->try_execute(args); \
	delete app; \
return r; \
} \
}

