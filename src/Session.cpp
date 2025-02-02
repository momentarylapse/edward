/*
 * Session.cpp
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#include "Session.h"
#include "view/EdwardWindow.h"
#include "Edward.h"
/*#include "mode/ModeNone.h"
#include "mode/ModeCreation.h"
#include "mode/administration/ModeAdministration.h"
#include "mode/administration/dialog/ConfigurationDialog.h"
#include "mode/model/ModeModel.h"
#include "mode/model/mesh/ModeModelMesh.h"
#include "mode/material/ModeMaterial.h"*/
#include "../mode_world/ModeWorld.h"
/*#include "mode/font/ModeFont.h"
#include "stuff/Progress.h"*/
#include "view/MultiView.h"
//#include "view/DrawingHelper.h"
#include "view/Mode.h"
#include "data/Data.h"
#include "storage/format/Format.h"
#include "storage/Storage.h"
#include "lib/xhui/config.h"
#include "lib/image/image.h"
#include "lib/os/msg.h"
#include <y/EngineData.h>
#include <y/helper/ResourceManager.h>
#include <y/world/Camera.h>
#include <y/world/World.h>
#include <y/graphics-impl.h>
#include <y/renderer/target/XhuiRenderer.h>

Session *create_session() {
	auto s = new Session;
	s->storage = new Storage(s);
	//s->mode_world = new ModeWorld(s);
	s->win = new EdwardWindow(s);
	s->win->renderer = new XhuiRenderer();
	return s;
}

base::future<Session*> emit_new_session() {
	auto s = create_session();
	return s->promise_started.get_future();
}

bool session_is_empty(Session *s) {
	if (s->cur_mode == s->mode_none)
		return true;
	return false;
}

base::future<Session*> emit_empty_session(Session* parent) {
	if (session_is_empty(parent))
		return parent->promise_started.get_future();

	return emit_new_session();
}

Session::Session() {
	ctx = nullptr;
	mode_none = nullptr;
#if 0
	mode_none = new ModeNone(this);
	cur_mode = mode_none;
	progress = new Progress;
#endif


	multi_view_2d = nullptr;
	multi_view_3d = nullptr;
	mode_model = nullptr;
	mode_admin = nullptr;
	mode_font = nullptr;
	mode_material = nullptr;
	mode_world = nullptr;

	storage = nullptr;
	resource_manager = nullptr;
	win = nullptr;
	drawing_helper = nullptr;
}

Session::~Session() {
#if 0
	if (mode_world)
		delete mode_world;
	/*delete mode_material;
	delete mode_model;
	delete mode_font;
	delete mode_admin;*/

	if (multi_view_2d)
		delete multi_view_2d;
	if (multi_view_3d)
		delete multi_view_3d;
	// saving the configuration data...
	if (storage) {
		hui::config.set_str("RootDir", storage->root_dir.str());
		hui::config.set_str("Language", hui::get_cur_language());
		/*HuiConfig.set_bool("LocalDocumentation", LocalDocumentation);
		HuiConfig.set_str("WorldScriptVarFile", WorldScriptVarFile);
		HuiConfig.set_str("ObjectScriptVarFile", ObjectScriptVarFile);
		HuiConfig.set_str("ItemScriptVarFile", ItemScriptVarFile);*/
		//HuiConfig.set_int("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));
		hui::config.save(app->directory | "config.txt");
		delete storage;
	}
#endif


	app->end();
}


