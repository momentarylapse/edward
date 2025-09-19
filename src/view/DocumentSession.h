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
class MultiView;

namespace xhui {
	class Panel;
}

class DocumentSession : public obs::Node<VirtualBase> {
public:
	explicit DocumentSession(Session* session);
	~DocumentSession() override;

	Session* session;
	base::promise<DocumentSession*> promise_started;

	xhui::Panel* panel;

	void set_mode(Mode *m);
	void set_mode_now(Mode *m);
	Mode *get_mode(int type);

	Mode *mode_none;
	ModeModel* mode_model;
	ModeMaterial* mode_material;
	ModeWorld* mode_world;
	//ModeFont* mode_font;
	//ModeAdministration* mode_admin;
	Mode *cur_mode = nullptr;
	Array<Mode*> mode_queue;
	Mode *find_mode_base(const string &name);
	template<class M>
	M *find_mode(const string &name) {
		return static_cast<M*>(find_mode_base(name));
	}
};

