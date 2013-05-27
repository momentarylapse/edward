/*
 * ModeCreation.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODECREATION_H_
#define MODECREATION_H_

#include "Mode.h"

class ModeCreationBase : public ModeBase
{
public:
	ModeCreationBase(const string &_name, ModeBase *_parent);
	virtual ~ModeCreationBase(){}
	virtual void OnDrawRecursive(bool multi_view_handled = false);
	virtual void Abort();
	string message;
	CHuiWindow *dialog;
};

template<class T>
class ModeCreation: public ModeCreationBase
{
public:
	ModeCreation(const string &_name, ModeBase *_parent) :
		ModeCreationBase(_name, _parent)
	{
		data = (T*)_parent->GetData();
	}
	virtual ~ModeCreation(){}
	T *data;
	virtual Data *GetData(){	return data;	}
};

#endif /* MODECREATION_H_ */
