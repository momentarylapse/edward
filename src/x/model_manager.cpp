#include "model_manager.h"
#include "model.h"
#include "material.h"
#include "world.h"
#include "../lib/file/file.h"
#include "../lib/config.h"

// models
static Array<Model*> ModelOriginal;
Array<Model*> ModelCopy;

string ObjectDir;
Model *ModelToIgnore;
static bool ModelManagerRunning = false;

void ScriptingModelInit(Model *);


void ModelManagerReset()
{
//	ModelManagerRunning = false;
	//msg_write(format("mmr %d %d", ModelCopy.num, ModelOriginal.num));
	Model::AllowDeleteRecursive = false;

	// delete copies of models
	while(ModelCopy.num > 0)
		delete(ModelCopy.back());

	// delete original
	while(ModelOriginal.num > 0)
		delete(ModelOriginal.back());
	Model::AllowDeleteRecursive = true;

	ModelToIgnore = NULL;
	ModelManagerRunning = true;
}

Model *LoadModelX(const string &filename, bool allow_script_init)
{
	if (filename.num == 0)
		return NULL;

	// already existing? -> copy
	for (int i=0;i<ModelOriginal.num;i++)
		if (filename == ModelOriginal[i]->_template->filename)
			return ModelOriginal[i]->GetCopy(allow_script_init);

	// new
	Model *m = new Model(filename);
	if (m->error)
		return NULL;
	return m->GetCopy(allow_script_init);
}

Model *LoadModel(const string &filename)
{
	return LoadModelX(filename, true);
}

Model *CopyModel(Model *m, bool allow_script_init)
{
	if (!m)
		return NULL;
	// which original
	if (!m->_template){
		msg_error("CopyModel: no template!");
		return NULL;
	}
	Model *orig = m->_template->model;
	if (!orig){
		msg_error("CopyModel: no original model found! " + m->GetFilename());
		return NULL;
	}

	return orig->GetCopy(allow_script_init);
}

bool IsModel(void *m)
{
	for (int i=0;i<ModelCopy.num;i++)
		if (ModelCopy[i] == m)
			return true;
	for (int i=0;i<ModelOriginal.num;i++)
		if (ModelOriginal[i] == m)
			return true;
	return false;
}

typedef void model_callback_func(Model*);

void RegisterModel(Model *m, bool allow_script_init)
{
	//msg_write("reg " + m->GetFilename());
	//msg_write(m->is_copy);
	if (m->is_copy){
		ModelCopy.add(m);

		// execute
		if (allow_script_init)
			m->OnInit();
	}else{
		ModelOriginal.add(m);

#ifdef _X_ALLOW_X_
		// object wants a script
		if (m->_template->script_filename.num > 0)
			ScriptingModelInit(m);
#endif
	}
}

void UnregisterModel(Model *m)
{
	if (!ModelManagerRunning)
		return;
	//msg_write("unreg " + m->GetFilename());
	//msg_write(m->is_copy);
	if (m->is_copy){

		if (Model::AllowDeleteRecursive)
			m->OnDelete();

		foreachi(Model *mm, ModelCopy, i)
			if (mm == m){
				ModelCopy.erase(i);
				return;
			}
	}else{
		foreachi(Model *mm, ModelOriginal, i)
			if (mm == m){
				ModelOriginal.erase(i);
				return;
			}
	}
	msg_error("UnregisterModel: model not registered");
	msg_write(m->GetFilename());
}

void ModelManagerIterate(float dt)
{
	for (int i=0;i<ModelCopy.num;i++)
		ModelCopy[i]->OnIterate(dt);
}
