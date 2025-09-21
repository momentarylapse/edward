/*
 * Session.h
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#pragma once

#include <lib/base/base.h>
#include <lib/base/map.h>
#include <lib/base/future.h>
#include <lib/os/time.h>
#include <lib/os/path.h>
#include <lib/pattern/Observable.h>
#include <lib/ygraphics/graphics-fwd.h>
#include <lib/yrenderer/Context.h>


class MultiView;
class Mode;
class ModeModel;
class ModeMaterial;
class ModeWorld;
class ModeFont;
class ModeAdministration;
class Storage;
class Progress;
class ResourceManager;
namespace edward {
	class PluginManager;
}
class DrawingHelper;
class EdwardWindow;
namespace kaba {
	class Context;
}
class DocumentSession;



// multiview data
enum class MultiViewType {
	MODEL_SURFACE,
	MODEL_VERTEX,
	MODEL_SKIN_VERTEX,
	MODEL_POLYGON,
	MODEL_EDGE,
	MODEL_BALL,
	MODEL_CYLINDER,
	MODEL_POLYHEDRON,
	SKELETON_BONE,
	WORLD_ENTITY,
	WORLD_OBJECT,
	WORLD_TERRAIN,
	WORLD_TERRAIN_VERTEX,
	WORLD_LIGHT,
	WORLD_LINK,
	WORLD_SCRIPT,
//	WORLD_CAMERA,
	WORLD_CAM_POINT,
	WORLD_CAM_POINT_VEL,
	ACTION_MANAGER,
	GRID,
	_NUM
};


class Session : public obs::Node<VirtualBase> {
public:
	Session();
	~Session() override;

	obs::source out_started{this, "started"};

	void on_command(const string &id);
	void on_close();

	void universal_new(int type);
	void universal_open(int preferred_type);
	void universal_edit(int type, const Path &filename, bool relative_path);

	void set_message(const string &message);
	void remove_message();
	void error(const string &message);
	Array<string> message_str;

	base::future<void> allow_termination();

	EdwardWindow *win;

	yrenderer::Context *ctx;
	ResourceManager *resource_manager;
	DrawingHelper *drawing_helper;
	edward::PluginManager* plugin_manager;

	owned_array<DocumentSession> documents;
	DocumentSession* cur_doc = nullptr;
	DocumentSession* create_doc();
	base::future<DocumentSession*> emit_doc();
	void set_active_doc(DocumentSession* doc);

	Storage *storage;

	Progress *progress;

	base::map<ygfx::Texture*, string> icon_image;
	string get_tex_image(ygfx::Texture *tex);

	os::Timer timer;
	owned<kaba::Context> kaba_ctx;

	base::promise<Session*> promise_started;

	Path project_dir;
	void load_project(const Path& dir);
};

Session *create_session();
