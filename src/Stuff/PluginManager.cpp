/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../lib/kaba/kaba.h"
#include "../lib/os/filesystem.h"
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
		auto s = kaba::default_context->load_module(filename);
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

	auto ext = kaba::default_context->external.get();

	ext->link("edward", &GlobalMainWin);
	ext->link("ed", &ed);
	ext->link("model", &ed->mode_model->data);
	ext->link("world", &ed->mode_world->data);

	ext->declare_class_element("Edward.cur_mode", &Edward::cur_mode);

	ext->declare_class_element("Mode.name", &ModeBase::name);
	ext->declare_class_element("Mode.multi_view", &ModeBase::multi_view);

	ext->declare_class_size("Data", sizeof(Data));
	ext->declare_class_element("Data.filename", &Data::filename);
	ext->declare_class_element("Data.file_time", &Data::file_time);
	ext->declare_class_element("Data.binary_file_format", &Data::binary_file_format);
	ext->declare_class_element("Data.type", &Data::type);
	ext->link_class_func("Data.begin_action", &Data::begin_action_group);
	ext->link_class_func("Data.end_action", &Data::end_action_group);

	ext->declare_class_size("MultiView.SingleData", sizeof(MultiView::SingleData));
	ext->declare_class_element("MultiView.SingleData.pos", &MultiView::SingleData::pos);
	ext->declare_class_element("MultiView.SingleData.view_stage", &MultiView::SingleData::view_stage);
	ext->declare_class_element("MultiView.SingleData.is_selected", &MultiView::SingleData::is_selected);
	ext->declare_class_element("MultiView.SingleData.m_delta", &MultiView::SingleData::m_delta);
	ext->declare_class_element("MultiView.SingleData.m_old", &MultiView::SingleData::m_old);
	ext->declare_class_element("MultiView.SingleData.is_special", &MultiView::SingleData::is_special);


	ext->declare_class_element("MultiView.action_con", &MultiView::MultiView::action_con);
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
	ext->declare_class_element("MultiView.ActionController.pos", &MultiView::ActionController::pos);

	// model

	ext->declare_class_size("Model.Effect", sizeof(ModelEffect));
	ext->declare_class_element("Model.Effect.kind", &ModelEffect::type);
	ext->declare_class_element("Model.Effect.vertex", &ModelEffect::vertex);
	ext->declare_class_element("Model.Effect.size", &ModelEffect::size);
	ext->declare_class_element("Model.Effect.speed", &ModelEffect::speed);
	ext->declare_class_element("Model.Effect.intensity", &ModelEffect::intensity);
	ext->declare_class_element("Model.Effect.colors", &ModelEffect::colors);
	ext->declare_class_element("Model.Effect.inv_quad", &ModelEffect::inv_quad);
	ext->declare_class_element("Model.Effect.file", &ModelEffect::file);

	ext->declare_class_size("Geometry.Vertex", sizeof(ModelVertex));
	ext->declare_class_element("Geometry.Vertex.normal_mode", &ModelVertex::normal_mode);
	ext->declare_class_element("Geometry.Vertex.bone_index", &ModelVertex::bone_index);
	ext->declare_class_element("Geometry.Vertex.normal_dirty", &ModelVertex::normal_dirty);
	ext->declare_class_element("Geometry.Vertex.ref_count", &ModelVertex::ref_count);

	ext->declare_class_size("Geometry.Polygon.Side", sizeof(ModelPolygonSide));
	ext->declare_class_element("Geometry.Polygon.Side.vertex", &ModelPolygonSide::vertex);
	ext->declare_class_element("Geometry.Polygon.Side.edge", &ModelPolygonSide::edge);
	ext->declare_class_element("Geometry.Polygon.Side.edge_direction", &ModelPolygonSide::edge_direction);
	ext->declare_class_element("Geometry.Polygon.Side.skin_vertex", &ModelPolygonSide::skin_vertex);
	ext->declare_class_element("Geometry.Polygon.Side.normal_index", &ModelPolygonSide::normal_index);
	ext->declare_class_element("Geometry.Polygon.Side.normal", &ModelPolygonSide::normal);
	ext->declare_class_element("Geometry.Polygon.Side.triangulation", &ModelPolygonSide::triangulation);

	ext->declare_class_size("Geometry.Polygon", sizeof(ModelPolygon));
	ext->declare_class_element("Geometry.Polygon.side", &ModelPolygon::side);
	ext->declare_class_element("Geometry.Polygon.temp_normal", &ModelPolygon::temp_normal);
	ext->declare_class_element("Geometry.Polygon.normal_dirty", &ModelPolygon::normal_dirty);
	ext->declare_class_element("Geometry.Polygon.triangulation_dirty", &ModelPolygon::triangulation_dirty);
	ext->declare_class_element("Geometry.Polygon.material", &ModelPolygon::material);

	/*ext->declare_class_size("ModelSurface", sizeof(ModelSurface));
	ext->declare_class_element("ModelSurface.polygon", &ModelSurface::polygon);
	ext->declare_class_element("ModelSurface.edge", &ModelSurface::edge);
	ext->declare_class_element("ModelSurface.vertex", &ModelSurface::vertex);
	ext->declare_class_element("ModelSurface.is_closed", &ModelSurface::is_closed);
	ext->declare_class_element("ModelSurface.is_physical", &ModelSurface::is_physical);
	ext->declare_class_element("ModelSurface.is_visible", &ModelSurface::is_visible);
	ext->declare_class_element("ModelSurface.model", &ModelSurface::model);
*/
	ext->declare_class_size("Model.Bone", sizeof(ModelBone));
	ext->declare_class_element("Model.Bone.parent", &ModelBone::parent);
	ext->declare_class_element("Model.Bone.model_file", &ModelBone::model_file);
	ext->declare_class_element("Model.Bone.model", &ModelBone::model);
	ext->declare_class_element("Model.Bone.const_pos", &ModelBone::const_pos);
	ext->declare_class_element("Model.Bone.matrix", &ModelBone::_matrix);

	ext->declare_class_size("Model.Move", sizeof(ModelMove));
	ext->declare_class_element("Model.Move.name", &ModelMove::name);
	ext->declare_class_element("Model.Move.type", &ModelMove::type);
	ext->declare_class_element("Model.Move.FramesPerSecConst", &ModelMove::frames_per_sec_const);
	ext->declare_class_element("Model.Move.FramesPerSecFactor", &ModelMove::frames_per_sec_factor);
	ext->declare_class_element("Model.Move.frame", &ModelMove::frame);

	ext->declare_class_size("Model.Move.Frame", sizeof(ModelFrame));
	ext->declare_class_element("Model.Move.Frame.skel_ang", &ModelFrame::skel_ang);
	ext->declare_class_element("Model.Move.Frame.skel_dpos", &ModelFrame::skel_dpos);
	ext->declare_class_element("Model.Move.Frame.dpos", &ModelFrame::skin);
	ext->declare_class_element("Model.Move.Frame.vertex_dpos", &ModelFrame::vertex_dpos);


	ext->declare_class_size("Model.Mesh", sizeof(ModelMesh));
	ext->declare_class_element("Model.Mesh.vertex", &ModelMesh::vertex);
	ext->declare_class_element("Model.Mesh.polygon", &ModelMesh::polygon);
	ext->declare_class_element("Model.Mesh.edge", &ModelMesh::edge);
	ext->declare_class_element("Model.Mesh.ball", &ModelMesh::ball);
	ext->declare_class_element("Model.Mesh.cylinder", &ModelMesh::cylinder);

	ext->declare_class_size("Model", sizeof(DataModel));
	ext->declare_class_element("Model.bone", &DataModel::bone);
	ext->declare_class_element("Model.move", &DataModel::move);
	//ext->declare_class_element("Model.move", &DataModel::move);
	//ext->declare_class_element("Model.CurrentMove", &DataModel::CurrentMove);
	//ext->declare_class_element("Model.CurrentFrame", &DataModel::CurrentFrame);
	ext->declare_class_element("Model.mesh", &DataModel::mesh);
	ext->declare_class_element("Model.phys_mesh", &DataModel::phys_mesh);
	ext->declare_class_element("Model.material", &DataModel::material);
	ext->declare_class_element("Model.fx", &DataModel::fx);
	ext->declare_class_element("Model.meta_data", &DataModel::meta_data);
	//ext->declare_class_element("Model.CurrentMaterial", &DataModel::CurrentMaterial);
	//ext->declare_class_element("Model.CurrentTextureLevel", &DataModel::CurrentTextureLevel);
	//ext->declare_class_element("Model.SkinVertex", &DataModel::SkinVertex);
	//ext->declare_class_element("Model.SkinVertMat", &DataModel::SkinVertMat);
	/*ext->declare_class_element("Model.SkinVertTL", &DataModel::SkinVertTL);
	ext->declare_class_element("Model.Playing", &DataModel::Playing);
	ext->declare_class_element("Model.PlayLoop", &DataModel::PlayLoop);
	ext->declare_class_element("Model.TimeScale", &DataModel::TimeScale);
	ext->declare_class_element("Model.TimeParam", &DataModel::TimeParam);
	ext->declare_class_element("Model.SimFrame", &DataModel::SimFrame);*/
	ext->link_class_func("Model.clear_selection", &DataModel::clearSelection);
	ext->link_class_func("Model.invert_polygons", &DataModel::invert_polygons);
	ext->link_class_func("Model.selection_from_vertices", &DataModel::selectionFromVertices);
	ext->link_class_func("Model.selection_from_polygons", &DataModel::selectionFromPolygons);
	ext->link_class_func("Model.add_vertex", &DataModel::addVertex);
	ext->link_class_func("Model.add_triangle", &DataModel::addTriangle);
	ext->link_class_func("Model.add_polygon", &DataModel::addPolygon);
	ext->link_class_func("Model.delete_selection", &DataModel::delete_selection);
	ext->link_class_func("Model.delete_polygon", &DataModel::delete_polygon);
	ext->link_class_func("Model.subtract_selection", &DataModel::subtractSelection);
	ext->link_class_func("Model.convert_selection_to_triangles", &DataModel::convertSelectionToTriangles);
	ext->link_class_func("Model.triangulate_selected_vertices", &DataModel::triangulateSelectedVertices);
	ext->link_class_func("Model.bevel_selected_vertices", &DataModel::bevelSelectedEdges);
	ext->link_class_func("Model.extrude_selected_polygons", &DataModel::extrudeSelectedPolygons);
	ext->link_class_func("Model.paste_geometry", &DataModel::pasteGeometry);
	ext->link_class_func("Model.add_animation", &DataModel::addAnimation);
	ext->link_class_func("Model.duplicate_animation", &DataModel::duplicateAnimation);
	ext->link_class_func("Model.delete_animation", &DataModel::deleteAnimation);
	ext->link_class_func("Model.animation_set_data", &DataModel::setAnimationData);
	ext->link_class_func("Model.animation_add_frame", &DataModel::animationAddFrame);
	ext->link_class_func("Model.animation_delete_frame", &DataModel::animationDeleteFrame);
	ext->link_class_func("Model.animation_set_frame_duration", &DataModel::animationSetFrameDuration);
	ext->link_class_func("Model.animation_set_bone", &DataModel::animationSetBone);


	ext->declare_class_size("Geometry", sizeof(Geometry));
	ext->link_class_func("Geometry.Ball.__init__", &GeometryBall::__init__);
	ext->link_class_func("Geometry.Cube.__init__", &GeometryCube::__init__);
	ext->link_class_func("Geometry.Sphere.__init__", &GeometrySphere::__init__);
	ext->link_class_func("Geometry.Cylinder.__init__", &GeometryCylinder::__init__);
	ext->link_class_func("Geometry.CylinderComplex.__init__", &GeometryCylinder::__init2__);
	ext->link_class_func("Geometry.Torus.__init__", &GeometryTorus::__init__);
	ext->link_class_func("Geometry.TorusKnot.__init__", &GeometryTorusKnot::__init__);
	ext->link_class_func("Geometry.Plane.__init__", &GeometryPlane::__init__);
	ext->link_class_func("Geometry.Platonic.__init__", &GeometryPlatonic::__init__);
	ext->link_class_func("Geometry.Teapot.__init__", &GeometryTeapot::__init__);
	ext->link("Geometry.subtract", (void*)&GeometrySubtract);
	ext->link("Geometry.and", (void*)&GeometryAnd);

	// world

	ext->declare_class_size("World", sizeof(DataWorld));
	ext->declare_class_element("World.terrains", &DataWorld::terrains);
	ext->declare_class_element("World.objects", &DataWorld::objects);
	ext->declare_class_element("World.ego_index", &DataWorld::EgoIndex);
	ext->link_class_func("World.add_object", &DataWorld::add_object);
	ext->link_class_func("World.add_terrain", &DataWorld::add_terrain);
	ext->link_class_func("World.add_new_terrain", &DataWorld::add_new_terrain);


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
}

void PluginManager::find_plugins() {
	Path dir0 = (PluginManager::directory << "Shader Graph").absolute();
	auto list = os::fs::search(dir0, "*", "fd");
	for (auto &e: list) {
		Path dir = dir0 << e;
		if (os::fs::is_directory(dir)) {
			auto list2 = os::fs::search(dir, "*.kaba", "f");
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
