/*
 * Observable.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observable.h"
#include "Observer.h"

Observable::Observable(const string &name)
{
	notify_level = 0;
	observable_name = name;
}

Observable::~Observable()
{
	observer.clear();
}

void Observable::Subscribe(Observer *o, const string &message)
{
	observer.add(o);
	observer_message.add(message);
}

void Observable::Subscribe(Observer *o)
{	Subscribe(o, "");	}



void Observable::Unsubscribe(Observer *o)
{
	foreachi(Observer *obs, observer, i)
		if (obs == o){
			observer.erase(i);
			observer_message.erase(i);
			break;
		}
}



string Observable::GetName()
{	return observable_name;	}



string Observable::GetMessage()
{	return cur_message;	}


void Observable::NotifySend()
{
	// send
	foreach(string &m, message_queue){
		cur_message = m;
		//msg_write("send " + observable_name + ": " + m);

		// foreachi(Observer *o, observer, i) might break when OnUpdate causes
		// new subscription!

		for (int i=0;i<observer.num;i++)
			if ((observer_message[i] == m) or (observer_message[i].num == 0))
				observer[i]->OnUpdate(this);
	}

	// clear queue
	message_queue.clear();
}


void Observable::NotifyEnqueue(const string &message)
{
	// already enqueued?
	foreach(string &m, message_queue)
		if (message == m)
			return;

	// add
	message_queue.add(message);
}

void Observable::NotifyBegin()
{
	notify_level ++;
}

void Observable::NotifyEnd()
{
	notify_level --;
	if (notify_level == 0)
		NotifySend();
}


void Observable::Notify(const string &message)
{
	NotifyEnqueue(message);
	if (notify_level == 0)
		NotifySend();
}

