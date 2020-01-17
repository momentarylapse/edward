/*
 * ModeCreation.h
 *
 *  Created on: 07.03.2012
 *      Author: michi
 */

#ifndef MODECREATION_H_
#define MODECREATION_H_

#include "Mode.h"

class ModeCreationBase : public ModeBase {
public:
	ModeCreationBase(const string &_name, ModeBase *_parent);
	void on_draw() override;
	void on_draw_win(MultiView::Window *win) override;
	virtual void on_draw_post() {};
	void on_set_multi_view() override;
	void abort();
	string message;
	hui::Window *dialog;
};

template<class T>
class ModeCreation: public ModeCreationBase {
public:
	ModeCreation(const string &_name, ModeBase *_parent) :
		ModeCreationBase(_name, _parent)
	{
		data = (T*)_parent->get_data();
	}
	T *data;
	virtual Data *get_data() { return data; }
};

#endif /* MODECREATION_H_ */
