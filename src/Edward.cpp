#include <data/world/WorldObject.h>
#include <data/world/WorldTerrain.h>
#include <lib/math/quaternion.h>

#include "lib/xhui/xhui.h"
#include "lib/xhui/controls/Button.h"
#include "lib/xhui/controls/Label.h"
#include "lib/xhui/controls/Edit.h"
#include "lib/xhui/controls/Grid.h"
#include "lib/xhui/controls/DrawingArea.h"
#include "lib/xhui/Painter.h"
#include <lib/xhui/ContextVulkan.h>
#include <renderer/world/geometry/RenderViewData.h>
#include <renderer/world/geometry/SceneView.h>
#include <renderer/base.h>
#include <renderer/path/RenderPath.h>
#include <world/Model.h>
#include <world/ModelManager.h>
#include <world/Terrain.h>
#include <y/EngineData.h>

#include "lib/os/msg.h"
#include "lib/xhui/Theme.h"
#include "lib/xhui/draw/font.h"
#include "y/renderer/Renderer.h"
#include "y/helper/ResourceManager.h"
#include "y/world/Material.h"
#include "y/world/Camera.h"
#include "y/world/Light.h"
#include "y/y/Entity.h"
#include "data/world/DataWorld.h"
#include "storage/Storage.h"
#include "Session.h"

string AppVersion = "0.5.-1.0";
string AppName = "Edward";

//EdwardApp *app = nullptr;
void* app = nullptr;

ResourceManager* _resource_manager;
DataWorld* data_world;
Session* session;

class TestRenderer : public Renderer {
public:
	vulkan::VertexBuffer* vbx;
	SceneView scene_view;
	RenderViewData rvd;
	shared<Shader> shader;
	Material* material;
	quaternion ang = quaternion::ID;
	Camera* cam;
	Light* light;
	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!

	TestRenderer() : Renderer("test") {
		resource_manager = _resource_manager;
		vbx = new VertexBuffer("3f,3f,2f");
		vbx->create_quad(rect::ID_SYM);
		try {
			shader = resource_manager->load_surface_shader("default.shader", "forward", "default", "");
			material = resource_manager->load_material("");
			material->albedo = White;
			material->metal = 0.0f;
			material->roughness = 0.9f;
			material->emission = color(1, 0.1f, 0.1f, 0.1f);
			material->textures = {tex_white};
			material->pass0.cull_mode = 0;
		} catch(Exception& e) {
			msg_error(e.message());
		}

		cam = new Camera();
		cam->owner = new Entity;
		scene_view.cam = cam;
		cam->owner->pos = {0, 0,-10};
		cam->min_depth = 1;
		cam->max_depth = 100;
		rvd.scene_view = &scene_view;

		light = new Light(White, -1, -1);
		light->owner = new Entity;
		//light->owner->ang = quaternion::rotation({0,1,0}, pi);
		//light->light.harshness = 0.5f;
	}
	void prepare(const RenderParams& params) override {
		ang = quaternion::rotation({0,1,0}, 0.02f) * ang;
	}
	void draw(const RenderParams& params) override {
		auto cb = params.command_buffer;
		cb->clear(params.area, {data_world->meta_data.background_color}, 1.0);

	//	scene_view.choose_lights();
		{
			scene_view.lights.clear();
			scene_view.shadow_index = -1;
			//	if (l->allow_shadow)
			//		scene_view.shadow_index = scene_view.lights.num;
			scene_view.lights.add(light);
		}

		scene_view.cam->update_matrices(params.desired_aspect_ratio);
		rvd.set_projection_matrix(scene_view.cam->m_projection);
		rvd.set_view_matrix(scene_view.cam->m_view);
		rvd.update_lights();
		rvd.ubo.num_lights = scene_view.lights.num;
		rvd.ubo.shadow_index = scene_view.shadow_index;

		rvd.begin_draw();


		auto& rd = rvd.start(params,  mat4::rotation(ang), shader.get(), *material, 0, PrimitiveTopology::TRIANGLES, vbx);
		rd.apply(params);
		cb->draw(vbx);
	}
};

