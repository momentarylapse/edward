#include "MultilineEdit.h"

namespace xhui {

MultilineEdit::MultilineEdit(const string &_id, const string &t) : Edit(_id, t) {
	size_mode_y = SizeMode::Expand;
	multiline = true;
}

}
