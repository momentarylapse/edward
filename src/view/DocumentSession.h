//
// Created by michi on 9/19/25.
//

#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/future.h>
#include <lib/os/time.h>
#include <lib/os/path.h>
#include <lib/pattern/Observable.h>

class Session;
class Mode;

class ModeModel;
class ModeMaterial;
class ModeWorld;
class ModeCoding;
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
	void set_mode_now(Mode* m);
	Mode* get_mode(int type);

	string title() const;

	Mode* mode_none;
	ModeModel* mode_model;
	ModeMaterial* mode_material;
	ModeWorld* mode_world;
	ModeCoding* mode_coding;
	//ModeFont* mode_font;
	//ModeAdministration* mode_admin;
	Mode* cur_mode = nullptr;
	Array<Mode*> mode_queue;
	Mode* find_mode_base(const string &name);
	template<class M>
	M* find_mode(const string &name) {
		return static_cast<M*>(find_mode_base(name));
	}

	int _last_usage_counter = -1;


	Array<int> event_ids;
	void event(const string &id, const std::function<void()>& f);
	void enter();
	void leave();
};

/*class DocumentPanel : public xhui::Panel {
public:
	DocumentPanel();
};*/

