/*
 * Session.cpp
 *
 *  Created on: 12 Sept 2023
 *      Author: michi
 */

#include "Session.h"
#include <view/DocumentSession.h>
#include <view/EdwardWindow.h>
#include <view/MaterialPreviewManager.h>
#include "Edward.h"
/*#include "mode/ModeNone.h"
#include "mode/ModeCreation.h"
#include "mode/administration/ModeAdministration.h"
#include "mode/administration/dialog/ConfigurationDialog.h"*/
#include <mode_model/ModeModel.h>
#include <mode_model/mesh/ModeMesh.h>
#include <mode_material/ModeMaterial.h>
#include <mode_world/ModeWorld.h>
#include <mode_coding/ModeCoding.h>
/*#include "mode/font/ModeFont.h"
#include "stuff/Progress.h"*/
#include <stuff/PluginManager.h>
#include <view/MultiView.h>
//#include "view/DrawingHelper.h"
#include "Session.h"
#include "Session.h"

#include <lib/base/algo.h>
#include "view/Mode.h"
#include "data/Data.h"
#include "storage/format/Format.h"
#include "storage/Storage.h"
#include "lib/xhui/config.h"
#include "lib/image/image.h"
#include <lib/os/app.h>
#include <lib/os/msg.h>
#include <y/EngineData.h>
#include <y/helper/ResourceManager.h>
#include <y/world/components/Camera.h>
#include <y/world/World.h>
#include <lib/ygraphics/graphics-impl.h>
#include <lib/yrenderer/target/XhuiRenderer.h>
#include <lib/yrenderer/MaterialManager.h>
#include <lib/yrenderer/TextureManager.h>
#include <lib/kaba/kaba.h>

#include "lib/os/filesystem.h"

static Array<Session*> all_sessions;
bool any_session_running() {
	return all_sessions.num > 0;
}

Session* create_session(bool with_window) {
	auto s = new Session;
	s->kaba_ctx = kaba::Context::create();
	s->storage = new Storage(s);
	s->storage->set_root_directory(xhui::config.get_str("RootDir", ""));
	if (os::app::installed)
		s->plugin_manager = new edward::PluginManager(s, os::app::directory_static | "plugins");
	else
		s->plugin_manager = new edward::PluginManager(s, os::app::directory_static | ".." | "plugins");
	//s->load_project(xhui::config.get_str("RootDir", ""));
	if (with_window) {
		s->_win = new EdwardWindow(s);
		s->win = s->_win.get();
		xhui::fly(s->win);
	} else {
		s->resource_manager = new ResourceManager(nullptr, "", "", "");
		s->material_preview_manager = new MaterialPreviewManager(s);
	}
	return s;
}

base::future<Session*> emit_new_session() {
	auto s = create_session();
	return s->promise_started.get_future();
}

#if 0
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
#endif

Path Session::guess_root_directory(const Path &filename) {
	for (auto &d: filename.all_parents())
		if (os::fs::exists(d | "game.ini"))
			return d;

	return filename.parent();
}

void Session::load_project(const Path& dir, bool serious) {
	if (project_dir == dir)
		return;

	project_dir = dir;

	storage->set_root_directory(dir);
	engine.set_dirs(storage->root_dir_kind[FD_TEXTURE],
			storage->root_dir_kind[FD_WORLD],
			storage->root_dir_kind[FD_MODEL],
			storage->root_dir_kind[FD_SOUND],
			storage->root_dir_kind[FD_SCRIPT],
			storage->root_dir_kind[FD_MATERIAL],
			storage->root_dir_kind[FD_FONT]);
	resource_manager->texture_manager->texture_dir = storage->root_dir_kind[FD_TEXTURE];
	resource_manager->shader_manager->shader_dir = storage->root_dir_kind[FD_SHADERFILE];
	resource_manager->material_manager->material_dir = storage->root_dir_kind[FD_MATERIAL];

	if (serious)
		plugin_manager->load_project_stuff(project_dir);

	out_project_loaded();
}

Session::Session() {
	ctx = nullptr;
	cur_doc = nullptr;

	storage = nullptr;
	resource_manager = nullptr;
	plugin_manager = nullptr;
	win = nullptr;
	drawing_helper = nullptr;

	all_sessions.add(this);
}

Session::~Session() {
	// saving the configuration data...
	xhui::config.set_str("RootDir", str(project_dir));
	if (storage) {
		//xhui::config.set_str("Language", xhui::get_cur_language());
		/*HuiConfig.set_bool("LocalDocumentation", LocalDocumentation);
		HuiConfig.set_str("WorldScriptVarFile", WorldScriptVarFile);
		HuiConfig.set_str("ObjectScriptVarFile", ObjectScriptVarFile);
		HuiConfig.set_str("ItemScriptVarFile", ItemScriptVarFile);*/
		//HuiConfig.set_int("UpdateNormalMaxTime (ms)", int(UpdateNormalMaxTime * 1000.0f));
		xhui::config.save(os::app::directory_dynamic | "config.txt");
		delete storage;
	}

	base::remove(all_sessions, this);

	//app->end();
}

DocumentSession* Session::create_doc() {
	auto doc = new DocumentSession(this);
	documents.add(doc);
	if (win) { // only in UI mode
		doc->grid_id = format("x-grid-%d", documents.num);
		win->set_target("tab");
		win->add_control("Grid", "", documents.num-1, 0, doc->grid_id);
		win->embed(doc->grid_id, 0, 0, doc->base_panel);
		//win->set_target(grid_id);
		//win->add_control("Button", p2s(doc), 0, 0, "");
	}

	return doc;
}