class DataWorldRenderer : public Renderer {
public:
	SceneView scene_view;
	RenderViewData rvd;
	Camera* cam;
	vec3 cam_pos = v_0;
	float cam_radius = -1;
	Light* light;
	base::map<Material*, ShaderCache> multi_pass_shader_cache[4];
	// material as id!
	Shader* get_shader(Material* material, int pass_no, const string& vertex_shader_module, const string& geometry_shader_module) {
		if (!multi_pass_shader_cache[pass_no].contains(material))
			multi_pass_shader_cache[pass_no].set(material, {});
		auto& cache = multi_pass_shader_cache[pass_no][material];
		RenderPathType type = RenderPathType::Forward;
	//	if (is_shadow_pass())
	//		cache._prepare_shader_multi_pass(type, *material_shadow, vertex_shader_module, geometry_shader_module, pass_no);
	//	else
			cache._prepare_shader_multi_pass(type, *material, vertex_shader_module, geometry_shader_module, pass_no);
		return cache.get_shader(type);
	}

	DataWorldRenderer() : Renderer("world") {
		resource_manager = _resource_manager;

		cam = new Camera();
		cam->owner = new Entity;
		scene_view.cam = cam;
		cam->owner->ang = quaternion::rotation({1, 0, 0}, 0.33f);
		//cam->owner->pos = {1000,1000,-800};
		cam->min_depth = 1;
		cam->max_depth = 50000;
		rvd.scene_view = &scene_view;

		light = new Light(White, -1, -1);
		light->owner = new Entity;
		light->owner->ang = quaternion::rotation({1,0,0}, 0.5f);
		light->light.harshness = 0.5f;
	}
	void draw(const RenderParams& params) override {
		auto cb = params.command_buffer;
		cb->clear(params.area, {data_world->meta_data.background_color}, 1.0);

		if (cam_radius < 0) {
			vec3 vmin, vmax;
			data_world->get_bounding_box(vmin, vmax);
			cam_pos = (vmin + vmax) / 2;
			cam_radius = (vmax - vmin).length() * 0.7f;
		}
		cam->owner->pos = cam_pos - cam->owner->ang * vec3::EZ * cam_radius;

	//	scene_view.choose_lights();
		{
			scene_view.lights.clear();
			scene_view.shadow_index = -1;
			//	if (l->allow_shadow)
			//		scene_view.shadow_index = scene_view.lights.num;
			scene_view.lights.add(light);
		}

		scene_view.cam->update_matrices(params.desired_aspect_ratio);
		rvd.set_projection_matrix(scene_view.cam->m_projection);
		rvd.set_view_matrix(scene_view.cam->m_view);
		rvd.update_lights();
		rvd.ubo.num_lights = scene_view.lights.num;
		rvd.ubo.shadow_index = scene_view.shadow_index;

		rvd.begin_draw();

		for (auto& t: data_world->terrains) {
			t.terrain->prepare_draw(cam->owner->pos);
			auto material = t.terrain->material.get();
			auto vb = t.terrain->vertex_buffer.get();

			auto shader = get_shader(material, 0, t.terrain->vertex_shader_module, "");
			auto& rd = rvd.start(params,  mat4::translation(t.pos), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
			cb->push_constant(0, 4, &t.terrain->texture_scale[0].x);
			cb->push_constant(4, 4, &t.terrain->texture_scale[1].x);
			rd.apply(params);
			cb->draw(vb);
		}

		for (auto& o: data_world->objects) {
			for (int k=0; k<o.object->mesh[0]->sub.num; k++) {
				auto m = o.object;
				auto material = m->material[k];
				auto vb = m->mesh[0]->sub[k].vertex_buffer;

				auto shader = get_shader(material, 0, m->_template->vertex_shader_module, "");
				auto& rd = rvd.start(params,  mat4::translation(o.pos) * mat4::rotation(o.ang), shader, *material, 0, PrimitiveTopology::TRIANGLES, vb);
				rd.apply(params);
				cb->draw(vb);
			}
		}
	}
};

class XhuiRenderer : public RenderTask {
public:
	rect native_area_window = rect::ID;
	XhuiRenderer() : RenderTask("xhui") {
	}
	void render(const RenderParams& params) override {
		params.command_buffer->set_viewport(params.area);
		for (auto c: children)
			c->prepare(params);
		for (auto c: children)
			c->draw(params);
		params.command_buffer->set_viewport(native_area_window);
	}
	void render(Painter* p) {
		auto pp = (xhui::Painter*)p;
		RenderParams params;
		params.command_buffer = pp->cb;
		params.area = pp->native_area;
		params.render_pass = pp->context->render_pass;
		params.frame_buffer = pp->context->current_frame_buffer();
		params.desired_aspect_ratio = pp->native_area.width() / pp->native_area.height();
		native_area_window = pp->native_area_window;
		render(params);
	}
};

class EdwardWindowX : public xhui::Window {
public:
	XhuiRenderer* renderer = nullptr;
	DataWorldRenderer* world_renderer = nullptr;
	Array<string> args;

