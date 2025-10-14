/*
 * PluginManager.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "PluginManager.h"
#include <lib/kaba/kaba.h>
#include <lib/kaba/lib/extern.h>
#include "../audio/SoundSource.h"
#include "../audio/AudioBuffer.h"
#include "../audio/AudioStream.h"
#include "../audio/Listener.h"
#include "../fx/Particle.h"
#include "../fx/Beam.h"
#include "../fx/ParticleEmitter.h"
#include "../gui/gui.h"
#include "../gui/Node.h"
#include "../gui/Picture.h"
#include "../gui/Text.h"
#include "../gui/Canvas.h"
#include "../helper/DeletionQueue.h"
#include "../helper/ResourceManager.h"
#include <lib/yrenderer/ShaderManager.h>
#include <lib/yrenderer/_kaba_export.h>
#include <lib/profiler/_kaba_export.h>
#include "../helper/Scheduler.h"
#include "lib/any/conversion.h"
#if __has_include("../input/InputManager.h")
#include "../input/InputManager.h"
#include "../input/Gamepad.h"
#include "../input/Keyboard.h"
#include "../input/Mouse.h"
#include "../input/VR.h"
#define HAS_INPUT
#endif
#include "../net/NetworkManager.h"
#include <lib/yrenderer/Context.h>
#include "../renderer/helper/RendererFactory.h"
#include <lib/yrenderer/helper/CubeMapSource.h>
#include <lib/yrenderer/scene/SceneRenderer.h>
#include "../renderer/FullCameraRenderer.h"
#include <lib/yrenderer/scene/path/RenderPathForward.h>
#include <lib/yrenderer/scene/path/RenderPathDeferred.h>
#include "../renderer/gui/GuiRenderer.h"
#include <lib/yrenderer/post/PostProcessor.h>
#include <renderer/helper/Raytracing.h>
#include <lib/yrenderer/scene/SceneView.h>
#include "../y/EngineData.h"
#include "../y/Component.h"
#include "../y/ComponentManager.h"
#include "../y/System.h"
#include "../y/SystemManager.h"
#include "../world/Camera.h"
#include "../world/Link.h"
#include "../world/Model.h"
#include "../world/ModelManager.h"
#include "../world/Terrain.h"
#include "../world/World.h"
#include "../world/Light.h"
#include "../world/components/SolidBody.h"
#include "../world/components/Collider.h"
#include "../world/components/Animator.h"
#include "../world/components/Skeleton.h"
#include "../world/components/UserMesh.h"
#include "../world/components/MultiInstance.h"
#include "../world/components/CubeMapSource.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include "../lib/kaba/dynamic/exception.h"
#include "../lib/os/msg.h"
#include "../lib/image/image.h"
#include "y/EntityManager.h"
#include "y/BaseClass.h"

namespace kaba {
	extern const Class* TypePath;
}

namespace PluginManager {

//using namespace yrenderer;
using namespace ygfx;

/*void global_delete(BaseClass *e) {
	//msg_error("global delete... " + p2s(e));
	world.unregister(e);
	if (e->type == BaseClass::Type::ENTITY)
		reinterpret_cast<Entity*>(e)->on_delete_rec();
	else
		e->on_delete();
	delete e;
}*/


#pragma GCC push_options
#pragma GCC optimize("no-omit-frame-pointer")
#pragma GCC optimize("no-inline")
#pragma GCC optimize("0")

Model* _create_object(World *w, const Path &filename, const vec3 &pos, const quaternion &ang) {
	KABA_EXCEPTION_WRAPPER( return w->create_object(filename, pos, ang); );
	return nullptr;
}

MultiInstance* _create_object_multi(World *w, const Path &filename, const Array<vec3> &pos, const Array<quaternion> &ang) {
	KABA_EXCEPTION_WRAPPER( return w->create_object_multi(filename, pos, ang); );
	return nullptr;
}

Model* _attach_model(World* w, Entity* e, const Path& filename) {
	KABA_EXCEPTION_WRAPPER( return w->attach_model(e, filename); );
	return nullptr;
}

LegacyParticle* _world_add_legacy_particle(World* w, const kaba::Class* type, const vec3& pos, float radius, const color& c, shared<Texture>& tex, float ttl) {
	auto e = w->create_entity(pos, quaternion::ID);
	auto p = reinterpret_cast<LegacyParticle*>(EntityManager::global->_add_component_generic_(e, type));
	p->radius = radius;
	p->col = c;
	p->texture = tex;
	p->time_to_live = ttl;
	//w->add_legacy_particle(p);
	return p;
}


#pragma GCC pop_options

void screenshot(Image& im) {
#ifdef USING_VULKAN
	msg_error("unimplemented:  screenshot()");
#else
	engine.context->context->ctx->default_framebuffer->read(im);
#endif
}


xfer<Model> __load_model(const Path& filename) {
	return engine.resource_manager->load_model(filename);
}

shared<Shader> __load_shader(const Path& filename) {
	return engine.resource_manager->shader_manager->load_shader(filename);
}

xfer<Shader> __create_shader(const string& source) {
	return engine.resource_manager->shader_manager->create_shader(source);
}

shared<Texture> __load_texture(const Path& filename) {
	return engine.resource_manager->load_texture(filename);
}

xfer<yrenderer::Material> __load_material(const Path& filename) {
	return engine.resource_manager->load_material(filename);
}


CubeMap* camera_renderer_get_cubemap(FullCameraRenderer &r) {
	return r.render_path->scene_view.cube_map.get();
}

Array<Texture*> camera_renderer_get_shadow_map(FullCameraRenderer &r) {
	Array<Texture*> shadow_maps;
	for (auto s: r.render_path->scene_view.shadow_maps)
		shadow_maps.add(s);
	return shadow_maps;
}

//shared_array<Texture> camera_renderer_get_gbuffer(FullCameraRenderer &r) {
Array<Texture*> camera_renderer_get_gbuffer(FullCameraRenderer &r) {
	if (r.type == yrenderer::RenderPathType::Deferred)
		return weak(reinterpret_cast<yrenderer::RenderPathDeferred*>(r.render_path)->gbuffer_textures);
	return {};
}

audio::AudioStream* __create_audio_stream(Callable<Array<float>(int)>& f, float sample_rate) {
	return audio::create_stream([&f] (int n) { return f(n); }, sample_rate);
}

mat4 scene_view_shadow_projection(yrenderer::SceneView* s) {
	if (s->shadow_indices.num < 1)
		return mat4::ID;
	return s->lights[s->shadow_indices[0]]->shadow_projection;
}

void init() {
	kaba::default_context->register_package_init("y", engine.script_dir | "y", &export_kaba_package_y);
	import_kaba();
}

ComponentManager::List& __query_component_list(const kaba::Class* type) {
	return EntityManager::global->component_manager->_get_list(type);
}

ComponentManager::List& __query_component_list_family(const kaba::Class* type) {
	return EntityManager::global->component_manager->_get_list_family(type);
}

ComponentManager::PairList& __query_component_list2(const kaba::Class* type1, const kaba::Class* type2) {
	return EntityManager::global->component_manager->_get_list2(type1, type2);
}

class EntityWrapper : public Entity {
public:
	Component* add_component_generic(const kaba::Class* type, const string& vars) {
		if (vars != "")
			msg_error("TODO component params Any{}");
		return EntityManager::global->_add_component_generic_(this, type, {});
	}
	void delete_component(Component* c) {
		return EntityManager::global->delete_component(this, c);
	}
};
Light* attach_light_parallel(Entity* e, const color& c) {
	return world.attach_light_parallel(e, c);
}
Light* attach_light_point(Entity* e, const color& c, float r) {
	return world.attach_light_point(e, c, r);
}
Light* attach_light_cone(Entity* e, const color& c, float r, float theta) {
	return world.attach_light_cone(e, c, r, theta);
}

