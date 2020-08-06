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
	Kaba::config.directory = "";
	try {
		auto *s = Kaba::Load(filename);
		typedef void func_t();
		func_t *f = (func_t*)s->match_function("main", "void", {});
		if (f)
			f();
	} catch(Kaba::Exception &e) {
		ed->error_box(e.message());
	}

	//Kaba::DeleteAllScripts(true, true);
}


hui::Window *GlobalMainWin = ed;

void PluginManager::init() {
	Kaba::init();
	link_plugins();
	find_plugins();
}

void PluginManager::link_plugins() {

	GlobalMainWin = ed;

	Kaba::link_external("edward", &GlobalMainWin);
	Kaba::link_external("ed", &ed);
	Kaba::link_external("model", &mode_model->data);
	Kaba::link_external("world", &mode_world->data);

	Kaba::declare_class_element("Edward.cur_mode", &Edward::cur_mode);

	Kaba::declare_class_element("Mode.name", &ModeBase::name);
	Kaba::declare_class_element("Mode.multi_view", &ModeBase::multi_view);

	Kaba::declare_class_size("Data", sizeof(Data));
	Kaba::declare_class_element("Data.filename", &Data::filename);
	Kaba::declare_class_element("Data.file_time", &Data::file_time);
	Kaba::declare_class_element("Data.binary_file_format", &Data::binary_file_format);
	Kaba::declare_class_element("Data.type", &Data::type);
	Kaba::link_external_class_func("Data.begin_action", &Data::begin_action_group);
	Kaba::link_external_class_func("Data.end_action", &Data::end_action_group);

	Kaba::declare_class_size("MultiView.SingleData", sizeof(MultiView::SingleData));
	Kaba::declare_class_element("MultiView.SingleData.pos", &MultiView::SingleData::pos);
	Kaba::declare_class_element("MultiView.SingleData.view_stage", &MultiView::SingleData::view_stage);
	Kaba::declare_class_element("MultiView.SingleData.is_selected", &MultiView::SingleData::is_selected);
	Kaba::declare_class_element("MultiView.SingleData.m_delta", &MultiView::SingleData::m_delta);
	Kaba::declare_class_element("MultiView.SingleData.m_old", &MultiView::SingleData::m_old);
	Kaba::declare_class_element("MultiView.SingleData.is_special", &MultiView::SingleData::is_special);


	Kaba::declare_class_element("MultiView.action_con", &MultiView::MultiView::action_con);
	Kaba::declare_class_element("MultiView.active_win", &MultiView::MultiView::active_win);
	Kaba::declare_class_element("MultiView.mouse_win", &MultiView::MultiView::mouse_win);
	Kaba::declare_class_element("MultiView.cam", &MultiView::MultiView::cam);
	Kaba::link_external_class_func("MultiView.cam_move", &MultiView::MultiView::cam_move);
	Kaba::link_external_class_func("MultiView.cam_rotate", &MultiView::MultiView::cam_rotate);
	Kaba::link_external_class_func("MultiView.cam_zoom", &MultiView::MultiView::cam_zoom);

	Kaba::declare_class_element("MultiView.Window.local_ang", &MultiView::Window::local_ang);

	Kaba::link_external_class_func("MultiView.ActionController.start_action", &MultiView::ActionController::start_action);
	Kaba::link_external_class_func("MultiView.ActionController.update_action", &MultiView::ActionController::update_action);
	Kaba::link_external_class_func("MultiView.ActionController.update_param", &MultiView::ActionController::update_param);
	Kaba::link_external_class_func("MultiView.ActionController.end_action", &MultiView::ActionController::end_action);
	Kaba::declare_class_element("MultiView.ActionController.pos", &MultiView::ActionController::pos);

	// model

	Kaba::declare_class_size("Model.Effect", sizeof(ModelEffect));
	Kaba::declare_class_element("Model.Effect.kind", &ModelEffect::type);
	Kaba::declare_class_element("Model.Effect.vertex", &ModelEffect::vertex);
	Kaba::declare_class_element("Model.Effect.size", &ModelEffect::size);
	Kaba::declare_class_element("Model.Effect.speed", &ModelEffect::speed);
	Kaba::declare_class_element("Model.Effect.intensity", &ModelEffect::intensity);
	Kaba::declare_class_element("Model.Effect.colors", &ModelEffect::colors);
	Kaba::declare_class_element("Model.Effect.inv_quad", &ModelEffect::inv_quad);
	Kaba::declare_class_element("Model.Effect.file", &ModelEffect::file);

	Kaba::declare_class_size("Geometry.Vertex", sizeof(ModelVertex));
	Kaba::declare_class_element("Geometry.Vertex.normal_mode", &ModelVertex::normal_mode);
	Kaba::declare_class_element("Geometry.Vertex.bone_index", &ModelVertex::bone_index);
	Kaba::declare_class_element("Geometry.Vertex.normal_dirty", &ModelVertex::normal_dirty);
	Kaba::declare_class_element("Geometry.Vertex.ref_count", &ModelVertex::ref_count);

	Kaba::declare_class_size("Geometry.Polygon.Side", sizeof(ModelPolygonSide));
	Kaba::declare_class_element("Geometry.Polygon.Side.vertex", &ModelPolygonSide::vertex);
	Kaba::declare_class_element("Geometry.Polygon.Side.edge", &ModelPolygonSide::edge);
	Kaba::declare_class_element("Geometry.Polygon.Side.edge_direction", &ModelPolygonSide::edge_direction);
	Kaba::declare_class_element("Geometry.Polygon.Side.skin_vertex", &ModelPolygonSide::skin_vertex);
	Kaba::declare_class_element("Geometry.Polygon.Side.normal_index", &ModelPolygonSide::normal_index);
	Kaba::declare_class_element("Geometry.Polygon.Side.normal", &ModelPolygonSide::normal);
	Kaba::declare_class_element("Geometry.Polygon.Side.triangulation", &ModelPolygonSide::triangulation);

	Kaba::declare_class_size("Geometry.Polygon", sizeof(ModelPolygon));
	Kaba::declare_class_element("Geometry.Polygon.side", &ModelPolygon::side);
	Kaba::declare_class_element("Geometry.Polygon.temp_normal", &ModelPolygon::temp_normal);
	Kaba::declare_class_element("Geometry.Polygon.normal_dirty", &ModelPolygon::normal_dirty);
	Kaba::declare_class_element("Geometry.Polygon.triangulation_dirty", &ModelPolygon::triangulation_dirty);
	Kaba::declare_class_element("Geometry.Polygon.material", &ModelPolygon::material);

	/*Kaba::declare_class_size("ModelSurface", sizeof(ModelSurface));
	Kaba::declare_class_element("ModelSurface.polygon", &ModelSurface::polygon);
	Kaba::declare_class_element("ModelSurface.edge", &ModelSurface::edge);
	Kaba::declare_class_element("ModelSurface.vertex", &ModelSurface::vertex);
	Kaba::declare_class_element("ModelSurface.is_closed", &ModelSurface::is_closed);
	Kaba::declare_class_element("ModelSurface.is_physical", &ModelSurface::is_physical);
	Kaba::declare_class_element("ModelSurface.is_visible", &ModelSurface::is_visible);
	Kaba::declare_class_element("ModelSurface.model", &ModelSurface::model);
*/
	Kaba::declare_class_size("Model.Bone", sizeof(ModelBone));
	Kaba::declare_class_element("Model.Bone.parent", &ModelBone::parent);
	Kaba::declare_class_element("Model.Bone.model_file", &ModelBone::model_file);
	Kaba::declare_class_element("Model.Bone.model", &ModelBone::model);
	Kaba::declare_class_element("Model.Bone.const_pos", &ModelBone::const_pos);
	Kaba::declare_class_element("Model.Bone.matrix", &ModelBone::_matrix);

	Kaba::declare_class_size("Model.Move", sizeof(ModelMove));
	Kaba::declare_class_element("Model.Move.name", &ModelMove::name);
	Kaba::declare_class_element("Model.Move.type", &ModelMove::type);
	Kaba::declare_class_element("Model.Move.FramesPerSecConst", &ModelMove::frames_per_sec_const);
	Kaba::declare_class_element("Model.Move.FramesPerSecFactor", &ModelMove::frames_per_sec_factor);
	Kaba::declare_class_element("Model.Move.frame", &ModelMove::frame);

	Kaba::declare_class_size("Model.Move.Frame", sizeof(ModelFrame));
	Kaba::declare_class_element("Model.Move.Frame.skel_ang", &ModelFrame::skel_ang);
	Kaba::declare_class_element("Model.Move.Frame.skel_dpos", &ModelFrame::skel_dpos);
	Kaba::declare_class_element("Model.Move.Frame.dpos", &ModelFrame::skin);
	Kaba::declare_class_element("Model.Move.Frame.vertex_dpos", &ModelFrame::vertex_dpos);


	Kaba::declare_class_size("Model.Mesh", sizeof(ModelMesh));
	Kaba::declare_class_element("Model.Mesh.vertex", &ModelMesh::vertex);
	Kaba::declare_class_element("Model.Mesh.polygon", &ModelMesh::polygon);
	Kaba::declare_class_element("Model.Mesh.edge", &ModelMesh::edge);
	Kaba::declare_class_element("Model.Mesh.ball", &ModelMesh::ball);
	Kaba::declare_class_element("Model.Mesh.cylinder", &ModelMesh::cylinder);

	Kaba::declare_class_size("Model", sizeof(DataModel));
	Kaba::declare_class_element("Model.bone", &DataModel::bone);
	Kaba::declare_class_element("Model.move", &DataModel::move);
	//Kaba::declare_class_element("Model.move", &DataModel::move);
	//Kaba::declare_class_element("Model.CurrentMove", &DataModel::CurrentMove);
	//Kaba::declare_class_element("Model.CurrentFrame", &DataModel::CurrentFrame);
	Kaba::declare_class_element("Model.mesh", &DataModel::mesh);
	Kaba::declare_class_element("Model.phys_mesh", &DataModel::phys_mesh);
	Kaba::declare_class_element("Model.material", &DataModel::material);
	Kaba::declare_class_element("Model.fx", &DataModel::fx);
	Kaba::declare_class_element("Model.meta_data", &DataModel::meta_data);
	//Kaba::declare_class_element("Model.CurrentMaterial", &DataModel::CurrentMaterial);
	//Kaba::declare_class_element("Model.CurrentTextureLevel", &DataModel::CurrentTextureLevel);
	//Kaba::declare_class_element("Model.SkinVertex", &DataModel::SkinVertex);
	//Kaba::declare_class_element("Model.SkinVertMat", &DataModel::SkinVertMat);
	/*Kaba::declare_class_element("Model.SkinVertTL", &DataModel::SkinVertTL);
	Kaba::declare_class_element("Model.Playing", &DataModel::Playing);
	Kaba::declare_class_element("Model.PlayLoop", &DataModel::PlayLoop);
	Kaba::declare_class_element("Model.TimeScale", &DataModel::TimeScale);
	Kaba::declare_class_element("Model.TimeParam", &DataModel::TimeParam);
	Kaba::declare_class_element("Model.SimFrame", &DataModel::SimFrame);*/
	Kaba::link_external_class_func("Model.clear_selection", &DataModel::clearSelection);
	Kaba::link_external_class_func("Model.invert_polygons", &DataModel::invert_polygons);
	Kaba::link_external_class_func("Model.selection_from_vertices", &DataModel::selectionFromVertices);
	Kaba::link_external_class_func("Model.selection_from_polygons", &DataModel::selectionFromPolygons);
	Kaba::link_external_class_func("Model.add_vertex", &DataModel::addVertex);
	Kaba::link_external_class_func("Model.add_triangle", &DataModel::addTriangle);
	Kaba::link_external_class_func("Model.add_polygon", &DataModel::addPolygon);
	Kaba::link_external_class_func("Model.delete_selection", &DataModel::delete_selection);
	Kaba::link_external_class_func("Model.delete_polygon", &DataModel::delete_polygon);
	Kaba::link_external_class_func("Model.subtract_selection", &DataModel::subtractSelection);
	Kaba::link_external_class_func("Model.convert_selection_to_triangles", &DataModel::convertSelectionToTriangles);
	Kaba::link_external_class_func("Model.triangulate_selected_vertices", &DataModel::triangulateSelectedVertices);
	Kaba::link_external_class_func("Model.bevel_selected_vertices", &DataModel::bevelSelectedEdges);
	Kaba::link_external_class_func("Model.extrude_selected_polygons", &DataModel::extrudeSelectedPolygons);
	Kaba::link_external_class_func("Model.paste_geometry", &DataModel::pasteGeometry);
	Kaba::link_external_class_func("Model.add_animation", &DataModel::addAnimation);
	Kaba::link_external_class_func("Model.duplicate_animation", &DataModel::duplicateAnimation);
	Kaba::link_external_class_func("Model.delete_animation", &DataModel::deleteAnimation);
	Kaba::link_external_class_func("Model.animation_set_data", &DataModel::setAnimationData);
	Kaba::link_external_class_func("Model.animation_add_frame", &DataModel::animationAddFrame);
	Kaba::link_external_class_func("Model.animation_delete_frame", &DataModel::animationDeleteFrame);
	Kaba::link_external_class_func("Model.animation_set_frame_duration", &DataModel::animationSetFrameDuration);
	Kaba::link_external_class_func("Model.animation_set_bone", &DataModel::animationSetBone);


	Kaba::declare_class_size("Geometry", sizeof(Geometry));
	Kaba::link_external_class_func("Geometry.Ball.__init__", &GeometryBall::__init__);
	Kaba::link_external_class_func("Geometry.Cube.__init__", &GeometryCube::__init__);
	Kaba::link_external_class_func("Geometry.Sphere.__init__", &GeometrySphere::__init__);
	Kaba::link_external_class_func("Geometry.Cylinder.__init__", &GeometryCylinder::__init__);
	Kaba::link_external_class_func("Geometry.CylinderComplex.__init__", &GeometryCylinder::__init2__);
	Kaba::link_external_class_func("Geometry.Torus.__init__", &GeometryTorus::__init__);
	Kaba::link_external_class_func("Geometry.TorusKnot.__init__", &GeometryTorusKnot::__init__);
	Kaba::link_external_class_func("Geometry.Plane.__init__", &GeometryPlane::__init__);
	Kaba::link_external_class_func("Geometry.Platonic.__init__", &GeometryPlatonic::__init__);
	Kaba::link_external_class_func("Geometry.Teapot.__init__", &GeometryTeapot::__init__);
	Kaba::link_external("Geometry.subtract", (void*)&GeometrySubtract);
	Kaba::link_external("Geometry.and", (void*)&GeometryAnd);

	// world

	Kaba::declare_class_size("World", sizeof(DataWorld));
	Kaba::declare_class_element("World.terrains", &DataWorld::terrains);
	Kaba::declare_class_element("World.objects", &DataWorld::objects);
	Kaba::declare_class_element("World.ego_index", &DataWorld::EgoIndex);
	Kaba::link_external_class_func("World.add_object", &DataWorld::add_object);
	Kaba::link_external_class_func("World.add_terrain", &DataWorld::add_terrain);
	Kaba::link_external_class_func("World.add_new_terrain", &DataWorld::add_new_terrain);


	ShaderNode node("");
	Kaba::declare_class_size("ShaderNode", sizeof(ShaderNode));
	Kaba::declare_class_element("ShaderNode.x", &ShaderNode::x);
	Kaba::declare_class_element("ShaderNode.y", &ShaderNode::y);
	Kaba::declare_class_element("ShaderNode.type", &ShaderNode::type);
	Kaba::declare_class_element("ShaderNode.output", &ShaderNode::output);
	Kaba::declare_class_element("ShaderNode.params", &ShaderNode::params);
	Kaba::link_external_class_func("ShaderNode.__init__", &ShaderNode::__init__);
	Kaba::link_external_virtual("ShaderNode.__delete__", &ShaderNode::__delete__, &node);
	Kaba::link_external_virtual("ShaderNode.code_pixel", &ShaderNode::code_pixel, &node);
	Kaba::link_external_virtual("ShaderNode.dependencies", &ShaderNode::dependencies, &node);
	Kaba::declare_class_size("ShaderNode.Parameter", sizeof(ShaderNode::Parameter));
	Kaba::declare_class_element("ShaderNode.Parameter.type", &ShaderNode::Parameter::type);
	Kaba::declare_class_element("ShaderNode.Parameter.name", &ShaderNode::Parameter::name);
	Kaba::declare_class_element("ShaderNode.Parameter.value", &ShaderNode::Parameter::value);
	Kaba::declare_class_element("ShaderNode.Parameter.options", &ShaderNode::Parameter::options);
	Kaba::declare_class_size("ShaderNode.Port", sizeof(ShaderNode::Port));
	Kaba::declare_class_element("ShaderNode.Port.type", &ShaderNode::Port::type);
	Kaba::declare_class_element("ShaderNode.Port.name", &ShaderNode::Port::name);

	Kaba::declare_class_size("ShaderBuilderContext", sizeof(ShaderBuilderContext));
	Kaba::link_external_class_func("ShaderBuilderContext.build_value", &ShaderBuilderContext::build_value);
	Kaba::link_external_class_func("ShaderBuilderContext.build_const", &ShaderBuilderContext::build_const);
	Kaba::link_external_class_func("ShaderBuilderContext.find_temp", &ShaderBuilderContext::find_temp);
	Kaba::link_external_class_func("ShaderBuilderContext.create_temp", &ShaderBuilderContext::create_temp);
	Kaba::link_external_class_func("ShaderBuilderContext.create_out", &ShaderBuilderContext::create_out);
}

void PluginManager::find_plugins() {
	Path dir0 = PluginManager::directory << "Shader Graph";
	auto list = dir_search(dir0, "*", true);
	for (auto &e: list) {
		Path dir = dir0 << e;
		if (file_is_directory(dir)) {
			auto list2 = dir_search(dir, "*.kaba", false);
			for (auto &e2: list2) {
				Plugin p;
				p.filename = dir << e2;
				p.name = e2.replace(".kaba", "");
				p.category = e;
				p.type = PluginType::SHADER_NODE;
				plugins.add(p);
			}
		}
	}
}

void *PluginManager::create_instance(const Path &filename, const string &parent) {
	Kaba::config.directory = "";
	auto s = Kaba::Load(filename);
	for (auto c: s->classes())
		if (c->is_derived_from_s(parent)) {
			return c->create_instance();
		}
	throw Exception("no matching class defined");
	return nullptr;
}

void *PluginManager::Plugin::create_instance(const string &parent) const {
	return PluginManager::create_instance(filename, parent);
}
