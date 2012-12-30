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
	ScriptDirectory = "";
	CScript *s = new CScript(filename);
	if (s->Error){
		ed->ErrorBox(s->ErrorMsg);
		return;
	}
	s->Execute();

	DeleteAllScripts(true, true);
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
	ScriptInit();
	TestVar = "tes test var!";
	test.name = "test.name";
	test.i = 13;

	msg_write("_offsetof:");
	msg_write(_offsetof(DataModel, filename));

	/*msg_write("dm: " + p2s(mode_model->data));
	msg_write("dm.bone: " + p2s(&mode_model->data->Bone));
	msg_write("dm.fn: " + p2s(&mode_model->data->filename));
	msg_write("dm.obs: " + p2s(dynamic_cast<Observable*>(mode_model->data)));*/

	ScriptLinkSemiExternalVar("TestVar", &TestVar);
	ScriptLinkSemiExternalFunc("TestFunc", (void*)&TestFunc);
	ScriptLinkSemiExternalVar("test", &test);
	ScriptLinkSemiExternalFunc("TestClass.func", (void*)&TestClass::func);

	ScriptLinkSemiExternalVar("edward", &ed);
	ScriptLinkSemiExternalVar("data_model", &mode_model->data);
	ScriptLinkSemiExternalFunc("DataModel.ClearSelection", (void*)&DataModel::ClearSelection);
	ScriptLinkSemiExternalFunc("DataModel.SelectionFromVertices", (void*)&DataModel::SelectionFromVertices);
	ScriptLinkSemiExternalFunc("DataModel.SelectionFromPolygons", (void*)&DataModel::SelectionFromPolygons);
	ScriptLinkSemiExternalFunc("DataModel.SelectionFromSurfaces", (void*)&DataModel::SelectionFromSurfaces);
	ScriptLinkSemiExternalFunc("DataModel.SelectOnlySurface", (void*)&DataModel::SelectOnlySurface);
	ScriptLinkSemiExternalFunc("DataModel.AddVertex", (void*)&DataModel::AddVertex);
	ScriptLinkSemiExternalFunc("DataModel.AddTriangle", (void*)&DataModel::AddTriangle);
	ScriptLinkSemiExternalFunc("DataModel.AddPolygon", (void*)&DataModel::AddPolygon);
	ScriptLinkSemiExternalFunc("DataModel.AddBall", (void*)&DataModel::AddBall);
	ScriptLinkSemiExternalFunc("DataModel.AddSphere", (void*)&DataModel::AddSphere);
	ScriptLinkSemiExternalFunc("DataModel.AddPlane", (void*)&DataModel::AddPlane);
	ScriptLinkSemiExternalFunc("DataModel.AddCube", (void*)&DataModel::AddCube);
	ScriptLinkSemiExternalFunc("DataModel.AddCylinder", (void*)&DataModel::AddCylinder);
	ScriptLinkSemiExternalFunc("DataModel.DeleteSelection", (void*)&DataModel::DeleteSelection);
	ScriptLinkSemiExternalFunc("DataModel.SubtractSelection", (void*)&DataModel::SubtractSelection);
	ScriptLinkSemiExternalFunc("DataModel.TriangulateSelection", (void*)&DataModel::TriangulateSelection);
	ScriptLinkSemiExternalFunc("DataModel.BevelSelectedVertices", (void*)&DataModel::BevelSelectedVertices);
	ScriptLinkSemiExternalFunc("DataModel.ExtrudeSelectedPolygons", (void*)&DataModel::ExtrudeSelectedPolygons);
	ScriptLinkSemiExternalFunc("DataModel.BeginActionGroup", (void*)&DataModel::BeginActionGroup);
	ScriptLinkSemiExternalFunc("DataModel.EndActionGroup", (void*)&DataModel::EndActionGroup);
	ScriptLinkSemiExternalVar("data_world", &mode_world->data);
	ScriptLinkSemiExternalFunc("DataWorld.AddObject", (void*)&DataWorld::AddObject);
	ScriptLinkSemiExternalFunc("DataWorld.AddTerrain", (void*)&DataWorld::AddTerrain);
	ScriptLinkSemiExternalFunc("DataWorld.AddNewTerrain", (void*)&DataWorld::AddNewTerrain);
	ScriptLinkSemiExternalFunc("DataWorld.BeginActionGroup", (void*)&DataWorld::BeginActionGroup);
	ScriptLinkSemiExternalFunc("DataWorld.EndActionGroup", (void*)&DataWorld::EndActionGroup);

//	ScriptLinkDynamicExternalData();
}


