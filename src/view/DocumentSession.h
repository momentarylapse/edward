//
// Created by michi on 9/19/25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/future.h>
#include <lib/os/time.h>
#include <lib/os/path.h>
#include <lib/obs/Observable.h>

class Any;
class Session;
class Mode;
class MultiView;
class DocumentPanel;

namespace xhui {
	class Panel;
}

class DocumentSession : public obs::Node<VirtualBase> {
public:
	explicit DocumentSession(Session* session);
	~DocumentSession() override;

	obs::source out_started{this, "started"};

	Session* session;
	base::promise<DocumentSession*> promise_started;

	shared<xhui::Panel> base_panel;
	shared<xhui::Panel> document_panel;
	void set_document_panel(shared<xhui::Panel> panel);
	string grid_id;

	void set_mode(Mode* m);
	void set_root_mode(const string &id, const Any& params);
	void set_sub_mode(const string &id, const Any& params);

	Path filename() const;
	int file_type() const;
	bool is_save_state() const;
	string title(bool with_markup = false) const;

	owned<Mode> root_mode;
	template<class M>
	M* root_mode_as() {
		return reinterpret_cast<M*>(root_mode.get());
	}
	/*owned<Mode> mode_none;
	owned<ModeModel> mode_model;
	owned<ModeMaterial> mode_material;
	owned<ModeWorld> mode_world;
	owned<ModeCoding> mode_coding;
	//ModeFont* mode_font;
	//ModeAdministration* mode_admin;*/

	Mode* cur_mode = nullptr;
	Array<Mode*> mode_queue;

	int _last_usage_counter = -1;


	Array<int> event_ids;
	void event(const string &id, const std::function<void()>& f);
	void enter();
	void leave();

private:
	void set_mode_now(Mode* m);
};

/*class DocumentPanel : public xhui::Panel {
public:
	DocumentPanel();
};*/

