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


/*class WorldTerrainVertex: public MultiView::SingleData
{
public:
	float Height;
	vector Normal;
};*/

class WorldTerrain: public MultiView::SingleData
{
public:
	string FileName;
	//Array<WorldTerrainVertex> Vertex;
	int VertexBufferSingle;
	int ShowTexture;
	bool Changed;
	Terrain *terrain;

	bool Load(const vector &pos, const string &filename, bool deep = true);
	bool Save(const string &filename);
	virtual bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(MultiView::Window *win, rect &r, void *user_data);
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

class WorldObject: public MultiView::SingleData
{
public:
	string FileName, Name;
	Object *object;
	vector Ang;

	virtual bool hover(MultiView::Window *win, vector &m, vector &tp, float &z, void *user_data);
	virtual bool inRect(MultiView::Window *win, rect &r, void *user_data);
	void UpdateData();
};

class WorldLink: public MultiView::SingleData
{
public:
	int type;
	int object[2];
	vector axis[2];
	float param[4];
	float friction;
};

class WorldScriptVariable
{
public:
	string name;
	string type;
	string value;
};

class WorldScript
{
public:
	string filename;
	string class_name;
	Array<WorldScriptVariable> variables;
};

class DataWorld: public Data
{
public:
	DataWorld();
	virtual ~DataWorld();

	void reset();
	bool load(const string &_filename, bool deep = true);
	bool save(const string &_filename);


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

		// light
		bool SunEnabled;
		vector SunAng;
		color SunAmbient;
		color SunDiffuse;
		color SunSpecular;
		color Ambient;

		// scripts
		Array<WorldScript> scripts;

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
	void DeleteSelection();
};

#endif /* DATAWORLD_H_ */