void export_ecs(kaba::Exporter* ext) {
	BaseClass entity(BaseClass::Type::NONE);
	ext->declare_class_size("BaseClass", sizeof(BaseClass));
	//	ext->link_class_func("BaseClass.__init__", &Entity::__init__);
	ext->link_virtual("BaseClass.__delete__", &BaseClass::__delete__, &entity);
	ext->link_virtual("BaseClass.on_init", &BaseClass::on_init, &entity);
	ext->link_virtual("BaseClass.on_delete", &BaseClass::on_delete, &entity);
	ext->link_virtual("BaseClass.on_iterate", &BaseClass::on_iterate, &entity);

	ext->declare_class_size("Entity", sizeof(Entity));
	ext->declare_class_element("Entity.pos", &Entity::pos);
	ext->declare_class_element("Entity.ang", &Entity::ang);
	ext->declare_class_element("Entity.parent", &Entity::parent);
	ext->link_class_func("Entity.get_matrix", &Entity::get_matrix);
	ext->link_class_func("Entity.__get_component", &Entity::_get_component_generic_);
	ext->link_class_func("Entity.__get_component_derived", &Entity::_get_component_derived_generic_);
	ext->link_class_func("Entity.__add_component", &EntityWrapper::add_component_generic);
	ext->link_class_func("Entity.delete_component", &EntityWrapper::delete_component);
	ext->link_class_func("Entity.__del_override__", &DeletionQueue::add_entity);

	Component component;
	ext->declare_class_size("Component", sizeof(Component));
	ext->declare_class_element("Component.owner", &Component::owner);
	ext->link_class_func("Component.__init__", &kaba::generic_init<Component>);
	ext->link_virtual("Component.__delete__", &Component::__delete__, &component);
	ext->link_virtual("Component.on_init", &Component::on_init, &component);
	ext->link_virtual("Component.on_delete", &Component::on_delete, &component);
	ext->link_virtual("Component.on_iterate", &Component::on_iterate, &component);
	ext->link_virtual("Component.on_collide", &Component::on_collide, &component);
	ext->link_class_func("Component.set_variables", &Component::set_variables);

	ext->declare_class_size("NameTag", sizeof(NameTag));
	ext->declare_class_element("NameTag.name", &NameTag::name);

	System con;
	ext->declare_class_size("Controller", sizeof(System));
	ext->link_class_func("Controller.__init__", &System::__init__);
	ext->link_virtual("Controller.__delete__", &System::__delete__, &con);
	ext->link_virtual("Controller.on_init", &System::on_init, &con);
	ext->link_virtual("Controller.on_delete", &System::on_delete, &con);
	ext->link_virtual("Controller.on_iterate", &System::on_iterate, &con);
	ext->link_virtual("Controller.on_iterate_pre", &System::on_iterate_pre, &con);
	ext->link_virtual("Controller.on_draw_pre", &System::on_draw_pre, &con);
	ext->link_virtual("Controller.on_input", &System::on_input, &con);
	ext->link_virtual("Controller.on_key", &System::on_key, &con);
	ext->link_virtual("Controller.on_key_down", &System::on_key_down, &con);
	ext->link_virtual("Controller.on_key_up", &System::on_key_up, &con);
	ext->link_virtual("Controller.on_left_button_down", &System::on_left_button_down, &con);
	ext->link_virtual("Controller.on_left_button_up", &System::on_left_button_up, &con);
	ext->link_virtual("Controller.on_middle_button_down", &System::on_middle_button_down, &con);
	ext->link_virtual("Controller.on_middle_button_up", &System::on_middle_button_up, &con);
	ext->link_virtual("Controller.on_right_button_down", &System::on_right_button_down, &con);
	ext->link_virtual("Controller.on_right_button_up", &System::on_right_button_up, &con);
	ext->link_virtual("Controller.on_render_inject", &System::on_render_inject, &con);
	ext->link_class_func("Controller.__del_override__", &DeletionQueue::add);

	ext->link_func("__get_component_list", &__query_component_list);
	ext->link_func("__get_component_family_list", &__query_component_list_family);
	ext->link_func("__get_component_list2", &__query_component_list2);

	ext->link_func("__get_controller", &SystemManager::get);
}

