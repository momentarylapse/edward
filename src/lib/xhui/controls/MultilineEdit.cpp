#include "MultilineEdit.h"

namespace xhui {

MultilineEdit::MultilineEdit(const string &_id, const string &t) : Edit(_id, t) {
	expand_y = true;
	multiline = true;
}

}
