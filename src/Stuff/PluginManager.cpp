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
	Init();
}

PluginManager::~PluginManager()
{
}

void PluginManager::Execute(const string & filename)
{
	Script::config.Directory = "";
	try{
		Script::Script *s = Script::Load(filename);
		s->Execute();
	}catch(Script::Exception &e){
		ed->ErrorBox(e.message);
	}

	Script::DeleteAllScripts(true, true);
}

#define _offsetof(t, x)	((int)(long)((char*)&(((t*)ppp)->x) - ppp))

HuiWindow *GlobalMainWin = ed;

void PluginManager::Init()
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
	Script::LinkExternal("Data.BeginActionGroup", Script::mf(&Data::BeginActionGroup));
	Script::LinkExternal("Data.EndActionGroup", Script::mf(&Data::EndActionGroup));

	Script::DeclareClassSize("MultiViewSingleData", sizeof(MultiView::SingleData));
	Script::DeclareClassOffset("MultiViewSingleData", "pos", offsetof(MultiView::SingleData, pos));
	Script::DeclareClassOffset("MultiViewSingleData", "view_stage", offsetof(MultiView::SingleData, view_stage));
	Script::DeclareClassOffset("MultiViewSingleData", "is_selected", offsetof(MultiView::SingleData, is_selected));
	Script::DeclareClassOffset("MultiViewSingleData", "m_delta", offsetof(MultiView::SingleData, m_delta));
	Script::DeclareClassOffset("MultiViewSingleData", "m_old", offsetof(MultiView::SingleData, m_old));
	Script::DeclareClassOffset("MultiViewSingleData", "is_special", offsetof(MultiView::SingleData, is_special));

	// model

	Script::DeclareClassSize("ModelEffect", sizeof(ModelEffect));
	Script::DeclareClassOffset("ModelEffect", "Kind", offsetof(ModelEffect, Kind));
	Script::DeclareClassOffset("ModelEffect", "Surface", offsetof(ModelEffect, Surface));
	Script::DeclareClassOffset("ModelEffect", "Vertex", offsetof(ModelEffect, Vertex));
	Script::DeclareClassOffset("ModelEffect", "Size", offsetof(ModelEffect, Size));
	Script::DeclareClassOffset("ModelEffect", "Speed", offsetof(ModelEffect, Speed));
	Script::DeclareClassOffset("ModelEffect", "Intensity", offsetof(ModelEffect, Intensity));
	Script::DeclareClassOffset("ModelEffect", "Colors", offsetof(ModelEffect, Colors));
	Script::DeclareClassOffset("ModelEffect", "InvQuad", offsetof(ModelEffect, InvQuad));
	Script::DeclareClassOffset("ModelEffect", "File", offsetof(ModelEffect, File));

	Script::DeclareClassSize("ModelVertex", sizeof(ModelVertex));
	Script::DeclareClassOffset("ModelVertex", "NormalMode", offsetof(ModelVertex, NormalMode));
	Script::DeclareClassOffset("ModelVertex", "BoneIndex", offsetof(ModelVertex, BoneIndex));
	Script::DeclareClassOffset("ModelVertex", "NormalDirty", offsetof(ModelVertex, NormalDirty));
	Script::DeclareClassOffset("ModelVertex", "RefCount", offsetof(ModelVertex, RefCount));
	Script::DeclareClassOffset("ModelVertex", "Surface", offsetof(ModelVertex, Surface));

	Script::DeclareClassSize("ModelPolygonSide", sizeof(ModelPolygonSide));
	Script::DeclareClassOffset("ModelPolygonSide", "Vertex", offsetof(ModelPolygonSide, Vertex));
	Script::DeclareClassOffset("ModelPolygonSide", "Edge", offsetof(ModelPolygonSide, Edge));
	Script::DeclareClassOffset("ModelPolygonSide", "EdgeDirection", offsetof(ModelPolygonSide, EdgeDirection));
	Script::DeclareClassOffset("ModelPolygonSide", "SkinVertex", offsetof(ModelPolygonSide, SkinVertex));
	Script::DeclareClassOffset("ModelPolygonSide", "NormalIndex", offsetof(ModelPolygonSide, NormalIndex));
	Script::DeclareClassOffset("ModelPolygonSide", "Normal", offsetof(ModelPolygonSide, Normal));
	Script::DeclareClassOffset("ModelPolygonSide", "Triangulation", offsetof(ModelPolygonSide, Triangulation));

	Script::DeclareClassSize("ModelPolygon", sizeof(ModelPolygon));
	Script::DeclareClassOffset("ModelPolygon", "Side", offsetof(ModelPolygon, Side));
	Script::DeclareClassOffset("ModelPolygon", "TempNormal", offsetof(ModelPolygon, TempNormal));
	Script::DeclareClassOffset("ModelPolygon", "NormalDirty", offsetof(ModelPolygon, NormalDirty));
	Script::DeclareClassOffset("ModelPolygon", "TriangulationDirty", offsetof(ModelPolygon, TriangulationDirty));
	Script::DeclareClassOffset("ModelPolygon", "Material", offsetof(ModelPolygon, Material));

	Script::DeclareClassSize("ModelSurface", sizeof(ModelSurface));
	Script::DeclareClassOffset("ModelSurface", "Polygon", offsetof(ModelSurface, Polygon));
	Script::DeclareClassOffset("ModelSurface", "Edge", offsetof(ModelSurface, Edge));
	Script::DeclareClassOffset("ModelSurface", "Vertex", offsetof(ModelSurface, Vertex));
	Script::DeclareClassOffset("ModelSurface", "IsClosed", offsetof(ModelSurface, IsClosed));
	Script::DeclareClassOffset("ModelSurface", "IsPhysical", offsetof(ModelSurface, IsPhysical));
	Script::DeclareClassOffset("ModelSurface", "IsVisible", offsetof(ModelSurface, IsVisible));
	Script::DeclareClassOffset("ModelSurface", "model", offsetof(ModelSurface, model));

	Script::DeclareClassSize("DataModel", sizeof(DataModel));
	Script::DeclareClassOffset("DataModel", "Bone", offsetof(DataModel, Bone));
	Script::DeclareClassOffset("DataModel", "Move", offsetof(DataModel, Move));
	//Script::DeclareClassOffset("DataModel", "move", offsetof(DataModel, move));
	//Script::DeclareClassOffset("DataModel", "CurrentMove", offsetof(DataModel, CurrentMove));
	//Script::DeclareClassOffset("DataModel", "CurrentFrame", offsetof(DataModel, CurrentFrame));
	Script::DeclareClassOffset("DataModel", "Vertex", offsetof(DataModel, Vertex));
	Script::DeclareClassOffset("DataModel", "Surface", offsetof(DataModel, Surface));
	Script::DeclareClassOffset("DataModel", "Ball", offsetof(DataModel, Ball));
	Script::DeclareClassOffset("DataModel", "Poly", offsetof(DataModel, Poly));
	Script::DeclareClassOffset("DataModel", "Material", offsetof(DataModel, Material));
	Script::DeclareClassOffset("DataModel", "Fx", offsetof(DataModel, Fx));
	Script::DeclareClassOffset("DataModel", "meta_data", offsetof(DataModel, meta_data));
	Script::DeclareClassOffset("DataModel", "Min", offsetof(DataModel, Min));
	Script::DeclareClassOffset("DataModel", "Max", offsetof(DataModel, Max));
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
	Script::LinkExternal("DataModel.ClearSelection", Script::mf(&DataModel::ClearSelection));
	Script::LinkExternal("DataModel.InvertSelection", Script::mf(&DataModel::InvertSelection));
	Script::LinkExternal("DataModel.SelectionFromVertices", Script::mf(&DataModel::SelectionFromVertices));
	Script::LinkExternal("DataModel.SelectionFromPolygons", Script::mf(&DataModel::SelectionFromPolygons));
	Script::LinkExternal("DataModel.SelectionFromSurfaces", Script::mf(&DataModel::SelectionFromSurfaces));
	Script::LinkExternal("DataModel.SelectOnlySurface", Script::mf(&DataModel::SelectOnlySurface));
	Script::LinkExternal("DataModel.AddVertex", Script::mf(&DataModel::AddVertex));
	Script::LinkExternal("DataModel.AddTriangle", Script::mf(&DataModel::AddTriangle));
	Script::LinkExternal("DataModel.AddPolygon", Script::mf(&DataModel::AddPolygon));
	Script::LinkExternal("DataModel.DeleteSelection", Script::mf(&DataModel::DeleteSelection));
	Script::LinkExternal("DataModel.SubtractSelection", Script::mf(&DataModel::SubtractSelection));
	Script::LinkExternal("DataModel.ConvertSelectionToTriangles", Script::mf(&DataModel::ConvertSelectionToTriangles));
	Script::LinkExternal("DataModel.TriangulateSelectedVertices", Script::mf(&DataModel::TriangulateSelectedVertices));
	Script::LinkExternal("DataModel.BevelSelectedVertices", Script::mf(&DataModel::BevelSelectedEdges));
	Script::LinkExternal("DataModel.ExtrudeSelectedPolygons", Script::mf(&DataModel::ExtrudeSelectedPolygons));
	Script::LinkExternal("DataModel.PasteGeometry", Script::mf(&DataModel::PasteGeometry));



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


