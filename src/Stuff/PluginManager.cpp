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
		func_t *f = (func_t*)s->MatchFunction("main", "void", 0);
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
	Kaba::DeclareClassOffset("ModelEffect", "Kind", offsetof(ModelEffect, type));
	Kaba::DeclareClassOffset("ModelEffect", "Surface", offsetof(ModelEffect, surface));
	Kaba::DeclareClassOffset("ModelEffect", "Vertex", offsetof(ModelEffect, vertex));
	Kaba::DeclareClassOffset("ModelEffect", "Size", offsetof(ModelEffect, size));
	Kaba::DeclareClassOffset("ModelEffect", "Speed", offsetof(ModelEffect, speed));
	Kaba::DeclareClassOffset("ModelEffect", "Intensity", offsetof(ModelEffect, intensity));
	Kaba::DeclareClassOffset("ModelEffect", "Colors", offsetof(ModelEffect, colors));
	Kaba::DeclareClassOffset("ModelEffect", "InvQuad", offsetof(ModelEffect, inv_quad));
	Kaba::DeclareClassOffset("ModelEffect", "File", offsetof(ModelEffect, file));

	Kaba::DeclareClassSize("ModelVertex", sizeof(ModelVertex));
	Kaba::DeclareClassOffset("ModelVertex", "NormalMode", offsetof(ModelVertex, normal_mode));
	Kaba::DeclareClassOffset("ModelVertex", "BoneIndex", offsetof(ModelVertex, bone_index));
	Kaba::DeclareClassOffset("ModelVertex", "NormalDirty", offsetof(ModelVertex, normal_dirty));
	Kaba::DeclareClassOffset("ModelVertex", "RefCount", offsetof(ModelVertex, ref_count));
	Kaba::DeclareClassOffset("ModelVertex", "Surface", offsetof(ModelVertex, surface));

	Kaba::DeclareClassSize("ModelPolygonSide", sizeof(ModelPolygonSide));
	Kaba::DeclareClassOffset("ModelPolygonSide", "Vertex", offsetof(ModelPolygonSide, vertex));
	Kaba::DeclareClassOffset("ModelPolygonSide", "Edge", offsetof(ModelPolygonSide, edge));
	Kaba::DeclareClassOffset("ModelPolygonSide", "EdgeDirection", offsetof(ModelPolygonSide, edge_direction));
	Kaba::DeclareClassOffset("ModelPolygonSide", "SkinVertex", offsetof(ModelPolygonSide, skin_vertex));
	Kaba::DeclareClassOffset("ModelPolygonSide", "NormalIndex", offsetof(ModelPolygonSide, normal_index));
	Kaba::DeclareClassOffset("ModelPolygonSide", "Normal", offsetof(ModelPolygonSide, normal));
	Kaba::DeclareClassOffset("ModelPolygonSide", "Triangulation", offsetof(ModelPolygonSide, triangulation));

	Kaba::DeclareClassSize("ModelPolygon", sizeof(ModelPolygon));
	Kaba::DeclareClassOffset("ModelPolygon", "Side", offsetof(ModelPolygon, side));
	Kaba::DeclareClassOffset("ModelPolygon", "TempNormal", offsetof(ModelPolygon, temp_normal));
	Kaba::DeclareClassOffset("ModelPolygon", "NormalDirty", offsetof(ModelPolygon, normal_dirty));
	Kaba::DeclareClassOffset("ModelPolygon", "TriangulationDirty", offsetof(ModelPolygon, triangulation_dirty));
	Kaba::DeclareClassOffset("ModelPolygon", "Material", offsetof(ModelPolygon, material));

	Kaba::DeclareClassSize("ModelSurface", sizeof(ModelSurface));
	Kaba::DeclareClassOffset("ModelSurface", "Polygon", offsetof(ModelSurface, polygon));
	Kaba::DeclareClassOffset("ModelSurface", "Edge", offsetof(ModelSurface, edge));
	Kaba::DeclareClassOffset("ModelSurface", "Vertex", offsetof(ModelSurface, vertex));
	Kaba::DeclareClassOffset("ModelSurface", "IsClosed", offsetof(ModelSurface, is_closed));
	Kaba::DeclareClassOffset("ModelSurface", "IsPhysical", offsetof(ModelSurface, is_physical));
	Kaba::DeclareClassOffset("ModelSurface", "IsVisible", offsetof(ModelSurface, is_visible));
	Kaba::DeclareClassOffset("ModelSurface", "model", offsetof(ModelSurface, model));

	Kaba::DeclareClassSize("ModelBone", sizeof(ModelBone));
	Kaba::DeclareClassOffset("ModelBone", "Parent", offsetof(ModelBone, parent));
	Kaba::DeclareClassOffset("ModelBone", "ModelFile", offsetof(ModelBone, model_file));
	Kaba::DeclareClassOffset("ModelBone", "model", offsetof(ModelBone, model));
	Kaba::DeclareClassOffset("ModelBone", "ConstPos", offsetof(ModelBone, const_pos));
	Kaba::DeclareClassOffset("ModelBone", "Matrix", offsetof(ModelBone, _matrix));

	Kaba::DeclareClassSize("ModelMove", sizeof(ModelMove));
	Kaba::DeclareClassOffset("ModelMove", "Name", offsetof(ModelMove, name));
	Kaba::DeclareClassOffset("ModelMove", "Type", offsetof(ModelMove, type));
	Kaba::DeclareClassOffset("ModelMove", "FramesPerSecConst", offsetof(ModelMove, frames_per_sec_const));
	Kaba::DeclareClassOffset("ModelMove", "FramesPerSecFactor", offsetof(ModelMove, frames_per_sec_factor));
	Kaba::DeclareClassOffset("ModelMove", "Frame", offsetof(ModelMove, frame));

	Kaba::DeclareClassSize("ModelFrame", sizeof(ModelFrame));
	Kaba::DeclareClassOffset("ModelFrame", "SkelAng", offsetof(ModelFrame, skel_ang));
	Kaba::DeclareClassOffset("ModelFrame", "SkelDPos", offsetof(ModelFrame, skel_dpos));
	Kaba::DeclareClassOffset("ModelFrame", "DPos", offsetof(ModelFrame, skin));
	Kaba::DeclareClassOffset("ModelFrame", "VertexDPos", offsetof(ModelFrame, vertex_dpos));

	Kaba::DeclareClassSize("DataModel", sizeof(DataModel));
	Kaba::DeclareClassOffset("DataModel", "Bone", offsetof(DataModel, bone));
	Kaba::DeclareClassOffset("DataModel", "Move", offsetof(DataModel, move));
	//Kaba::DeclareClassOffset("DataModel", "move", offsetof(DataModel, move));
	//Kaba::DeclareClassOffset("DataModel", "CurrentMove", offsetof(DataModel, CurrentMove));
	//Kaba::DeclareClassOffset("DataModel", "CurrentFrame", offsetof(DataModel, CurrentFrame));
	Kaba::DeclareClassOffset("DataModel", "Vertex", offsetof(DataModel, vertex));
	Kaba::DeclareClassOffset("DataModel", "Surface", offsetof(DataModel, surface));
	Kaba::DeclareClassOffset("DataModel", "Ball", offsetof(DataModel, ball));
	Kaba::DeclareClassOffset("DataModel", "Poly", offsetof(DataModel, poly));
	Kaba::DeclareClassOffset("DataModel", "Material", offsetof(DataModel, material));
	Kaba::DeclareClassOffset("DataModel", "Fx", offsetof(DataModel, fx));
	Kaba::DeclareClassOffset("DataModel", "meta_data", offsetof(DataModel, meta_data));
	Kaba::DeclareClassOffset("DataModel", "Min", offsetof(DataModel, _min));
	Kaba::DeclareClassOffset("DataModel", "Max", offsetof(DataModel, _max));
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
	Kaba::DeclareClassOffset("DataWorld", "Terrains", offsetof(DataWorld, Terrains));
	Kaba::DeclareClassOffset("DataWorld", "Objects", offsetof(DataWorld, Objects));
	Kaba::DeclareClassOffset("DataWorld", "EgoIndex", offsetof(DataWorld, EgoIndex));
	Kaba::LinkExternal("DataWorld.AddObject", Kaba::mf(&DataWorld::AddObject));
	Kaba::LinkExternal("DataWorld.AddTerrain", Kaba::mf(&DataWorld::AddTerrain));
	Kaba::LinkExternal("DataWorld.AddNewTerrain", Kaba::mf(&DataWorld::AddNewTerrain));
}