	EdwardWindowX() : xhui::Window(AppName, 1024, 768) {

		auto g = new xhui::Grid("grid");
		add(g);
		auto g2 = new xhui::Grid("grid2");
		g->add(g2, 0, 0);
		g2->add(new xhui::Label("label1", "label"), 0, 0);
		g2->add(new xhui::Button("button1", "a small test g"), 1, 0);
		g2->add(new xhui::Button("button2", "a small test g"), 2, 0);
		g->add(new xhui::DrawingArea("area"), 0, 1);
		auto g3 = new xhui::Grid("grid3");
		g->add(g3, 0, 2);
		g3->add(new xhui::Button("button3", "a"), 0, 0);
		g3->add(new xhui::Button("button4", "b"), 1, 0);

		event("button1", [] {
			msg_write("event button1 click");
		});

		renderer = new XhuiRenderer();

		event_xp("area", "hui:initialize", [this] (Painter* p) {
			auto pp = (xhui::Painter*)p;
			vulkan::default_device = pp->context->device;
			api_init_external(pp->context->instance, pp->context->device);
			_resource_manager = new ResourceManager({});
			_resource_manager->default_shader = "default.shader";
			try {
				_resource_manager->load_shader_module("module-basic-data.shader");
				_resource_manager->load_shader_module("module-basic-interface.shader");
				_resource_manager->load_shader_module("module-vertex-default.shader");
				_resource_manager->load_shader_module("module-vertex-animated.shader");
				_resource_manager->load_shader_module("module-lighting-pbr.shader");
				_resource_manager->load_shader_module("forward/module-surface.shader");
			} catch(Exception& e) {
				msg_error(e.message());
			}
			//renderer->add_child(new TestRenderer());
			world_renderer = new DataWorldRenderer();
			renderer->add_child(world_renderer);


			engine.file_errors_are_critical = false;
			engine.ignore_missing_files = true;
			engine.resource_manager = _resource_manager;
			session = new Session;
			session->resource_manager = _resource_manager;
			session->storage = new Storage(session);
			data_world = new DataWorld(session);

			if (args.num >= 2)
				session->storage->load(args[1], data_world);
		});
		event_xp("area", "hui:draw", [this] (Painter* p) {
			renderer->render(p);
		});

		xhui::run_repeated(0.02f, [this] {
			request_redraw();
		});
	}

	void move_cam(const vec3& drel) {
		world_renderer->cam_pos = world_renderer->cam_pos + world_renderer->cam->owner->ang * drel * world_renderer->cam_radius;
	}
	void on_mouse_move(const vec2& m, const vec2& d) override {
		if (state.lbut)
			world_renderer->cam->owner->ang = world_renderer->cam->owner->ang * quaternion::rotation({d.y*0.003f, d.x*0.003f, 0});
		if (state.rbut)
			move_cam(vec3(-d.x, d.y, 0) / 800.0f); // / window size?
	}
	void on_mouse_wheel(const vec2& d) override {
		world_renderer->cam_radius *= exp(- d.y * 0.1f);
	}
	void on_key_down(int key) override {
		float d = 0.05f;
		if (key == xhui::KEY_UP)
			move_cam({0, d, 0});
		if (key == xhui::KEY_DOWN)
			move_cam({0, -d, 0});
		if (key == xhui::KEY_LEFT)
			move_cam({-d, 0, 0});
		if (key == xhui::KEY_RIGHT)
			move_cam({d, 0, 0});
	}
};

int hui_main(const Array<string>& args) {

	try {
		xhui::init(args, "edward");
	} catch (Exception &e) {
		msg_error(e.message());
		return 1;
	}

	auto w = new EdwardWindowX();
	w->args = args;

	xhui::run();
	return 0;
}

