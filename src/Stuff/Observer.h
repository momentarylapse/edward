/*
 * Observer.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "../lib/file/file.h"

class Observable;

class Observer
{
public:
	Observer();
	virtual ~Observer();

	void subscribe(Observable *o);
	void subscribe(Observable *o, const string &message);
	void unsubscribe(Observable *o);

	virtual void onUpdate(Observable *o){};
};

#endif /* OBSERVER_H_ */