void Session::create_initial_resources(Context *_ctx) {

	ctx = _ctx;

	// initialize engine
	resource_manager = new ResourceManager(ctx);
///	drawing_helper = new DrawingHelper(ctx, resource_manager, app->directory_static);

	engine.ignore_missing_files = true;
	engine.set_context(ctx, resource_manager);
	resource_manager->load_shader("module-vertex-default.shader");
	//ResourceManager::default_shader

	CameraInit();
	GodInit(0);

#if 0
	multi_view_3d = new MultiView::MultiView(this, true);
	multi_view_2d = new MultiView::MultiView(this, false);
	mode_model = new ModeModel(this, multi_view_3d, multi_view_2d);
	mode_world = new ModeWorld(this, multi_view_3d);
	mode_font = new ModeFont(this, multi_view_2d);
	mode_admin = new ModeAdministration(this);
#endif

	storage = new Storage(this);
	storage->set_root_directory(xhui::config.get_str("RootDir", ""));

#if 0
	mode_material = new ModeMaterial(this, multi_view_3d);

	/*mmodel->FFVBinary = mobject->FFVBinary = mitem->FFVBinary = mmaterial->FFVBinary = mworld->FFVBinary = mfont->FFVBinary = false;
	mworld->FFVBinaryMap = true;*/

	multi_view_3d->out_settings_changed >> create_sink([this] {
		win->update_menu();
	});
	multi_view_3d->out_selection_changed >> create_sink([this] {
		cur_mode->on_selection_change();
		win->update_menu();
	});
	multi_view_3d->out_viewstage_changed >> create_sink([this] {
		cur_mode->on_view_stage_change();
		win->update_menu();
	});
	multi_view_3d->out_redraw >> create_sink([this] {
		win->redraw("nix-area");
	});

	multi_view_2d->out_settings_changed >> create_sink([this]{
		win->update_menu();
	});
	multi_view_2d->out_selection_changed >> create_sink([this] {
		cur_mode->on_selection_change();
		win->update_menu();
	});
	multi_view_2d->out_viewstage_changed >> create_sink([this] {
		cur_mode->on_view_stage_change();
		win->update_menu();
	});
	multi_view_2d->out_redraw >> create_sink([this] {
		win->redraw("nix-area");
	});
#endif

	promise_started(this);
}

// do we change roots?
//  -> data loss?
base::future<void> mode_switch_allowed(Mode *m) {
//	if (!m->session->cur_mode or m->equal_roots(m->session->cur_mode)) {
		base::promise<void> promise;
		promise();
		return promise.get_future();
//	} else {
//		return m->session->allow_termination();
//	}
}

void Session::set_mode(Mode *m) {
	if (cur_mode == m)
		return;
	mode_switch_allowed(m).then([this, m] {
		set_mode_now(m);
	});
}

void Session::set_mode_now(Mode *m) {
	if (cur_mode == m)
		return;

#if 0
	// recursive use...
	mode_queue.add(m);
	if (mode_queue.num > 1)
		return;

	cur_mode->on_leave();
	if (cur_mode->get_data()) {
		cur_mode->get_data()->unsubscribe(win);
		cur_mode->get_data()->action_manager->unsubscribe(win);
	}

	m = mode_queue[0];
	while (m) {

		// close current modes
		while (cur_mode) {
			if (cur_mode->is_ancestor_of(m))
				break;
			msg_write("end " + cur_mode->name);
			cur_mode->on_end();
			if (cur_mode->multi_view)
				cur_mode->multi_view->pop_settings();
			cur_mode = cur_mode->parent_untyped;
		}

		//multi_view_3d->ResetMouseAction();
		//multi_view_2d->ResetMouseAction();

		// start new modes
		while (cur_mode != m) {
			cur_mode = cur_mode->get_next_child_to(m);
			msg_write("start " + cur_mode->name);
			if (cur_mode->multi_view)
				cur_mode->multi_view->push_settings();
			cur_mode->on_enter_rec();
		}
		cur_mode->on_enter();
		cur_mode->on_set_multi_view();

		// nested set calls?
		mode_queue.erase(0);
		m = nullptr;
		if (mode_queue.num > 0)
			m = mode_queue[0];
	}

	//win->set_menu(hui::create_resource_menu(cur_mode->menu_id, win));
	win->update_menu();
#endif

	cur_mode = m;
	win->renderer->add_child(cur_mode->multi_view);

	cur_mode->on_enter();


	//cur_mode->on_enter(); // ????
	cur_mode->out_redraw >> win->in_redraw;
	cur_mode->multi_view->out_selection_changed >> win->in_redraw;
	cur_mode->multi_view->view_port.out_changed >> win->in_data_selection_changed;
	if (cur_mode->get_data()) {
		cur_mode->get_data()->out_changed >> win->in_data_changed;
		cur_mode->get_data()->out_changed >> cur_mode->multi_view->in_data_changed;
		auto *am = cur_mode->get_data()->action_manager;
		am->out_failed >> win->in_action_failed;
		am->out_saved >> win->in_saved;
	}

	win->request_redraw();
}


