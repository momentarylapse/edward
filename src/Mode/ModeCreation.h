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
	virtual void onDrawMeta();
	virtual void onDrawWin(MultiView::Window *win);
	void abort();
	string message;
	hui::Window *dialog;
};

template<class T>
class ModeCreation: public ModeCreationBase
{
public:
	ModeCreation(const string &_name, ModeBase *_parent) :
		ModeCreationBase(_name, _parent)
	{
		data = (T*)_parent->getData();
	}
	virtual ~ModeCreation(){}
	T *data;
	virtual Data *getData(){	return data;	}
};

#endif /* MODECREATION_H_ */
