#include "DocumentEditor.h"

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

DocumentEditor::DocumentEditor() = default;

void DocumentEditor::create_controls(xhui::Window* win, int index) {

	static int counter = 0;
	grid_id = format("grid-%d", counter);
	edit_id = format("edit-%d", counter);
//	lines_id = format("lines-%d", counter);
	counter ++;

	win->set_target("tab");
	win->add_control("Grid", "", index, 0, grid_id);
	win->set_options(grid_id, "spacing=0");
	win->set_target(grid_id);
//	win->add_control("DrawingArea", "", 0, 0, lines_id);
//	win->set_options(lines_id, "width=50,hidden");
	win->add_control("MultilineEdit", "", 1, 0, edit_id);
	edit = (xhui::MultilineEdit*)win->get_control(edit_id);
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
	win->event_x(edit_id, xhui::event_id::Changed, [this] {
		xcounter ++;
		xhui::run_later(2.0f, [this] {
			xcounter --;
			if (xcounter == 0) {
				if (auto p = GetParser(filename)) {
					clear_markings(0, edit->text.explode("\n").num);
					p->prepare_symbols(edit->text, filename);
					for (const auto& m: p->create_markup(edit->text, 0))
						mark_word(m.start, m.end, m.type);
				}

			}
		});
	});

	xhui::run_later(0.1f, [this, win] {
		win->request_redraw();
	});
}


void DocumentEditor::load(const Path& _filename) {
	filename = _filename;
	edit->set_string(os::fs::read_text(filename));

	if (auto p = GetParser(filename)) {
		p->prepare_symbols(edit->text, filename);
		for (const auto& m: p->create_markup(edit->text, 0))
			mark_word(m.start, m.end, m.type);
	}

	edit->clear_history();

	out_changed();
}

string DocumentEditor::title() const {
	if (filename.is_empty())
		return "new document";
	return filename.basename();
}

int DocumentEditor::get_num_lines() const {
	return edit->cache.lines.num;
}

string DocumentEditor::get_line(int line_no) const {
	return edit->cache.lines[line_no];
}

DocumentEditor::Index DocumentEditor::line_start(int line_no) const {
	return edit->cache.line_first_index[line_no];
}

DocumentEditor::Index DocumentEditor::line_end(int line_no) const {
	return edit->cache.line_first_index[line_no] + edit->cache.line_num_characters[line_no];
}

void DocumentEditor::mark_word(Index i0, Index i1, MarkupType type) {
	const auto& c = syntaxhighlight::default_theme->context[(int)type];
	edit->add_markup({i0, i1, c.bold ? xhui::FontFlags::Bold : xhui::FontFlags::None, c.fg});
}


void DocumentEditor::clear_markings(int first_line, int last_line) {
	edit->clean_markup(line_start(first_line), line_end(last_line));
}

}




