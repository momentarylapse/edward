#pragma once

#include <lib/os/path.h>
#include <lib/pattern/Observable.h>
#include <lib/xhui/controls/MultilineEdit.h>

namespace xhui {
class MultilineEdit;
}

enum class MarkupType;

namespace codeedit {

class DocumentEditor : public obs::Node<VirtualBase> {
public:
	explicit DocumentEditor();
	void create_controls(xhui::Window* win, int index);

	void load(const Path& filename);
	string title() const;

	int get_num_lines() const;
	string get_line(int line_no) const;
	using Index = xhui::MultilineEdit::Index;
	Index line_start(int line_no) const;
	Index line_end(int line_no) const;

//	void mark_word(int line_no, int pos0, int num_uchars, int type, char* p0, char* p1);
	void mark_word(Index i0, Index i1, MarkupType type);
	void clear_markings(int first_line, int last_line);

	Path filename;
	xhui::MultilineEdit* edit = nullptr;
	string grid_id, __lines_id, edit_id;
};

}

