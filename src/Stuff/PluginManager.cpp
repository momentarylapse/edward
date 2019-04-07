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
#include "../Mode/Model/ModeModel.h"
#include "../Mode/World/ModeWorld.h"
#include "../Data/Model/Geometry/GeometryBall.h"
#include "../Data/Model/Geometry/GeometryCube.h"
#include "../Data/Model/Geometry/GeometryCylinder.h"
#include "../Data/Model/Geometry/GeometryPlane.h"
#include "../Data/Model/Geometry/GeometryPlatonic.h"
#include "../Data/Model/Geometry/GeometrySphere.h"
#include "../Data/Model/Geometry/GeometryTeapot.h"
#include "../Data/Model/Geometry/GeometryTorus.h"
#include "../Data/Model/Geometry/GeometryTorusKnot.h"

PluginManager::PluginManager()
{
	init();
}

PluginManager::~PluginManager()
{
}

void PluginManager::execute(const string & filename)
{
	Kaba::config.directory = "";
	try{
		Kaba::Script *s = Kaba::Load(filename);
		typedef void func_t();
		func_t *f = (func_t*)s->match_function("main", "void", {});
		if (f)
			f();
	}catch(Kaba::Exception &e){
		ed->error_box(e.message());
	}

	Kaba::DeleteAllScripts(true, true);
}

#define _offsetof(t, x)	((int)(long)((char*)&(((t*)ppp)->x) - ppp))

hui::Window *GlobalMainWin = ed;

