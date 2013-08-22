#if !defined(MODEL_MANAGER_H__INCLUDED_)
#define MODEL_MANAGER_H__INCLUDED_


#include "../lib/base/base.h"

class Model;

void ModelManagerReset();

extern string ObjectDir;
extern Model *ModelToIgnore;

// models
Model *_cdecl LoadModelX(const string &filename, bool allow_scripting);
Model *_cdecl LoadModel(const string &filename);
Model *CopyModel(Model *m, bool allow_script_init);
bool IsModel(void *m);
void RegisterModel(Model *m, bool allow_script_init = true);
void UnregisterModel(Model *m);

#endif
