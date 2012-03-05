/*
 * Mode.h
 *
 *  Created on: 04.03.2012
 *      Author: michi
 */

#ifndef MODE_H_
#define MODE_H_

#include "../lib/hui/hui.h"
#include "../Data/Data.h"

class Mode
{
public:
	Mode();
	virtual ~Mode();

	virtual void Start() = 0;
	virtual void End() = 0;

	virtual void OnMouseMove() = 0;
	virtual void OnLeftButtonDown() = 0;
	virtual void OnLeftButtonUp() = 0;
	virtual void OnMiddleButtonDown() = 0;
	virtual void OnMiddleButtonUp() = 0;
	virtual void OnRightButtonDown() = 0;
	virtual void OnRightButtonUp() = 0;
	virtual void OnKeyDown() = 0;
	virtual void OnKeyUp() = 0;
	virtual void OnCommand(const string &id) = 0;

	virtual void Draw() = 0;

	Mode *parent;

	CHuiMenu *menu;
};

#endif /* MODE_H_ */
