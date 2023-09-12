/*
 * Session.h
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#ifndef SRC_SESSION_H_
#define SRC_SESSION_H_

#include "lib/base/base.h"
#include "lib/base/map.h"
#include "lib/base/future.h"
#include "lib/os/time.h"
#include "lib/os/path.h"
#include "lib/pattern/Observable.h"

namespace nix {
	class Texture;
	class Context;
};
namespace MultiView {
	class MultiView;
};

class ModeBase;
class ModeModel;
class ModeMaterial;
class ModeWorld;
class ModeFont;
class ModeAdministration;
class Storage;
class Progress;
class ResourceManager;
class DrawingHelper;
class EdwardWindow;



// multiview data
enum {
	MVD_MODEL_SURFACE,
	MVD_MODEL_VERTEX,
	MVD_MODEL_SKIN_VERTEX,
	MVD_MODEL_POLYGON,
	MVD_MODEL_EDGE,
	MVD_MODEL_BALL,
	MVD_MODEL_CYLINDER,
	MVD_MODEL_POLYHEDRON,
	MVD_SKELETON_BONE,
	MVD_WORLD_OBJECT,
	MVD_WORLD_TERRAIN,
	MVD_WORLD_TERRAIN_VERTEX,
	MVD_WORLD_LIGHT,
	MVD_WORLD_LINK,
	MVD_WORLD_SCRIPT,
	MVD_WORLD_CAMERA,
	MVD_WORLD_CAM_POINT,
	MVD_WORLD_CAM_POINT_VEL,
};


class Session : public obs::Node<VirtualBase> {
public:
	Session();
	~Session() override;

	void create_initial_resources(nix::Context *gl);

	void set_mode(ModeBase *m);
	void set_mode_now(ModeBase *m);

	void on_command(const string &id);
	void on_close();

	ModeBase *get_mode(int type);
	void universal_new(int type);
	void universal_open(int preferred_type);
	void universal_edit(int type, const Path &filename, bool relative_path);

	void set_message(const string &message);
	void remove_message();
	void error(const string &message);
	Array<string> message_str;

	base::future<void> allow_termination();

	EdwardWindow *win;

	ModeBase *mode_none;
	ModeModel* mode_model;
	ModeMaterial* mode_material;
	ModeWorld* mode_world;
	ModeFont* mode_font;
	ModeAdministration* mode_admin;
	ModeBase *cur_mode;
	Array<ModeBase*> mode_queue;
	ModeBase *find_mode_base(const string &name);
	template<class M>
	M *find_mode(const string &name) {
		return static_cast<M*>(find_mode_base(name));
	}

	nix::Context *gl;
	ResourceManager *resource_manager;
	DrawingHelper *drawing_helper;

	Storage *storage;
	MultiView::MultiView *multi_view_2d;
	MultiView::MultiView *multi_view_3d;

	Progress *progress;

	base::map<nix::Texture*, string> icon_image;
	string get_tex_image(nix::Texture *tex);

	os::Timer timer;

	base::promise<void> promise_started;
};

Session *create_session();

#endif /* SRC_SESSION_H_ */
