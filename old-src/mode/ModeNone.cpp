/*
 * ModeNone.cpp
 *
 *  Created on: 20.12.2014
 *      Author: michi
 */

#include "ModeNone.h"

ModeNone::ModeNone(Session *s) :
	ModeBase(s, "ModeNone", nullptr, nullptr, "")
{
}

ModeNone::~ModeNone()
{
}

