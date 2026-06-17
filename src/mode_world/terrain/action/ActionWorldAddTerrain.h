/*
 * ActionWorldAddTerrain.h
 *
 *  Created on: 15.06.2012
 *      Author: michi
 */

#ifndef ACTIONWORLDADDTERRAIN_H_
#define ACTIONWORLDADDTERRAIN_H_

#include <lib/history/Action.h>
#include <lib/math/vec3.h>
#include <lib/os/path.h>

class ActionWorldAddTerrain : public history::Action {
public:
	ActionWorldAddTerrain(const vec3 &_pos, const Path &_filename);
	ActionWorldAddTerrain(const vec3 &_pos, const vec3 &_size, int _num_x, int _num_z);
	string name() const override {	return "WorldAddTerrain";	}

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	Path filename;
	vec3 pos;
	vec3 size;
	int num_x, num_z;
};

#endif /* ACTIONWORLDADDTERRAIN_H_ */
