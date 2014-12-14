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

#ifdef GetMessage
#undef GetMessage
#endif

class Observer;

class Observable
{
public:
	Observable(const string &name);
	virtual ~Observable();

	void subscribe(Observer *o);
	void subscribe(Observer *o, const string &message);
	void unsubscribe(Observer *o);
	string getName();
	string getMessage();

	void notifyBegin();
	void notify(const string &message);
	void notifyEnd();
private:
	void notifyEnqueue(const string &message);
	void notifySend();

private:
	string observable_name;

	// observers
	Array<Observer*> observer;
	Array<string> observer_message;

	// current notifies
	Array<string> message_queue;
	string cur_message;
	int notify_level;
};

#endif /* OBSERVABLE_H_ */