void export_world(kaba::Exporter* ext) {
	ext->declare_enum("PhysicsMode.NONE", PhysicsMode::NONE);
	ext->declare_enum("PhysicsMode.SIMPLE", PhysicsMode::SIMPLE);
	ext->declare_enum("PhysicsMode.FULL_EXTERNAL", PhysicsMode::FULL_EXTERNAL);
	ext->declare_enum("PhysicsMode.FULL_INTERNAL", PhysicsMode::FULL_INTERNAL);

	//ext->declare_enum("TraceMode.PHYSICAL", TraceMode::PHYSICAL);

	Camera _cam;
	ext->declare_class_size("Camera", sizeof(Camera));
	ext->declare_class_element("Camera.fov", &Camera::fov);
	ext->declare_class_element("Camera.exposure", &Camera::exposure);
	ext->declare_class_element("Camera.auto_exposure", &Camera::auto_exposure);
	ext->declare_class_element("Camera.auto_exposure_min", &Camera::auto_exposure_min);
	ext->declare_class_element("Camera.auto_exposure_max", &Camera::auto_exposure_max);
	ext->declare_class_element("Camera.auto_exposure_speed", &Camera::auto_exposure_speed);
	ext->declare_class_element("Camera.bloom_radius", &Camera::bloom_radius);
	ext->declare_class_element("Camera.bloom_factor", &Camera::bloom_factor);
	ext->declare_class_element("Camera.focus_enabled", &Camera::focus_enabled);
	ext->declare_class_element("Camera.focal_length", &Camera::focal_length);
	ext->declare_class_element("Camera.focal_blur", &Camera::focal_blur);
	ext->declare_class_element("Camera.enabled", &Camera::enabled);
	ext->declare_class_element("Camera.show", &Camera::show);
	ext->declare_class_element("Camera.min_depth", &Camera::min_depth);
	ext->declare_class_element("Camera.max_depth", &Camera::max_depth);
	ext->link_class_func("Camera.update_matrix_cache", &Camera::update_matrix_cache);
	ext->link_class_func("Camera.project", &Camera::project);
	ext->link_class_func("Camera.unproject", &Camera::unproject);


#define _OFFSET(VAR, MEMBER)	(char*)&VAR.MEMBER - (char*)&VAR

	{
		CubeMapSource source;
		ext->declare_class_size("CubeMapSource", sizeof(CubeMapSource));
		ext->declare_class_element("CubeMapSource.min_depth", _OFFSET(source, source.min_depth));
		ext->declare_class_element("CubeMapSource.max_depth", _OFFSET(source, source.max_depth));
		ext->declare_class_element("CubeMapSource.cube_map", _OFFSET(source, source.cube_map));
		ext->declare_class_element("CubeMapSource.resolution", _OFFSET(source, source.resolution));
		ext->declare_class_element("CubeMapSource.update_rate", _OFFSET(source, source.update_rate));
	}


	ext->declare_class_size("Model.Mesh", sizeof(Mesh));
	ext->declare_class_element("Model.Mesh.bone_index", &Mesh::bone_index);
	ext->declare_class_element("Model.Mesh.vertex", &Mesh::vertex);
	ext->declare_class_element("Model.Mesh.sub", &Mesh::sub);

	ext->declare_class_size("Model.Mesh.Sub", sizeof(SubMesh));
	ext->declare_class_element("Model.Mesh.Sub.num_triangles", &SubMesh::num_triangles);
	ext->declare_class_element("Model.Mesh.Sub.triangle_index", &SubMesh::triangle_index);
	ext->declare_class_element("Model.Mesh.Sub.skin_vertex", &SubMesh::skin_vertex);
	ext->declare_class_element("Model.Mesh.Sub.normal", &SubMesh::normal);

	Model model;
	ext->declare_class_size("Model", sizeof(Model));
	ext->declare_class_element("Model.mesh", &Model::mesh);
	ext->declare_class_element("Model.materials", &Model::material);
	ext->declare_class_element("Model.matrix", &Model::_matrix);
	ext->declare_class_element("Model.radius", (char*)&model.prop.radius - (char*)&model);
	ext->declare_class_element("Model.min", (char*)&model.prop.min - (char*)&model);
	ext->declare_class_element("Model.max", (char*)&model.prop.max- (char*)&model);
	ext->link_class_func("Model.__init__", &Model::__init__);
	ext->link_virtual("Model.__delete__", &Model::__delete__, &model);
	ext->link_class_func("Model.make_editable", &Model::make_editable);
	ext->link_class_func("Model.begin_edit", &Model::begin_edit);
	ext->link_class_func("Model.end_edit", &Model::end_edit);
	ext->link_class_func("Model.get_vertex", &Model::get_vertex);
	ext->link_class_func("Model.update_matrix", &Model::update_matrix);
//	ext->link_class_func("Model.set_bone_model", &Model::set_bone_model);

	ext->link_virtual("Model.on_init", &Model::on_init, &model);
	ext->link_virtual("Model.on_delete", &Model::on_delete, &model);
	ext->link_virtual("Model.on_iterate", &Model::on_iterate, &model);


	ext->declare_class_size("UserMesh", sizeof(UserMesh));
	ext->declare_class_element("UserMesh.vertex_buffer", &UserMesh::vertex_buffer);
	ext->declare_class_element("UserMesh.material", &UserMesh::material);
	ext->declare_class_element("UserMesh.vertex_shader_module", &UserMesh::vertex_shader_module);
	ext->declare_class_element("UserMesh.geometry_shader_module", &UserMesh::geometry_shader_module);
	ext->declare_class_element("UserMesh.topology", &UserMesh::topology);


	ext->declare_class_size("Animator", sizeof(Animator));
	ext->link_class_func("Animator.reset", &Animator::reset);
	ext->link_class_func("Animator.add", &Animator::add);
	ext->link_class_func("Animator.add_x", &Animator::add_x);
	ext->link_class_func("Animator.is_done", &Animator::is_done);
	ext->link_class_func("Animator.begin_edit", &Animator::reset); // JUST FOR COMPATIBILITY WITH OLD BRANCH


	ext->declare_class_size("Skeleton", sizeof(Skeleton));
	ext->declare_class_element("Skeleton.bones", &Skeleton::bones);
	ext->declare_class_element("Skeleton.parents", &Skeleton::parents);
	ext->declare_class_element("Skeleton.dpos", &Skeleton::dpos);
	ext->declare_class_element("Skeleton.pos0", &Skeleton::pos0);
	ext->link_class_func("Skeleton.reset", &Skeleton::reset);


	ext->declare_class_size("SolidBody", sizeof(SolidBody));
	ext->declare_class_element("SolidBody.vel", &SolidBody::vel);
	ext->declare_class_element("SolidBody.rot", &SolidBody::rot);
	ext->declare_class_element("SolidBody.mass", &SolidBody::mass);
	//ext->declare_class_element("SolidBody.theta", &SolidBody::theta_world);
	ext->declare_class_element("SolidBody.theta", &SolidBody::theta_0);
	ext->declare_class_element("SolidBody.g_factor", &SolidBody::g_factor);
	ext->declare_class_element("SolidBody.physics_active", &SolidBody::active);
	ext->declare_class_element("SolidBody.physics_passive", &SolidBody::passive);
	ext->link_class_func("SolidBody.add_force", &SolidBody::add_force);
	ext->link_class_func("SolidBody.add_impulse", &SolidBody::add_impulse);
	ext->link_class_func("SolidBody.add_torque", &SolidBody::add_torque);
	ext->link_class_func("SolidBody.add_torque_impulse", &SolidBody::add_torque_impulse);
	ext->link_class_func("SolidBody.update_motion", &SolidBody::update_motion);
	ext->link_class_func("SolidBody.update_mass", &SolidBody::update_mass);

	ext->declare_class_size("Collider", sizeof(Collider));

	ext->declare_class_size("MultiInstance", sizeof(MultiInstance));
	ext->declare_class_element("MultiInstance.model", &MultiInstance::model);
	ext->declare_class_element("MultiInstance.matrices", &MultiInstance::matrices);


	ext->declare_class_size("Terrain", sizeof(Terrain));
	//ext->declare_class_element("Terrain.pos", &Terrain::pos);
	ext->declare_class_element("Terrain.material", &Terrain::material);
	ext->declare_class_element("Terrain.height", &Terrain::height);
	ext->declare_class_element("Terrain.vertex", &Terrain::vertex);
	ext->declare_class_element("Terrain.normal", &Terrain::normal);
	ext->declare_class_element("Terrain.pattern", &Terrain::pattern);
	ext->declare_class_element("Terrain.num_x", &Terrain::num_x);
	ext->declare_class_element("Terrain.num_z", &Terrain::num_z);
	ext->declare_class_element("Terrain.vertex_shader_module", &Terrain::vertex_shader_module);
	ext->declare_class_element("Terrain.texture_scale", &Terrain::texture_scale);
	ext->link_class_func("Terrain.update", &Terrain::update);
	ext->link_class_func("Terrain.get_height", &Terrain::gimme_height);

	ext->declare_class_size("CollisionData", sizeof(CollisionData));
	ext->declare_class_element("CollisionData.entity", &CollisionData::entity);
	ext->declare_class_element("CollisionData.body", &CollisionData::body);
	ext->declare_class_element("CollisionData.pos", &CollisionData::pos);
	ext->declare_class_element("CollisionData.n", &CollisionData::n);

	ext->declare_class_size("ModelRef", sizeof(ModelRef));
	ext->declare_class_element("ModelRef.filename", &ModelRef::filename);
	ext->declare_class_element("ModelRef.model", &ModelRef::model);

	ext->declare_class_size("TerrainRef", sizeof(TerrainRef));
	ext->declare_class_element("TerrainRef.filename", &TerrainRef::filename);
	ext->declare_class_element("TerrainRef.terrain", &TerrainRef::terrain);

	ext->declare_class_element("World.background", &World::background);
	ext->declare_class_element("World.skyboxes", &World::skybox);
	ext->declare_class_element("World.links", &World::links);
	ext->declare_class_element("World.ego", &World::ego);
	ext->declare_class_element("World.fog", &World::fog);
	ext->declare_class_element("World.gravity", &World::gravity);
	ext->declare_class_element("World.physics_mode", &World::physics_mode);
	ext->declare_class_element("World.msg_data", &World::msg_data);
	ext->link_class_func("World.load_soon", &World::load_soon);
	ext->link_class_func("World.load_template", &World::load_template);
	ext->link_class_func("World.create_object", &_create_object);
	ext->link_class_func("World.create_object_multi", &_create_object_multi);
	ext->link_class_func("World.create_terrain", &World::create_terrain);
	ext->link_class_func("World.create_entity", &World::create_entity);
	ext->link_class_func("World.set_active_physics", &World::set_active_physics);
	ext->link_class_func("World.create_light_parallel", &World::create_light_parallel);
	ext->link_class_func("World.create_light_point", &World::create_light_point);
	ext->link_class_func("World.create_light_cone", &World::create_light_cone);
	ext->link_class_func("World.create_camera", &World::create_camera);
	ext->link_class_func("World.attach_model", &_attach_model);
	ext->link_class_func("World.unattach_model", &World::unattach_model);
	ext->link_class_func("World.add_link", &World::add_link);
	ext->link_class_func("World._add_particle", &_world_add_legacy_particle);
	ext->link_class_func("World.shift_all", &World::shift_all);
	ext->link_class_func("World.get_g", &World::get_g);
	ext->link_class_func("World.trace", &World::trace);
	ext->link_class_func("World.delete_entity", &World::delete_entity);
	ext->link_class_func("World.delete_link", &World::delete_link);
	ext->link_class_func("World.subscribe", &World::subscribe);


	ext->declare_class_element("World.MessageData.e", &World::MessageData::e);
	ext->declare_class_element("World.MessageData.v", &World::MessageData::v);

	ext->declare_class_element("Fog.color", &Fog::_color);
	ext->declare_class_element("Fog.enabled", &Fog::enabled);
	ext->declare_class_element("Fog.distance", &Fog::distance);



#define _OFFSET(VAR, MEMBER)	(char*)&VAR.MEMBER - (char*)&VAR

	Light light(yrenderer::LightType::DIRECTIONAL, Black);
	ext->declare_class_size("Light", sizeof(Light));
	ext->declare_class_element("Light.type", _OFFSET(light, light.type));
	ext->declare_class_element("Light.color", _OFFSET(light, light.col));
	ext->declare_class_element("Light.power", _OFFSET(light, light.power));
	ext->declare_class_element("Light.theta", _OFFSET(light, light.theta));
	ext->declare_class_element("Light.harshness", _OFFSET(light, light.harshness));
	ext->declare_class_element("Light.enabled", _OFFSET(light, light.enabled));
	ext->declare_class_element("Light.allow_shadow", _OFFSET(light, light.allow_shadow));
	ext->declare_class_element("Light.user_shadow_control", _OFFSET(light, light.user_shadow_control));
	ext->declare_class_element("Light.user_shadow_theta", _OFFSET(light, light.user_shadow_theta));
	ext->declare_class_element("Light.shadow_dist_max", _OFFSET(light, light.shadow_dist_max));
	ext->declare_class_element("Light.shadow_dist_min", _OFFSET(light, light.shadow_dist_min));
	ext->link_class_func("Light.set_direction", &Light::set_direction);

	/*ext->link_class_func("Light.Parallel.__init__", &Light::__init_parallel__);
	ext->link_class_func("Light.Spherical.__init__", &Light::__init_spherical__);
	ext->link_class_func("Light.Cone.__init__", &Light::__init_cone__);*/

	ext->declare_class_size("Link", sizeof(Link));
	ext->declare_class_element("Link.a", &Link::a);
	ext->declare_class_element("Link.b", &Link::b);
	ext->link_class_func("Link.set_motor", &Link::set_motor);
	ext->link_class_func("Link.set_frame", &Link::set_frame);
	//ext->link_class_func("Link.set_axis", &Link::set_axis);
	ext->link_class_func("Link.__del_override__", &DeletionQueue::add);

	ext->link("world", &world);
	ext->link("cam", &cam_main);
	ext->link_func("load_model", &__load_model);
	ext->link_func("load_material", &__load_material);

	ext->link_func("attach_light_parallel", &attach_light_parallel);
	ext->link_func("attach_light_point", &attach_light_point);
	ext->link_func("attach_light_cone", &attach_light_cone);
}

