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
	explicit ActionWorldEditComponent(int index, const kaba::Class* type, const ScriptInstanceData& c);
	string name() override { return "WorldEditComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	const kaba::Class* type;
	ScriptInstanceData component;
};

class ActionWorldAddComponent : public Action {
public:
	explicit ActionWorldAddComponent(int index, const kaba::Class* type, const base::map<string, Any>& variables);
	string name() override { return "WorldAddComponent"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	const kaba::Class* type;
	base::map<string, Any> variables;
	Component* component = nullptr;
};

class ActionWorldAddUserComponent : public Action {
public:
	explicit ActionWorldAddUserComponent(int index, const ScriptInstanceData& c);
	string name() override { return "WorldAddUserComponent"; }

	void* execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	ScriptInstanceData component;
};

class ActionWorldRemoveComponent : public Action {
public:
	explicit ActionWorldRemoveComponent(int index, const kaba::Class* type);
	string name() override { return "WorldRemoveComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index;
	const kaba::Class* type;
	ScriptInstanceData component;
};

class ActionWorldRemoveUserComponent : public Action {
public:
	explicit ActionWorldRemoveUserComponent(int index, int cindex);
	string name() override { return "WorldRemoveUserComponent"; }

	void *execute(Data* d) override;
	void undo(Data* d) override;

private:
	int index, cindex;
	ScriptInstanceData component;
};



#endif //ACTIONWORLDEDITCOMPONENT_H
