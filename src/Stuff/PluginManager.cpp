/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../lib/script/script.h"
#include "../Edward.h"
#include "../Mode/Model/ModeModel.h"
#include "../Mode/World/ModeWorld.h"
#include "../Data/Model/Geometry/ModelGeometryBall.h"
#include "../Data/Model/Geometry/ModelGeometryCube.h"
#include "../Data/Model/Geometry/ModelGeometryCylinder.h"
#include "../Data/Model/Geometry/ModelGeometryPlane.h"
#include "../Data/Model/Geometry/ModelGeometryPlatonic.h"
#include "../Data/Model/Geometry/ModelGeometrySphere.h"
#include "../Data/Model/Geometry/ModelGeometryTeapot.h"
#include "../Data/Model/Geometry/ModelGeometryTorus.h"
#include "../Data/Model/Geometry/ModelGeometryTorusKnot.h"

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

void PluginManager::Init()
{
	Script::Init();

	Script::LinkExternal("edward", &ed);
	Script::LinkExternal("data_model", &mode_model->data);
	Script::LinkExternal("data_world", &mode_world->data);

	Script::DeclareClassSize("Observable", sizeof(Observable));
	//Script::DeclareClassOffset("Observable", "observable_name", offsetof(Observable, observable_name));

	Script::DeclareClassSize("Data", sizeof(Data));
	Script::DeclareClassOffset("Data", "filename", offsetof(Data, filename));
	Script::DeclareClassOffset("Data", "file_time", offsetof(Data, file_time));
	Script::DeclareClassOffset("Data", "binary_file_format", offsetof(Data, binary_file_format));
	Script::DeclareClassOffset("Data", "type", offsetof(Data, type));
	Script::LinkExternal("Data.BeginActionGroup", (void*)&Data::BeginActionGroup);
	Script::LinkExternal("Data.EndActionGroup", (void*)&Data::EndActionGroup);

	Script::DeclareClassSize("MultiViewSingleData", sizeof(MultiViewSingleData));
	Script::DeclareClassOffset("MultiViewSingleData", "pos", offsetof(MultiViewSingleData, pos));
	Script::DeclareClassOffset("MultiViewSingleData", "view_stage", offsetof(MultiViewSingleData, view_stage));
	Script::DeclareClassOffset("MultiViewSingleData", "is_selected", offsetof(MultiViewSingleData, is_selected));
	Script::DeclareClassOffset("MultiViewSingleData", "m_delta", offsetof(MultiViewSingleData, m_delta));
	Script::DeclareClassOffset("MultiViewSingleData", "m_old", offsetof(MultiViewSingleData, m_old));
	Script::DeclareClassOffset("MultiViewSingleData", "is_special", offsetof(MultiViewSingleData, is_special));

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
	Script::LinkExternal("DataModel.ClearSelection", (void*)&DataModel::ClearSelection);
	Script::LinkExternal("DataModel.InvertSelection", (void*)&DataModel::InvertSelection);
	Script::LinkExternal("DataModel.SelectionFromVertices", (void*)&DataModel::SelectionFromVertices);
	Script::LinkExternal("DataModel.SelectionFromPolygons", (void*)&DataModel::SelectionFromPolygons);
	Script::LinkExternal("DataModel.SelectionFromSurfaces", (void*)&DataModel::SelectionFromSurfaces);
	Script::LinkExternal("DataModel.SelectOnlySurface", (void*)&DataModel::SelectOnlySurface);
	Script::LinkExternal("DataModel.AddVertex", (void*)&DataModel::AddVertex);
	Script::LinkExternal("DataModel.AddTriangle", (void*)&DataModel::AddTriangle);
	Script::LinkExternal("DataModel.AddPolygon", (void*)&DataModel::AddPolygon);
	Script::LinkExternal("DataModel.DeleteSelection", (void*)&DataModel::DeleteSelection);
	Script::LinkExternal("DataModel.SubtractSelection", (void*)&DataModel::SubtractSelection);
	Script::LinkExternal("DataModel.TriangulateSelection", (void*)&DataModel::TriangulateSelection);
	Script::LinkExternal("DataModel.BevelSelectedVertices", (void*)&DataModel::BevelSelectedEdges);
	Script::LinkExternal("DataModel.ExtrudeSelectedPolygons", (void*)&DataModel::ExtrudeSelectedPolygons);
	Script::LinkExternal("DataModel.PasteGeometry", (void*)&DataModel::PasteGeometry);



	Script::DeclareClassSize("ModelGeometry", sizeof(ModelGeometry));
	Script::LinkExternal("ModelGeometryBall.__init__", (void*)&ModelGeometryBall::__init__);
	Script::LinkExternal("ModelGeometryCube.__init__", (void*)&ModelGeometryCube::__init__);
	Script::LinkExternal("ModelGeometrySphere.__init__", (void*)&ModelGeometrySphere::__init__);
	Script::LinkExternal("ModelGeometryCylinder.__init__", (void*)&ModelGeometryCylinder::__init__);
	Script::LinkExternal("ModelGeometryCylinderComplex.__init__", (void*)&ModelGeometryCylinder::__init2__);
	Script::LinkExternal("ModelGeometryTorus.__init__", (void*)&ModelGeometryTorus::__init__);
	Script::LinkExternal("ModelGeometryTorusKnot.__init__", (void*)&ModelGeometryTorusKnot::__init__);
	Script::LinkExternal("ModelGeometryPlane.__init__", (void*)&ModelGeometryPlane::__init__);
	Script::LinkExternal("ModelGeometryPlatonic.__init__", (void*)&ModelGeometryPlatonic::__init__);
	Script::LinkExternal("ModelGeometryTeapot.__init__", (void*)&ModelGeometryTeapot::__init__);
	Script::LinkExternal("ModelGeometrySubtract", (void*)&ModelGeometrySubtract);

	// world

	Script::DeclareClassSize("DataWorld", sizeof(DataWorld));
	Script::DeclareClassOffset("DataWorld", "Terrains", offsetof(DataWorld, Terrains));
	Script::DeclareClassOffset("DataWorld", "Objects", offsetof(DataWorld, Objects));
	Script::DeclareClassOffset("DataWorld", "EgoIndex", offsetof(DataWorld, EgoIndex));
	Script::LinkExternal("DataWorld.AddObject", (void*)&DataWorld::AddObject);
	Script::LinkExternal("DataWorld.AddTerrain", (void*)&DataWorld::AddTerrain);
	Script::LinkExternal("DataWorld.AddNewTerrain", (void*)&DataWorld::AddNewTerrain);
}


