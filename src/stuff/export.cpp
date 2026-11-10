#include "PluginManager.h"
#include <lib/base/callable.h>
#include <lib/kaba/Context.h>
#include <lib/kaba/lib/extern.h>
#include <lib/kaba/lib/future.h>
#include <lib/polymesh/Mesh.h>
#include <lib/polymesh/create/Ball.h>
#include <lib/polymesh/create/Cube.h>
#include <lib/polymesh/create/Plane.h>
#include <lib/polymesh/create/Platonic.h>
#include <lib/polymesh/create/Sphere.h>
#include <lib/polymesh/create/Teapot.h>
#include <lib/polymesh/create/Torus.h>
#include <lib/polymesh/create/TorusKnot.h>
#include <mode_material/ModeMaterial.h>
#include <mode_model/ModeModel.h>
#include <mode_model/mesh/processing/MeshRandomSurfacePoints.h>
#include <mode_world/ModeWorld.h>
#include <mode_world/data/DataWorld.h>
#include <mode_world/data/WorldTerrain.h>
#include <storage/Storage.h>
#include <storage/format/Format.h>
#include <view/DocumentSession.h>
#include <view/DrawingHelper.h>
#include <view/dialogs/CommonDialogs.h>
#include <view/multiview/MultiView.h>
#include <view/multiview/ViewPort.h>


