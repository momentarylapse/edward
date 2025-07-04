/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"
#include <lib/kaba/kaba.h>
#include <lib/os/filesystem.h>
#include <view/EdwardWindow.h>
#include <Session.h>
#include <view/Mode.h>
#include <view/MultiView.h>
#include <view/ActionController.h>
#include <view/MultiViewWindow.h>
#include <data/Data.h>
#include <data/mesh/PolygonMesh.h>
#include <data/mesh/GeometryBall.h>
#include <data/mesh/GeometryCube.h>
#include <data/mesh/GeometryCylinder.h>
#include <data/mesh/GeometryPlane.h>
#include <data/mesh/GeometryPlatonic.h>
#include <data/mesh/GeometrySphere.h>
#include <data/mesh/GeometryTeapot.h>
#include <data/mesh/GeometryTorus.h>
#include <data/mesh/GeometryTorusKnot.h>

#include <mode_material/data/DataMaterial.h>
#include <mode_material/ModeMaterial.h>
#include <mode_model/data/DataModel.h>
#include <mode_model/ModeModel.h>
#include <mode_model/mesh/processing/MeshRandomSurfacePoints.h>
#include <mode_world/data/DataWorld.h>
#include <mode_world/ModeWorld.h>

/*#include "../mode/world/ModeWorld.h"
#include "../data/model/DataModel.h"
#include "../data/model/ModelMesh.h"
#include "../data/model/ModelPolygon.h"
#include "../data/model/ModelSelection.h"
#include "../data/model/ModelMaterial.h"*/

/*#include "../data/material/ShaderNode.h"
#include "../data/material/ShaderBuilderContext.h"*/


Path PluginManager::directory;


Session* cur_session = nullptr;


PluginManager::PluginManager(const Path &dir) {
	directory = dir;
	init_edward();
}

PluginManager::~PluginManager() = default;

void PluginManager::execute(Session* session, const Path& filename) {
	cur_session = session;
	//kaba::config.directory = "";
	try {
		auto s = kaba::default_context->load_module(filename);
		typedef void func_t();
		if (auto f = (func_t*)s->match_function("main", "void", {}))
			f();
	} catch(kaba::Exception& e) {
		session->error(e.message());
	}

	//kaba::DeleteAllScripts(true, true);
}


//hui::Window *GlobalMainWin = ed;

void PluginManager::init_edward() {
	kaba::init();
	link_plugins();
	find_plugins();
}

