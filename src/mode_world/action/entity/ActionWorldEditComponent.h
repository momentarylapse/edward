//
// Created by Michael Ankele on 2025-02-03.
//

#ifndef ACTIONWORLDEDITCOMPONENT_H
#define ACTIONWORLDEDITCOMPONENT_H


#include "../../../action/Action.h"
#include "../../data/DataWorld.h"
#include "../../data/WorldLight.h"
#include "../../data/WorldCamera.h"

class ActionWorldEditBaseEntity : public Action {
public:
	explicit ActionWorldEditBaseEntity(int index, const WorldEntity& e);
	string name() override { return "WorldEditBaseEntity"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	WorldEntity entity;
};

class ActionWorldEditLight : public Action {
public:
	explicit ActionWorldEditLight(int index, const WorldLight& l);
	string name() override { return "WorldEditLight"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	WorldLight light;
};

/*class ActionWorldEditCamera : public Action {
public:
	explicit ActionWorldEditCamera(int index, const WorldCamera& c);
	string name() override { return "WorldEditCamera"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	WorldCamera camera;
};*/

class ActionWorldEditComponent : public Action {
public:
	explicit ActionWorldEditComponent(int index, int cindex, const ScriptInstanceData& c);
	string name() override { return "WorldEditComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index, cindex;
	ScriptInstanceData component;
};

class ActionWorldAddComponent : public Action {
public:
	explicit ActionWorldAddComponent(int index, const ScriptInstanceData& c);
	string name() override { return "WorldAddComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	ScriptInstanceData component;
};

class ActionWorldRemoveComponent : public Action {
public:
	explicit ActionWorldRemoveComponent(int index, int cindex);
	string name() override { return "WorldRemoveComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index, cindex;
	ScriptInstanceData component;
};



#endif //ACTIONWORLDEDITCOMPONENT_H
