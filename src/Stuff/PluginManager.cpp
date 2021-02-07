/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../lib/kaba/kaba.h"
#include "../Edward.h"
#include "../MultiView/MultiView.h"
#include "../MultiView/ActionController.h"
#include "../MultiView/Window.h"
#include "../Mode/Model/ModeModel.h"
#include "../Mode/Model/ModeModel.h"
#include "../Mode/World/ModeWorld.h"
#include "../Data/Model/DataModel.h"
#include "../Data/Model/ModelMesh.h"
#include "../Data/Model/ModelPolygon.h"
#include "../Data/Model/ModelSelection.h"
#include "../Data/Model/ModelMaterial.h"
#include "../Data/Model/Geometry/GeometryBall.h"
#include "../Data/Model/Geometry/GeometryCube.h"
#include "../Data/Model/Geometry/GeometryCylinder.h"
#include "../Data/Model/Geometry/GeometryPlane.h"
#include "../Data/Model/Geometry/GeometryPlatonic.h"
#include "../Data/Model/Geometry/GeometrySphere.h"
#include "../Data/Model/Geometry/GeometryTeapot.h"
#include "../Data/Model/Geometry/GeometryTorus.h"
#include "../Data/Model/Geometry/GeometryTorusKnot.h"
#include "../Data/Material/ShaderNode.h"
#include "../Data/Material/ShaderBuilderContext.h"


Path PluginManager::directory;



PluginManager::PluginManager(const Path &dir) {
	directory = dir;
	init();
}

PluginManager::~PluginManager() {
}

void PluginManager::execute(const Path &filename) {
	//kaba::config.directory = "";
	try {
		auto s = kaba::load(filename);
		typedef void func_t();
		func_t *f = (func_t*)s->match_function("main", "void", {});
		if (f)
			f();
	} catch(kaba::Exception &e) {
		ed->error_box(e.message());
	}

	//kaba::DeleteAllScripts(true, true);
}


hui::Window *GlobalMainWin = ed;

void PluginManager::init() {
	kaba::init();
	link_plugins();
	find_plugins();
}

