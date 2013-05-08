#include "model_manager.h"
#include "model.h"
#include "material.h"
#include "world.h"
#include "../lib/file/file.h"

// models
struct sModelRefCopy
{
	Model *model;
	int OriginalIndex;
};
static Array<Model*> ModelOriginal, ModelOriginal2;
static Array<sModelRefCopy> ModelCopy, ModelCopy2;

string ObjectDir;
Model *ModelToIgnore;


void ModelManagerReset()
{
#ifdef _X_DEBUG_MODEL_MANAGER_
	for (int i=0;i<ModelCopy.num;i++)
		msg_write(p2s(ModelCopy[i].model));
#endif

	// delete copies of models
	for (int i=0;i<ModelCopy.num;i++)
		DeleteModel(ModelCopy[i].model);
	ModelCopy.clear();

	// delete original
	msg_db_m("-models",1);
	for (int i=0;i<ModelOriginal.num;i++)
		delete(ModelOriginal[i]);
	ModelOriginal.clear();

	ModelOriginal2.clear();
	ModelCopy2.clear();

	ModelToIgnore = NULL;
}


void MetaListTest()
{
#ifdef _X_DEBUG_MODEL_MANAGER_
	if (abs((int)ModelCopy.num - (int)ModelCopy2.num) > 1){
		msg_error("size !=");
		msg_write(ModelCopy.num);
		msg_write(ModelCopy2.num);
	}
	for (int i=0;i<min(ModelCopy.num, ModelCopy2.num);i++)
		if (ModelCopy[i].model != ModelCopy2[i].model){
			msg_error("model !=");
			msg_write(i);
			msg_write(ModelCopy.num);
			msg_write(ModelCopy2.num);
			msg_write(p2s(ModelCopy[i].model));
			msg_write(p2s(ModelCopy2[i].model));
		}
	if (abs((int)ModelOriginal.num - (int)ModelOriginal2.num) > 1){
		msg_error("size orig !=");
		msg_write(ModelOriginal.num);
		msg_write(ModelOriginal2.num);
	}
	for (int i=0;i<min(ModelOriginal.num, ModelOriginal2.num);i++)
		if (ModelOriginal[i] != ModelOriginal2[i]){
			msg_error("model orig !=");
			msg_write(i);
			msg_write(ModelOriginal.num);
			msg_write(ModelOriginal2.num);
			msg_write(p2s(ModelOriginal[i]));
			msg_write(p2s(ModelOriginal2[i]));
		}
#endif
}

void MetaListUpdate()
{
	MetaListTest();
	ModelCopy2.assign(&ModelCopy);
	ModelOriginal2.assign(&ModelOriginal);
}

static void db_o(const string &msg)
{
	#ifdef _X_DEBUG_MODEL_MANAGER_
		msg_write(msg);
	#endif
}

void AddModelCopy(Model *m, int orig_id)
{
	sModelRefCopy c;
	c.model = m;
	db_o(p2s(m));
	c.OriginalIndex = orig_id;
	ModelCopy.add(c);
	MetaListUpdate();
}

void AddModelOrig(Model *m)
{
	ModelOriginal.add(m);
	db_o(p2s(m));
	MetaListUpdate();
}

void ReplaceModelOrig(int id, Model *m)
{
	Model *m0 = ModelOriginal[id];
	ModelOriginal[id] = m;
	ModelOriginal2[id] = m;
}

Model *LoadModel(const string &filename)
{
	if (filename.num == 0)
		return NULL;

	MetaListTest();

	msg_db_f("MetaLoadModel", 2);
	// already existing? -> copy
	for (int i=0;i<ModelOriginal.num;i++){
		if (filename == ModelOriginal[i]->_template->filename){
			//msg_write("copy...");
			Model *m = ModelOriginal[i]->GetCopy(ModelCopyRecursive);
			AddModelCopy(m, i);
			db_o("#######");
			return m;
		}
	}

	//msg_write("new....");
	Model *m = new Model(filename);
	if (m->error)
		return NULL;
	AddModelOrig(m);
	db_o("####### orig");
	return m;
}

// make sure we can edit this object without destroying an original one
void ModelMakeEditable(Model *m)
{
	msg_db_f("ModelMakeEditable", 2);

	// original -> create copy
	if (!m->is_copy){
		bool registered = m->registered;
		if (m->registered)
			GodUnregisterModel(m);
		int id = -1;
		for (int i=0;i<ModelOriginal.num;i++)
			if (ModelOriginal[i] == m){
				id = i;
				break;
			}
		if (id < 0){
			msg_error("ModelMakeEditable: no original model found!");
			return;
		}

		// create a "temporary" copy and save it as the original
		Model *m2 = m->GetCopy(ModelCopyKeepSubModels | ModelCopyInverse);
		ReplaceModelOrig(id, m2);

		// swap data
		char *data = new char[sizeof(Model)];
		memcpy(data, m, sizeof(Model));
		memcpy(m, m2, sizeof(Model));
		memcpy(m2, data, sizeof(Model));
		delete[](data);

		// create copy
		sModelRefCopy c;
		db_o(p2s(m));
		db_o(p2s(m2));
		c.model = m; // we are the copy now
		c.OriginalIndex = id;
		ModelCopy.add(c);
		MetaListUpdate();
		if (registered)
			GodRegisterModel(m);
	}

	// must have its own materials
	if (m->material_is_reference){
		m->material.make_own();
		m->material_is_reference = false;
	}
}

Model *CopyModel(Model *m)
{
	if (!m)
		return NULL;
	msg_db_f("CopyModel", 2);
	// which original
	int id = -1;
	if (m->is_copy){
		for (int i=0;i<ModelCopy.num;i++)
			if (ModelCopy[i].model == m){
				id = ModelCopy[i].OriginalIndex;
				break;
			}
	}else{
		for (int i=0;i<ModelOriginal.num;i++)
			if (ModelOriginal[i] == m){
				id = i;
				break;
			}
	}
	if (id < 0){
		msg_error("CopyModel: no original model found!");
		return NULL;
	}

	Model *copy = ModelOriginal[id]->GetCopy(ModelCopyRecursive);
	AddModelCopy(copy, id);

	return copy;
}

bool IsModel(Model *m)
{
	for (int i=0;i<ModelCopy.num;i++)
		if (ModelCopy[i].model == m)
			return true;
	for (int i=0;i<ModelOriginal.num;i++)
		if (ModelOriginal[i] == m)
			return true;
	return false;
}

void DeleteModel(Model *m)
{
	if (!m)	return;
	msg_db_f("DeleteModel",1);
	db_o("meta del");

	if (IsModel(m)){
		db_o("m");
		db_o(p2s(m));
		db_o(p2s(m->_template));
		db_o(m->_template->filename);
		GodUnregisterModel(m);
		if (m->object_id >= 0)
			GodDeleteObject(m->object_id);
	}
		db_o(".");

	// deletes only copies created by meta
	for (int i=0;i<ModelCopy.num;i++)
		if (ModelCopy[i].model == m){
			msg_db_m("-really deleting...",1);
			//msg_error("DeleteModel");
			//msg_write(ModelOriginal[ModelCopy[i].OriginalIndex].Filename);
			delete(m);
			ModelCopy.erase(i);
			break;
		}
}
