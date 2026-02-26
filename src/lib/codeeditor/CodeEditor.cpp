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
		Overlay ? ''
			MultilineEdit edit '' grabfocus monospace linenumbers focusframe=no
			Grid ? ''
				Label ? '' height=25
				---|
				Label ? '' expandx
				Grid search-grid '' class=card hidden
					Edit search-pattern '' width=300
					Grid ? ''
						Button search-next '>' width=25
						Button search-prev '<' width=25
						Button search-close 'x' width=25
					---|
					Edit search-replace '' noexpandx
					Grid ? ''
						Button search-replace-next 'replace' noexpandx
				Label ? '' width=25
				---|
					Label ? '' expandy
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
#ifdef OS_MAC
	constexpr int mod = xhui::KEY_SUPER;
#else
	constexpr int mod = xhui::KEY_CONTROL;
#endif
	event_x(id_edit, xhui::event_id::KeyDown, [this] {
		if (auto w = get_window()) {
			if (w->state.key_code == xhui::KEY_F + mod) {
				search_start();
			} else if (w->state.key_code == xhui::KEY_R + mod) {
				search_start_replace();
			} else if (w->state.key_code == xhui::KEY_B + mod) {
				show_errors();
			}
		}
	});
	event_x("search-pattern", xhui::event_id::Changed, [this] {
		search_find(search_pos);
	});
	event_x("search-pattern", xhui::event_id::ActivateDialogDefault, [this] {
		// [Return]
		search_next();
	});
	event_x("search-pattern", xhui::event_id::KeyDown, [this] {
		if (auto w = get_window()) {
			if (w->state.key_code == xhui::KEY_ESCAPE) {
				search_end();
			}
		}
	});
	event("search-next", [this] {
		search_next();
	});
	event_x("search-replace", xhui::event_id::Changed, [this] {
		search_find(search_pos);
	});
	event_x("search-replace", xhui::event_id::ActivateDialogDefault, [this] {
		// [Return]
		search_replace_next();
	});
	event_x("search-replace", xhui::event_id::KeyDown, [this] {
		if (auto w = get_window()) {
			if (w->state.key_code == xhui::KEY_ESCAPE) {
				search_end();
			}
		}
	});
	event("search-close", [this] {
		search_end();
	});
	event("search-replace-next", [this] {
		search_replace_next();
	});
	event_x("structure", xhui::event_id::Select, [this] {
		int n = get_int("structure");
		if (n >= 0 and n < label_line_numbers.num)
			edit->set_cursor_pos(edit->line_pos_to_index({label_line_numbers[n], 0}));
		activate(id_edit);
	});

	xhui::run_later(0.1f, [this] {
		activate(id_edit);
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

void CodeEditor::search_start() {
	int p0 = min(edit->selection_start, edit->cursor_pos);
	int p1 = max(edit->selection_start, edit->cursor_pos);
	search_pos = p0;
	set_visible("search-grid", true);
	activate("search-pattern");

	if (p0 == p1)
		search_find(search_pos);
	else
		set_string("search-pattern", edit->text.sub(p0, p1).escape());
}

void CodeEditor::search_start_replace() {
	search_start();
}

void CodeEditor::search_end() {
	set_visible("search-grid", false);
	activate(id_edit);
}

bool CodeEditor::search_find(int pos0) {
	const string pattern = get_string("search-pattern").unescape();
	if (pattern.num == 0)
		return false;
	int p1 = edit->text.find(pattern, pos0);
	if (p1 > 0) {
		edit->set_cursor_pos(p1);
		edit->set_cursor_pos(p1 + pattern.num, true);
		return true;
	}
	return false;
}

void CodeEditor::search_next() {
	int p0 = min(edit->selection_start, edit->cursor_pos);
	if (search_find(p0 + 1)) {
		search_pos = edit->selection_start;
	}
}

void CodeEditor::search_replace_next() {
	const string filler = get_string("search-replace").unescape();
	if (filler.num == 0)
		return;

	int p0 = min(edit->selection_start, edit->cursor_pos);
	int p1 = max(edit->selection_start, edit->cursor_pos);
	if (search_find(p0)) {
		edit->replace_range(p0, p1, filler);
	}
	if (search_find(p0 + 1)) {
		search_pos = edit->selection_start;
	}
}

void CodeEditor::show_errors() {
	if (auto p = GetParser(filename)) {
		auto errors = p->find_errors(edit->text);
		if (errors.num > 0) {
			edit->set_cursor_pos(errors[0].position);
			out_error(errors[0].message);
		} else {
			out_no_error();
		}
	}
}
}




