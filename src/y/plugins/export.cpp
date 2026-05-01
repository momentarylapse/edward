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
#include "../fx/ParticleManager.h"
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
#include <EngineData.h>
#include <ecs/Component.h>
#include <ecs/ComponentManager.h>
#include <ecs/System.h>
#include <ecs/SystemManager.h>
#include "../world/components/Camera.h"
#include "../world/components/Link.h"
#include "../world/Model.h"
#include "../world/Terrain.h"
#include "../world/World.h"
#include "../world/systems/Physics.h"
#include "../world/components/Light.h"
#include "../world/components/RigidBody.h"
#include "../world/components/Collider.h"
#include "../world/components/Animator.h"
#include "../world/components/Skeleton.h"
#include "../world/components/UserMesh.h"
#include "../world/components/MultiInstance.h"
#include "../world/components/CubeMapSource.h"
#include <lib/ygraphics/graphics-impl.h>
#include <lib/ygraphics/Context.h>
#include <lib/kaba/dynamic/exception.h>
#include <lib/os/msg.h>
#include <lib/image/image.h>
#include <ecs/EntityManager.h>
#include <ecs/BaseClass.h>

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

ModelRef* _attach_model(World* w, ecs::Entity* e, const Path& filename) {
	KABA_EXCEPTION_WRAPPER( return w->attach_model(e, filename); );
	return nullptr;
}