void PluginManager::init()
{
	Kaba::Init();

	GlobalMainWin = ed;

	Kaba::LinkExternal("edward", &GlobalMainWin);
	Kaba::LinkExternal("data_model", &mode_model->data);
	Kaba::LinkExternal("data_world", &mode_world->data);

	Kaba::DeclareClassSize("Observable", sizeof(Observable));
	//Kaba::DeclareClassOffset("Observable", "observable_name", offsetof(Observable, observable_name));

	Kaba::DeclareClassSize("Data", sizeof(Data));
	Kaba::DeclareClassOffset("Data", "filename", offsetof(Data, filename));
	Kaba::DeclareClassOffset("Data", "file_time", offsetof(Data, file_time));
	Kaba::DeclareClassOffset("Data", "binary_file_format", offsetof(Data, binary_file_format));
	Kaba::DeclareClassOffset("Data", "type", offsetof(Data, type));
	Kaba::LinkExternal("Data.BeginActionGroup", Kaba::mf(&Data::begin_action_group));
	Kaba::LinkExternal("Data.EndActionGroup", Kaba::mf(&Data::end_action_group));

	Kaba::DeclareClassSize("MultiViewSingleData", sizeof(MultiView::SingleData));
	Kaba::DeclareClassOffset("MultiViewSingleData", "pos", offsetof(MultiView::SingleData, pos));
	Kaba::DeclareClassOffset("MultiViewSingleData", "view_stage", offsetof(MultiView::SingleData, view_stage));
	Kaba::DeclareClassOffset("MultiViewSingleData", "is_selected", offsetof(MultiView::SingleData, is_selected));
	Kaba::DeclareClassOffset("MultiViewSingleData", "m_delta", offsetof(MultiView::SingleData, m_delta));
	Kaba::DeclareClassOffset("MultiViewSingleData", "m_old", offsetof(MultiView::SingleData, m_old));
	Kaba::DeclareClassOffset("MultiViewSingleData", "is_special", offsetof(MultiView::SingleData, is_special));

	// model

	Kaba::DeclareClassSize("ModelEffect", sizeof(ModelEffect));
	Kaba::DeclareClassOffset("ModelEffect", "kind", offsetof(ModelEffect, type));
	Kaba::DeclareClassOffset("ModelEffect", "surface", offsetof(ModelEffect, surface));
	Kaba::DeclareClassOffset("ModelEffect", "vertex", offsetof(ModelEffect, vertex));
	Kaba::DeclareClassOffset("ModelEffect", "size", offsetof(ModelEffect, size));
	Kaba::DeclareClassOffset("ModelEffect", "speed", offsetof(ModelEffect, speed));
	Kaba::DeclareClassOffset("ModelEffect", "intensity", offsetof(ModelEffect, intensity));
	Kaba::DeclareClassOffset("ModelEffect", "colors", offsetof(ModelEffect, colors));
	Kaba::DeclareClassOffset("ModelEffect", "inv_quad", offsetof(ModelEffect, inv_quad));
	Kaba::DeclareClassOffset("ModelEffect", "file", offsetof(ModelEffect, file));

	Kaba::DeclareClassSize("ModelVertex", sizeof(ModelVertex));
	Kaba::DeclareClassOffset("ModelVertex", "normal_mode", offsetof(ModelVertex, normal_mode));
	Kaba::DeclareClassOffset("ModelVertex", "bone_index", offsetof(ModelVertex, bone_index));
	Kaba::DeclareClassOffset("ModelVertex", "normal_dirty", offsetof(ModelVertex, normal_dirty));
	Kaba::DeclareClassOffset("ModelVertex", "ref_count", offsetof(ModelVertex, ref_count));
	Kaba::DeclareClassOffset("ModelVertex", "surface", offsetof(ModelVertex, surface));

	Kaba::DeclareClassSize("ModelPolygonSide", sizeof(ModelPolygonSide));
	Kaba::DeclareClassOffset("ModelPolygonSide", "vertex", offsetof(ModelPolygonSide, vertex));
	Kaba::DeclareClassOffset("ModelPolygonSide", "edge", offsetof(ModelPolygonSide, edge));
	Kaba::DeclareClassOffset("ModelPolygonSide", "edge_direction", offsetof(ModelPolygonSide, edge_direction));
	Kaba::DeclareClassOffset("ModelPolygonSide", "skin_vertex", offsetof(ModelPolygonSide, skin_vertex));
	Kaba::DeclareClassOffset("ModelPolygonSide", "normal_index", offsetof(ModelPolygonSide, normal_index));
	Kaba::DeclareClassOffset("ModelPolygonSide", "normal", offsetof(ModelPolygonSide, normal));
	Kaba::DeclareClassOffset("ModelPolygonSide", "triangulation", offsetof(ModelPolygonSide, triangulation));

	Kaba::DeclareClassSize("ModelPolygon", sizeof(ModelPolygon));
	Kaba::DeclareClassOffset("ModelPolygon", "side", offsetof(ModelPolygon, side));
	Kaba::DeclareClassOffset("ModelPolygon", "temp_normal", offsetof(ModelPolygon, temp_normal));
	Kaba::DeclareClassOffset("ModelPolygon", "normal_dirty", offsetof(ModelPolygon, normal_dirty));
	Kaba::DeclareClassOffset("ModelPolygon", "triangulation_dirty", offsetof(ModelPolygon, triangulation_dirty));
	Kaba::DeclareClassOffset("ModelPolygon", "material", offsetof(ModelPolygon, material));

	Kaba::DeclareClassSize("ModelSurface", sizeof(ModelSurface));
	Kaba::DeclareClassOffset("ModelSurface", "polygon", offsetof(ModelSurface, polygon));
	Kaba::DeclareClassOffset("ModelSurface", "edge", offsetof(ModelSurface, edge));
	Kaba::DeclareClassOffset("ModelSurface", "vertex", offsetof(ModelSurface, vertex));
	Kaba::DeclareClassOffset("ModelSurface", "is_closed", offsetof(ModelSurface, is_closed));
	Kaba::DeclareClassOffset("ModelSurface", "is_physical", offsetof(ModelSurface, is_physical));
	Kaba::DeclareClassOffset("ModelSurface", "is_visible", offsetof(ModelSurface, is_visible));
	Kaba::DeclareClassOffset("ModelSurface", "model", offsetof(ModelSurface, model));

	Kaba::DeclareClassSize("ModelBone", sizeof(ModelBone));
	Kaba::DeclareClassOffset("ModelBone", "parent", offsetof(ModelBone, parent));
	Kaba::DeclareClassOffset("ModelBone", "model_file", offsetof(ModelBone, model_file));
	Kaba::DeclareClassOffset("ModelBone", "model", offsetof(ModelBone, model));
	Kaba::DeclareClassOffset("ModelBone", "const_pos", offsetof(ModelBone, const_pos));
	Kaba::DeclareClassOffset("ModelBone", "matrix", offsetof(ModelBone, _matrix));

	Kaba::DeclareClassSize("ModelMove", sizeof(ModelMove));
	Kaba::DeclareClassOffset("ModelMove", "name", offsetof(ModelMove, name));
	Kaba::DeclareClassOffset("ModelMove", "type", offsetof(ModelMove, type));
	Kaba::DeclareClassOffset("ModelMove", "FramesPerSecConst", offsetof(ModelMove, frames_per_sec_const));
	Kaba::DeclareClassOffset("ModelMove", "FramesPerSecFactor", offsetof(ModelMove, frames_per_sec_factor));
	Kaba::DeclareClassOffset("ModelMove", "frame", offsetof(ModelMove, frame));

	Kaba::DeclareClassSize("ModelFrame", sizeof(ModelFrame));
	Kaba::DeclareClassOffset("ModelFrame", "skel_ang", offsetof(ModelFrame, skel_ang));
	Kaba::DeclareClassOffset("ModelFrame", "skel_dpos", offsetof(ModelFrame, skel_dpos));
	Kaba::DeclareClassOffset("ModelFrame", "dpos", offsetof(ModelFrame, skin));
	Kaba::DeclareClassOffset("ModelFrame", "vertex_dpos", offsetof(ModelFrame, vertex_dpos));

	Kaba::DeclareClassSize("DataModel", sizeof(DataModel));
	Kaba::DeclareClassOffset("DataModel", "bone", offsetof(DataModel, bone));
	Kaba::DeclareClassOffset("DataModel", "move", offsetof(DataModel, move));
	//Kaba::DeclareClassOffset("DataModel", "move", offsetof(DataModel, move));
	//Kaba::DeclareClassOffset("DataModel", "CurrentMove", offsetof(DataModel, CurrentMove));
	//Kaba::DeclareClassOffset("DataModel", "CurrentFrame", offsetof(DataModel, CurrentFrame));
	Kaba::DeclareClassOffset("DataModel", "vertex", offsetof(DataModel, vertex));
	Kaba::DeclareClassOffset("DataModel", "surface", offsetof(DataModel, surface));
	Kaba::DeclareClassOffset("DataModel", "ball", offsetof(DataModel, ball));
	Kaba::DeclareClassOffset("DataModel", "poly", offsetof(DataModel, poly));
	Kaba::DeclareClassOffset("DataModel", "material", offsetof(DataModel, material));
	Kaba::DeclareClassOffset("DataModel", "fx", offsetof(DataModel, fx));
	Kaba::DeclareClassOffset("DataModel", "meta_data", offsetof(DataModel, meta_data));
	Kaba::DeclareClassOffset("DataModel", "min", offsetof(DataModel, _min));
	Kaba::DeclareClassOffset("DataModel", "max", offsetof(DataModel, _max));
	//Kaba::DeclareClassOffset("DataModel", "CurrentMaterial", offsetof(DataModel, CurrentMaterial));
	//Kaba::DeclareClassOffset("DataModel", "CurrentTextureLevel", offsetof(DataModel, CurrentTextureLevel));
	//Kaba::DeclareClassOffset("DataModel", "SkinVertex", offsetof(DataModel, SkinVertex));
	//Kaba::DeclareClassOffset("DataModel", "SkinVertMat", offsetof(DataModel, SkinVertMat));
	/*Kaba::DeclareClassOffset("DataModel", "SkinVertTL", offsetof(DataModel, SkinVertTL));
	Kaba::DeclareClassOffset("DataModel", "Playing", offsetof(DataModel, Playing));
	Kaba::DeclareClassOffset("DataModel", "PlayLoop", offsetof(DataModel, PlayLoop));
	Kaba::DeclareClassOffset("DataModel", "TimeScale", offsetof(DataModel, TimeScale));
	Kaba::DeclareClassOffset("DataModel", "TimeParam", offsetof(DataModel, TimeParam));
	Kaba::DeclareClassOffset("DataModel", "SimFrame", offsetof(DataModel, SimFrame));*/
	Kaba::LinkExternal("DataModel.ClearSelection", Kaba::mf(&DataModel::clearSelection));
	Kaba::LinkExternal("DataModel.InvertSelection", Kaba::mf(&DataModel::invertSelection));
	Kaba::LinkExternal("DataModel.SelectionFromVertices", Kaba::mf(&DataModel::selectionFromVertices));
	Kaba::LinkExternal("DataModel.SelectionFromPolygons", Kaba::mf(&DataModel::selectionFromPolygons));
	Kaba::LinkExternal("DataModel.SelectionFromSurfaces", Kaba::mf(&DataModel::selectionFromSurfaces));
	Kaba::LinkExternal("DataModel.SelectOnlySurface", Kaba::mf(&DataModel::selectOnlySurface));
	Kaba::LinkExternal("DataModel.AddVertex", Kaba::mf(&DataModel::addVertex));
	Kaba::LinkExternal("DataModel.AddTriangle", Kaba::mf(&DataModel::addTriangle));
	Kaba::LinkExternal("DataModel.AddPolygon", Kaba::mf(&DataModel::addPolygon));
	Kaba::LinkExternal("DataModel.DeleteSelection", Kaba::mf(&DataModel::deleteSelection));
	Kaba::LinkExternal("DataModel.SubtractSelection", Kaba::mf(&DataModel::subtractSelection));
	Kaba::LinkExternal("DataModel.ConvertSelectionToTriangles", Kaba::mf(&DataModel::convertSelectionToTriangles));
	Kaba::LinkExternal("DataModel.TriangulateSelectedVertices", Kaba::mf(&DataModel::triangulateSelectedVertices));
	Kaba::LinkExternal("DataModel.BevelSelectedVertices", Kaba::mf(&DataModel::bevelSelectedEdges));
	Kaba::LinkExternal("DataModel.ExtrudeSelectedPolygons", Kaba::mf(&DataModel::extrudeSelectedPolygons));
	Kaba::LinkExternal("DataModel.PasteGeometry", Kaba::mf(&DataModel::pasteGeometry));
	Kaba::LinkExternal("DataModel.add_animation", Kaba::mf(&DataModel::addAnimation));
	Kaba::LinkExternal("DataModel.duplicate_animation", Kaba::mf(&DataModel::duplicateAnimation));
	Kaba::LinkExternal("DataModel.delete_animation", Kaba::mf(&DataModel::deleteAnimation));
	Kaba::LinkExternal("DataModel.animation_set_data", Kaba::mf(&DataModel::setAnimationData));
	Kaba::LinkExternal("DataModel.animation_add_frame", Kaba::mf(&DataModel::animationAddFrame));
	Kaba::LinkExternal("DataModel.animation_delete_frame", Kaba::mf(&DataModel::animationDeleteFrame));
	Kaba::LinkExternal("DataModel.animation_set_frame_duration", Kaba::mf(&DataModel::animationSetFrameDuration));
	Kaba::LinkExternal("DataModel.animation_set_bone", Kaba::mf(&DataModel::animationSetBone));


	Kaba::DeclareClassSize("Geometry", sizeof(Geometry));
	Kaba::LinkExternal("GeometryBall.__init__", Kaba::mf(&GeometryBall::__init__));
	Kaba::LinkExternal("GeometryCube.__init__", Kaba::mf(&GeometryCube::__init__));
	Kaba::LinkExternal("GeometrySphere.__init__", Kaba::mf(&GeometrySphere::__init__));
	Kaba::LinkExternal("GeometryCylinder.__init__", Kaba::mf(&GeometryCylinder::__init__));
	Kaba::LinkExternal("GeometryCylinderComplex.__init__", Kaba::mf(&GeometryCylinder::__init2__));
	Kaba::LinkExternal("GeometryTorus.__init__", Kaba::mf(&GeometryTorus::__init__));
	Kaba::LinkExternal("GeometryTorusKnot.__init__", Kaba::mf(&GeometryTorusKnot::__init__));
	Kaba::LinkExternal("GeometryPlane.__init__", Kaba::mf(&GeometryPlane::__init__));
	Kaba::LinkExternal("GeometryPlatonic.__init__", Kaba::mf(&GeometryPlatonic::__init__));
	Kaba::LinkExternal("GeometryTeapot.__init__", Kaba::mf(&GeometryTeapot::__init__));
	Kaba::LinkExternal("GeometrySubtract", Kaba::mf(&GeometrySubtract));
	Kaba::LinkExternal("GeometryAnd", Kaba::mf(&GeometryAnd));

	// world

	Kaba::DeclareClassSize("DataWorld", sizeof(DataWorld));
	Kaba::DeclareClassOffset("DataWorld", "terrains", offsetof(DataWorld, Terrains));
	Kaba::DeclareClassOffset("DataWorld", "objects", offsetof(DataWorld, Objects));
	Kaba::DeclareClassOffset("DataWorld", "ego_index", offsetof(DataWorld, EgoIndex));
	Kaba::LinkExternal("DataWorld.add_object", Kaba::mf(&DataWorld::AddObject));
	Kaba::LinkExternal("DataWorld.add_terrain", Kaba::mf(&DataWorld::AddTerrain));
	Kaba::LinkExternal("DataWorld.add_new_terrain", Kaba::mf(&DataWorld::AddNewTerrain));
}


