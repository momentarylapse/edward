#include "CodeEditor.h"
#include <lib/syntaxhighlight/Theme.h>
#include <lib/base/iter.h>
#include <lib/syntaxhighlight/BaseParser.h>
#include <lib/os/file.h>
#include <lib/xhui/Theme.h>
#include <lib/xhui/Window.h>
#include <lib/xhui/xhui.h>
#include <lib/xhui/controls/MultilineEdit.h>

#include "lib/os/msg.h"

namespace codeedit {

CodeEditor::CodeEditor() : obs::Node<xhui::Panel>("") {
	from_source(R"foodelim(
Dialog coding-panel ''
	Grid ? ''
		Grid search-grid '' hidden
			Edit search '' width=300
			Button search-next '>' width=25
			Button search-prev '<' width=25
			Label ? '' expandx
		---|
		Grid ? ''
			MultilineEdit edit grabfocus monospace linenumbers focusframe=no
			ListView structure 'symbol' nobar style=compact width=250
)foodelim");
	propagate_events = true;

	id_edit = "edit";

	edit = (xhui::MultilineEdit*)get_control(id_edit);
	edit->set_option("focusframe", "no");
	edit->set_option("monospace", "");
	edit->set_option("fontsize", "12");
//	edit->set_option("lineheightscale", "1.1f");
	edit->set_option("altbg", "");
	edit->set_option("linenumbers", "");

	/*win->event(edit_id, [this, win] {
		int n = edit->cache.lines.num;
		int digits = log10(n);
		win->set_options(lines_id, "width=50");
	});*/

	static int xcounter = 0;
	event_x(id_edit, xhui::event_id::Changed, [this] {
		//msg_write(edit->is_save_state());
		xcounter ++;
		update_highlight_current_line();
		xhui::run_later(2.0f, [this] {
			xcounter --;
			if (xcounter == 0) {
				update_highlight_all();
				update_structure();
			}
		});
		out_changed();
	});
	event_x("structure", xhui::event_id::Select, [this] {
		int n = get_int("structure");
		if (n >= 0 and n < label_line_numbers.num)
			edit->set_cursor_pos(edit->line_pos_to_index({label_line_numbers[n], 0}));
		activate(id_edit);
	});

	xhui::run_later(0.1f, [this] {
		request_redraw();
	});
}

void CodeEditor::update_highlight_current_line() {
	if (auto p = GetParser(filename)) {
		auto lp = edit->index_to_line_pos(edit->cursor_pos);
		int i0 = line_start(lp.line);
		int i1 = line_end(lp.line);

		clear_markings(i0, i1);
		p->prepare_symbols(edit->text, filename);
		for (const auto& m: p->create_markup(edit->text.sub_ref(i0, i1), i0))
			mark_word(m.start, m.end, m.type);
	}
}

void CodeEditor::update_highlight_all() {
	if (auto p = GetParser(filename)) {
		clear_markings(0, edit->text.num);
		p->prepare_symbols(edit->text, filename);
		for (const auto& m: p->create_markup(edit->text, 0))
			mark_word(m.start, m.end, m.type);
	}
}

void CodeEditor::update_structure() {
	if (auto p = GetParser(filename)) {
		reset("structure");
		label_line_numbers.clear();
		for (const auto& l: p->find_labels(edit->text)) {
			add_string("structure", string("        ").repeat(l.level) + l.name);
			label_line_numbers.add(l.line);
		}
	}
}


void CodeEditor::load(const Path& _filename) {
	filename = _filename;
	edit->set_string(os::fs::read_text(filename));
	update_highlight_all();
	update_structure();

	edit->clear_history();
	edit->set_save_state();

	out_changed();
}

void CodeEditor::save(const Path& _filename) {
	filename = _filename;
	os::fs::write_text(filename, edit->text);
	edit->set_save_state();

	out_changed();
}

string CodeEditor::title() const {
	if (filename.is_empty())
		return "new document";
	return filename.basename();
}

bool CodeEditor::is_save_state() const {
	return edit->is_save_state();
}

void CodeEditor::set_save_state() {
	edit->set_save_state();
}

bool CodeEditor::is_undoable() const {
	return edit->is_undoable();
}

bool CodeEditor::is_redoable() const {
	return edit->is_redoable();
}

void CodeEditor::undo() {
	edit->undo();
}

void CodeEditor::redo() {
	edit->redo();
}

int CodeEditor::get_num_lines() const {
	return edit->cache.lines.num;
}

string CodeEditor::get_line(int line_no) const {
	return edit->cache.lines[line_no];
}

CodeEditor::Index CodeEditor::line_start(int line_no) const {
	return edit->cache.line_first_index[line_no];
}

CodeEditor::Index CodeEditor::line_end(int line_no) const {
	return edit->cache.line_first_index[line_no] + edit->cache.line_num_characters[line_no];
}

void CodeEditor::mark_word(Index i0, Index i1, MarkupType type) {
	const auto& c = syntaxhighlight::default_theme->context[(int)type];
	edit->add_markup({i0, i1, c.bold ? xhui::FontFlags::Bold : xhui::FontFlags::None, c.fg});
}


void CodeEditor::clear_markings(Index i0, Index i1) {
	edit->clean_markup(i0, i1);
}

}