LegacyParticle* _world_add_legacy_particle(World* w, const kaba::Class* type, const vec3& pos, float radius, const color& c, shared<Texture>& tex, float ttl) {
	auto e = w->create_entity(pos, quaternion::ID);
	auto p = reinterpret_cast<LegacyParticle*>(ecs::EntityManager::global->_add_component_generic_(e, type));
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


Model* __load_model(const Path& filename) {
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

yrenderer::Material* __load_material(const Path& filename) {
	return engine.resource_manager->load_material(filename);
}

yrenderer::Material* make_material_unique(yrenderer::Material* m) {
	//if (engine.resource_manager->material_manager->is_from_file(m))
	if (!m->is_unique) {
		// TODO should be owned by MaterialManager
		auto mm = m->copy();
		mm->is_unique = true;
		return mm;
	}
	return m;
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

ecs::ComponentManager::List& __query_component_list(const kaba::Class* type) {
	return ecs::EntityManager::global->component_manager->_get_list(type);
}

ecs::ComponentManager::List& __query_component_list_family(const kaba::Class* type) {
	return ecs::EntityManager::global->component_manager->_get_list_family(type);
}

ecs::ComponentManager::PairList& __query_component_list2(const kaba::Class* type1, const kaba::Class* type2) {
	return ecs::EntityManager::global->component_manager->_get_list2(type1, type2);
}

class EntityWrapper : public ecs::Entity {
public:
	ecs::Component* add_component_generic(const kaba::Class* type, const string& vars) {
		if (vars != "")
			msg_error("TODO component params Any{}");
		return ecs::EntityManager::global->_add_component_generic_(this, type, {});
	}
	void delete_component(ecs::Component* c) {
		return ecs::EntityManager::global->delete_component(this, c);
	}
};
Light* attach_light_parallel(ecs::Entity* e, const color& c) {
	return world.attach_light_parallel(e, c);
}
Light* attach_light_point(ecs::Entity* e, const color& c, float r) {
	return world.attach_light_point(e, c, r);
}
Light* attach_light_cone(ecs::Entity* e, const color& c, float r, float theta) {
	return world.attach_light_cone(e, c, r, theta);
}

void export_ecs(kaba::Exporter* ext) {
	ecs::BaseClass entity;
	ext->declare_class_size("BaseClass", sizeof(ecs::BaseClass));
	//	ext->link_class_func("BaseClass.__init__", &ecs::Entity::__init__);
	ext->link_virtual("BaseClass.__delete__", &ecs::BaseClass::__delete__, &entity);
	ext->link_virtual("BaseClass.on_init", &ecs::BaseClass::on_init, &entity);
	ext->link_virtual("BaseClass.on_delete", &ecs::BaseClass::on_delete, &entity);
	ext->link_virtual("BaseClass.on_iterate", &ecs::BaseClass::on_iterate, &entity);

	ext->declare_class_size("Entity", sizeof(ecs::Entity));
	ext->declare_class_element("Entity.pos", &ecs::Entity::pos);
	ext->declare_class_element("Entity.ang", &ecs::Entity::ang);
	ext->declare_class_element("Entity.parent", &ecs::Entity::parent);
	ext->link_class_func("Entity.get_matrix", &ecs::Entity::get_matrix);
	ext->link_class_func("Entity.__get_component", &ecs::Entity::_get_component_generic_);
	ext->link_class_func("Entity.__get_component_derived", &ecs::Entity::_get_component_derived_generic_);
	ext->link_class_func("Entity.__add_component", &EntityWrapper::add_component_generic);
	ext->link_class_func("Entity.delete_component", &EntityWrapper::delete_component);
	ext->link_class_func("Entity.__del_override__", &DeletionQueue::add_entity);

	ecs::Component component;
	ext->declare_class_size("Component", sizeof(ecs::Component));
	ext->declare_class_element("Component.owner", &ecs::Component::owner);
	ext->link_class_func("Component.__init__", &kaba::generic_init<ecs::Component>);
	ext->link_virtual("Component.__delete__", &ecs::Component::__delete__, &component);
	ext->link_virtual("Component.on_init", &ecs::Component::on_init, &component);
	ext->link_virtual("Component.on_delete", &ecs::Component::on_delete, &component);
	ext->link_virtual("Component.on_iterate", &ecs::Component::on_iterate, &component);
	ext->link_virtual("Component.on_collide", &ecs::Component::on_collide, &component);
	ext->link_class_func("Component.set_variables", &ecs::Component::set_variables);

	ext->declare_class_size("NameTag", sizeof(NameTag));
	ext->declare_class_element("NameTag.name", &NameTag::name);

	ecs::System con;
	ext->declare_class_size("System", sizeof(ecs::System));
	ext->link_class_func("System.__init__", &kaba::generic_init<ecs::System>);
	ext->link_virtual("System.__delete__", &kaba::generic_virtual<ecs::System>::__delete__, &con);
	ext->link_virtual("System.on_init", &ecs::System::on_init, &con);
	ext->link_virtual("System.on_delete", &ecs::System::on_delete, &con);
	ext->link_virtual("System.on_finished_loading", &ecs::System::on_finished_loading, &con);
	ext->link_virtual("System.on_add_component", &ecs::System::on_add_component, &con);
	ext->link_virtual("System.on_remove_component", &ecs::System::on_remove_component, &con);
	ext->link_virtual("System.on_iterate", &ecs::System::on_iterate, &con);
	ext->link_virtual("System.on_iterate_pre", &ecs::System::on_iterate_pre, &con);
	ext->link_virtual("System.on_draw_pre", &ecs::System::on_draw_pre, &con);
	ext->link_virtual("System.on_input", &ecs::System::on_input, &con);
	ext->link_virtual("System.on_key", &ecs::System::on_key, &con);
	ext->link_virtual("System.on_key_down", &ecs::System::on_key_down, &con);
	ext->link_virtual("System.on_key_up", &ecs::System::on_key_up, &con);
	ext->link_virtual("System.on_left_button_down", &ecs::System::on_left_button_down, &con);
	ext->link_virtual("System.on_left_button_up", &ecs::System::on_left_button_up, &con);
	ext->link_virtual("System.on_middle_button_down", &ecs::System::on_middle_button_down, &con);
	ext->link_virtual("System.on_middle_button_up", &ecs::System::on_middle_button_up, &con);
	ext->link_virtual("System.on_right_button_down", &ecs::System::on_right_button_down, &con);
	ext->link_virtual("System.on_right_button_up", &ecs::System::on_right_button_up, &con);
	ext->link_virtual("System.on_render_inject", &ecs::System::on_render_inject, &con);
	ext->link_class_func("System.__del_override__", &DeletionQueue::add);

	ext->link_func("__get_component_list", &__query_component_list);
	ext->link_func("__get_component_family_list", &__query_component_list_family);
	ext->link_func("__get_component_list2", &__query_component_list2);

	ext->link_func("__get_system", &ecs::SystemManager::_get_generic);
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
	ext->declare_class_element("Model.materials", &Model::materials);
	ext->declare_class_element("Model.radius", (char*)&model.prop.radius - (char*)&model);
	ext->declare_class_element("Model.min", (char*)&model.prop.min - (char*)&model);
	ext->declare_class_element("Model.max", (char*)&model.prop.max- (char*)&model);
	ext->link_class_func("Model.__init__", &kaba::generic_init<Model>);
	ext->link_class_func("Model.__delete__", &kaba::generic_delete<Model>);
	ext->link_class_func("Model.make_editable", &Model::make_editable);
	ext->link_class_func("Model.begin_edit", &Model::begin_edit);
	ext->link_class_func("Model.end_edit", &Model::end_edit);
	ext->link_class_func("Model.get_vertex", &Model::get_vertex);
//	ext->link_class_func("Model.set_bone_model", &Model::set_bone_model);


	ext->declare_class_size("UserMesh", sizeof(UserMesh));
	ext->declare_class_element("UserMesh.vertex_buffer", &UserMesh::vertex_buffer);
	ext->declare_class_element("UserMesh.material", &UserMesh::material);
	ext->declare_class_element("UserMesh.vertex_shader_module", &UserMesh::vertex_shader_module);
	ext->declare_class_element("UserMesh.geometry_shader_module", &UserMesh::geometry_shader_module);
	ext->declare_class_element("UserMesh.tessellation_shader_module", &UserMesh::tessellation_shader_module);
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


	ext->declare_class_size("RigidBody", sizeof(RigidBody));
	ext->declare_class_element("RigidBody.vel", &RigidBody::vel);
	ext->declare_class_element("RigidBody.rot", &RigidBody::rot);
	ext->declare_class_element("RigidBody.mass", &RigidBody::mass);
	//ext->declare_class_element("RigidBody.theta", &RigidBody::theta_world);
	ext->declare_class_element("RigidBody.theta", &RigidBody::theta_0);
	ext->declare_class_element("RigidBody.g_factor", &RigidBody::g_factor);
	ext->declare_class_element("RigidBody.dynamic", &RigidBody::dynamic);
	ext->link_class_func("RigidBody.add_force", &RigidBody::add_force);
	ext->link_class_func("RigidBody.add_impulse", &RigidBody::add_impulse);
	ext->link_class_func("RigidBody.add_torque", &RigidBody::add_torque);
	ext->link_class_func("RigidBody.add_torque_impulse", &RigidBody::add_torque_impulse);
	ext->link_class_func("RigidBody.update_motion", &RigidBody::update_motion);
	ext->link_class_func("RigidBody.update_mass", &RigidBody::update_mass);

	ext->declare_class_size("Collider", sizeof(Collider));

	ext->declare_class_size("SphereCollider", sizeof(SphereCollider));
	ext->declare_class_element("SphereCollider.radius", &SphereCollider::radius);

	ext->declare_class_size("BoxCollider", sizeof(BoxCollider));
	ext->declare_class_element("BoxCollider.min", &BoxCollider::min);
	ext->declare_class_element("BoxCollider.max", &BoxCollider::max);

	ext->declare_class_size("MultiInstance", sizeof(MultiInstance));
	ext->declare_class_element("MultiInstance.model", &MultiInstance::model);
	ext->declare_class_element("MultiInstance.matrices", &MultiInstance::matrices);


	ext->declare_class_size("Terrain", sizeof(Terrain));
	//ext->declare_class_element("Terrain.pos", &Terrain::pos);
	//ext->declare_class_element("Terrain.material", &Terrain::material);
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
	ext->declare_class_element("ModelRef.model", &ModelRef::model);
	ext->declare_class_element("ModelRef.materials", &ModelRef::materials);
	ext->link_class_func("ModelRef.get_material", &ModelRef::get_material);
	ext->link_class_func("ModelRef.set_material", &ModelRef::set_material);

	ext->declare_class_size("TerrainRef", sizeof(TerrainRef));
	ext->declare_class_element("TerrainRef.terrain", &TerrainRef::terrain);
	ext->declare_class_element("TerrainRef.material", &TerrainRef::material);

	ext->declare_class_size("TemplateRef", sizeof(TemplateRef));
	ext->declare_class_element("TemplateRef.template", &TemplateRef::_template);


	ext->declare_class_size("Physics", sizeof(Physics));
	ext->declare_class_element("Physics.gravity", &Physics::gravity);
	ext->declare_class_element("Physics.mode", &Physics::mode);
	ext->declare_class_element("Physics.enabled", &Physics::enabled);
	ext->declare_class_element("Physics.collisions_enabled", &Physics::collisions_enabled);
	ext->link_class_func("Physics.set_dynamic", &Physics::set_dynamic);
	ext->link_class_func("Physics.get_g", &Physics::get_g);


	ext->declare_class_element("World.background", &World::background);
	ext->declare_class_element("World.skyboxes", &World::skybox);
	ext->declare_class_element("World.fog", &World::fog);
	ext->declare_class_element("World.msg_data", &World::msg_data);
	ext->link_class_func("World.ego", &World::ego);
	ext->link_class_func("World.entity", &World::get_entity);
	ext->link_class_func("World.load_soon", &World::load_soon);
	ext->link_class_func("World.load_template", &World::create_from_template);
	ext->link_class_func("World.create_object", &_create_object);
	ext->link_class_func("World.create_object_multi", &_create_object_multi);
	ext->link_class_func("World.create_terrain", &World::create_terrain);
	ext->link_class_func("World.create_entity", &World::create_entity);
	ext->link_class_func("World.create_light_parallel", &World::create_light_parallel);
	ext->link_class_func("World.create_light_point", &World::create_light_point);
	ext->link_class_func("World.create_light_cone", &World::create_light_cone);
	ext->link_class_func("World.create_camera", &World::create_camera);
	ext->link_class_func("World.emit_sound", &World::emit_sound);
	ext->link_class_func("World.emit_sound_file", &World::emit_sound_file);
	ext->link_class_func("World.emit_sound_stream", &World::emit_sound_stream);
	ext->link_class_func("World.attach_model", &_attach_model);
	ext->link_class_func("World._add_particle", &_world_add_legacy_particle);
	ext->link_class_func("World.shift_all", &World::shift_all);
	ext->link_class_func("World.trace", &World::trace);
	ext->link_class_func("World.delete_entity", &World::delete_entity);
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
	ext->declare_class_element("Link.type", &Link::link_type);
	ext->declare_class_element("Link.a", &Link::a);
	ext->declare_class_element("Link.b", &Link::b);
	ext->link_class_func("Link.set_motor", &Link::set_motor);
	ext->link_class_func("Link.set_frame", &Link::set_frame);
	//ext->link_class_func("Link.set_axis", &Link::set_axis);
	//ext->link_class_func("Link.__del_override__", &DeletionQueue::add);

	ext->link("world", &world);
	ext->link("cam", &cam_main);
	ext->link_func("load_model", &__load_model);
	ext->link_func("load_material", &__load_material);
	ext->link_func("make_material_unique", &make_material_unique);

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
		ext->link_class_func("LegacyParticle.__init__", &kaba::generic_init<LegacyParticle>);
		ext->link_virtual("LegacyParticle.__delete__", &kaba::generic_virtual<LegacyParticle>::__delete__, &particle);
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
	ext->link_class_func("ParticleGroup.__init__", &kaba::generic_init<ParticleGroup>);
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
	ext->link_class_func("ParticleEmitter.__init__", &kaba::generic_init<ParticleEmitter>);
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

void export_kaba_package_yengine(kaba::Exporter* ext) {
	ext->package_info("yengine", EngineData::CURRENT_API_VERSION);
	export_gfx(ext);
	export_ecs(ext);
	export_world(ext);
	export_fx(ext);
	export_ui(ext);
	export_sound(ext);
	export_net(ext);
	export_engine(ext);
	export_renderer(ext);
	_export_package_yrenderer_internal(ext);
	export_package_profiler(ext);
}

}
