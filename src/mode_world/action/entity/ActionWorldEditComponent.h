//
// Created by Michael Ankele on 2025-02-03.
//

#ifndef ACTIONWORLDEDITCOMPONENT_H
#define ACTIONWORLDEDITCOMPONENT_H


#include "../../../action/Action.h"
#include "../../data/DataWorld.h"

class ActionWorldEditBaseEntity : public Action {
public:
	explicit ActionWorldEditBaseEntity(int index, const vec3& pos, const quaternion& ang);
	string name() override { return "WorldEditBaseEntity"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	vec3 pos;
	quaternion ang;
};

class ActionWorldEditComponent : public Action {
public:
	explicit ActionWorldEditComponent(int index, const kaba::Class* type, const ecs::InstanceData& c);
	string name() override { return "WorldEditComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	const kaba::Class* type;
	ecs::InstanceData component;
};

class ActionWorldAddComponent : public Action {
public:
	explicit ActionWorldAddComponent(int index, const kaba::Class* type, const Array<ecs::InstanceDataVariable>& variables);
	string name() override { return "WorldAddComponent"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	const kaba::Class* type;
	Array<ecs::InstanceDataVariable> variables;
	ecs::Component* component = nullptr;
};

/*class ActionWorldAddUserComponent : public Action {
public:
	explicit ActionWorldAddUserComponent(int index, const ecs::InstanceData& c);
	string name() override { return "WorldAddUserComponent"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	ecs::InstanceData component;
};*/

class ActionWorldRemoveComponent : public Action {
public:
	explicit ActionWorldRemoveComponent(int index, const kaba::Class* type);
	string name() override { return "WorldRemoveComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	const kaba::Class* type;
	ecs::InstanceData component;
};

class ActionWorldRemoveUnknownComponent : public Action {
public:
	explicit ActionWorldRemoveUnknownComponent(int index, int cindex);
	string name() override { return "WorldRemoveUnknownComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index, cindex;
	ecs::InstanceData component;
};



#endif //ACTIONWORLDEDITCOMPONENT_H
