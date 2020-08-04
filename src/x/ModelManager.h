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
class Path;

class ModelManager {
public:
	static Model *load(const Path &filename);

	static Array<Model*> originals;
};

#endif /* SRC_WORLD_MODELMANAGER_H_ */
