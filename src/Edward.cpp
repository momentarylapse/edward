
#include "Session.h"
#include "view/EdwardWindow.h"
#include "lib/os/msg.h"

string AppVersion = "0.5.-1.0";
string AppName = "Edward";

//EdwardApp *app = nullptr;
void* app = nullptr;

namespace hui {
	string get_language_s(const string& lang) {
		return "";
	}
}

int xhui_main(const Array<string>& args) {

	try {
		xhui::init(args, "edward");
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	auto s = create_session();
	s->win->args = args;

	try {
		xhui::run();
	} catch (Exception& e) {
		msg_error(e.message());
	}
	return 0;
}

