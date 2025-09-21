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

CodeEditor::CodeEditor(xhui::Panel* _panel, const string& _id) {
	panel = _panel;
	id = _id;

	edit = (xhui::MultilineEdit*)panel->get_control(id);
	edit->set_option("focusframe", "no");
	edit->set_option("monospace", "");
	edit->set_option("fontsize", "15");
	//	edit->set_option("fontsize", "13");
	edit->set_option("lineheightscale", "1.1f");
	edit->set_option("altbg", "");
	edit->set_option("linenumbers", "");

	/*win->event(edit_id, [this, win] {
		int n = edit->cache.lines.num;
		int digits = log10(n);
		win->set_options(lines_id, "width=50");
	});*/

	static int xcounter = 0;
	panel->event_x(id, xhui::event_id::Changed, [this] {
		xcounter ++;
		update_highlight_current_line();
		xhui::run_later(2.0f, [this] {
			xcounter --;
			if (xcounter == 0) {
				update_highlight_all();
			}
		});
	});

	xhui::run_later(0.1f, [this] {
		panel->get_window()->request_redraw();
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


void CodeEditor::load(const Path& _filename) {
	filename = _filename;
	edit->set_string(os::fs::read_text(filename));
	update_highlight_all();

	edit->clear_history();

	out_changed();
}

string CodeEditor::title() const {
	if (filename.is_empty())
		return "new document";
	return filename.basename();
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




