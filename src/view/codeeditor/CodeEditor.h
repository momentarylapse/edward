#pragma once

#include <lib/os/path.h>
#include <lib/pattern/Observable.h>
#include <lib/xhui/controls/MultilineEdit.h>
#include <lib/xhui/Panel.h>

namespace xhui {
class MultilineEdit;
}

enum class MarkupType;

namespace codeedit {

class CodeEditor : public obs::Node<xhui::Panel> {
public:
	explicit CodeEditor();

	void load(const Path& filename);
	void save(const Path& filename);
	string title() const;

	bool is_save_state() const;
	void set_save_state();
	bool is_undoable() const;
	bool is_redoable() const;
	void undo();
	void redo();

	int get_num_lines() const;
	string get_line(int line_no) const;
	using Index = xhui::MultilineEdit::Index;
	Index line_start(int line_no) const;
	Index line_end(int line_no) const;

//	void mark_word(int line_no, int pos0, int num_uchars, int type, char* p0, char* p1);
	void mark_word(Index i0, Index i1, MarkupType type);
	void clear_markings(Index i0, Index i1);

	void update_highlight_current_line();
	void update_highlight_all();

	void update_structure();

	Path filename;
	xhui::MultilineEdit* edit = nullptr;
	string id_edit;

	Array<int> label_line_numbers;
};

}

