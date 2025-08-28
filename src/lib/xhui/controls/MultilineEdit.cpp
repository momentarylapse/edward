#include "MultilineEdit.h"

namespace xhui {

MultilineEdit::MultilineEdit(const string &_id, const string &t) : Edit(_id, t) {
	size_mode_y = SizeMode::Expand;
	multiline = true;
	margin_x = 0;
	margin_y = 0;
	//show_focus_frame = false;
}

}
