/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include "Observer.h"

const string Observable::MESSAGE_ALL = "";
const string Observable::MESSAGE_CHANGE = "Change";
const string Observable::MESSAGE_DELETE = "Delete";
const bool Observable::DEBUG_MESSAGES = false;


struct ObserverRequest
{
	ObserverRequest(){}
	ObserverRequest(Observer *o, const string &_message)
	{
		observer = o;
		message = &_message;
	}
	Observer* observer;
	const string *message;
};

typedef ObserverRequest Notification;


Observable::Observable(const string &name)
{
	notify_level = 0;
	observable_name = name;
}

Observable::~Observable()
{
	notify(MESSAGE_DELETE);
}

void Observable::add_observer(Observer *o, const string &message)
{
	requests.add(ObserverRequest(o, message));
}

void Observable::remove_observer(Observer *o)
{
	for (int i=requests.num-1; i>=0; i--)
		if (requests[i].observer == o){
			requests.erase(i);
		}
}

void Observable::add_wrapped_observer(void* handler, void* func)
{
	Observer *o = new ObserverWrapper(handler, func);
	add_observer(o, MESSAGE_ALL);
}

void Observable::remove_wrapped_observer(void* handler)
{
	foreachi(ObserverRequest &r, requests, i)
		if (dynamic_cast<ObserverWrapper*>(r.observer)){
			if (dynamic_cast<ObserverWrapper*>(r.observer)->handler == handler){
				delete(r.observer);
				requests.erase(i);
				break;
			}
		}
}



string Observable::get_name()
{	return observable_name;	}

void Observable::notify_send()
{
	Array<Notification> notifications;

	// decide whom to send what
	for (const string *m: message_queue){
		//msg_write("send " + observable_name + ": " + *m);
		for (ObserverRequest &r: requests){
			if ((r.message == m) or (r.message == &MESSAGE_ALL))
				notifications.add(Notification(r.observer, *m));
		}
	}

	message_queue.clear();

	// send
	for (Notification &n: notifications){
		if (DEBUG_MESSAGES)
			msg_write("send " + get_name() + "/" + *n.message + "  >>  " + n.observer->get_name());
		n.observer->on_update(this, *n.message);
	}
}


void Observable::notify_enqueue(const string &message)
{
	// already enqueued?
	for (const string *m: message_queue)
		if (&message == m)
			return;

	// add
	message_queue.add(&message);
}

void Observable::notify_begin()
{
	notify_level ++;
	//msg_write("notify ++");
}

void Observable::notify_end()
{
	notify_level --;
	//msg_write("notify --");
	if (notify_level == 0)
		notify_send();
}


void Observable::notify(const string &message)
{
	notify_enqueue(message);
	if (notify_level == 0)
		notify_send();
}


