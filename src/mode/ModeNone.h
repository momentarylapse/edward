/*
 * ModeNone.h
 *
 *  Created on: 20.12.2014
 *      Author: michi
 */

#ifndef SRC_MODE_MODENONE_H_
#define SRC_MODE_MODENONE_H_

#include "Mode.h"

class ModeNone : public ModeBase
{
public:
	ModeNone(EdwardWindow *ed);
	virtual ~ModeNone();

	virtual Data *get_data(){ return NULL; };
};

#endif /* SRC_MODE_MODENONE_H_ */
