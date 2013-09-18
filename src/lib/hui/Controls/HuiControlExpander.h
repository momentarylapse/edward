/*
 * HuiControlExpander.h
 *
 *  Created on: 18.09.2013
 *      Author: michi
 */

#ifndef HUICONTROLEXPANDER_H_
#define HUICONTROLEXPANDER_H_

#include "HuiControl.h"

class HuiControlExpander : public HuiControl
{
public:
	HuiControlExpander(const string &text, const string &id);
	virtual ~HuiControlExpander();

	virtual void add(HuiControl *child, int x, int y);
};

#endif /* HUICONTROLEXPANDER_H_ */
