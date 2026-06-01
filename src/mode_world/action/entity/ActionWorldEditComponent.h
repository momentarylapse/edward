//
// Created by Michael Ankele on 2025-02-03.
//

#ifndef ACTIONWORLDEDITCOMPONENT_H
#define ACTIONWORLDEDITCOMPONENT_H


#include <lib/history/Action.h>
#include "../../data/DataWorld.h"

class ActionWorldEditBaseEntity : public history::Action {
public:
	explicit ActionWorldEditBaseEntity(int index, const vec3& pos, const quaternion& ang);
	string name() const override { return "WorldEditBaseEntity"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	vec3 pos;
	quaternion ang;
};

class ActionWorldEditComponent : public history::Action {
public:
	explicit ActionWorldEditComponent(int index, const kaba::Class* type, const ecs::InstanceData& c);
	string name() const override { return "WorldEditComponent"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	const kaba::Class* type;
	ecs::InstanceData component;
};

class ActionWorldAddComponent : public history::Action {
public:
	explicit ActionWorldAddComponent(int index, const kaba::Class* type, const Array<ecs::InstanceDataVariable>& variables);
	string name() const override { return "WorldAddComponent"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	const kaba::Class* type;
	Array<ecs::InstanceDataVariable> variables;
	ecs::Component* component = nullptr;
};

/*class ActionWorldAddUserComponent : public history::Action {
public:
	explicit ActionWorldAddUserComponent(int index, const ecs::InstanceData& c);
	string name() const override { return "WorldAddUserComponent"; }

	void* execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	ecs::InstanceData component;
};*/

class ActionWorldRemoveComponent : public history::Action {
public:
	explicit ActionWorldRemoveComponent(int index, const kaba::Class* type);
	string name() const override { return "WorldRemoveComponent"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index;
	const kaba::Class* type;
	ecs::InstanceData component;
};

class ActionWorldRemoveUnknownComponent : public history::Action {
public:
	explicit ActionWorldRemoveUnknownComponent(int index, int cindex);
	string name() const override { return "WorldRemoveUnknownComponent"; }

	void *execute(history::Data* d) override;
	void undo(history::Data* d) override;

private:
	int index, cindex;
	ecs::InstanceData component;
};



#endif //ACTIONWORLDEDITCOMPONENT_H
