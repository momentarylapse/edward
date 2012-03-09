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
	has_to_notify = false;
	observable_name = name;
}

Observable::~Observable()
{
	observer.clear();
}

void Observable::Subscribe(Observer *o)
{
	observer.add(o);
}



void Observable::Unsubscribe(Observer *o)
{
	foreachi(observer, obs, i)
		if (obs == o){
			observer.erase(i);
			break;
		}
}



string Observable::GetName()
{	return observable_name;	}



void Observable::Notify()
{
	if (notify_level == 0){
		//msg_write("notify ==");
		foreach(observer, o)
			o->OnUpdate(this);
		has_to_notify = false;
	}else
		has_to_notify = true;
}

void Observable::NotifyBegin()
{
	notify_level ++;
	//msg_write("notify ++");
}

void Observable::NotifyEnd()
{
	notify_level --;
	//msg_write("notify --");
	if ((notify_level == 0) && (has_to_notify))
		Notify();
}

