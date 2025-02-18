
#include <lib/kaba/lib/lib.h>
#include <storage/format/Format.h>

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

	kaba::init();

	auto s = create_session();
	if (args.num >= 2) {
		string ext = Path(args[1]).extension();
		if (ext == "model")
			s->universal_edit(FD_MODEL, args[1], false);
		else if (ext == "world")
			s->universal_edit(FD_WORLD, args[1], false);
	} else {
		s->universal_new(FD_MODEL);
	}

	try {
		xhui::run();
	} catch (Exception& e) {
		msg_error(e.message());
	}
	return 0;
}

