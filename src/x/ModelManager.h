/*
 * ModelManager.h
 *
 *  Created on: 19.01.2020
 *      Author: michi
 */

#ifndef SRC_WORLD_MODELMANAGER_H_
#define SRC_WORLD_MODELMANAGER_H_

#include "../lib/base/base.h"

class Model;

class ModelManager {
public:
	static Model *load(const string &filename);

	static Array<Model*> originals;
};

#endif /* SRC_WORLD_MODELMANAGER_H_ */
