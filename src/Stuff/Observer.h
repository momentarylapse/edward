/*
 * Observer.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVER_H_
#define OBSERVER_H_

#include "Observable.h"

class Observable;

class Observer
{
public:
	Observer();
	virtual ~Observer();

	void Subscribe(Observable *o);
	void Unsubscribe(Observable *o);

	virtual void OnUpdate(Observable *o) = 0;
};

#endif /* OBSERVER_H_ */