void export_gfx(kaba::Exporter* ext) {

	ext->link_func("load_shader", &__load_shader);
	ext->link_func("create_shader", &__create_shader);
	ext->link_func("load_texture", &__load_texture);

	static void* dummy = nullptr;

	if (engine.context)
		ext->link("tex_white", &engine.context->tex_white);
	else
		ext->link("tex_white", &dummy);
}

void export_fx(kaba::Exporter* ext) {
	ext->declare_class_size("Particle", sizeof(Particle));
	ext->declare_class_element("Particle.pos", &Particle::pos);
	ext->declare_class_element("Particle.vel", &Particle::vel);
	ext->declare_class_element("Particle.radius", &Particle::radius);
	ext->declare_class_element("Particle.time_to_live", &Particle::time_to_live);
	ext->declare_class_element("Particle.suicidal", &Particle::suicidal);
	ext->declare_class_element("Particle.color", &Particle::col);
	ext->declare_class_element("Particle.enabled", &Particle::enabled);

	ext->declare_class_size("Beam", sizeof(Beam));
	ext->declare_class_element("Beam.length", &Beam::length);


	{
		LegacyParticle particle;
		ext->declare_class_size("LegacyParticle", sizeof(LegacyParticle));
		ext->declare_class_element("LegacyParticle.vel", &LegacyParticle::vel);
		ext->declare_class_element("LegacyParticle.radius", &LegacyParticle::radius);
		ext->declare_class_element("LegacyParticle.time_to_live", &LegacyParticle::time_to_live);
		ext->declare_class_element("LegacyParticle.texture", &LegacyParticle::texture);
		ext->declare_class_element("LegacyParticle.color", &LegacyParticle::col);
		ext->declare_class_element("LegacyParticle.source", &LegacyParticle::source);
		ext->declare_class_element("LegacyParticle.enabled", &LegacyParticle::enabled);
		ext->link_class_func("LegacyParticle.__init__", &LegacyParticle::__init__);
		ext->link_virtual("LegacyParticle.__delete__", &LegacyParticle::__delete__, &particle);
		//ext->link_virtual("LegacyParticle.on_iterate", &Particle::on_iterate, &particle);
		//ext->link_class_func("LegacyParticle.__del_override__", &global_delete);
		ext->link_class_func("LegacyParticle.__del_override__", &DeletionQueue::add);
	}

	ext->declare_class_size("LegacyBeam", sizeof(LegacyBeam));
	ext->declare_class_element("LegacyBeam.length", &LegacyBeam::length);
	ext->link_class_func("LegacyBeam.__init__", &LegacyBeam::__init_beam__);

	{
	ParticleGroup group;
	ext->declare_class_size("ParticleGroup", sizeof(ParticleGroup));
	ext->declare_class_element("ParticleGroup.source", &ParticleGroup::source);
	ext->link_class_func("ParticleGroup.__init__", &ParticleGroup::__init__);
	ext->link_class_func("ParticleGroup.emit", &ParticleGroup::emit_particle);
	ext->link_class_func("ParticleGroup.emit_beam", &ParticleGroup::emit_beam);
	ext->link_class_func("ParticleGroup.iterate_particles", &ParticleGroup::iterate_particles);
	ext->link_virtual("ParticleGroup.__delete__", &ParticleGroup::__delete__, &group);
	ext->link_virtual("ParticleGroup.on_iterate", &ParticleGroup::on_iterate, &group);
	ext->link_virtual("ParticleGroup.on_iterate_particle", &ParticleGroup::on_iterate_particle, &group);
	ext->link_virtual("ParticleGroup.on_iterate_beam", &ParticleGroup::on_iterate_beam, &group);
	//ext->link_class_func("ParticleGroup.__del_override__", &DeletionQueue::add);
	}

	{
	ParticleEmitter emitter;
	ext->declare_class_size("ParticleEmitter", sizeof(ParticleEmitter));
	ext->declare_class_element("ParticleEmitter.spawn_beams", &ParticleEmitter::spawn_beams);
	ext->declare_class_element("ParticleEmitter.spawn_dt", &ParticleEmitter::spawn_dt);
	ext->declare_class_element("ParticleEmitter.spawn_time_to_live", &ParticleEmitter::spawn_time_to_live);
	ext->link_class_func("ParticleEmitter.__init__", &ParticleEmitter::__init__);
	ext->link_class_func("ParticleEmitter.emit_particle", &ParticleEmitter::emit_particle);
	ext->link_class_func("ParticleEmitter.iterate_emitter", &ParticleEmitter::iterate_emitter);
	ext->link_virtual("ParticleEmitter.__delete__", &ParticleEmitter::__delete__, &emitter);
	ext->link_virtual("ParticleEmitter.on_iterate", &ParticleEmitter::on_iterate, &emitter);
	ext->link_virtual("ParticleEmitter.on_init_particle", &ParticleEmitter::on_init_particle, &emitter);
	ext->link_virtual("ParticleEmitter.on_init_beam", &ParticleEmitter::on_init_beam, &emitter);
	//ext->link_class_func("ParticleEmitter.__del_override__", &DeletionQueue::add);
	}
}

