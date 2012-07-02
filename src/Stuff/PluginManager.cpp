/*
 * PluginManager.cpp
 *
 *  Created on: 01.07.2012
 *      Author: michi
 */

#include "PluginManager.h"

#include "../lib/script/script.h"
#include "../Edward.h"

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


void PluginManager::Init()
{
	ScriptInit();
	TestVar = "tes test var!";
	test.name = "test.name";
	test.i = 13;

	ScriptLinkSemiExternalVar("TestVar", &TestVar);
	ScriptLinkSemiExternalFunc("TestFunc", (void*)&TestFunc);
	ScriptLinkSemiExternalVar("test", &test);
	ScriptLinkSemiExternalFunc("TestClass.func", (void*)&TestClass::func);
//	ScriptLinkDynamicExternalData();
}


