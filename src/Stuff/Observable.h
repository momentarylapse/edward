/*
 * Observable.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef OBSERVABLE_H_
#define OBSERVABLE_H_

#include "../lib/base/base.h"
#include "../lib/hui/hui.h"

class Observer;
struct ObserverRequest;

class Observable : public hui::EventHandler
{
public:
	Observable(const string &name);
	virtual ~Observable();

	static const string MESSAGE_CHANGE;
	static const string MESSAGE_DELETE;
	static const string MESSAGE_ALL;

	void add_observer(Observer *o, const string &message = MESSAGE_ALL);
	void add_wrapped_observer(void *handler, void *func);
	void remove_observer(Observer *o);
	void remove_wrapped_observer(void *handler);
	string get_name();

	void notify_begin();
	void notify(const string &message = MESSAGE_CHANGE);
	void notify_end();
private:
	void notify_enqueue(const string &message);
	void notify_send();

private:
	string observable_name;

	// observers
	Array<ObserverRequest> requests;

	// current notifies
	Array<const string*> message_queue;
	int notify_level;

	static const bool DEBUG_MESSAGES;
};

#endif /* OBSERVABLE_H_ */
