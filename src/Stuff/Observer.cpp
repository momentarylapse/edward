/*
 * Observer.cpp
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#include "Observer.h"
#include "Observable.h"

Observer::Observer()
{
}

Observer::~Observer()
{
}



void Observer::subscribe(Observable *o, const string &message)
{
	o->subscribe(this, message);
}

void Observer::subscribe(Observable *o)
{
	o->subscribe(this);
}

void Observer::unsubscribe(Observable *o)
{
	o->unsubscribe(this);
}


