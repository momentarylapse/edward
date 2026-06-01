/*
 * ActionMultiView.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef ACTIONMULTIVIEW_H_
#define ACTIONMULTIVIEW_H_

#include <lib/history/Action.h>
#include <lib/math/mat4.h>
#include <lib/math/vec3.h>

class ActionMultiView: public history::Action {
public:
	ActionMultiView();

	// continuous editing
	void abort(history::Data *d) override;
	void abort_and_notify(history::Data *d);
	void update_and_notify(history::Data *d, const mat4 &m);

protected:
	Array<int> index;
	mat4 mat;
	Array<vec3> old_data;
};

//ActionMultiView *ActionMultiViewFactory(const string &name, Data *d);

#endif /* ACTIONMULTIVIEW_H_ */