void Session::remove_message() {
	message_str.erase(0);
	win->request_redraw();
}

void Session::set_message(const string &message) {
	msg_write(message);
	message_str.add(message);
	win->request_redraw();
	xhui::run_later(2.0f, [this] {
		remove_message();
	});
}


void Session::error(const string &message) {
#if 0
	//set_info_text(message, {"error", "allow-close"});
	hui::error_box(win, _("Error"), message);
#else
	set_message("ERROR: " + message);
#endif
}

Mode *Session::get_mode(int preferred_type) {
#if 0
	if (preferred_type == FD_MODEL)
		return mode_model;
	if (preferred_type == FD_WORLD)
		return mode_world;
	if (preferred_type == FD_MATERIAL)
		return mode_material;
	if (preferred_type == FD_FONT)
		return mode_font;
#endif
	return mode_none;
}

void Session::universal_new(int preferred_type) {
#if 1
	auto call_new = [preferred_type] (Session* session) {
		/*if (preferred_type == FD_MODEL) {
			session->mode_model->_new();
			session->set_mode(session->mode_model);
			session->mode_model->mode_model_mesh->optimize_view();
		} else*/ if (preferred_type == FD_WORLD) {
			session->mode_world = new ModeWorld(session);
			session->set_mode(session->mode_world);
			//session->mode_world->optimize_view();
		} /*else if (preferred_type == FD_MATERIAL) {
			session->mode_material->_new();
			session->set_mode(session->mode_material);
			session->mode_material->optimize_view();
		} else if (preferred_type == FD_FONT) {
			session->mode_font->_new();
			session->set_mode(session->mode_font);
			session->mode_font->optimize_view();
		}*/
	};

	if (false) {
		// replace
		allow_termination().then([this, call_new] { call_new(this); });

	} else {
		// new window
		emit_empty_session(this).then(call_new);
	}
#endif
}

void Session::universal_open(int preferred_type) {
#if 1
	storage->file_dialog_x({FD_MODEL, FD_MATERIAL, FD_WORLD}, preferred_type, false, false).then([this] (const auto& p) {

		auto call_open = [kind=p.kind, path=p.complete] (Session* session) {
			/*if (kind == FD_MODEL) {
				session->storage->load(path, session->mode_model->data);
				session->set_mode(session->mode_model);
				session->mode_model->mode_model_mesh->optimize_view();
			} else*/ if (kind == FD_WORLD) {
				if (!session->mode_world)
					session->mode_world = new ModeWorld(session);
				session->storage->load(path, session->mode_world->data);
				session->set_mode(session->mode_world);
				session->mode_world->optimize_view();
			} /*else if (kind == FD_MATERIAL) {
				session->storage->load(path, session->mode_material->data);
				session->set_mode(session->mode_material);
				session->mode_material->optimize_view();
			}*/
		};

		emit_empty_session(this).then(call_open);
	});
#endif
}

Path add_extension_if_needed(Session *session, int type, const Path &filename) {
	auto e = filename.extension();
	if (e.num == 0)
		return filename.with("." + session->storage->fd_ext(type));
	return filename;
}

Path make_absolute_path(Session *session, int type, const Path &filename, bool relative_path) {
	if (relative_path)
		return session->storage->get_root_dir(type) | filename;
	return filename;
}

