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



#define WORLD_MAX_TEXTURES_PER_TERRAIN	8

#define CPKSetCamPos	0
#define CPKSetCamPosRel	1
#define CPKSetCamAng	2
#define CPKSetCamPosAng	4
#define CPKCamFlight	10
#define CPKCamFlightRel	11

class ModeWorldCamPoint: public MultiViewSingleData
{
public:
	int Type;
	vector Vel, Ang;
	float Duration;
};

class ModeWorldCamPointVel: public MultiViewSingleData
{
};

struct ModeWorldScriptRule
{
	string Function;
	int Location;
};

struct ModeWorldScript
{
	string Filename;
	Array<ModeWorldScriptRule> Rule;
};

/*class ModeWorldTerrainVertex: public MultiViewSingleData
{
public:
	float Height;
	vector Normal;
};*/

class ModeWorldTerrain: public MultiViewSingleData
{
public:
	string FileName;
	//Array<ModeWorldTerrainVertex> Vertex;
	int VertexBufferSingle;
	int ShowTexture;
	bool Changed;
	CTerrain *terrain;

	bool Load(const vector &pos, const string &filename, bool deep = true);
	void UpdateData();
};

class ModeWorldObject: public MultiViewSingleData
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

	// terrains
	Array<ModeWorldTerrain> Terrain;

	// objects
	Array<ModeWorldObject> Object;
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
		Array<ModeWorldScript> Script;

		// music
		Array<string> MusicFile;

		void Reset();
	};
	MetaData meta_data;



	// camera script
	string CamScriptFile;
	int CamPointBeing;
	Array<ModeWorldCamPoint> CamPoint;
	Array<ModeWorldCamPointVel> CamPointVel;
};

#endif /* DATAWORLD_H_ */
