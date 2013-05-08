/*
 * DataWorld.h
 *
 *  Created on: 08.03.2012
 *      Author: michi
 */

#ifndef DATAWORLD_H_
#define DATAWORLD_H_

#include "../Data.h"
#include "../../MultiView.h"
#include "../../x/material.h"

class DataWorld;
class Terrain;
class Object;


#define WORLD_MAX_TEXTURES_PER_TERRAIN	8

/*class WorldTerrainVertex: public MultiViewSingleData
{
public:
	float Height;
	vector Normal;
};*/

class WorldTerrain: public MultiViewSingleData
{
public:
	string FileName;
	//Array<WorldTerrainVertex> Vertex;
	int VertexBufferSingle;
	int ShowTexture;
	bool Changed;
	Terrain *terrain;

	bool Load(const vector &pos, const string &filename, bool deep = true);
	void UpdateData();
};

class WorldEditingTerrain
{
public:
	string FileName;
	int NumX, NumZ;
	vector Pattern;
	string MaterialFile;
	int NumTextures;
	string TextureFile[MATERIAL_MAX_TEXTURES];
	vector TextureScale[MATERIAL_MAX_TEXTURES];
};

class WorldObject: public MultiViewSingleData
{
public:
	string FileName, Name;
	Object *object;
	vector Ang;

	void UpdateData();
};

class WorldLink: public MultiViewSingleData
{
public:
	int type;
	int object[2];
	vector axis[2];
	float param[4];
	float friction;
};

class DataWorld: public Data
{
public:
	DataWorld();
	virtual ~DataWorld();

	void Reset();
	bool Load(const string &_filename, bool deep = true);
	bool Save(const string &_filename);


	void GetBoundaryBox(vector &min, vector &max);
	int GetSelectedObjects();
	int GetSelectedTerrains();

	void UpdateData();

	void ClearSelection();

	// terrains
	Array<WorldTerrain> Terrains;

	// objects
	Array<WorldObject> Objects;
	int EgoIndex;

	Array<WorldLink> Links;

	struct MetaData{

		// physics
		bool PhysicsEnabled;
		vector Gravity;

		// background
		color BackGroundColor;
		Array<string> SkyBoxFile;

		// fog
		bool FogEnabled;
		int FogMode;
		float FogStart;
		float FogEnd;
		float FogDensity;
		color FogColor;
		Array<float> ScriptVar;

		// light
		bool SunEnabled;
		vector SunAng;
		color SunAmbient;
		color SunDiffuse;
		color SunSpecular;
		color Ambient;

		// scripts
		Array<string> ScriptFile;

		// music
		Array<string> MusicFile;

		void Reset();
		void DrawBackground();
		void ApplyToDraw();
	};
	MetaData meta_data;




	// actions
	WorldObject *AddObject(const string &filename, const vector &pos);
	WorldTerrain *AddTerrain(const string &filename, const vector &pos);
	WorldTerrain *AddNewTerrain(const vector &pos, const vector &size, int num_x, int num_z);
	void Copy(Array<WorldObject> &objects, Array<WorldTerrain> &terrains); // actually not an action
	void Paste(Array<WorldObject> &objects, Array<WorldTerrain> &terrains);
};

#endif /* DATAWORLD_H_ */