void Session::universal_edit(int type, const Path &_filename, bool relative_path) {
#if 1
	Path filename = make_absolute_path(this, type, add_extension_if_needed(this, type, _filename), relative_path);
	msg_write("EDIT");
	msg_write(_filename.str());
	msg_write(filename.str());

		switch (type){
			/*case -1:
				if (filename.basename() == "config.txt")
					hui::open_document(filename);
				else if (filename.basename() == "game.ini")
					mode_admin->basic_settings();
				break;*/
			case FD_MODEL:
			case FD_MATERIAL:
			case FD_FONT:
			case FD_WORLD:
			case FD_TERRAIN:
			case FD_CAMERAFLIGHT:
				emit_empty_session(this).then([type, filename] (Session* session) {
#if 0
					if (type == FD_MODEL) {
						session->storage->load(filename, session->mode_model->data, true);
						session->set_mode(session->mode_model);
						session->mode_model->mode_model_mesh->optimize_view();
					} else if (type == FD_MATERIAL) {
						session->storage->load(filename, session->mode_material->data, true);
						session->set_mode(session->mode_material);
						session->mode_material->optimize_view();
					} else if (type == FD_FONT) {
						session->storage->load(filename, session->mode_font->data, true);
						session->set_mode(session->mode_font);
					} else if (type == FD_TERRAIN) {
						session->mode_world->data->add_terrain(filename.relative_to(engine.map_dir).no_ext(), v_0);
						session->set_mode(session->mode_world);
						session->mode_world->optimize_view();
					} else if (type == FD_CAMERAFLIGHT) {
						/*mode_world->data->Reset();
						strcpy(mworld->CamScriptFile,a->Name);
						if (mworld->LoadCameraScript()){
							SetMode(ModeWorld);
							mworld->OptimizeView();
						}*/
					} else if (type == FD_WORLD) {
#endif
						if (!session->mode_world)
							session->mode_world = new ModeWorld(session);
						session->storage->load(filename, session->mode_world->data, true);
						session->set_mode(session->mode_world);
						session->mode_world->optimize_view();
					//}
				});
				break;
			case FD_TEXTURE:
			case FD_SOUND:
			case FD_SHADERFILE:
			case FD_SCRIPT:
			case FD_FILE:
				//hui::open_document(filename);
				break;
		}
		//return true;
#endif
}

base::future<void> Session::allow_termination() {
	base::promise<void> promise;
#if 0

	if (!cur_mode) {
		promise();
		return promise.get_future();
	}
	Data *d = cur_mode->get_data();
	if (!d) {
		promise();
		return promise.get_future();
	}
	if (d->action_manager->is_save()) {
		promise();
		return promise.get_future();
	}
	hui::question_box(win,_("Quite a polite question"),_("You increased entropy. Do you wish to save your work?"), true)
		.then([promise] (bool answer) mutable {
			if (!answer) {
				promise();
			} else {
				//bool saved = cur_mode->save();
				//return saved;
				promise.fail();
			}
		}).on_fail([promise] () mutable {
			promise.fail();
		});
#else
	promise();
#endif
	return promise.get_future();
}

string Session::get_tex_image(Texture *tex) {
#if 0
	if (icon_image.contains(tex))
		return icon_image[tex];

	string img;
	if (tex) {
		Image im;
#if HAS_LIB_GL
		tex->read(im);
#endif
		auto *small = im.scale(48, 48);
		img = hui::set_image(small);
		delete small;
	} else {
		Image empty;
		empty.create(48, 48, White);
		img = hui::set_image(&empty);
	}
	icon_image.set(tex, img);
	return img;
#endif
	return "";
}

Mode *Session::find_mode_base(const string &name) {
#if 0
	if (name == "model")
		return mode_model;
	if (name == "model-mesh")
		return mode_model->mode_model_mesh;
	if (name == "model-mesh-texture")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_texture;
	if (name == "model-mesh-deform")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_deform;
	if (name == "model-mesh-material")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_material;
	if (name == "model-mesh-paint")
		return (ModeBase*)mode_model->mode_model_mesh->mode_model_mesh_paint;
	if (name == "model-skeleton")
		return (ModeBase*)mode_model->mode_model_skeleton;
	if (name == "model-animation")
		return (ModeBase*)mode_model->mode_model_animation;
	if (name == "material")
		return mode_material;
	if (name == "font")
		return mode_font;
	if (name == "world")
		return mode_world;
#endif
	return mode_none;
}


