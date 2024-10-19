/*
 * Controller.cpp
 *
 *  Created on: 02.01.2020
 *      Author: michi
 */

#include "Controller.h"

Controller::Controller() : BaseClass(Type::CONTROLLER) {
	_class = nullptr;
}

void Controller::__init__() {
	new(this) Controller;
}

void Controller::__delete__() {
	this->Controller::~Controller();
}

