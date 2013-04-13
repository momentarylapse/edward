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

class TestClass
{
public:
	string name;
	int i;
	void func(){	msg_write("TestClass.func() " + name + ": " + i2s(i));	}
};

static TestClass test;

static string TestVar;
void TestFunc()
{
	msg_write("execute test func!");
}

char *ppp;
#define _offsetof(t, x)	((int)(long)((char*)&(((t*)ppp)->x) - ppp))

void PluginManager::Init()
{
	Script::Init();
	TestVar = "tes test var!";
	test.name = "test.name";
	test.i = 13;

	msg_write("_offsetof:");
	msg_write(_offsetof(DataModel, filename));

	/*msg_write("dm: " + p2s(mode_model->data));
	msg_write("dm.bone: " + p2s(&mode_model->data->Bone));
	msg_write("dm.fn: " + p2s(&mode_model->data->filename));
	msg_write("dm.obs: " + p2s(dynamic_cast<Observable*>(mode_model->data)));*/

	Script::LinkExternal("TestVar", &TestVar);
	Script::LinkExternal("TestFunc", (void*)&TestFunc);
	Script::LinkExternal("test", &test);
	Script::LinkExternal("TestClass.func", (void*)&TestClass::func);

	Script::LinkExternal("edward", &ed);
	Script::LinkExternal("data_model", &mode_model->data);
	Script::LinkExternal("DataModel.ClearSelection", (void*)&DataModel::ClearSelection);
	Script::LinkExternal("DataModel.InvertSelection", (void*)&DataModel::InvertSelection);
	Script::LinkExternal("DataModel.SelectionFromVertices", (void*)&DataModel::SelectionFromVertices);
	Script::LinkExternal("DataModel.SelectionFromPolygons", (void*)&DataModel::SelectionFromPolygons);
	Script::LinkExternal("DataModel.SelectionFromSurfaces", (void*)&DataModel::SelectionFromSurfaces);
	Script::LinkExternal("DataModel.SelectOnlySurface", (void*)&DataModel::SelectOnlySurface);
	Script::LinkExternal("DataModel.AddVertex", (void*)&DataModel::AddVertex);
	Script::LinkExternal("DataModel.AddTriangle", (void*)&DataModel::AddTriangle);
	Script::LinkExternal("DataModel.AddPolygon", (void*)&DataModel::AddPolygon);
	Script::LinkExternal("DataModel.AddBall", (void*)&DataModel::AddBall);
	Script::LinkExternal("DataModel.AddSphere", (void*)&DataModel::AddSphere);
	Script::LinkExternal("DataModel.AddPlane", (void*)&DataModel::AddPlane);
	Script::LinkExternal("DataModel.AddCube", (void*)&DataModel::AddCube);
	Script::LinkExternal("DataModel.AddCylinder", (void*)&DataModel::AddCylinder);
	Script::LinkExternal("DataModel.DeleteSelection", (void*)&DataModel::DeleteSelection);
	Script::LinkExternal("DataModel.SubtractSelection", (void*)&DataModel::SubtractSelection);
	Script::LinkExternal("DataModel.TriangulateSelection", (void*)&DataModel::TriangulateSelection);
	Script::LinkExternal("DataModel.BevelSelectedVertices", (void*)&DataModel::BevelSelectedVertices);
	Script::LinkExternal("DataModel.ExtrudeSelectedPolygons", (void*)&DataModel::ExtrudeSelectedPolygons);
	Script::LinkExternal("DataModel.BeginActionGroup", (void*)&DataModel::BeginActionGroup);
	Script::LinkExternal("DataModel.EndActionGroup", (void*)&DataModel::EndActionGroup);
	Script::LinkExternal("data_world", &mode_world->data);
	Script::LinkExternal("DataWorld.AddObject", (void*)&DataWorld::AddObject);
	Script::LinkExternal("DataWorld.AddTerrain", (void*)&DataWorld::AddTerrain);
	Script::LinkExternal("DataWorld.AddNewTerrain", (void*)&DataWorld::AddNewTerrain);
	Script::LinkExternal("DataWorld.BeginActionGroup", (void*)&DataWorld::BeginActionGroup);
	Script::LinkExternal("DataWorld.EndActionGroup", (void*)&DataWorld::EndActionGroup);

//	Script::LinkDynamicExternalData();
}