void export_ui(kaba::Exporter* ext) {
#ifdef HAS_INPUT
	ext->declare_enum("VRDeviceRole.NONE", input::VRDeviceRole::None);
	ext->declare_enum("VRDeviceRole.CONTROLLER_RIGHT", input::VRDeviceRole::ControllerRight);
	ext->declare_enum("VRDeviceRole.CONTROLLER_LEFT", input::VRDeviceRole::ControllerLeft);
	ext->declare_enum("VRDeviceRole.HEADSET", input::VRDeviceRole::Headset);
	ext->declare_enum("VRDeviceRole.LIGHTHOUSE0", input::VRDeviceRole::Lighthouse0);
	ext->declare_enum("VRDeviceRole.LIGHTHOUSE1", input::VRDeviceRole::Lighthouse1);
#endif

	{
		gui::Node node;
		ext->declare_class_size("Node", sizeof(gui::Node));
		ext->declare_class_element("Node.x", &gui::Node::pos);
		ext->declare_class_element("Node.y", _OFFSET(node, pos.y));
		ext->declare_class_element("Node.pos", &gui::Node::pos);
		ext->declare_class_element("Node.width", &gui::Node::width);
		ext->declare_class_element("Node.height", &gui::Node::height);
		ext->declare_class_element("Node._eff_area", &gui::Node::eff_area);
		ext->declare_class_element("Node.margin", &gui::Node::margin);
		ext->declare_class_element("Node.align", &gui::Node::align);
		ext->declare_class_element("Node.dz", &gui::Node::dz);
		ext->declare_class_element("Node.color", &gui::Node::col);
		ext->declare_class_element("Node.visible", &gui::Node::visible);
		ext->declare_class_element("Node.children", &gui::Node::children);
		ext->declare_class_element("Node.parent", &gui::Node::parent);
		ext->link_class_func("Node.__init__:Node", &kaba::generic_init<gui::Node>);
		ext->link_class_func("Node.__init__:Node:math.Rect", &kaba::generic_init_ext<gui::Node, const rect&>);
		ext->link_virtual("Node.__delete__", &kaba::generic_virtual<gui::Node>::__delete__, &node);
		ext->link_class_func("Node.__del_override__", &DeletionQueue::add);
		ext->link_class_func("Node.add", &gui::Node::add);
		ext->link_class_func("Node.add_from_source", &gui::Node::add_from_source);
		ext->link_class_func("Node.remove", &gui::Node::remove);
		ext->link_class_func("Node.remove_all_children", &gui::Node::remove_all_children);
		ext->link_class_func("Node.set_area", &gui::Node::set_area);
		ext->link_class_func("Node._get", &gui::Node::get);
		ext->link_virtual("Node.on_iterate", &gui::Node::on_iterate, &node);
		ext->link_virtual("Node.on_enter", &gui::Node::on_enter, &node);
		ext->link_virtual("Node.on_leave", &gui::Node::on_leave, &node);
		ext->link_virtual("Node.on_left_button_down", &gui::Node::on_left_button_down, &node);
		ext->link_virtual("Node.on_left_button_up", &gui::Node::on_left_button_up, &node);
		ext->link_virtual("Node.on_middle_button_down", &gui::Node::on_middle_button_down, &node);
		ext->link_virtual("Node.on_middle_button_up", &gui::Node::on_middle_button_up, &node);
		ext->link_virtual("Node.on_right_button_down", &gui::Node::on_right_button_down, &node);
		ext->link_virtual("Node.on_right_button_up", &gui::Node::on_right_button_up, &node);
	}

	{
		gui::Picture picture;//(rect::ID, nullptr);
		ext->declare_class_size("Picture", sizeof(gui::Picture));
		ext->declare_class_element("Picture.source", &gui::Picture::source);
		ext->declare_class_element("Picture.texture", &gui::Picture::texture);
		ext->declare_class_element("Picture.shader", &gui::Picture::shader);
		ext->declare_class_element("Picture.shader_data", &gui::Picture::shader_data);
		ext->declare_class_element("Picture.blur", &gui::Picture::bg_blur);
		ext->declare_class_element("Picture.angle", &gui::Picture::angle);
		ext->link_class_func("Picture.__init__:Picture", &kaba::generic_init<gui::Picture>);
		ext->link_class_func("Picture.__init__:Picture:math.Rect:shared![Texture]:math.Rect", &kaba::generic_init_ext<gui::Picture, const rect&, shared<Texture>, const rect&>);
		ext->link_virtual("Picture.__delete__", &kaba::generic_virtual<gui::Picture>::__delete__, &picture);
	}

	{
		gui::Text text;//(":::fake:::", 0, vec2::ZERO);
		ext->declare_class_size("Text", sizeof(gui::Text));
		ext->declare_class_element("Text.font_size", &gui::Text::font_size);
		ext->declare_class_element("Text.text", &gui::Text::text);
		ext->link_class_func("Text.__init__:Text", &kaba::generic_init<gui::Text>);
		ext->link_class_func("Text.__init__:Text:string:f32:math.vec2", &kaba::generic_init_ext<gui::Text, const string&, float, const vec2&>);
		ext->link_virtual("Text.__delete__", &kaba::generic_virtual<gui::Text>::__delete__, &text);
		ext->link_class_func("Text.set_text", &gui::Text::set_text);
	}

	{
		gui::Canvas canvas;
		ext->declare_class_size("Canvas", sizeof(gui::Canvas));
		ext->link_class_func("Canvas.__init__:Canvas", &kaba::generic_init<gui::Canvas>);
		ext->link_virtual("Canvas.__delete__", &kaba::generic_virtual<gui::Canvas>::__delete__, &canvas);
	}

	ext->link_class_func("HBox.__init__", &kaba::generic_init<gui::HBox>);
	ext->link_class_func("VBox.__init__", &kaba::generic_init<gui::VBox>);

#ifdef HAS_INPUT
	ext->link_func("key_state", &input::get_key);
	ext->link_func("key_down", &input::get_key_down);
	ext->link_func("key_up", &input::get_key_up);
	ext->link_func("button", &input::get_button);
	ext->link("mouse", &input::mouse);
	ext->link("dmouse", &input::dmouse);
	ext->link("scroll", &input::scroll);
	ext->link("vr_active", &input::vr_active);
	ext->link("link_mouse_and_keyboard_into_pad", &input::link_mouse_and_keyboard_into_pad);
	ext->link_func("get_pad", &input::get_pad);
	ext->link_func("get_vr_device", &input::get_vr_device);

	ext->declare_class_size("Gamepad", sizeof(input::Gamepad));
	ext->declare_class_element("Gamepad.deadzone", &input::Gamepad::deadzone);
	ext->link_class_func("Gamepad.update", &input::Gamepad::update);
	ext->link_class_func("Gamepad.is_present", &input::Gamepad::is_present);
	ext->link_class_func("Gamepad.name", &input::Gamepad::name);
	ext->link_class_func("Gamepad.axis", &input::Gamepad::axis);
	ext->link_class_func("Gamepad.button", &input::Gamepad::button);
	ext->link_class_func("Gamepad.clicked", &input::Gamepad::clicked);

	ext->declare_class_size("VRDevice", sizeof(input::VRDevice));
	ext->declare_class_element("VRDevice.role", &input::VRDevice::role);
	ext->declare_class_element("VRDevice.name", &input::VRDevice::name);
	ext->declare_class_element("VRDevice.pos", &input::VRDevice::pos);
	ext->declare_class_element("VRDevice.ang", &input::VRDevice::ang);
	ext->link_class_func("VRDevice.button", &input::VRDevice::button);
	ext->link_class_func("VRDevice.clicked", &input::VRDevice::clicked);
	ext->link_class_func("VRDevice.axis", &input::VRDevice::axis);
#else
	static int dummy;
	ext->link("key_state", &dummy);
	ext->link("key_down", &dummy);
	ext->link("key_up", &dummy);
	ext->link("button", &dummy);
	ext->link("mouse", &dummy);
	ext->link("dmouse", &dummy);
	ext->link("scroll", &dummy);
	ext->link("vr_active", &dummy);
	ext->link("link_mouse_and_keyboard_into_pad", &dummy);
	ext->link("get_pad", &dummy);
	ext->link("get_vr_device", &dummy);

	ext->declare_class_size("Gamepad", 1);
	ext->declare_class_element("Gamepad.deadzone", &dummy);
	ext->link_class_func("Gamepad.update", &dummy);
	ext->link_class_func("Gamepad.is_present", &dummy);
	ext->link_class_func("Gamepad.name", &dummy);
	ext->link_class_func("Gamepad.axis", &dummy);
	ext->link_class_func("Gamepad.button", &dummy);
	ext->link_class_func("Gamepad.clicked", &dummy);

	ext->link_class_func("VRDevice.button", &dummy);
	ext->link_class_func("VRDevice.clicked", &dummy);
	ext->link_class_func("VRDevice.axis", &dummy);
#endif

	ext->link("toplevel", &gui::toplevel);
}

