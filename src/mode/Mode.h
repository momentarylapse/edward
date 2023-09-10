/*
 * Mode.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODE_H_
#define MODE_H_

#include "../lib/hui/hui.h"
#include "../lib/math/math.h"
#include "../lib/pattern/Observable.h"
#include "../data/Data.h"

namespace MultiView {
	class MultiView;
	class Window;
};

class EdwardWindow;

class ModeBase : public obs::Node<hui::EventHandler> {
public:
	ModeBase(EdwardWindow *ed, const string &name, ModeBase *parent, MultiView::MultiView *multi_view, const string &menu_id);
	virtual ~ModeBase();

	// Start/End: (once) entering this mode or a sub mode
	virtual void on_start() {};
	virtual void on_end() {};
	// Enter/Leave: exactly this mode
	virtual void on_enter() {};
	virtual void on_leave() {};

	// gets called when multiview changes
	virtual void on_selection_change() {};
	virtual void on_view_stage_change() {};
	// gets called when entering
	virtual void on_set_multi_view() {};

	// events to be handled by derived modes
	virtual void on_mouse_move() {};
	virtual void on_mouse_wheel() {};
	virtual void on_mouse_enter() {};
	virtual void on_mouse_leave() {};
	virtual void on_left_button_down() {};
	virtual void on_left_button_up() {};
	virtual void on_middle_button_down() {};
	virtual void on_middle_button_up() {};
	virtual void on_right_button_down() {};
	virtual void on_right_button_up() {};
	virtual void on_key_down(int key_code){};
	virtual void on_key_up(int key_code){};
	virtual void on_command(const string &id){};
	virtual void on_update_menu() {};

	void on_update_menu_recursive();
	void on_command_recursive(const string &id);

	virtual void on_draw() {};
	virtual void on_draw_win(MultiView::Window *win){};

	virtual bool optimize_view(){ return false; };
	virtual void optimize_view_recursice();

	ModeBase *get_root();
	bool is_ancestor_of(ModeBase *m);
	ModeBase *get_next_child_to(ModeBase *target);
	bool equal_roots(ModeBase *m);
	virtual Data *get_data() = 0;

	ModeBase *find_mode_base(const string &name);
	template<class M>
	M *find_mode(const string &name) {
		return static_cast<M*>(find_mode_base(name));
	}

	string name;

	EdwardWindow *ed;
	ModeBase *parent_untyped;
	MultiView::MultiView *multi_view;

	string menu_id;
};

template<class M, class T>
class Mode : public ModeBase {
public:
	Mode(EdwardWindow *ed, const string &name, M *_parent, Data *_data, MultiView::MultiView *multi_view, const string &menu) :
		ModeBase(ed, name, _parent, multi_view, menu)
	{
		parent = _parent;
		data = (T*)_data;
	}
	Mode(EdwardWindow *ed, const string &name, M *_parent, MultiView::MultiView *multi_view, const string &menu) :
		ModeBase(ed, name, _parent, multi_view, menu)
	{
		parent = _parent;
		data = (T*)parent->get_data();
	}
	M *parent;
	T *data;
	Data *get_data() override {
		return (Data*)data;
	}
};

#endif /* MODE_H_ */
