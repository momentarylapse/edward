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
#include "../../lib/x/x.h"

class DataWorld;


#define WORLD_MAX_TEXTURES_PER_TERRAIN	8

#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11

class WorldCamPoint: public MultiViewSingleData
{
public:
	int Type;
	vector Vel, Ang;
	float Duration;
};

class WorldCamPointVel: public MultiViewSingleData
{
};

struct WorldScriptRule
{
	string Function;
	int Location;
};

struct WorldScript
{
	string Filename;
	Array<WorldScriptRule> Rule;
};

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
	CTerrain *terrain;

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
	string TextureFile[TERRAIN_MAX_TEXTURES];
	vector TextureScale[TERRAIN_MAX_TEXTURES];
};

class WorldObject: public MultiViewSingleData
{
public:
	string FileName, Name;
	CObject *object;
	vector Ang;

	void UpdateData();
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

	// terrains
	Array<WorldTerrain> Terrain;

	// objects
	Array<WorldObject> Object;
	int EgoIndex;

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
		Array<WorldScript> Script;

		// music
		Array<string> MusicFile;

		void Reset();
		void DrawBackground();
		void ApplyToDraw();
	};
	MetaData meta_data;



	// camera script
	string CamScriptFile;
	int CamPointBeing;
	Array<WorldCamPoint> CamPoint;
	Array<WorldCamPointVel> CamPointVel;


	// actions
	WorldObject *AddObject(const string &filename, const vector &pos);
	WorldTerrain *AddTerrain(const string &filename, const vector &pos);
	WorldTerrain *AddNewTerrain(const vector &pos, const vector &size, int num_x, int num_z);
};

#endif /* DATAWORLD_H_ */