void export_sound(kaba::Exporter* ext) {
	ext->declare_class_size("SoundSource", sizeof(audio::SoundSource));
	ext->declare_class_element("SoundSource.loop", &audio::SoundSource::loop);
	ext->declare_class_element("SoundSource.suicidal", &audio::SoundSource::suicidal);
	ext->declare_class_element("SoundSource.min_distance", &audio::SoundSource::min_distance);
	ext->declare_class_element("SoundSource.max_distance", &audio::SoundSource::max_distance);
	ext->declare_class_element("SoundSource.volume", &audio::SoundSource::volume);
	ext->declare_class_element("SoundSource.speed", &audio::SoundSource::speed);
	ext->link_class_func("SoundSource.play", &audio::SoundSource::play);
	ext->link_class_func("SoundSource.stop", &audio::SoundSource::stop);
	ext->link_class_func("SoundSource.pause", &audio::SoundSource::pause);
	ext->link_class_func("SoundSource.has_ended", &audio::SoundSource::has_ended);
	ext->link_class_func("SoundSource.update", &audio::SoundSource::_apply_data);
	ext->link_class_func("SoundSource.set_buffer", &audio::SoundSource::set_buffer);
	ext->link_class_func("SoundSource.set_stream", &audio::SoundSource::set_stream);
	ext->link_class_func("SoundSource.__del_override__", &DeletionQueue::add);

	ext->declare_class_size("Listener", sizeof(audio::Listener));

	ext->link_func("load_buffer", &audio::load_buffer);
	ext->link_func("create_buffer", &audio::create_buffer);
	ext->link_func("load_audio_stream", &audio::load_stream);
	ext->link_func("create_audio_stream", &__create_audio_stream);
	ext->link_func("emit_sound", &audio::emit_sound);
	ext->link_func("emit_sound_file", &audio::emit_sound_file);
	ext->link_func("emit_sound_stream", &audio::emit_sound_stream);
}

void export_net(kaba::Exporter* ext) {
	ext->declare_class_size("NetworkManager", sizeof(NetworkManager));
	ext->declare_class_element("NewtorkManager.cur_con", &NetworkManager::cur_con);
	ext->link_class_func("NetworkManager.connect_to_host", &NetworkManager::connect_to_host);
	ext->link_class_func("NetworkManager.event", &NetworkManager::event);


	ext->declare_class_size("Connection", sizeof(NetworkManager::Connection));
	ext->declare_class_element("Connection.s", &NetworkManager::Connection::s);
	ext->declare_class_element("Connection.buffer", &NetworkManager::Connection::buffer);
	ext->link_class_func("Connection.start_block", &NetworkManager::Connection::start_block);
	ext->link_class_func("Connection.end_block", &NetworkManager::Connection::end_block);
	ext->link_class_func("Connection.send", &NetworkManager::Connection::send);

	ext->link("network", &network_manager);
}

void export_engine(kaba::Exporter* ext) {

	// unused
/*	ext->declare_class_size("ResourceManager", sizeof(ResourceManager));
	ext->link_class_func("ResourceManager.load_shader", &ResourceManager::load_shader);
	ext->link_class_func("ResourceManager.create_shader", &ResourceManager::create_shader);
	ext->link_class_func("ResourceManager.load_texture", &ResourceManager::load_texture);
	ext->link_class_func("ResourceManager.load_material", &ResourceManager::load_material);
	ext->link_class_func("ResourceManager.load_model", &ResourceManager::load_model);*/

	ext->declare_class_size("EngineData", sizeof(EngineData));
	ext->declare_class_element("EngineData.app_name", &EngineData::app_name);
	ext->declare_class_element("EngineData.version", &EngineData::version);
	ext->declare_class_element("EngineData.context", &EngineData::context);
	ext->declare_class_element("EngineData.physics_enabled", &EngineData::physics_enabled);
	ext->declare_class_element("EngineData.collisions_enabled", &EngineData::collisions_enabled);
	ext->declare_class_element("EngineData.elapsed", &EngineData::elapsed);
	ext->declare_class_element("EngineData.elapsed_rt", &EngineData::elapsed_rt);
	ext->declare_class_element("EngineData.time_scale", &EngineData::time_scale);
	ext->declare_class_element("EngineData.fps_min", &EngineData::fps_min);
	ext->declare_class_element("EngineData.fps_max", &EngineData::fps_max);
	ext->declare_class_element("EngineData.resolution_scale", &EngineData::resolution_scale_x);
	ext->declare_class_element("EngineData.width", &EngineData::width);
	ext->declare_class_element("EngineData.height", &EngineData::height);
	ext->declare_class_element("EngineData.debug", &EngineData::debug);
	ext->declare_class_element("EngineData.console_enabled", &EngineData::console_enabled);
	ext->declare_class_element("EngineData.wire_mode", &EngineData::wire_mode);
	ext->declare_class_element("EngineData.show_timings", &EngineData::show_timings);
	ext->declare_class_element("EngineData.first_frame", &EngineData::first_frame);
	ext->declare_class_element("EngineData.resetting_game", &EngineData::resetting_game);
	ext->declare_class_element("EngineData.game_running", &EngineData::game_running);
	ext->declare_class_element("EngineData.default_font", &EngineData::default_font);
	ext->declare_class_element("EngineData.detail_level", &EngineData::detail_level);
	ext->declare_class_element("EngineData.initial_world_file", &EngineData::initial_world_file);
	ext->declare_class_element("EngineData.second_world_file", &EngineData::second_world_file);
	ext->declare_class_element("EngineData.physical_aspect_ratio", &EngineData::physical_aspect_ratio);
	ext->declare_class_element("EngineData.window_renderer", &EngineData::window_renderer);
	ext->declare_class_element("EngineData.gui_renderer", &EngineData::gui_renderer);
	ext->declare_class_element("EngineData.region_renderer", &EngineData::region_renderer);
	ext->declare_class_element("EngineData.render_paths", &EngineData::camera_renderers);
	ext->link_class_func("EngineData.exit", &EngineData::exit);
	ext->link_class_func("EngineData.add_render_task", &EngineData::add_render_task);


	ext->declare_class_size("Scheduler", sizeof(Scheduler));
	ext->link_class_func("Scheduler.__init__", &kaba::generic_init<Scheduler>);
	ext->link_class_func("Scheduler.__delete__", &kaba::generic_delete<Scheduler>);
	ext->link_class_func("Scheduler.later", &Scheduler::later);
	ext->link_class_func("Scheduler.repeat", &Scheduler::repeat);
	ext->link_class_func("Scheduler.clear", &Scheduler::clear);


	ext->declare_class_size("RayRequest", sizeof(RayRequest));
	ext->declare_class_element("RayRequest.p0", &RayRequest::p0);
	ext->declare_class_element("RayRequest.p1", &RayRequest::p1);

	ext->declare_class_size("RayReply", sizeof(RayReply));
	ext->declare_class_element("RayReply.p", &RayReply::p);
	ext->declare_class_element("RayReply.n", &RayReply::n);
	ext->declare_class_element("RayReply.f", &RayReply::f);
	ext->declare_class_element("RayReply.g", &RayReply::g);
	ext->declare_class_element("RayReply.t", &RayReply::t);
	ext->declare_class_element("RayReply.index", &RayReply::index);
	ext->declare_class_element("RayReply.mesh", &RayReply::mesh);

	ext->link("engine", &engine);
	ext->link_func("screenshot", &screenshot);
	ext->link_func("create_camera_renderer", &create_camera_renderer);
	ext->link_func("rt_setup", &rt_setup);
	ext->link_func("rt_update_frame", &rt_update_frame);
	ext->link_func("rt_vtrace", &vtrace);


	ext->link_func("SceneView.shadow_projection", &scene_view_shadow_projection);
}