base::future<DocumentSession*> Session::emit_doc() {
	auto doc = create_doc();
	promise_started.get_future().then([this, doc] (Session*) {
		set_active_doc(doc);
		doc->promise_started(doc);
	});
	return doc->promise_started.get_future();
}

void Session::set_active_doc(DocumentSession* doc) {
	if (doc == cur_doc)
		return;

	if (cur_doc)
		cur_doc->leave();

	int i = weak(documents).find(doc);
	cur_doc = doc;
	static int counter = 0;
	if (cur_doc)
		cur_doc->_last_usage_counter = counter ++;
	win->set_int("tab", i);

	if (cur_doc) {
		cur_doc->enter();
		win->update_menu();
	}
	out_document_switched();
}

void Session::close_doc(DocumentSession* doc) {
	if (!doc)
		return;
	int index = weak(documents).find(doc);
	if (documents.num > 0)
		set_active_doc(weak(documents)[(index + 1) % documents.num]);
	else
		set_active_doc(nullptr);
	documents.erase(index);
	win->get_control("tab")->remove_child(win->get_control(doc->grid_id));
}


void Session::remove_message() {
	messages.erase(0);
	win->request_redraw();
}

void Session::info(const string &message) {
	messages.add({Message::Type::INFO, message});
	win->request_redraw();
	xhui::run_later(4.0f, [this] {
		remove_message();
	});
}

void Session::error(const string &message) {
	messages.add({Message::Type::ERROR, message});
	win->request_redraw();
	xhui::run_later(4.0f, [this] {
		remove_message();
	});
}

void Session::warning(const string &message) {
	messages.add({Message::Type::WARNING, message});
	win->request_redraw();
	xhui::run_later(4.0f, [this] {
		remove_message();
	});
}

#if 0
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
#endif

void Session::universal_new(int preferred_type) {
	auto call_new = [preferred_type] (DocumentSession* doc) {
		if (preferred_type == FD_MODEL) {
			doc->mode_model = new ModeModel(doc);
			doc->set_mode(doc->mode_model->mode_mesh.get());
			doc->mode_model->mode_mesh->optimize_view();
		} else if (preferred_type == FD_WORLD) {
			doc->mode_world = new ModeWorld(doc);
			doc->mode_world->data->add_initial_data();
			doc->set_mode(doc->mode_world);
			doc->mode_world->optimize_view();
		} else if (preferred_type == FD_MATERIAL) {
			doc->mode_material = new ModeMaterial(doc);
			//doc->mode_material->_new();
			doc->set_mode(doc->mode_material);
			doc->mode_material->optimize_view();
		} /*else if (preferred_type == FD_FONT) {
			doc->mode_font->_new();
			doc->set_mode(doc->mode_font);
			doc->mode_font->optimize_view();
		}*/
	};

	emit_doc().then(call_new);
}

void Session::universal_open(int preferred_type) {
	storage->file_dialog_x({FD_MODEL, FD_MATERIAL, FD_WORLD, FD_SCRIPT}, preferred_type, false, false).then([this] (const auto& p) {
		universal_edit(p.kind, p.complete, false);
	});
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
	Path filename = make_absolute_path(this, type, add_extension_if_needed(this, type, _filename), relative_path);

	for (auto d: documents)
		if (d->filename() == filename) {
			set_active_doc(d);
			return;
		}

	emit_doc().then([this, type, filename] (DocumentSession* doc) {
		switch (type){
			/*case -1:
				if (filename.basename() == "config.txt")
					hui::open_document(filename);
				else if (filename.basename() == "game.ini")
					mode_admin->basic_settings();
				break;*/
			case FD_MODEL:
				if (!doc->mode_model)
					doc->mode_model = new ModeModel(doc);
				doc->session->storage->load(filename, doc->mode_model->data.get(), true);
				doc->set_mode(doc->mode_model->mode_mesh.get());
				doc->mode_model->mode_mesh->optimize_view();
				break;
			case FD_MATERIAL:
				if (!doc->mode_material)
					doc->mode_material = new ModeMaterial(doc);
				doc->session->storage->load(filename, doc->mode_material->data, true);
				doc->set_mode(doc->mode_material);
				doc->mode_material->optimize_view();
				break;
			case FD_FONT:
				break;
			case FD_WORLD:
				if (!doc->mode_world)
					doc->mode_world = new ModeWorld(doc);
				doc->session->storage->load(filename, doc->mode_world->data, true);
				doc->set_mode(doc->mode_world);
				doc->mode_world->optimize_view();
				break;
			case FD_TERRAIN:
				break;
			case FD_CAMERAFLIGHT:
				break;
			case FD_SCRIPT:
			case FD_SHADERFILE:
			case FD_TEMPLATE:
				if (!doc->mode_coding)
					doc->mode_coding = new ModeCoding(doc);
				doc->mode_coding->load(filename);
				doc->set_mode(doc->mode_coding);
				break;
				if (type == FD_MODEL) {
				} else if (type == FD_MATERIAL) {
#if 0
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
#endif
				} else if (type == FD_WORLD) {
				} else if (type == FD_SCRIPT or type == FD_SHADERFILE) {
				}
				break;
			case FD_TEXTURE:
			case FD_SOUND:
			case FD_FILE:
				//hui::open_document(filename);
				break;
		}
		//return true;
	});
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

string Session::get_tex_image(ygfx::Texture *tex) {
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

#if 0
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
#endif



