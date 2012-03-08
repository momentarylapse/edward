/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "Observer.h"
#include "../lib/file/file.h"

class Observer;

class Observable
{
public:
	Observable(const string &name);
	virtual ~Observable();

	void Subscribe(Observer *o);
	void Unsubscribe(Observer *o);
	string GetName();

	void NotifyBegin();
	void Notify();
	void NotifyEnd();

private:
	string observable_name;
	Array<Observer*> observer;
	int notify_level;
	bool has_to_notify;
};

#endif /* OBSERVABLE_H_ */
