/*
 * System.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "System.h"

System::System() : BaseClass(Type::SYSTEM) {
}

void System::__init__() {
	new(this) System;
}

void System::__delete__() {
	this->System::~System();
}