void link_mesh(kaba::ExternalLinkData* ext) {
	ext->declare_class_size("Mesh.Vertex", sizeof(MeshVertex));
	ext->declare_class_element("Mesh.Vertex.normal_mode", &MeshVertex::normal_mode);
	ext->declare_class_element("Mesh.Vertex.bone_index", &MeshVertex::bone_index);
	ext->declare_class_element("Mesh.Vertex.normal_dirty", &MeshVertex::normal_dirty);
	ext->declare_class_element("Mesh.Vertex.ref_count", &MeshVertex::ref_count);

	ext->declare_class_size("Mesh.Polygon.Side", sizeof(PolygonSide));
	ext->declare_class_element("Mesh.Polygon.Side.vertex", &PolygonSide::vertex);
	//	ext->declare_class_element("Mesh.Polygon.Side.edge", &PolygonSide::edge);
	//	ext->declare_class_element("Mesh.Polygon.Side.edge_direction", &ModelPolygonSide::edge_direction);
	ext->declare_class_element("Mesh.Polygon.Side.skin_vertex", &PolygonSide::skin_vertex);
	ext->declare_class_element("Mesh.Polygon.Side.normal_index", &PolygonSide::normal_index);
	ext->declare_class_element("Mesh.Polygon.Side.normal", &PolygonSide::normal);
	ext->declare_class_element("Mesh.Polygon.Side.triangulation", &PolygonSide::triangulation);

	ext->declare_class_size("Mesh.Polygon", sizeof(Polygon));
	ext->declare_class_element("Mesh.Polygon.side", &Polygon::side);
	ext->declare_class_element("Mesh.Polygon.temp_normal", &Polygon::temp_normal);
	ext->declare_class_element("Mesh.Polygon.normal_dirty", &Polygon::normal_dirty);
	ext->declare_class_element("Mesh.Polygon.triangulation_dirty", &Polygon::triangulation_dirty);
	ext->declare_class_element("Mesh.Polygon.material", &Polygon::material);

	ext->declare_class_size("Mesh", sizeof(PolygonMesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<PolygonMesh>);
	ext->link_class_func("Mesh.__delete__", &kaba::generic_delete<PolygonMesh>);
	ext->link_class_func("Mesh.__assign__", &kaba::generic_assign<PolygonMesh>);
	ext->link_class_func("Mesh.add", &PolygonMesh::add);
	ext->link_class_func("Mesh.transform", &PolygonMesh::transform);
	ext->link_class_func("Mesh.invert", &PolygonMesh::invert);
	ext->declare_class_element("Mesh.vertices", &PolygonMesh::vertices);
	ext->declare_class_element("Mesh.polygons", &PolygonMesh::polygons);
	//	ext->declare_class_element("Mesh.edges", &PolygonMesh::edges);
	ext->declare_class_element("Mesh.spheres", &PolygonMesh::spheres);
	ext->declare_class_element("Mesh.cylinders", &PolygonMesh::cylinders);

	ext->link("Mesh.create_ball", (void*)&GeometryBall::create);
	ext->link("Mesh.create_cube", (void*)&GeometryCube::create);
	ext->link("Mesh.create_sphere", (void*)&GeometrySphere::create);
	//ext->link_class_func("Geometry.Cylinder.__init__", &GeometryCylinder::__init__);
	//ext->link_class_func("Geometry.CylinderComplex.__init__", &GeometryCylinder::__init2__);
	ext->link("Mesh.create_torus", (void*)&GeometryTorus::create);
	ext->link("Mesh.create_torus_knot", (void*)&GeometryTorusKnot::create);
	ext->link("Mesh.create_plane", (void*)&GeometryPlane::create);
	ext->link("Mesh.create_platonic", (void*)&GeometryPlatonic::create);
	ext->link("Mesh.create_teapot", (void*)&GeometryTeapot::create);
//	ext->link("Geometry.subtract", (void*)&GeometrySubtract);
//	ext->link("Geometry.and", (void*)&GeometryAnd);

	ext->link("mesh_surface_points", (void*)&mesh_surface_points);
}

void link_material(kaba::ExternalLinkData* ext) {
	ext->declare_class_size("ModeMaterial", sizeof(ModeMaterial));
	ext->declare_class_element("ModeMaterial.data", &ModeMaterial::data);
}

void link_model(kaba::ExternalLinkData* ext) {
	ext->declare_class_size("ModeModel", sizeof(ModeModel));
	ext->declare_class_element("ModeModel.data", &ModeModel::data);


	ext->declare_class_size("DataModel.Effect", sizeof(ModelEffect));
	ext->declare_class_element("DataModel.Effect.kind", &ModelEffect::type);
	ext->declare_class_element("DataModel.Effect.vertex", &ModelEffect::vertex);
	ext->declare_class_element("DataModel.Effect.size", &ModelEffect::size);
	ext->declare_class_element("DataModel.Effect.speed", &ModelEffect::speed);
	ext->declare_class_element("DataModel.Effect.intensity", &ModelEffect::intensity);
	ext->declare_class_element("DataModel.Effect.colors", &ModelEffect::colors);
	ext->declare_class_element("DataModel.Effect.inv_quad", &ModelEffect::inv_quad);
	ext->declare_class_element("DataModel.Effect.file", &ModelEffect::file);

	/*ext->declare_class_size("ModelSurface", sizeof(ModelSurface));
	ext->declare_class_element("ModelSurface.polygon", &ModelSurface::polygon);
	ext->declare_class_element("ModelSurface.edge", &ModelSurface::edge);
	ext->declare_class_element("ModelSurface.vertex", &ModelSurface::vertex);
	ext->declare_class_element("ModelSurface.is_closed", &ModelSurface::is_closed);
	ext->declare_class_element("ModelSurface.is_physical", &ModelSurface::is_physical);
	ext->declare_class_element("ModelSurface.is_visible", &ModelSurface::is_visible);
	ext->declare_class_element("ModelSurface.model", &ModelSurface::model);
*/
	ext->declare_class_size("DataModel.Bone", sizeof(ModelBone));
	ext->declare_class_element("DataModel.Bone.parent", &ModelBone::parent);
	ext->declare_class_element("DataModel.Bone.model_file", &ModelBone::model_file);
	ext->declare_class_element("DataModel.Bone.model", &ModelBone::model);
	ext->declare_class_element("DataModel.Bone.const_pos", &ModelBone::const_pos);
	ext->declare_class_element("DataModel.Bone.matrix", &ModelBone::_matrix);

	ext->declare_class_size("DataModel.Move", sizeof(ModelMove));
	ext->declare_class_element("DataModel.Move.name", &ModelMove::name);
	ext->declare_class_element("DataModel.Move.type", &ModelMove::type);
	ext->declare_class_element("DataModel.Move.FramesPerSecConst", &ModelMove::frames_per_sec_const);
	ext->declare_class_element("DataModel.Move.FramesPerSecFactor", &ModelMove::frames_per_sec_factor);
	ext->declare_class_element("DataModel.Move.frames", &ModelMove::frames);

	ext->declare_class_size("DataModel.Move.Frame", sizeof(ModelFrame));
	ext->declare_class_element("DataModel.Move.Frame.skel_ang", &ModelFrame::skel_ang);
	ext->declare_class_element("DataModel.Move.Frame.skel_dpos", &ModelFrame::skel_dpos);
	ext->declare_class_element("DataModel.Move.Frame.dpos", &ModelFrame::skin);
	ext->declare_class_element("DataModel.Move.Frame.vertex_dpos", &ModelFrame::vertex_dpos);


	ext->declare_class_size("DataModel", sizeof(DataModel));
	ext->declare_class_element("DataModel.bones", &DataModel::bones);
	ext->declare_class_element("DataModel.moves", &DataModel::moves);
	//ext->declare_class_element("DataModel.move", &DataModel::move);
	//ext->declare_class_element("DataModel.CurrentMove", &DataModel::CurrentMove);
	//ext->declare_class_element("DataModel.CurrentFrame", &DataModel::CurrentFrame);
	ext->declare_class_element("DataModel.mesh", &DataModel::mesh);
	ext->declare_class_element("DataModel.phys_mesh", &DataModel::phys_mesh);
	ext->declare_class_element("DataModel.materials", &DataModel::materials);
	ext->declare_class_element("DataModel.fx", &DataModel::fx);
	ext->declare_class_element("DataModel.meta_data", &DataModel::meta_data);
/*	ext->link_class_func("DataModel.clear_selection", &DataModel::clearSelection);
	ext->link_class_func("DataModel.invert_polygons", &DataModel::invert_polygons);
	ext->link_class_func("DataModel.selection_from_vertices", &DataModel::selectionFromVertices);
	ext->link_class_func("DataModel.selection_from_polygons", &DataModel::selectionFromPolygons);*/
	ext->link_class_func("DataModel.add_vertex", &DataModel::add_vertex);
	/*ext->link_class_func("DataModel.add_triangle", &DataModel::addTriangle);
	ext->link_class_func("DataModel.add_polygon", &DataModel::addPolygon);
	ext->link_class_func("DataModel.delete_selection", &DataModel::delete_selection);
	ext->link_class_func("DataModel.delete_polygon", &DataModel::delete_polygon);
	ext->link_class_func("DataModel.subtract_selection", &DataModel::subtractSelection);
	ext->link_class_func("DataModel.convert_selection_to_triangles", &DataModel::convertSelectionToTriangles);
	ext->link_class_func("DataModel.triangulate_selected_vertices", &DataModel::triangulateSelectedVertices);
	ext->link_class_func("DataModel.bevel_selected_vertices", &DataModel::bevelSelectedEdges);
	ext->link_class_func("DataModel.extrude_selected_polygons", &DataModel::extrudeSelectedPolygons);*/
	ext->link_class_func("DataModel.paste_mesh", &DataModel::paste_mesh);
	/*ext->link_class_func("DataModel.add_animation", &DataModel::addAnimation);
	ext->link_class_func("DataModel.duplicate_animation", &DataModel::duplicateAnimation);
	ext->link_class_func("DataModel.delete_animation", &DataModel::deleteAnimation);
	ext->link_class_func("DataModel.animation_set_data", &DataModel::setAnimationData);
	ext->link_class_func("DataModel.animation_add_frame", &DataModel::animationAddFrame);
	ext->link_class_func("DataModel.animation_delete_frame", &DataModel::animationDeleteFrame);
	ext->link_class_func("DataModel.animation_set_frame_duration", &DataModel::animationSetFrameDuration);
	ext->link_class_func("DataModel.animation_set_bone", &DataModel::animationSetBone);*/

}

void link_world(kaba::ExternalLinkData* ext) {
	ext->declare_class_size("ModeWorld", sizeof(ModeWorld));
	ext->declare_class_element("ModeWorld.data", &ModeWorld::data);
}

void PluginManager::link_plugins() {

	//GlobalMainWin = ed;

	auto ext = kaba::default_context->external.get();

	link_mesh(ext);
	link_model(ext);
	link_world(ext);

	ext->link("cur_session", &cur_session);

	ext->declare_class_element("Edward.cur_mode", &Session::cur_mode);

//	ext->declare_class_element("Mode.name", &ModeBase::name);
//	ext->declare_class_element("Mode.multi_view", &ModeBase::multi_view);

	ext->declare_class_size("Data", sizeof(Data));
	ext->declare_class_element("Data.filename", &Data::filename);
	ext->declare_class_element("Data.file_time", &Data::file_time);
	ext->declare_class_element("Data.binary_file_format", &Data::binary_file_format);
	ext->declare_class_element("Data.type", &Data::type);
	ext->link_class_func("Data.begin_action", &Data::begin_action_group);
	ext->link_class_func("Data.end_action", &Data::end_action_group);

	ext->declare_class_size("MultiView.SingleData", sizeof(multiview::SingleData));
	ext->declare_class_element("MultiView.SingleData.pos", &multiview::SingleData::pos);
	ext->declare_class_element("MultiView.SingleData.view_stage", &multiview::SingleData::view_stage);
	ext->declare_class_element("MultiView.SingleData.is_special", &multiview::SingleData::is_special);

	/*ext->declare_class_element("MultiView.action_con", &MultiView::MultiView::action_con);
	ext->declare_class_element("MultiView.active_win", &MultiView::MultiView::active_win);
	ext->declare_class_element("MultiView.mouse_win", &MultiView::MultiView::mouse_win);
	ext->declare_class_element("MultiView.cam", &MultiView::MultiView::cam);
	ext->link_class_func("MultiView.cam_move", &MultiView::MultiView::cam_move);
	ext->link_class_func("MultiView.cam_rotate", &MultiView::MultiView::cam_rotate);
	ext->link_class_func("MultiView.cam_zoom", &MultiView::MultiView::cam_zoom);

	ext->declare_class_element("MultiView.Window.local_ang", &MultiView::Window::local_ang);

	ext->link_class_func("MultiView.ActionController.start_action", &MultiView::ActionController::start_action);
	ext->link_class_func("MultiView.ActionController.update_action", &MultiView::ActionController::update_action);
	ext->link_class_func("MultiView.ActionController.update_param", &MultiView::ActionController::update_param);
	ext->link_class_func("MultiView.ActionController.end_action", &MultiView::ActionController::end_action);
	ext->declare_class_element("MultiView.ActionController.pos", &MultiView::ActionController::pos);*/

#if 0
	// world
	ext->declare_class_size("World", sizeof(DataWorld));
	ext->declare_class_element("World.terrains", &DataWorld::terrains);
	ext->declare_class_element("World.objects", &DataWorld::objects);
	ext->declare_class_element("World.ego_index", &DataWorld::EgoIndex);
	ext->link_class_func("World.add_object", &DataWorld::add_object);
	ext->link_class_func("World.add_terrain", &DataWorld::add_terrain);
	ext->link_class_func("World.add_new_terrain", &DataWorld::add_new_terrain);
#endif

	ext->declare_class_size("Session", sizeof(Session));
	ext->declare_class_element("Session.mode_model", &Session::mode_model);
	ext->declare_class_element("Session.mode_world", &Session::mode_world);
	ext->declare_class_element("Session.mode_material", &Session::mode_material);
	ext->declare_class_element("Session.win", &Session::win);

#if 0
	ShaderNode node("");
	ext->declare_class_size("shader.Node", sizeof(ShaderNode));
	ext->declare_class_element("shader.Node.pos", &ShaderNode::pos);
	ext->declare_class_element("shader.Node.type", &ShaderNode::type);
	ext->declare_class_element("shader.Node.output", &ShaderNode::output);
	ext->declare_class_element("shader.Node.params", &ShaderNode::params);
	ext->link_class_func("shader.Node.__init__", &ShaderNode::__init__);
	ext->link_virtual("shader.Node.__delete__", &ShaderNode::__delete__, &node);
	ext->link_virtual("shader.Node.code_pixel", &ShaderNode::code_pixel, &node);
	ext->link_virtual("shader.Node.code_pixel_pre", &ShaderNode::code_pixel_pre, &node);
	ext->link_virtual("shader.Node.dependencies", &ShaderNode::dependencies, &node);
	ext->link_virtual("shader.Node.uniform_dependencies", &ShaderNode::uniform_dependencies, &node);
	ext->declare_class_size("shader.Node.Parameter", sizeof(ShaderNode::Parameter));
	ext->declare_class_element("shader.Node.Parameter.type", &ShaderNode::Parameter::type);
	ext->declare_class_element("shader.Node.Parameter.name", &ShaderNode::Parameter::name);
	ext->declare_class_element("shader.Node.Parameter.value", &ShaderNode::Parameter::value);
	ext->declare_class_element("shader.Node.Parameter.options", &ShaderNode::Parameter::options);
	ext->declare_class_size("shader.Node.Port", sizeof(ShaderNode::Port));
	ext->declare_class_element("shader.Node.Port.type", &ShaderNode::Port::type);
	ext->declare_class_element("shader.Node.Port.name", &ShaderNode::Port::name);

	ext->declare_class_size("shader.BuilderContext", sizeof(ShaderBuilderContext));
	ext->link_class_func("shader.BuilderContext.build_value", &ShaderBuilderContext::build_value);
	ext->link_class_func("shader.BuilderContext.build_const", &ShaderBuilderContext::build_const);
	ext->link_class_func("shader.BuilderContext.find_temp", &ShaderBuilderContext::find_temp);
	ext->link_class_func("shader.BuilderContext.create_temp", &ShaderBuilderContext::create_temp);
	ext->link_class_func("shader.BuilderContext.create_out", &ShaderBuilderContext::create_out);
#endif
}

void PluginManager::find_plugins() {
	Path dir0 = (PluginManager::directory | "shader graph").absolute();
	auto list = os::fs::search(dir0, "*", "fd");
	for (auto &e: list) {
		Path dir = dir0 | e;
		if (os::fs::is_directory(dir)) {
			auto list2 = os::fs::search(dir, "*.kaba", "f");
			for (auto &e2: list2) {
				Plugin p;
				p.filename = dir | e2;
				p.name = e2.str().replace(".kaba", "");
				p.category = e.str();
				p.type = PluginType::SHADER_NODE;
				plugins.add(p);
			}
		}
	}
}

void *PluginManager::create_instance(const Path &filename, const string &parent) {
	//kaba::config.directory = "";
	auto s = kaba::default_context->load_module(filename);
	for (auto c: s->classes()){
		if (c->is_derived_from_s(parent)) {
			return c->create_instance();
		}
	}
	throw Exception(format("no class derived from '%s' defined", parent));
	return nullptr;
}

void *PluginManager::Plugin::create_instance(const string &parent) const {
	return PluginManager::create_instance(filename, parent);
}