namespace edward {

extern Session* cur_session;

class DocumentSessionWrapper: public DocumentSession {
public:
	void event_kaba(const string& id, Callable<void()>& f) {
		event(id, [&f] {
			f();
		});
	}
};

void link_mesh(kaba::ExternalLinkData* ext) {
	// TODO use polymesh instead

	ext->declare_class_size("Mesh.Vertex", sizeof(polymesh::Vertex));
	ext->declare_class_element("Mesh.Vertex.normal_mode", &polymesh::Vertex::normal_mode);
	ext->declare_class_element("Mesh.Vertex.bone_index", &polymesh::Vertex::bone_index);
	ext->declare_class_element("Mesh.Vertex.normal_dirty", &polymesh::Vertex::normal_dirty);
	ext->declare_class_element("Mesh.Vertex.ref_count", &polymesh::Vertex::ref_count);

	ext->declare_class_size("Mesh.Polygon.Side", sizeof(polymesh::PolygonSide));
	ext->declare_class_element("Mesh.Polygon.Side.vertex", &polymesh::PolygonSide::vertex);
	//	ext->declare_class_element("Mesh.Polygon.Side.edge", &polymesh::PolygonSide::edge);
	//	ext->declare_class_element("Mesh.Polygon.Side.edge_direction", &polymesh::PolygonSide::edge_direction);
	ext->declare_class_element("Mesh.Polygon.Side.skin_vertex", &polymesh::PolygonSide::uv);
	ext->declare_class_element("Mesh.Polygon.Side.normal_index", &polymesh::PolygonSide::normal_index);
	ext->declare_class_element("Mesh.Polygon.Side.normal", &polymesh::PolygonSide::normal);
	ext->declare_class_element("Mesh.Polygon.Side.triangulation", &polymesh::PolygonSide::triangulation);

	ext->declare_class_size("Mesh.Polygon", sizeof(polymesh::Polygon));
	ext->declare_class_element("Mesh.Polygon.side", &polymesh::Polygon::side);
	ext->declare_class_element("Mesh.Polygon.temp_normal", &polymesh::Polygon::temp_normal);
	ext->declare_class_element("Mesh.Polygon.normal_dirty", &polymesh::Polygon::normal_dirty);
	ext->declare_class_element("Mesh.Polygon.triangulation_dirty", &polymesh::Polygon::triangulation_dirty);
	ext->declare_class_element("Mesh.Polygon.material", &polymesh::Polygon::material);

	ext->declare_class_size("Mesh", sizeof(polymesh::Mesh));
	ext->link_class_func("Mesh.__init__", &kaba::generic_init<polymesh::Mesh>);
	ext->link_class_func("Mesh.__delete__", &kaba::generic_delete<polymesh::Mesh>);
	ext->link_class_func("Mesh.__assign__", &kaba::generic_assign<polymesh::Mesh>);
	ext->link_class_func("Mesh.add", &polymesh::Mesh::add);
	ext->link_class_func("Mesh.transform", &polymesh::Mesh::transform);
	ext->link_class_func("Mesh.invert", &polymesh::Mesh::invert);
	ext->declare_class_element("Mesh.vertices", &polymesh::Mesh::vertices);
	ext->declare_class_element("Mesh.polygons", &polymesh::Mesh::polygons);
	//	ext->declare_class_element("Mesh.edges", &polymesh::PolygonMesh::edges);
	ext->declare_class_element("Mesh.spheres", &polymesh::Mesh::spheres);
	ext->declare_class_element("Mesh.cylinders", &polymesh::Mesh::cylinders);

	ext->link("Mesh.create_ball", (void*)&polymesh::create_ball);
	ext->link("Mesh.create_cube", (void*)&polymesh::create_cube);
	ext->link("Mesh.create_sphere", (void*)&polymesh::create_sphere);
	//ext->link_class_func("Geometry.Cylinder.__init__", &GeometryCylinder::__init__);
	//ext->link_class_func("Geometry.CylinderComplex.__init__", &GeometryCylinder::__init2__);
	ext->link("Mesh.create_torus", (void*)&polymesh::create_torus);
	ext->link("Mesh.create_torus_knot", (void*)&polymesh::create_torus_knot);
	ext->link("Mesh.create_plane", (void*)&polymesh::create_plane);
	ext->link("Mesh.create_platonic", (void*)&polymesh::create_platonic);
	ext->link("Mesh.create_teapot", (void*)&polymesh::create_teapot);
//	ext->link("Geometry.subtract", (void*)&GeometrySubtract);
//	ext->link("Geometry.and", (void*)&GeometryAnd);

	ext->link("mesh_surface_points", (void*)&mesh_surface_points);
}

void link_material(kaba::ExternalLinkData* ext) {
	ext->declare_class_size("ModeMaterial", sizeof(ModeMaterial));
	ext->declare_class_element("ModeMaterial.data", &ModeMaterial::data);

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
	ext->declare_class_size("DataWorld.Terrain", sizeof(WorldTerrain));
	ext->declare_class_element("DataWorld.Terrain.terrain", &WorldTerrain::terrain);
	ext->link_class_func("DataWorld.Terrain.__init__", &kaba::generic_init<WorldTerrain>);


	ext->declare_class_size("DataWorld", sizeof(DataWorld));
	ext->declare_class_element("DataWorld.out_terrain_changed", &DataWorld::out_terrain_changed);
	ext->link_class_func("DataWorld.entity", &DataWorld::entity);
	ext->link_class_func("DataWorld.edit_terrain_meta_data", &DataWorld::edit_terrain_meta_data);


	ext->declare_class_size("ModeWorld", sizeof(ModeWorld));
	ext->declare_class_element("ModeWorld.data", &ModeWorld::data);
	ext->link_class_func("ModeWorld.get_hover", &ModeWorld::get_hover);
	ext->link_class_func("ModeWorld.get_hover_terrains", &ModeWorld::get_hover_terrains);
}

class MultiViewWrapper: public MultiView {
public:
	struct WinVec2 {
		MultiViewWindow* win;
		vec2 m;
	};
	struct WinRect {
		MultiViewWindow* win;
		rect r;
	};
	//void set_f_hover(const Callable<base::optional<Hover>(MultiViewWindow*,const vec2&)>& f) {
	void set_f_hover(Callable<base::optional<Hover>(const WinVec2& x)>* f) {
		f_hover = nullptr;
		if (f)
			f_hover = [f] (MultiViewWindow* win,const vec2& m) {
				//return f(win, m);
				return (*f)({win, m});
			};
	}
	//void set_f_select(const Callable<Selection(MultiViewWindow*,const rect&)>& f) {
	void set_f_select(Callable<Selection(const WinRect&)>* f) {
		f_select = nullptr;
		if (f)
			f_select = [f] (MultiViewWindow* win,const rect& r) {
				//return f(win, r);
				return (*f)({win, r});
			};
	}
	void set_f_make_selection_consistent(Callable<void(Selection&)>* f) {
		f_make_selection_consistent = nullptr;
		if (f)
			f_make_selection_consistent = [f] (Selection& sel) {
				(*f)(sel);
			};
	}
	void set_f_get_selection_box(Callable<base::optional<Box>(const Selection&)>* f) {
		f_get_selection_box = nullptr;
		if (f)
			f_get_selection_box = [f] (const Selection& sel) {
				return (*f)(sel);
			};
	}
};


void link_multiview(kaba::ExternalLinkData* ext) {
	ext->declare_class_size("Hover", sizeof(Hover));
	ext->declare_class_element("Hover.type", &Hover::type);
	ext->declare_class_element("Hover.index", &Hover::index);
	ext->declare_class_element("Hover.tp", &Hover::tp);


	ext->declare_class_size("Selection", sizeof(Selection));
	ext->link_class_func("Selection.__init__", &kaba::generic_init<Selection>);
	ext->link_class_func("Selection.__delete__", &kaba::generic_delete<Selection>);
	ext->link_class_func("Selection.__assign__", &kaba::generic_assign<Selection>);


	ext->declare_class_size("MultiView.ViewPort", sizeof(ViewPort));
	ext->declare_class_element("MultiView.ViewPort.pos", &ViewPort::pos);
	ext->declare_class_element("MultiView.ViewPort.ang", &ViewPort::ang);
	ext->declare_class_element("MultiView.ViewPort.radius", &ViewPort::radius);
	ext->declare_class_element("MultiView.ViewPort.out_changed", &ViewPort::out_changed);


	ext->declare_class_size("MultiView.SingleData", sizeof(multiview::SingleData));
	ext->declare_class_element("MultiView.SingleData.pos", &multiview::SingleData::pos);
	ext->declare_class_element("MultiView.SingleData.view_stage", &multiview::SingleData::view_stage);
	ext->declare_class_element("MultiView.SingleData.is_special", &multiview::SingleData::is_special);


	ext->declare_class_element("MultiView.hover", &MultiView::hover);
	ext->declare_class_element("MultiView.cam", &MultiView::view_port);
	ext->declare_class_element("MultiView.view_port", &MultiView::view_port);
	ext->declare_class_element("MultiView.active_window", &MultiView::active_window);
	ext->declare_class_element("MultiView.hover_window", &MultiView::hover_window);
	ext->link_class_func("MultiView.set_allow_action", &MultiView::set_allow_action);
	ext->link_class_func("MultiView.set_allow_select", &MultiView::set_allow_select);
	ext->link_class_func("MultiView.set_f_hover", &MultiViewWrapper::set_f_hover);
	ext->link_class_func("MultiView.set_f_select", &MultiViewWrapper::set_f_select);
	ext->link_class_func("MultiView.set_f_make_selection_consistent", &MultiViewWrapper::set_f_make_selection_consistent);
	ext->link_class_func("MultiView.set_f_get_selection_box", &MultiViewWrapper::set_f_get_selection_box);
}

void link_mode(kaba::ExternalLinkData* ext) {
//	ext->declare_class_element("Edward.cur_mode", &Session::cur_mode);

	{
		Mode mode(nullptr);
		ext->declare_class_size("Mode", sizeof(Mode));
		ext->declare_class_element("Mode.menu_id", &Mode::menu_id);
		ext->declare_class_element("Mode.toolbar_id", &Mode::toolbar_id);
		ext->declare_class_element("Mode.dialog", &Mode::dialog);
		ext->declare_class_element("Mode.doc", &Mode::doc);
		ext->declare_class_element("Mode.session", &Mode::session);
		ext->declare_class_element("Mode.multi_view", &Mode::multi_view);
		ext->declare_class_element("Mode.data", &Mode::generic_data);
		ext->declare_class_element("Mode.out_redraw", &Mode::out_redraw);
		ext->link_class_func("Mode.__init__", &kaba::generic_init_ext<Mode, DocumentSession*>);
		ext->link_class_func("Mode.draw_info", &Mode::draw_info);
		ext->link_class_func("Mode.is_ancestor_of", &Mode::is_ancestor_of);
		ext->link_class_func("Mode.set_side_panel", &Mode::set_side_panel);
		ext->link_class_func("Mode.set_overlay_panel", &Mode::set_overlay_panel);

		ext->link_virtual("Mode.on_enter_rec", &Mode::on_enter_rec, &mode);
		ext->link_virtual("Mode.on_connect_events_rec", &Mode::on_connect_events_rec, &mode);
		ext->link_virtual("Mode.on_leave_rec", &Mode::on_leave_rec, &mode);
		ext->link_virtual("Mode.on_enter", &Mode::on_enter, &mode);
		ext->link_virtual("Mode.on_connect_events", &Mode::on_connect_events, &mode);
		ext->link_virtual("Mode.on_leave", &Mode::on_leave, &mode);
		ext->link_virtual("Mode.on_command", &Mode::on_command, &mode);
		ext->link_virtual("Mode.on_update_menu", &Mode::on_update_menu, &mode);
		ext->link_virtual("Mode.on_mouse_move", &Mode::on_mouse_move, &mode);
		ext->link_virtual("Mode.on_mouse_leave", &Mode::on_mouse_leave, &mode);

		ext->link_virtual("Mode.on_left_button_down", &Mode::on_left_button_down, &mode);
		ext->link_virtual("Mode.on_left_button_up", &Mode::on_left_button_up, &mode);
		ext->link_virtual("Mode.on_key_down", &Mode::on_key_down, &mode);
		ext->link_virtual("Mode.on_prepare_scene", &Mode::on_prepare_scene, &mode);
		ext->link_virtual("Mode.on_draw_background", &Mode::on_draw_background, &mode);
		ext->link_virtual("Mode.on_draw_win", &Mode::on_draw_win, &mode);
		ext->link_virtual("Mode.on_draw_shadow", &Mode::on_draw_shadow, &mode);
		ext->link_virtual("Mode.on_draw_post", &Mode::on_draw_post, &mode);

		ext->link_virtual("Mode.get_parent", &Mode::get_parent, &mode);
		ext->link_virtual("Mode.get_filename", &Mode::get_filename, &mode);
		ext->link_virtual("Mode.is_save_state", &Mode::is_save_state, &mode);
		ext->link_virtual("Mode.is_undoable", &Mode::is_undoable, &mode);
		ext->link_virtual("Mode.is_redoable", &Mode::is_redoable, &mode);
	}

	{
		SubMode mode(nullptr);
		ext->declare_class_size("SubMode", sizeof(SubMode));
		ext->declare_class_element("SubMode.parent", &SubMode::_parent);
		ext->link_class_func("SubMode.__init__", &kaba::generic_init_ext<SubMode, Mode*>);
		ext->link_class_func("SubMode.request_mode_end", &SubMode::request_mode_end);

		ext->link_virtual("SubMode.get_parent", &SubMode::get_parent, &mode);

		ext->link_virtual("SubMode.on_command", &SubMode::on_command, &mode);
		ext->link_virtual("SubMode.on_update_menu", &SubMode::on_update_menu, &mode);
		ext->link_virtual("SubMode.on_connect_events_rec", &SubMode::on_connect_events_rec, &mode);

		ext->link_virtual("SubMode.on_prepare_scene", &SubMode::on_prepare_scene, &mode);
		ext->link_virtual("SubMode.on_draw_background", &SubMode::on_draw_background, &mode);
		ext->link_virtual("SubMode.on_draw_win", &SubMode::on_draw_win, &mode);
		ext->link_virtual("SubMode.on_draw_shadow", &SubMode::on_draw_shadow, &mode);
	}
}

void link_common(kaba::ExternalLinkData* ext) {
	{
		class XX: public history::Data {
		public:
			XX() : Data(0) {}
			void reset() override {}
		};
		XX data;
		ext->declare_class_size("Data", sizeof(history::Data));
		ext->declare_class_element("Data.filename", &history::Data::filename);
		ext->declare_class_element("Data.file_time", &history::Data::file_time);
		ext->declare_class_element("Data.type", &history::Data::type);
		ext->declare_class_element("Data.out_changed", &history::Data::out_changed);
		ext->link_class_func("Data.__init__", &kaba::generic_init<XX>);
		ext->link_class_func("Data.begin_action", &history::Data::begin_action_group);
		ext->link_class_func("Data.end_action", &history::Data::end_action_group);
		ext->link_class_func("Data.undo", &history::Data::undo);
		ext->link_class_func("Data.redo", &history::Data::redo);
		ext->link_class_func("Data.undoable", &history::Data::undoable);
		ext->link_class_func("Data.redoable", &history::Data::redoable);
		ext->link_class_func("Data.reset_history", &history::Data::reset_history);
		ext->link_virtual("Data.__delete__", &kaba::generic_virtual<XX>::__delete__, &data);
		ext->link_virtual("Data.reset", &XX::reset, &data);
	}



	ext->declare_class_size("DrawingHelper", sizeof(DrawingHelper));


	ext->declare_class_size("ResourceManager", sizeof(ResourceManager));
	ext->link_class_func("ResourceManager.is_from_file:ResourceManager:Terrain*", (bool(ResourceManager::*)(const Terrain*) const)&ResourceManager::is_from_file);
	ext->link_class_func("ResourceManager.is_from_file:ResourceManager:Material*", (bool(ResourceManager::*)(const yrenderer::Material*) const)&ResourceManager::is_from_file);
	ext->link_class_func("ResourceManager.filename:ResourceManager:Terrain*", (Path(ResourceManager::*)(const Terrain*) const)&ResourceManager::filename);
	ext->link_class_func("ResourceManager.filename:ResourceManager:Material*", (Path(ResourceManager::*)(const yrenderer::Material*) const)&ResourceManager::filename);


	ext->declare_enum("DataType.WORLD", FD_WORLD);
	ext->declare_enum("DataType.MATERIAL", FD_MATERIAL);
	ext->declare_enum("DataType.MODEL", FD_MODEL);
	ext->declare_enum("DataType.TEMPLATE", FD_TEMPLATE);
	ext->declare_enum("DataType.TERRAIN", FD_TERRAIN);
	ext->declare_enum("DataType.TEXTURE", FD_TEXTURE);
	ext->declare_enum("DataType.SCRIPT", FD_SCRIPT);
	ext->declare_enum("DataType.SHADER", FD_SHADERFILE);
	ext->declare_enum("DataType.SOUND", FD_SOUND);
	ext->declare_enum("DataType.PROJECT", FD_PROJECT);
	ext->declare_enum("DataType.FILE", FD_FILE);
}

void link_session(kaba::ExternalLinkData* ext) {
	ext->link("cur_session", &cur_session);


	ext->declare_class_size("ComplexPath", sizeof(ComplexPath));
	ext->declare_class_element("ComplexPath.complete", &ComplexPath::complete);
	ext->declare_class_element("ComplexPath.relative", &ComplexPath::relative);
	ext->declare_class_element("ComplexPath.simple", &ComplexPath::simple);
	ext->declare_class_element("ComplexPath.type", &ComplexPath::kind);


	ext->declare_class_size("Storage", sizeof(Storage));
	ext->link_class_func("Storage.load", &Storage::load);
	ext->link_class_func("Storage.save", &Storage::save);
	ext->link_class_func("Storage.save_as", &Storage::save_as);
	ext->link_class_func("Storage.auto_save", &Storage::auto_save);
	ext->link_class_func("Storage.open", &Storage::open);
	ext->link_class_func("Storage.file_dialog", &Storage::file_dialog);


	ext->declare_class_size("Session", sizeof(Session));
	ext->declare_class_element("Session.ctx", &Session::ctx);
	ext->declare_class_element("Session.drawing_helper", &Session::drawing_helper);
	ext->declare_class_element("Session.line_helper", &Session::line_helper);
	ext->declare_class_element("Session.storage", &Session::storage);
	ext->declare_class_element("Session.win", &Session::win);
	ext->declare_class_element("Session.cur_doc", &Session::cur_doc);
	ext->link_class_func("Session.load_project", &Session::load_project);
	ext->link_class_func("Session.universal_new", &Session::universal_new);
	ext->link_class_func("Session.universal_open", &Session::universal_open);
	ext->link_class_func("Session.universal_edit", &Session::universal_edit);
	ext->link_class_func("Session.info", &Session::info);
	ext->link_class_func("Session.warning", &Session::warning);
	ext->link_class_func("Session.error", &Session::error);
	ext->link("Session.guess_root_directory", (void*)&Session::guess_root_directory);


	ext->declare_class_size("DocumentSession", sizeof(DocumentSession));
	ext->declare_class_element("DocumentSession.root_mode", &DocumentSession::root_mode);
	ext->declare_class_element("DocumentSession.session", &DocumentSession::session);
	ext->declare_class_element("DocumentSession.cur_mode", &DocumentSession::cur_mode);
	ext->link_class_func("DocumentSession.set_mode", &DocumentSession::set_mode);
	ext->link_class_func("DocumentSession.set_document_panel", &DocumentSession::set_document_panel);
	ext->link_class_func("DocumentSession.event", &DocumentSessionWrapper::event_kaba);

	ext->link("FloatDialog.ask", (void*)&FloatDialog::ask);
	ext->link("IntPairDialog.ask", (void*)&IntPairDialog::ask);
}

void PluginManager::link_plugins() {
	//GlobalMainWin = ed;

	auto ext = session->kaba_ctx->external.get();

	link_common(ext);
	link_mode(ext);
	link_session(ext);
	link_multiview(ext);
	link_mesh(ext);
	link_model(ext);
	link_world(ext);

	ext->link_class_func("future[f32].__delete__", &kaba::generic_delete<base::future<float>>);
	ext->link_class_func("future[f32].then", &kaba::KabaFuture<float>::kaba_then);
	ext->link_class_func("future[f32].then_or_fail", &kaba::KabaFuture<float>::kaba_then_or_fail);
	ext->link_class_func("future[IntPair].__delete__", &kaba::generic_delete<base::future<base::tuple<int,int>>>);
	ext->link_class_func("future[IntPair].then", &kaba::KabaFuture<base::tuple<int,int>>::kaba_then);
	ext->link_class_func("future[IntPair].then_or_fail", &kaba::KabaFuture<base::tuple<int,int>>::kaba_then_or_fail);
	ext->link_class_func("future[ComplexPath].__delete__", &kaba::generic_delete<base::future<ComplexPath>>);
	ext->link_class_func("future[ComplexPath].then", &kaba::KabaFuture<ComplexPath>::kaba_then);
	ext->link_class_func("future[ComplexPath].then_or_fail", &kaba::KabaFuture<ComplexPath>::kaba_then_or_fail);
}

}