void PluginManager::link_plugins() {

	GlobalMainWin = ed;

	kaba::link_external("edward", &GlobalMainWin);
	kaba::link_external("ed", &ed);
	kaba::link_external("model", &mode_model->data);
	kaba::link_external("world", &mode_world->data);

	kaba::declare_class_element("Edward.cur_mode", &Edward::cur_mode);

	kaba::declare_class_element("Mode.name", &ModeBase::name);
	kaba::declare_class_element("Mode.multi_view", &ModeBase::multi_view);

	kaba::declare_class_size("Data", sizeof(Data));
	kaba::declare_class_element("Data.filename", &Data::filename);
	kaba::declare_class_element("Data.file_time", &Data::file_time);
	kaba::declare_class_element("Data.binary_file_format", &Data::binary_file_format);
	kaba::declare_class_element("Data.type", &Data::type);
	kaba::link_external_class_func("Data.begin_action", &Data::begin_action_group);
	kaba::link_external_class_func("Data.end_action", &Data::end_action_group);

	kaba::declare_class_size("MultiView.SingleData", sizeof(MultiView::SingleData));
	kaba::declare_class_element("MultiView.SingleData.pos", &MultiView::SingleData::pos);
	kaba::declare_class_element("MultiView.SingleData.view_stage", &MultiView::SingleData::view_stage);
	kaba::declare_class_element("MultiView.SingleData.is_selected", &MultiView::SingleData::is_selected);
	kaba::declare_class_element("MultiView.SingleData.m_delta", &MultiView::SingleData::m_delta);
	kaba::declare_class_element("MultiView.SingleData.m_old", &MultiView::SingleData::m_old);
	kaba::declare_class_element("MultiView.SingleData.is_special", &MultiView::SingleData::is_special);


	kaba::declare_class_element("MultiView.action_con", &MultiView::MultiView::action_con);
	kaba::declare_class_element("MultiView.active_win", &MultiView::MultiView::active_win);
	kaba::declare_class_element("MultiView.mouse_win", &MultiView::MultiView::mouse_win);
	kaba::declare_class_element("MultiView.cam", &MultiView::MultiView::cam);
	kaba::link_external_class_func("MultiView.cam_move", &MultiView::MultiView::cam_move);
	kaba::link_external_class_func("MultiView.cam_rotate", &MultiView::MultiView::cam_rotate);
	kaba::link_external_class_func("MultiView.cam_zoom", &MultiView::MultiView::cam_zoom);

	kaba::declare_class_element("MultiView.Window.local_ang", &MultiView::Window::local_ang);

	kaba::link_external_class_func("MultiView.ActionController.start_action", &MultiView::ActionController::start_action);
	kaba::link_external_class_func("MultiView.ActionController.update_action", &MultiView::ActionController::update_action);
	kaba::link_external_class_func("MultiView.ActionController.update_param", &MultiView::ActionController::update_param);
	kaba::link_external_class_func("MultiView.ActionController.end_action", &MultiView::ActionController::end_action);
	kaba::declare_class_element("MultiView.ActionController.pos", &MultiView::ActionController::pos);

	// model

	kaba::declare_class_size("Model.Effect", sizeof(ModelEffect));
	kaba::declare_class_element("Model.Effect.kind", &ModelEffect::type);
	kaba::declare_class_element("Model.Effect.vertex", &ModelEffect::vertex);
	kaba::declare_class_element("Model.Effect.size", &ModelEffect::size);
	kaba::declare_class_element("Model.Effect.speed", &ModelEffect::speed);
	kaba::declare_class_element("Model.Effect.intensity", &ModelEffect::intensity);
	kaba::declare_class_element("Model.Effect.colors", &ModelEffect::colors);
	kaba::declare_class_element("Model.Effect.inv_quad", &ModelEffect::inv_quad);
	kaba::declare_class_element("Model.Effect.file", &ModelEffect::file);

	kaba::declare_class_size("Geometry.Vertex", sizeof(ModelVertex));
	kaba::declare_class_element("Geometry.Vertex.normal_mode", &ModelVertex::normal_mode);
	kaba::declare_class_element("Geometry.Vertex.bone_index", &ModelVertex::bone_index);
	kaba::declare_class_element("Geometry.Vertex.normal_dirty", &ModelVertex::normal_dirty);
	kaba::declare_class_element("Geometry.Vertex.ref_count", &ModelVertex::ref_count);

	kaba::declare_class_size("Geometry.Polygon.Side", sizeof(ModelPolygonSide));
	kaba::declare_class_element("Geometry.Polygon.Side.vertex", &ModelPolygonSide::vertex);
	kaba::declare_class_element("Geometry.Polygon.Side.edge", &ModelPolygonSide::edge);
	kaba::declare_class_element("Geometry.Polygon.Side.edge_direction", &ModelPolygonSide::edge_direction);
	kaba::declare_class_element("Geometry.Polygon.Side.skin_vertex", &ModelPolygonSide::skin_vertex);
	kaba::declare_class_element("Geometry.Polygon.Side.normal_index", &ModelPolygonSide::normal_index);
	kaba::declare_class_element("Geometry.Polygon.Side.normal", &ModelPolygonSide::normal);
	kaba::declare_class_element("Geometry.Polygon.Side.triangulation", &ModelPolygonSide::triangulation);

	kaba::declare_class_size("Geometry.Polygon", sizeof(ModelPolygon));
	kaba::declare_class_element("Geometry.Polygon.side", &ModelPolygon::side);
	kaba::declare_class_element("Geometry.Polygon.temp_normal", &ModelPolygon::temp_normal);
	kaba::declare_class_element("Geometry.Polygon.normal_dirty", &ModelPolygon::normal_dirty);
	kaba::declare_class_element("Geometry.Polygon.triangulation_dirty", &ModelPolygon::triangulation_dirty);
	kaba::declare_class_element("Geometry.Polygon.material", &ModelPolygon::material);

	/*kaba::declare_class_size("ModelSurface", sizeof(ModelSurface));
	kaba::declare_class_element("ModelSurface.polygon", &ModelSurface::polygon);
	kaba::declare_class_element("ModelSurface.edge", &ModelSurface::edge);
	kaba::declare_class_element("ModelSurface.vertex", &ModelSurface::vertex);
	kaba::declare_class_element("ModelSurface.is_closed", &ModelSurface::is_closed);
	kaba::declare_class_element("ModelSurface.is_physical", &ModelSurface::is_physical);
	kaba::declare_class_element("ModelSurface.is_visible", &ModelSurface::is_visible);
	kaba::declare_class_element("ModelSurface.model", &ModelSurface::model);
*/
	kaba::declare_class_size("Model.Bone", sizeof(ModelBone));
	kaba::declare_class_element("Model.Bone.parent", &ModelBone::parent);
	kaba::declare_class_element("Model.Bone.model_file", &ModelBone::model_file);
	kaba::declare_class_element("Model.Bone.model", &ModelBone::model);
	kaba::declare_class_element("Model.Bone.const_pos", &ModelBone::const_pos);
	kaba::declare_class_element("Model.Bone.matrix", &ModelBone::_matrix);

	kaba::declare_class_size("Model.Move", sizeof(ModelMove));
	kaba::declare_class_element("Model.Move.name", &ModelMove::name);
	kaba::declare_class_element("Model.Move.type", &ModelMove::type);
	kaba::declare_class_element("Model.Move.FramesPerSecConst", &ModelMove::frames_per_sec_const);
	kaba::declare_class_element("Model.Move.FramesPerSecFactor", &ModelMove::frames_per_sec_factor);
	kaba::declare_class_element("Model.Move.frame", &ModelMove::frame);

	kaba::declare_class_size("Model.Move.Frame", sizeof(ModelFrame));
	kaba::declare_class_element("Model.Move.Frame.skel_ang", &ModelFrame::skel_ang);
	kaba::declare_class_element("Model.Move.Frame.skel_dpos", &ModelFrame::skel_dpos);
	kaba::declare_class_element("Model.Move.Frame.dpos", &ModelFrame::skin);
	kaba::declare_class_element("Model.Move.Frame.vertex_dpos", &ModelFrame::vertex_dpos);


	kaba::declare_class_size("Model.Mesh", sizeof(ModelMesh));
	kaba::declare_class_element("Model.Mesh.vertex", &ModelMesh::vertex);
	kaba::declare_class_element("Model.Mesh.polygon", &ModelMesh::polygon);
	kaba::declare_class_element("Model.Mesh.edge", &ModelMesh::edge);
	kaba::declare_class_element("Model.Mesh.ball", &ModelMesh::ball);
	kaba::declare_class_element("Model.Mesh.cylinder", &ModelMesh::cylinder);

	kaba::declare_class_size("Model", sizeof(DataModel));
	kaba::declare_class_element("Model.bone", &DataModel::bone);
	kaba::declare_class_element("Model.move", &DataModel::move);
	//kaba::declare_class_element("Model.move", &DataModel::move);
	//kaba::declare_class_element("Model.CurrentMove", &DataModel::CurrentMove);
	//kaba::declare_class_element("Model.CurrentFrame", &DataModel::CurrentFrame);
	kaba::declare_class_element("Model.mesh", &DataModel::mesh);
	kaba::declare_class_element("Model.phys_mesh", &DataModel::phys_mesh);
	kaba::declare_class_element("Model.material", &DataModel::material);
	kaba::declare_class_element("Model.fx", &DataModel::fx);
	kaba::declare_class_element("Model.meta_data", &DataModel::meta_data);
	//kaba::declare_class_element("Model.CurrentMaterial", &DataModel::CurrentMaterial);
	//kaba::declare_class_element("Model.CurrentTextureLevel", &DataModel::CurrentTextureLevel);
	//kaba::declare_class_element("Model.SkinVertex", &DataModel::SkinVertex);
	//kaba::declare_class_element("Model.SkinVertMat", &DataModel::SkinVertMat);
	/*kaba::declare_class_element("Model.SkinVertTL", &DataModel::SkinVertTL);
	kaba::declare_class_element("Model.Playing", &DataModel::Playing);
	kaba::declare_class_element("Model.PlayLoop", &DataModel::PlayLoop);
	kaba::declare_class_element("Model.TimeScale", &DataModel::TimeScale);
	kaba::declare_class_element("Model.TimeParam", &DataModel::TimeParam);
	kaba::declare_class_element("Model.SimFrame", &DataModel::SimFrame);*/
	kaba::link_external_class_func("Model.clear_selection", &DataModel::clearSelection);
	kaba::link_external_class_func("Model.invert_polygons", &DataModel::invert_polygons);
	kaba::link_external_class_func("Model.selection_from_vertices", &DataModel::selectionFromVertices);
	kaba::link_external_class_func("Model.selection_from_polygons", &DataModel::selectionFromPolygons);
	kaba::link_external_class_func("Model.add_vertex", &DataModel::addVertex);
	kaba::link_external_class_func("Model.add_triangle", &DataModel::addTriangle);
	kaba::link_external_class_func("Model.add_polygon", &DataModel::addPolygon);
	kaba::link_external_class_func("Model.delete_selection", &DataModel::delete_selection);
	kaba::link_external_class_func("Model.delete_polygon", &DataModel::delete_polygon);
	kaba::link_external_class_func("Model.subtract_selection", &DataModel::subtractSelection);
	kaba::link_external_class_func("Model.convert_selection_to_triangles", &DataModel::convertSelectionToTriangles);
	kaba::link_external_class_func("Model.triangulate_selected_vertices", &DataModel::triangulateSelectedVertices);
	kaba::link_external_class_func("Model.bevel_selected_vertices", &DataModel::bevelSelectedEdges);
	kaba::link_external_class_func("Model.extrude_selected_polygons", &DataModel::extrudeSelectedPolygons);
	kaba::link_external_class_func("Model.paste_geometry", &DataModel::pasteGeometry);
	kaba::link_external_class_func("Model.add_animation", &DataModel::addAnimation);
	kaba::link_external_class_func("Model.duplicate_animation", &DataModel::duplicateAnimation);
	kaba::link_external_class_func("Model.delete_animation", &DataModel::deleteAnimation);
	kaba::link_external_class_func("Model.animation_set_data", &DataModel::setAnimationData);
	kaba::link_external_class_func("Model.animation_add_frame", &DataModel::animationAddFrame);
	kaba::link_external_class_func("Model.animation_delete_frame", &DataModel::animationDeleteFrame);
	kaba::link_external_class_func("Model.animation_set_frame_duration", &DataModel::animationSetFrameDuration);
	kaba::link_external_class_func("Model.animation_set_bone", &DataModel::animationSetBone);


	kaba::declare_class_size("Geometry", sizeof(Geometry));
	kaba::link_external_class_func("Geometry.Ball.__init__", &GeometryBall::__init__);
	kaba::link_external_class_func("Geometry.Cube.__init__", &GeometryCube::__init__);
	kaba::link_external_class_func("Geometry.Sphere.__init__", &GeometrySphere::__init__);
	kaba::link_external_class_func("Geometry.Cylinder.__init__", &GeometryCylinder::__init__);
	kaba::link_external_class_func("Geometry.CylinderComplex.__init__", &GeometryCylinder::__init2__);
	kaba::link_external_class_func("Geometry.Torus.__init__", &GeometryTorus::__init__);
	kaba::link_external_class_func("Geometry.TorusKnot.__init__", &GeometryTorusKnot::__init__);
	kaba::link_external_class_func("Geometry.Plane.__init__", &GeometryPlane::__init__);
	kaba::link_external_class_func("Geometry.Platonic.__init__", &GeometryPlatonic::__init__);
	kaba::link_external_class_func("Geometry.Teapot.__init__", &GeometryTeapot::__init__);
	kaba::link_external("Geometry.subtract", (void*)&GeometrySubtract);
	kaba::link_external("Geometry.and", (void*)&GeometryAnd);

	// world

	kaba::declare_class_size("World", sizeof(DataWorld));
	kaba::declare_class_element("World.terrains", &DataWorld::terrains);
	kaba::declare_class_element("World.objects", &DataWorld::objects);
	kaba::declare_class_element("World.ego_index", &DataWorld::EgoIndex);
	kaba::link_external_class_func("World.add_object", &DataWorld::add_object);
	kaba::link_external_class_func("World.add_terrain", &DataWorld::add_terrain);
	kaba::link_external_class_func("World.add_new_terrain", &DataWorld::add_new_terrain);


	ShaderNode node("");
	kaba::declare_class_size("shader.Node", sizeof(ShaderNode));
	kaba::declare_class_element("shader.Node.x", &ShaderNode::x);
	kaba::declare_class_element("shader.Node.y", &ShaderNode::y);
	kaba::declare_class_element("shader.Node.type", &ShaderNode::type);
	kaba::declare_class_element("shader.Node.output", &ShaderNode::output);
	kaba::declare_class_element("shader.Node.params", &ShaderNode::params);
	kaba::link_external_class_func("shader.Node.__init__", &ShaderNode::__init__);
	kaba::link_external_virtual("shader.Node.__delete__", &ShaderNode::__delete__, &node);
	kaba::link_external_virtual("shader.Node.code_pixel", &ShaderNode::code_pixel, &node);
	kaba::link_external_virtual("shader.Node.dependencies", &ShaderNode::dependencies, &node);
	kaba::link_external_virtual("shader.Node.uniform_dependencies", &ShaderNode::uniform_dependencies, &node);
	kaba::declare_class_size("shader.Node.Parameter", sizeof(ShaderNode::Parameter));
	kaba::declare_class_element("shader.Node.Parameter.type", &ShaderNode::Parameter::type);
	kaba::declare_class_element("shader.Node.Parameter.name", &ShaderNode::Parameter::name);
	kaba::declare_class_element("shader.Node.Parameter.value", &ShaderNode::Parameter::value);
	kaba::declare_class_element("shader.Node.Parameter.options", &ShaderNode::Parameter::options);
	kaba::declare_class_size("shader.Node.Port", sizeof(ShaderNode::Port));
	kaba::declare_class_element("shader.Node.Port.type", &ShaderNode::Port::type);
	kaba::declare_class_element("shader.Node.Port.name", &ShaderNode::Port::name);

	kaba::declare_class_size("shader.BuilderContext", sizeof(ShaderBuilderContext));
	kaba::link_external_class_func("shader.BuilderContext.build_value", &ShaderBuilderContext::build_value);
	kaba::link_external_class_func("shader.BuilderContext.build_const", &ShaderBuilderContext::build_const);
	kaba::link_external_class_func("shader.BuilderContext.find_temp", &ShaderBuilderContext::find_temp);
	kaba::link_external_class_func("shader.BuilderContext.create_temp", &ShaderBuilderContext::create_temp);
	kaba::link_external_class_func("shader.BuilderContext.create_out", &ShaderBuilderContext::create_out);
}

void PluginManager::find_plugins() {
	Path dir0 = (PluginManager::directory << "Shader Graph").absolute();
	auto list = dir_search(dir0, "*", "fd");
	for (auto &e: list) {
		Path dir = dir0 << e;
		if (file_is_directory(dir)) {
			auto list2 = dir_search(dir, "*.kaba", "f");
			for (auto &e2: list2) {
				Plugin p;
				p.filename = dir << e2;
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
	auto s = kaba::load(filename);
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