void export_renderer(kaba::Exporter* ext) {

	using RenderPath = yrenderer::RenderPath;
	ext->declare_class_size("RenderPath", sizeof(RenderPath));
	//ext->declare_class_element("WorldRenderer.shader_fx", &RenderPath::shader_fx);
	ext->declare_class_element("RenderPath.wireframe", &RenderPath::wireframe);
	ext->declare_class_element("RenderPath.scene_view", &RenderPath::scene_view);
	ext->link_class_func("RenderPath.render_into_cubemap", &RenderPath::render_into_cubemap);


	using PostProcessor = yrenderer::PostProcessor;
	ext->declare_class_size("PostProcessor", sizeof(PostProcessor));
	ext->declare_class_element("PostProcessor.fb1", &PostProcessor::fb1);
	ext->declare_class_element("PostProcessor.fb2", &PostProcessor::fb2);
	ext->link_class_func("PostProcessor.next_fb", &PostProcessor::next_fb);
	ext->link_class_func("PostProcessor.process", &PostProcessor::process);
	ext->link_class_func("PostProcessor.add_stage", &PostProcessor::add_stage);

	ext->declare_class_size("RayTracingData", sizeof(RayTracingData));
	ext->declare_class_element("RayTracingData.buffer_meshes", &RayTracingData::buffer_meshes);
	ext->declare_class_element("RayTracingData.num_meshes", &RayTracingData::num_meshes);

	ext->declare_class_size("FullCameraRenderer", sizeof(FullCameraRenderer));
	ext->declare_class_element("FullCameraRenderer.hdr_resolver", &FullCameraRenderer::hdr_resolver);
	ext->declare_class_element("FullCameraRenderer.render_path", &FullCameraRenderer::render_path);
	ext->declare_class_element("FullCameraRenderer.post_processor", &FullCameraRenderer::post_processor);
	ext->declare_class_element("FullCameraRenderer.light_meter", &FullCameraRenderer::light_meter);
	ext->declare_class_element("FullCameraRenderer.type", &FullCameraRenderer::type);
	ext->link_class_func("FullCameraRenderer.get_shadow_map", &camera_renderer_get_shadow_map);
	ext->link_class_func("FullCameraRenderer.get_gbuffer", &camera_renderer_get_gbuffer);
	//	ext->link_virtual("FullCameraRenderer.render_into_texture", &RPF::render_into_texture, engine.world_renderer);
	ext->link_class_func("FullCameraRenderer.get_cubemap", &camera_renderer_get_cubemap);
}

void export_kaba_package_y(kaba::Exporter* ext) {
	export_gfx(ext);
	export_ecs(ext);
	export_world(ext);
	export_fx(ext);
	export_ui(ext);
	export_sound(ext);
	export_net(ext);
	export_engine(ext);
	export_renderer(ext);
	export_package_yrenderer(ext);
	export_package_profiler(ext);
}

template<class C>
void import_component_class(shared<kaba::Module> m, const string &name) {
	for (auto c: m->classes()) {
		if (c->name == name)
			C::_class = c;
	}
	if (!C::_class)
		throw Exception(format("y.kaba: %s missing", name));
	if (!C::_class->is_derived_from_s("ecs.Component"))
		throw Exception(format("y.kaba: %s not derived from Component", name));
}

void import_kaba() {
	auto m_model = kaba::default_context->load_module("y/model.kaba");
	import_component_class<Animator>(m_model, "Animator");
	import_component_class<Skeleton>(m_model, "Skeleton");
	import_component_class<Model>(m_model, "Model");

	auto m_world = kaba::default_context->load_module("y/world.kaba");
	import_component_class<SolidBody>(m_world, "SolidBody");
	import_component_class<Collider>(m_world, "Collider");
	import_component_class<MeshCollider>(m_world, "MeshCollider");
	import_component_class<SphereCollider>(m_world, "SphereCollider");
	import_component_class<BoxCollider>(m_world, "BoxCollider");
	import_component_class<TerrainCollider>(m_world, "TerrainCollider");
	import_component_class<MultiInstance>(m_world, "MultiInstance");
	import_component_class<Terrain>(m_world, "Terrain");
	import_component_class<Light>(m_world, "Light");
	import_component_class<Camera>(m_world, "Camera");
	import_component_class<::CubeMapSource>(m_world, "CubeMapSource");
	import_component_class<NameTag>(m_world, "NameTag");
	import_component_class<ModelRef>(m_world, "ModelRef");
	import_component_class<TerrainRef>(m_world, "TerrainRef");

	auto m_fx = kaba::default_context->load_module("y/fx.kaba");
	import_component_class<ParticleGroup>(m_fx, "ParticleGroup");
	import_component_class<ParticleEmitter>(m_fx, "ParticleEmitter");
	import_component_class<LegacyParticle>(m_fx, "LegacyParticle");
	import_component_class<LegacyBeam>(m_fx, "LegacyBeam");

	auto m_audio = kaba::default_context->load_module("y/audio.kaba");
	import_component_class<audio::SoundSource>(m_audio, "SoundSource");
	import_component_class<audio::Listener>(m_audio, "Listener");

	auto m_y = kaba::default_context->load_module("y/y.kaba");
	import_component_class<UserMesh>(m_y, "UserMesh");

	//msg_write(MeshCollider::_class->name);
	//msg_write(MeshCollider::_class->parent->name);
	//msg_write(MeshCollider::_class->parent->parent->name);
}

