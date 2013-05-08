#if !defined(MODEL_MANAGER_H__INCLUDED_)
#define MODEL_MANAGER_H__INCLUDED_


#include "../lib/base/base.h"

class Model;

void ModelManagerReset();

extern string ObjectDir;
extern Model *ModelToIgnore;

// models
Model *_cdecl LoadModel(const string &filename);
Model *CopyModel(Model *m);
void _cdecl DeleteModel(Model *m);
void _cdecl ModelMakeEditable(Model *m);
int _cdecl GetModelOID(Model *m);

#endif
