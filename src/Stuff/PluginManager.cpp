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
	Script::Directory = "";
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

	Script::LinkSemiExternalVar("TestVar", &TestVar);
	Script::LinkSemiExternalFunc("TestFunc", (void*)&TestFunc);
	Script::LinkSemiExternalVar("test", &test);
	Script::LinkSemiExternalFunc("TestClass.func", (void*)&TestClass::func);

	Script::LinkSemiExternalVar("edward", &ed);
	Script::LinkSemiExternalVar("data_model", &mode_model->data);
	Script::LinkSemiExternalFunc("DataModel.ClearSelection", (void*)&DataModel::ClearSelection);
	Script::LinkSemiExternalFunc("DataModel.SelectionFromVertices", (void*)&DataModel::SelectionFromVertices);
	Script::LinkSemiExternalFunc("DataModel.SelectionFromPolygons", (void*)&DataModel::SelectionFromPolygons);
	Script::LinkSemiExternalFunc("DataModel.SelectionFromSurfaces", (void*)&DataModel::SelectionFromSurfaces);
	Script::LinkSemiExternalFunc("DataModel.SelectOnlySurface", (void*)&DataModel::SelectOnlySurface);
	Script::LinkSemiExternalFunc("DataModel.AddVertex", (void*)&DataModel::AddVertex);
	Script::LinkSemiExternalFunc("DataModel.AddTriangle", (void*)&DataModel::AddTriangle);
	Script::LinkSemiExternalFunc("DataModel.AddPolygon", (void*)&DataModel::AddPolygon);
	Script::LinkSemiExternalFunc("DataModel.AddBall", (void*)&DataModel::AddBall);
	Script::LinkSemiExternalFunc("DataModel.AddSphere", (void*)&DataModel::AddSphere);
	Script::LinkSemiExternalFunc("DataModel.AddPlane", (void*)&DataModel::AddPlane);
	Script::LinkSemiExternalFunc("DataModel.AddCube", (void*)&DataModel::AddCube);
	Script::LinkSemiExternalFunc("DataModel.AddCylinder", (void*)&DataModel::AddCylinder);
	Script::LinkSemiExternalFunc("DataModel.DeleteSelection", (void*)&DataModel::DeleteSelection);
	Script::LinkSemiExternalFunc("DataModel.SubtractSelection", (void*)&DataModel::SubtractSelection);
	Script::LinkSemiExternalFunc("DataModel.TriangulateSelection", (void*)&DataModel::TriangulateSelection);
	Script::LinkSemiExternalFunc("DataModel.BevelSelectedVertices", (void*)&DataModel::BevelSelectedVertices);
	Script::LinkSemiExternalFunc("DataModel.ExtrudeSelectedPolygons", (void*)&DataModel::ExtrudeSelectedPolygons);
	Script::LinkSemiExternalFunc("DataModel.BeginActionGroup", (void*)&DataModel::BeginActionGroup);
	Script::LinkSemiExternalFunc("DataModel.EndActionGroup", (void*)&DataModel::EndActionGroup);
	Script::LinkSemiExternalVar("data_world", &mode_world->data);
	Script::LinkSemiExternalFunc("DataWorld.AddObject", (void*)&DataWorld::AddObject);
	Script::LinkSemiExternalFunc("DataWorld.AddTerrain", (void*)&DataWorld::AddTerrain);
	Script::LinkSemiExternalFunc("DataWorld.AddNewTerrain", (void*)&DataWorld::AddNewTerrain);
	Script::LinkSemiExternalFunc("DataWorld.BeginActionGroup", (void*)&DataWorld::BeginActionGroup);
	Script::LinkSemiExternalFunc("DataWorld.EndActionGroup", (void*)&DataWorld::EndActionGroup);

//	Script::LinkDynamicExternalData();
}


