/*----------------------------------------------------------------------------*\
| CObject                                                                      |
| -> physical entities of a model in the game                                  |
| -> manages physics on its own                                                |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2009.12.03 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/
#if !defined(OBJECT_H__INCLUDED_)
#define OBJECT_H__INCLUDED_

#include "model.h"


class Object : public Model
{
public:
//	Object(const char *filename, const char *name, const vector &pos);
//	Object(Model *model);
	Object();
//	~Object();
	void UpdateMatrix();

	void _cdecl UpdateData(); // script...

	void UpdateTheta();
//	void SetMaterial(Material *material, int mode);
//	void ObjectCalcMove();
	void DoPhysics();
//	CObject *CuttingPlane(plane pl);
//	void CorrectVel(Object *partner);

	void _cdecl AddForce(const vector &f, const vector &rho);
	void _cdecl AddTorque(const vector &t);

	void _cdecl MakeVisible(bool visible);
};


#endif

