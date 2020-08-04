/*
 * DataWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAWORLD_H_
#define DATAWORLD_H_

#include "../Data.h"
#include "../../MultiView/SingleData.h"
#include "../../x/material.h"

class DataWorld;
class Terrain;
class Object;
class WorldLink;
class WorldLight;
class WorldObject;
class WorldTerrain;
class WorldCamera;




class WorldScriptVariable {
public:
	string name;
	string type;
	string value;
};

class WorldScript {
public:
	Path filename;
	string class_name;
	Array<WorldScriptVariable> variables;
};

class DataWorld: public Data {
public:
	DataWorld();
	virtual ~DataWorld();

	void reset() override;


	void GetBoundaryBox(vector &min, vector &max);
	int GetSelectedObjects();
	int GetSelectedTerrains();
	int get_selected_cameras();
	int get_selected_lights();

	void UpdateData();

	void ClearSelection();

	// terrains
	Array<WorldTerrain> terrains;

	// objects
	Array<WorldObject> objects;
	int EgoIndex;

	Array<WorldLink> links;

	Array<WorldLight> lights;
	Array<WorldCamera> cameras;

	struct MetaData {

		// physics
		bool PhysicsEnabled;
		vector Gravity;

		// background
		color BackGroundColor;
		Array<Path> SkyBoxFile;

		// fog
		bool FogEnabled;
		int FogMode;
		float FogStart;
		float FogEnd;
		float FogDensity;
		color FogColor;

		// scripts
		Array<WorldScript> scripts;

		// music
		Array<Path> MusicFile;

		void Reset();
	};
	MetaData meta_data;



	// actions
	WorldObject *AddObject(const Path &filename, const vector &pos);
	WorldTerrain *AddTerrain(const Path &filename, const vector &pos);
	WorldTerrain *AddNewTerrain(const vector &pos, const vector &size, int num_x, int num_z);
	void Copy(Array<WorldObject> &objects, Array<WorldTerrain> &terrains); // actually not an action
	void Paste(Array<WorldObject> &objects, Array<WorldTerrain> &terrains);
	void DeleteSelection();
};

#endif /* DATAWORLD_H_ */