Array<ScriptInstanceDataVariable> parse_variables(const string &var) {
	Array<ScriptInstanceDataVariable> r;
	auto xx = var.explode(",");
	for (auto &x: xx) {
		auto y = x.explode(":");
		auto name = y[0].trim().lower().replace("_", "");
		if (y[1].trim().match("\"*\""))
			r.add({name, "", y[1].trim().sub_ref(1, -1)});
		else
			r.add({name, "", y[1].trim().unescape()});
	}
	return r;
}

vec3 s2v(const string &s) {
	auto x = s.explode(" ");
	return vec3(x[0]._float(), x[1]._float(), x[2]._float());
}

color s2c(const string &s) {
	auto x = s.explode(" ");
	return color(x[3]._float(), x[0]._float(), x[1]._float(), x[2]._float());
}

mat3 s2mat3(const string &s) {
	const auto a = Any::parse(s);
	mat3 m = mat3::ZERO;
	if (a.is_list())
		for (int i=0; i<min(9, a.length()); i++)
			m.e[i] = a[i].to_f32();
	return m;
}

string whatever_to_string(const void* instance, int offset, const kaba::Class* c) {
	if (!instance)
		return "";
	auto p = (const char*)instance + offset;
	if (c == kaba::TypeString)
		return *(const string*)p;
	if (c == kaba::TypePath)
		return str(*(const Path*)p);
	if (c == kaba::TypeFloat32)
		return f2s(*(const float*)p, 3);
	if (c == kaba::TypeInt32 or c->is_enum())
		return str(*(const int*)p);
	if (c == kaba::TypeBool)
		return b2s(*(const bool*)p);
	if (c == kaba::TypeVec3) {
		const auto v = *(const vec3*)p;
		return format("%.3f %.3f %.3f", v.x, v.y, v.z);
	}
	if (c == kaba::TypeColor) {
		const auto v = *(const color*)p;
		return format("%.3f %.3f %.3f %.3f", v.r, v.g, v.b, v.a);
	}
	if (c->name == "mat3")
		return mat3_to_any(*(const mat3*)p).str();
	return "???";
}

void whatever_from_string(void* p, const kaba::Class* type, const string& value) {
	if (type == kaba::TypeString)
		*(string*)p = value;
	if (type == kaba::TypePath)
		*(Path*)p = value;
	if (type == kaba::TypeFloat32)
		*(float*)p = value._float();
	if (type == kaba::TypeInt32 or type->is_enum())
		*(int*)p = value._int();
	if (type == kaba::TypeBool)
		*(bool*)p = value._bool();
	if (type == kaba::TypeVec3)
		*(vec3*)p = s2v(value);
	if (type == kaba::TypeColor)
		*(color*)p = s2c(value);
	if (type->name == "mat3")
		*(mat3*)p = s2mat3(value);
}

void assign_variables(void* p, const kaba::Class* c, const Array<ScriptInstanceDataVariable>& variables) {
	for (const auto& v: variables)
		for (const auto& e: c->elements)
			if (v.name == e.name) {
				//msg_write("  " + e.type->long_name() + " " + e.name + " = " + v.value);
				whatever_from_string((char*)p + e.offset, e.type, v.value);
			}
}

const kaba::Class *find_class_derived(const Path &filename, const string &base_class) {
	//msg_write(format("INSTANCE  %s:   %s", filename, base_class));
	try {
		auto s = kaba::default_context->load_module(filename);
		for (auto c: s->classes()) {
			if (c->is_derived_from_s(base_class)) {
				return c;
			}
		}
	} catch (kaba::Exception &e) {
		msg_error(e.message());
		throw Exception(e.message());
	}
	throw Exception(format("script does not contain a class derived from '%s'", base_class));
	return nullptr;
}

const kaba::Class *find_class(const Path &filename, const string &name) {
	//msg_write(format("INSTANCE  %s:   %s", filename, base_class));
	if (filename.is_empty()) {
		if (name == "Camera")
			return Camera::_class;
		if (name == "Light")
			return Light::_class;
		if (name == "ModelRef")
			return ModelRef::_class;
		if (name == "TerrainRef")
			return TerrainRef::_class;
		if (name == "SolidBody")
			return SolidBody::_class;
		if (name == "Skeleton")
			return Skeleton::_class;
		if (name == "Animator")
			return Animator::_class;
		if (name == "MeshCollider")
			return MeshCollider::_class;
		if (name == "TerrainCollider")
			return TerrainCollider::_class;
		if (name == "BoxCollider")
			return BoxCollider::_class;
		if (name == "SphereCollider")
			return SphereCollider::_class;
	}
	try {
		auto s = kaba::default_context->load_module(filename);
		for (auto c: s->classes()) {
			if (c->name == name) {
				return c;
			}
		}
	} catch (kaba::Exception &e) {
		msg_error(e.message());
		throw;
	}
	throw Exception(format("script does not contain a class named '%s'", name));
	return nullptr;
}

void *create_instance(const kaba::Class *c, const string &variables) {
	return create_instance(c, parse_variables(variables));
}

void *create_instance(const kaba::Class *c, const Array<ScriptInstanceDataVariable> &variables) {
	//msg_write(format("INSTANCE  %s:   %s", filename, base_class));
	msg_write(format("creating instance  %s", c->long_name()));
	if (c == SolidBody::_class)
		return new SolidBody;
	if (c == ModelRef::_class)
		return new ModelRef;
	if (c == TerrainRef::_class)
		return new TerrainRef;
	if (c == MeshCollider::_class)
		return new MeshCollider;
	if (c == TerrainCollider::_class)
		return new TerrainCollider;
	if (c == Terrain::_class)
		return new Terrain;
	if (c == Animator::_class)
		return new Animator;
	if (c == Skeleton::_class)
		return new Skeleton;
	if (c == Light::_class)
		return new Light(yrenderer::LightType::POINT, White);
	if (c == Camera::_class)
		return new Camera;
	if (c == audio::SoundSource::_class)
		return new audio::SoundSource;
	if (c == LegacyParticle::_class)
		return new LegacyParticle;
	if (c == LegacyBeam::_class)
		return new LegacyBeam;
	if (c == CubeMapSource::_class)
		return new CubeMapSource;
	if (c == NameTag::_class)
		return new NameTag;
	void *p = c->create_instance();
	assign_variables(p, c, variables);
	return p;
}

void* create_instance_auto(const string& extended_type_name) {
	auto x = extended_type_name.explode(".");
	string type = x.back();

	for (auto m: weak(kaba::default_context->public_modules)) {
		for (auto c: m->classes())
			if (c->name == type) {
				msg_error("create: " + c->long_name());
				return create_instance(c, "");
			}
	}
	return nullptr;
}




string callable_name(const void *c) {
	auto t = kaba::default_context->get_dynamic_type((const VirtualBase*)c);
	if (!t)
		return "callable:" + p2s(c);
	static const bool EXTRACT_FUNCTION_NAME = false;
	if (t->is_callable_bind()) {
		if (EXTRACT_FUNCTION_NAME) {
			for (auto &e: t->elements)
				if (e.name == "_fp")
					return "BIND:" + callable_name(*(const char**)((const char*)c + e.offset));
			return "kaba:bind:" + t->name_space->name;
		}
		return t->name_space->owner->module->filename.basename();
	}
	if (t->is_callable_fp()) {
		if (EXTRACT_FUNCTION_NAME) {
			for (auto &e: t->elements)
				if (e.name == "_fp") {
					auto func = *(const kaba::Function**)((const char*)c + e.offset);
					return "FUNC:" + func->long_name();
				}
			return "func:" + t->long_name();
		}
		return t->name_space->owner->module->filename.basename();//relative_to(engine.script_dir).str();
	}
	return "callable:" + p2s(c);
}


}


