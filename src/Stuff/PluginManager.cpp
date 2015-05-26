/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../lib/script/script.h"
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
	Script::config.directory = "";
	try{
		Script::Script *s = Script::Load(filename);
		typedef void func_t();
		func_t *f = (func_t*)s->MatchFunction("main", "void", 0);
		if (f)
			f();
	}catch(Script::Exception &e){
		ed->errorBox(e.message);
	}

	Script::DeleteAllScripts(true, true);
}

#define _offsetof(t, x)	((int)(long)((char*)&(((t*)ppp)->x) - ppp))

HuiWindow *GlobalMainWin = ed;

void PluginManager::init()
{
	Script::Init();

	GlobalMainWin = ed;

	Script::LinkExternal("edward", &GlobalMainWin);
	Script::LinkExternal("data_model", &mode_model->data);
	Script::LinkExternal("data_world", &mode_world->data);

	Script::DeclareClassSize("Observable", sizeof(Observable));
	//Script::DeclareClassOffset("Observable", "observable_name", offsetof(Observable, observable_name));

	Script::DeclareClassSize("Data", sizeof(Data));
	Script::DeclareClassOffset("Data", "filename", offsetof(Data, filename));
	Script::DeclareClassOffset("Data", "file_time", offsetof(Data, file_time));
	Script::DeclareClassOffset("Data", "binary_file_format", offsetof(Data, binary_file_format));
	Script::DeclareClassOffset("Data", "type", offsetof(Data, type));
	Script::LinkExternal("Data.BeginActionGroup", Script::mf(&Data::beginActionGroup));
	Script::LinkExternal("Data.EndActionGroup", Script::mf(&Data::endActionGroup));

	Script::DeclareClassSize("MultiViewSingleData", sizeof(MultiView::SingleData));
	Script::DeclareClassOffset("MultiViewSingleData", "pos", offsetof(MultiView::SingleData, pos));
	Script::DeclareClassOffset("MultiViewSingleData", "view_stage", offsetof(MultiView::SingleData, view_stage));
	Script::DeclareClassOffset("MultiViewSingleData", "is_selected", offsetof(MultiView::SingleData, is_selected));
	Script::DeclareClassOffset("MultiViewSingleData", "m_delta", offsetof(MultiView::SingleData, m_delta));
	Script::DeclareClassOffset("MultiViewSingleData", "m_old", offsetof(MultiView::SingleData, m_old));
	Script::DeclareClassOffset("MultiViewSingleData", "is_special", offsetof(MultiView::SingleData, is_special));

	// model

	Script::DeclareClassSize("ModelEffect", sizeof(ModelEffect));
	Script::DeclareClassOffset("ModelEffect", "Kind", offsetof(ModelEffect, type));
	Script::DeclareClassOffset("ModelEffect", "Surface", offsetof(ModelEffect, surface));
	Script::DeclareClassOffset("ModelEffect", "Vertex", offsetof(ModelEffect, vertex));
	Script::DeclareClassOffset("ModelEffect", "Size", offsetof(ModelEffect, size));
	Script::DeclareClassOffset("ModelEffect", "Speed", offsetof(ModelEffect, speed));
	Script::DeclareClassOffset("ModelEffect", "Intensity", offsetof(ModelEffect, intensity));
	Script::DeclareClassOffset("ModelEffect", "Colors", offsetof(ModelEffect, colors));
	Script::DeclareClassOffset("ModelEffect", "InvQuad", offsetof(ModelEffect, inv_quad));
	Script::DeclareClassOffset("ModelEffect", "File", offsetof(ModelEffect, file));

	Script::DeclareClassSize("ModelVertex", sizeof(ModelVertex));
	Script::DeclareClassOffset("ModelVertex", "NormalMode", offsetof(ModelVertex, normal_mode));
	Script::DeclareClassOffset("ModelVertex", "BoneIndex", offsetof(ModelVertex, bone_index));
	Script::DeclareClassOffset("ModelVertex", "NormalDirty", offsetof(ModelVertex, normal_dirty));
	Script::DeclareClassOffset("ModelVertex", "RefCount", offsetof(ModelVertex, ref_count));
	Script::DeclareClassOffset("ModelVertex", "Surface", offsetof(ModelVertex, surface));

	Script::DeclareClassSize("ModelPolygonSide", sizeof(ModelPolygonSide));
	Script::DeclareClassOffset("ModelPolygonSide", "Vertex", offsetof(ModelPolygonSide, vertex));
	Script::DeclareClassOffset("ModelPolygonSide", "Edge", offsetof(ModelPolygonSide, edge));
	Script::DeclareClassOffset("ModelPolygonSide", "EdgeDirection", offsetof(ModelPolygonSide, edge_direction));
	Script::DeclareClassOffset("ModelPolygonSide", "SkinVertex", offsetof(ModelPolygonSide, skin_vertex));
	Script::DeclareClassOffset("ModelPolygonSide", "NormalIndex", offsetof(ModelPolygonSide, normal_index));
	Script::DeclareClassOffset("ModelPolygonSide", "Normal", offsetof(ModelPolygonSide, normal));
	Script::DeclareClassOffset("ModelPolygonSide", "Triangulation", offsetof(ModelPolygonSide, triangulation));

	Script::DeclareClassSize("ModelPolygon", sizeof(ModelPolygon));
	Script::DeclareClassOffset("ModelPolygon", "Side", offsetof(ModelPolygon, side));
	Script::DeclareClassOffset("ModelPolygon", "TempNormal", offsetof(ModelPolygon, temp_normal));
	Script::DeclareClassOffset("ModelPolygon", "NormalDirty", offsetof(ModelPolygon, normal_dirty));
	Script::DeclareClassOffset("ModelPolygon", "TriangulationDirty", offsetof(ModelPolygon, triangulation_dirty));
	Script::DeclareClassOffset("ModelPolygon", "Material", offsetof(ModelPolygon, material));

	Script::DeclareClassSize("ModelSurface", sizeof(ModelSurface));
	Script::DeclareClassOffset("ModelSurface", "Polygon", offsetof(ModelSurface, polygon));
	Script::DeclareClassOffset("ModelSurface", "Edge", offsetof(ModelSurface, edge));
	Script::DeclareClassOffset("ModelSurface", "Vertex", offsetof(ModelSurface, vertex));
	Script::DeclareClassOffset("ModelSurface", "IsClosed", offsetof(ModelSurface, is_closed));
	Script::DeclareClassOffset("ModelSurface", "IsPhysical", offsetof(ModelSurface, is_physical));
	Script::DeclareClassOffset("ModelSurface", "IsVisible", offsetof(ModelSurface, is_visible));
	Script::DeclareClassOffset("ModelSurface", "model", offsetof(ModelSurface, model));

	Script::DeclareClassSize("ModelBone", sizeof(ModelBone));
	Script::DeclareClassOffset("ModelBone", "Parent", offsetof(ModelBone, parent));
	Script::DeclareClassOffset("ModelBone", "ModelFile", offsetof(ModelBone, model_file));
	Script::DeclareClassOffset("ModelBone", "model", offsetof(ModelBone, model));
	Script::DeclareClassOffset("ModelBone", "ConstPos", offsetof(ModelBone, const_pos));
	Script::DeclareClassOffset("ModelBone", "Matrix", offsetof(ModelBone, _matrix));

	Script::DeclareClassSize("ModelMove", sizeof(ModelMove));
	Script::DeclareClassOffset("ModelMove", "Name", offsetof(ModelMove, name));
	Script::DeclareClassOffset("ModelMove", "Type", offsetof(ModelMove, type));
	Script::DeclareClassOffset("ModelMove", "FramesPerSecConst", offsetof(ModelMove, frames_per_sec_const));
	Script::DeclareClassOffset("ModelMove", "FramesPerSecFactor", offsetof(ModelMove, frames_per_sec_factor));
	Script::DeclareClassOffset("ModelMove", "Frame", offsetof(ModelMove, frame));

	Script::DeclareClassSize("ModelFrame", sizeof(ModelFrame));
	Script::DeclareClassOffset("ModelFrame", "SkelAng", offsetof(ModelFrame, skel_ang));
	Script::DeclareClassOffset("ModelFrame", "SkelDPos", offsetof(ModelFrame, skel_dpos));
	Script::DeclareClassOffset("ModelFrame", "DPos", offsetof(ModelFrame, skin));
	Script::DeclareClassOffset("ModelFrame", "VertexDPos", offsetof(ModelFrame, vertex_dpos));

	Script::DeclareClassSize("DataModel", sizeof(DataModel));
	Script::DeclareClassOffset("DataModel", "Bone", offsetof(DataModel, bone));
	Script::DeclareClassOffset("DataModel", "Move", offsetof(DataModel, move));
	//Script::DeclareClassOffset("DataModel", "move", offsetof(DataModel, move));
	//Script::DeclareClassOffset("DataModel", "CurrentMove", offsetof(DataModel, CurrentMove));
	//Script::DeclareClassOffset("DataModel", "CurrentFrame", offsetof(DataModel, CurrentFrame));
	Script::DeclareClassOffset("DataModel", "Vertex", offsetof(DataModel, vertex));
	Script::DeclareClassOffset("DataModel", "Surface", offsetof(DataModel, surface));
	Script::DeclareClassOffset("DataModel", "Ball", offsetof(DataModel, ball));
	Script::DeclareClassOffset("DataModel", "Poly", offsetof(DataModel, poly));
	Script::DeclareClassOffset("DataModel", "Material", offsetof(DataModel, material));
	Script::DeclareClassOffset("DataModel", "Fx", offsetof(DataModel, fx));
	Script::DeclareClassOffset("DataModel", "meta_data", offsetof(DataModel, meta_data));
	Script::DeclareClassOffset("DataModel", "Min", offsetof(DataModel, _min));
	Script::DeclareClassOffset("DataModel", "Max", offsetof(DataModel, _max));
	//Script::DeclareClassOffset("DataModel", "CurrentMaterial", offsetof(DataModel, CurrentMaterial));
	//Script::DeclareClassOffset("DataModel", "CurrentTextureLevel", offsetof(DataModel, CurrentTextureLevel));
	//Script::DeclareClassOffset("DataModel", "SkinVertex", offsetof(DataModel, SkinVertex));
	//Script::DeclareClassOffset("DataModel", "SkinVertMat", offsetof(DataModel, SkinVertMat));
	/*Script::DeclareClassOffset("DataModel", "SkinVertTL", offsetof(DataModel, SkinVertTL));
	Script::DeclareClassOffset("DataModel", "Playing", offsetof(DataModel, Playing));
	Script::DeclareClassOffset("DataModel", "PlayLoop", offsetof(DataModel, PlayLoop));
	Script::DeclareClassOffset("DataModel", "TimeScale", offsetof(DataModel, TimeScale));
	Script::DeclareClassOffset("DataModel", "TimeParam", offsetof(DataModel, TimeParam));
	Script::DeclareClassOffset("DataModel", "SimFrame", offsetof(DataModel, SimFrame));*/
	Script::LinkExternal("DataModel.ClearSelection", Script::mf(&DataModel::clearSelection));
	Script::LinkExternal("DataModel.InvertSelection", Script::mf(&DataModel::invertSelection));
	Script::LinkExternal("DataModel.SelectionFromVertices", Script::mf(&DataModel::selectionFromVertices));
	Script::LinkExternal("DataModel.SelectionFromPolygons", Script::mf(&DataModel::selectionFromPolygons));
	Script::LinkExternal("DataModel.SelectionFromSurfaces", Script::mf(&DataModel::selectionFromSurfaces));
	Script::LinkExternal("DataModel.SelectOnlySurface", Script::mf(&DataModel::selectOnlySurface));
	Script::LinkExternal("DataModel.AddVertex", Script::mf(&DataModel::addVertex));
	Script::LinkExternal("DataModel.AddTriangle", Script::mf(&DataModel::addTriangle));
	Script::LinkExternal("DataModel.AddPolygon", Script::mf(&DataModel::addPolygon));
	Script::LinkExternal("DataModel.DeleteSelection", Script::mf(&DataModel::deleteSelection));
	Script::LinkExternal("DataModel.SubtractSelection", Script::mf(&DataModel::subtractSelection));
	Script::LinkExternal("DataModel.ConvertSelectionToTriangles", Script::mf(&DataModel::convertSelectionToTriangles));
	Script::LinkExternal("DataModel.TriangulateSelectedVertices", Script::mf(&DataModel::triangulateSelectedVertices));
	Script::LinkExternal("DataModel.BevelSelectedVertices", Script::mf(&DataModel::bevelSelectedEdges));
	Script::LinkExternal("DataModel.ExtrudeSelectedPolygons", Script::mf(&DataModel::extrudeSelectedPolygons));
	Script::LinkExternal("DataModel.PasteGeometry", Script::mf(&DataModel::pasteGeometry));



	Script::DeclareClassSize("Geometry", sizeof(Geometry));
	Script::LinkExternal("GeometryBall.__init__", Script::mf(&GeometryBall::__init__));
	Script::LinkExternal("GeometryCube.__init__", Script::mf(&GeometryCube::__init__));
	Script::LinkExternal("GeometrySphere.__init__", Script::mf(&GeometrySphere::__init__));
	Script::LinkExternal("GeometryCylinder.__init__", Script::mf(&GeometryCylinder::__init__));
	Script::LinkExternal("GeometryCylinderComplex.__init__", Script::mf(&GeometryCylinder::__init2__));
	Script::LinkExternal("GeometryTorus.__init__", Script::mf(&GeometryTorus::__init__));
	Script::LinkExternal("GeometryTorusKnot.__init__", Script::mf(&GeometryTorusKnot::__init__));
	Script::LinkExternal("GeometryPlane.__init__", Script::mf(&GeometryPlane::__init__));
	Script::LinkExternal("GeometryPlatonic.__init__", Script::mf(&GeometryPlatonic::__init__));
	Script::LinkExternal("GeometryTeapot.__init__", Script::mf(&GeometryTeapot::__init__));
	Script::LinkExternal("GeometrySubtract", Script::mf(&GeometrySubtract));
	Script::LinkExternal("GeometryAnd", Script::mf(&GeometryAnd));

	// world

	Script::DeclareClassSize("DataWorld", sizeof(DataWorld));
	Script::DeclareClassOffset("DataWorld", "Terrains", offsetof(DataWorld, Terrains));
	Script::DeclareClassOffset("DataWorld", "Objects", offsetof(DataWorld, Objects));
	Script::DeclareClassOffset("DataWorld", "EgoIndex", offsetof(DataWorld, EgoIndex));
	Script::LinkExternal("DataWorld.AddObject", Script::mf(&DataWorld::AddObject));
	Script::LinkExternal("DataWorld.AddTerrain", Script::mf(&DataWorld::AddTerrain));
	Script::LinkExternal("DataWorld.AddNewTerrain", Script::mf(&DataWorld::AddNewTerrain));
}


