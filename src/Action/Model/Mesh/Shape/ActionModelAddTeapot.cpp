/*
 * ActionModelAddTeapot.cpp
 *
 *  Created on: 11.01.2013
 *      Author: michi
 */

#include "ActionModelAddTeapot.h"
#include "../Vertex/ActionModelAddVertex.h"
#include "../Polygon/ActionModelAddPolygonSingleTexture.h"
#include "../Surface/ActionModelSurfaceAutoWeld.h"
#include "../../../../Data/Model/DataModel.h"

// Utah teapot by Martin Newell

const vector tp_vert[] = {
	vector(0,0,0), // lazy index compensation
	vector(1.4,0.0,2.4),
	vector(1.4,-0.784,2.4),
	vector(0.784,-1.4,2.4),
	vector(0.0,-1.4,2.4),
	vector(1.3375,0.0,2.53125),
	vector(1.3375,-0.749,2.53125),
	vector(0.749,-1.3375,2.53125),
	vector(0.0,-1.3375,2.53125),
	vector(1.4375,0.0,2.53125),
	vector(1.4375,-0.805,2.53125),
	vector(0.805,-1.4375,2.53125),
	vector(0.0,-1.4375,2.53125),
	vector(1.5,0.0,2.4),
	vector(1.5,-0.84,2.4),
	vector(0.84,-1.5,2.4),
	vector(0.0,-1.5,2.4),
	vector(-0.784,-1.4,2.4),
	vector(-1.4,-0.784,2.4),
	vector(-1.4,0.0,2.4),
	vector(-0.749,-1.3375,2.53125),
	vector(-1.3375,-0.749,2.53125),
	vector(-1.3375,0.0,2.53125),
	vector(-0.805,-1.4375,2.53125),
	vector(-1.4375,-0.805,2.53125),
	vector(-1.4375,0.0,2.53125),
	vector(-0.84,-1.5,2.4),
	vector(-1.5,-0.84,2.4),
	vector(-1.5,0.0,2.4),
	vector(-1.4,0.784,2.4),
	vector(-0.784,1.4,2.4),
	vector(0.0,1.4,2.4),
	vector(-1.3375,0.749,2.53125),
	vector(-0.749,1.3375,2.53125),
	vector(0.0,1.3375,2.53125),
	vector(-1.4375,0.805,2.53125),
	vector(-0.805,1.4375,2.53125),
	vector(0.0,1.4375,2.53125),
	vector(-1.5,0.84,2.4),
	vector(-0.84,1.5,2.4),
	vector(0.0,1.5,2.4),
	vector(0.784,1.4,2.4),
	vector(1.4,0.784,2.4),
	vector(0.749,1.3375,2.53125),
	vector(1.3375,0.749,2.53125),
	vector(0.805,1.4375,2.53125),
	vector(1.4375,0.805,2.53125),
	vector(0.84,1.5,2.4),
	vector(1.5,0.84,2.4),
	vector(1.75,0.0,1.875),
	vector(1.75,-0.98,1.875),
	vector(0.98,-1.75,1.875),
	vector(0.0,-1.75,1.875),
	vector(2.0,0.0,1.35),
	vector(2.0,-1.12,1.35),
	vector(1.12,-2.0,1.35),
	vector(0.0,-2.0,1.35),
	vector(2.0,0.0,0.9),
	vector(2.0,-1.12,0.9),
	vector(1.12,-2.0,0.9),
	vector(0.0,-2.0,0.9),
	vector(-0.98,-1.75,1.875),
	vector(-1.75,-0.98,1.875),
	vector(-1.75,0.0,1.875),
	vector(-1.12,-2.0,1.35),
	vector(-2.0,-1.12,1.35),
	vector(-2.0,0.0,1.35),
	vector(-1.12,-2.0,0.9),
	vector(-2.0,-1.12,0.9),
	vector(-2.0,0.0,0.9),
	vector(-1.75,0.98,1.875),
	vector(-0.98,1.75,1.875),
	vector(0.0,1.75,1.875),
	vector(-2.0,1.12,1.35),
	vector(-1.12,2.0,1.35),
	vector(0.0,2.0,1.35),
	vector(-2.0,1.12,0.9),
	vector(-1.12,2.0,0.9),
	vector(0.0,2.0,0.9),
	vector(0.98,1.75,1.875),
	vector(1.75,0.98,1.875),
	vector(1.12,2.0,1.35),
	vector(2.0,1.12,1.35),
	vector(1.12,2.0,0.9),
	vector(2.0,1.12,0.9),
	vector(2.0,0.0,0.45),
	vector(2.0,-1.12,0.45),
	vector(1.12,-2.0,0.45),
	vector(0.0,-2.0,0.45),
	vector(1.5,0.0,0.225),
	vector(1.5,-0.84,0.225),
	vector(0.84,-1.5,0.225),
	vector(0.0,-1.5,0.225),
	vector(1.5,0.0,0.15),
	vector(1.5,-0.84,0.15),
	vector(0.84,-1.5,0.15),
	vector(0.0,-1.5,0.15),
	vector(-1.12,-2.0,0.45),
	vector(-2.0,-1.12,0.45),
	vector(-2.0,0.0,0.45),
	vector(-0.84,-1.5,0.225),
	vector(-1.5,-0.84,0.225),
	vector(-1.5,0.0,0.225),
	vector(-0.84,-1.5,0.15),
	vector(-1.5,-0.84,0.15),
	vector(-1.5,0.0,0.15),
	vector(-2.0,1.12,0.45),
	vector(-1.12,2.0,0.45),
	vector(0.0,2.0,0.45),
	vector(-1.5,0.84,0.225),
	vector(-0.84,1.5,0.225),
	vector(0.0,1.5,0.225),
	vector(-1.5,0.84,0.15),
	vector(-0.84,1.5,0.15),
	vector(0.0,1.5,0.15),
	vector(1.12,2.0,0.45),
	vector(2.0,1.12,0.45),
	vector(0.84,1.5,0.225),
	vector(1.5,0.84,0.225),
	vector(0.84,1.5,0.15),
	vector(1.5,0.84,0.15),
	vector(-1.6,0.0,2.025),
	vector(-1.6,-0.3,2.025),
	vector(-1.5,-0.3,2.25),
	vector(-1.5,0.0,2.25),
	vector(-2.3,0.0,2.025),
	vector(-2.3,-0.3,2.025),
	vector(-2.5,-0.3,2.25),
	vector(-2.5,0.0,2.25),
	vector(-2.7,0.0,2.025),
	vector(-2.7,-0.3,2.025),
	vector(-3.0,-0.3,2.25),
	vector(-3.0,0.0,2.25),
	vector(-2.7,0.0,1.8),
	vector(-2.7,-0.3,1.8),
	vector(-3.0,-0.3,1.8),
	vector(-3.0,0.0,1.8),
	vector(-1.5,0.3,2.25),
	vector(-1.6,0.3,2.025),
	vector(-2.5,0.3,2.25),
	vector(-2.3,0.3,2.025),
	vector(-3.0,0.3,2.25),
	vector(-2.7,0.3,2.025),
	vector(-3.0,0.3,1.8),
	vector(-2.7,0.3,1.8),
	vector(-2.7,0.0,1.575),
	vector(-2.7,-0.3,1.575),
	vector(-3.0,-0.3,1.35),
	vector(-3.0,0.0,1.35),
	vector(-2.5,0.0,1.125),
	vector(-2.5,-0.3,1.125),
	vector(-2.65,-0.3,0.9375),
	vector(-2.65,0.0,0.9375),
	vector(-2.0,-0.3,0.9),
	vector(-1.9,-0.3,0.6),
	vector(-1.9,0.0,0.6),
	vector(-3.0,0.3,1.35),
	vector(-2.7,0.3,1.575),
	vector(-2.65,0.3,0.9375),
	vector(-2.5,0.3,1.125),
	vector(-1.9,0.3,0.6),
	vector(-2.0,0.3,0.9),
	vector(1.7,0.0,1.425),
	vector(1.7,-0.66,1.425),
	vector(1.7,-0.66,0.6),
	vector(1.7,0.0,0.6),
	vector(2.6,0.0,1.425),
	vector(2.6,-0.66,1.425),
	vector(3.1,-0.66,0.825),
	vector(3.1,0.0,0.825),
	vector(2.3,0.0,2.1),
	vector(2.3,-0.25,2.1),
	vector(2.4,-0.25,2.025),
	vector(2.4,0.0,2.025),
	vector(2.7,0.0,2.4),
	vector(2.7,-0.25,2.4),
	vector(3.3,-0.25,2.4),
	vector(3.3,0.0,2.4),
	vector(1.7,0.66,0.6),
	vector(1.7,0.66,1.425),
	vector(3.1,0.66,0.825),
	vector(2.6,0.66,1.425),
	vector(2.4,0.25,2.025),
	vector(2.3,0.25,2.1),
	vector(3.3,0.25,2.4),
	vector(2.7,0.25,2.4),
	vector(2.8,0.0,2.475),
	vector(2.8,-0.25,2.475),
	vector(3.525,-0.25,2.49375),
	vector(3.525,0.0,2.49375),
	vector(2.9,0.0,2.475),
	vector(2.9,-0.15,2.475),
	vector(3.45,-0.15,2.5125),
	vector(3.45,0.0,2.5125),
	vector(2.8,0.0,2.4),
	vector(2.8,-0.15,2.4),
	vector(3.2,-0.15,2.4),
	vector(3.2,0.0,2.4),
	vector(3.525,0.25,2.49375),
	vector(2.8,0.25,2.475),
	vector(3.45,0.15,2.5125),
	vector(2.9,0.15,2.475),
	vector(3.2,0.15,2.4),
	vector(2.8,0.15,2.4),
	vector(0.0,0.0,3.15),
	vector(0.0,-0.002,3.15),
	vector(0.002,0.0,3.15),
	vector(0.8,0.0,3.15),
	vector(0.8,-0.45,3.15),
	vector(0.45,-0.8,3.15),
	vector(0.0,-0.8,3.15),
	vector(0.0,0.0,2.85),
	vector(0.2,0.0,2.7),
	vector(0.2,-0.112,2.7),
	vector(0.112,-0.2,2.7),
	vector(0.0,-0.2,2.7),
	vector(-0.002,0.0,3.15),
	vector(-0.45,-0.8,3.15),
	vector(-0.8,-0.45,3.15),
	vector(-0.8,0.0,3.15),
	vector(-0.112,-0.2,2.7),
	vector(-0.2,-0.112,2.7),
	vector(-0.2,0.0,2.7),
	vector(0.0,0.002,3.15),
	vector(-0.8,0.45,3.15),
	vector(-0.45,0.8,3.15),
	vector(0.0,0.8,3.15),
	vector(-0.2,0.112,2.7),
	vector(-0.112,0.2,2.7),
	vector(0.0,0.2,2.7),
	vector(0.45,0.8,3.15),
	vector(0.8,0.45,3.15),
	vector(0.112,0.2,2.7),
	vector(0.2,0.112,2.7),
	vector(0.4,0.0,2.55),
	vector(0.4,-0.224,2.55),
	vector(0.224,-0.4,2.55),
	vector(0.0,-0.4,2.55),
	vector(1.3,0.0,2.55),
	vector(1.3,-0.728,2.55),
	vector(0.728,-1.3,2.55),
	vector(0.0,-1.3,2.55),
	vector(1.3,0.0,2.4),
	vector(1.3,-0.728,2.4),
	vector(0.728,-1.3,2.4),
	vector(0.0,-1.3,2.4),
	vector(-0.224,-0.4,2.55),
	vector(-0.4,-0.224,2.55),
	vector(-0.4,0.0,2.55),
	vector(-0.728,-1.3,2.55),
	vector(-1.3,-0.728,2.55),
	vector(-1.3,0.0,2.55),
	vector(-0.728,-1.3,2.4),
	vector(-1.3,-0.728,2.4),
	vector(-1.3,0.0,2.4),
	vector(-0.4,0.224,2.55),
	vector(-0.224,0.4,2.55),
	vector(0.0,0.4,2.55),
	vector(-1.3,0.728,2.55),
	vector(-0.728,1.3,2.55),
	vector(0.0,1.3,2.55),
	vector(-1.3,0.728,2.4),
	vector(-0.728,1.3,2.4),
	vector(0.0,1.3,2.4),
	vector(0.224,0.4,2.55),
	vector(0.4,0.224,2.55),
	vector(0.728,1.3,2.55),
	vector(1.3,0.728,2.55),
	vector(0.728,1.3,2.4),
	vector(1.3,0.728,2.4),
	vector(0.0,0.0,0.0),
	vector(1.5,0.0,0.15),
	vector(1.5,0.84,0.15),
	vector(0.84,1.5,0.15),
	vector(0.0,1.5,0.15),
	vector(1.5,0.0,0.075),
	vector(1.5,0.84,0.075),
	vector(0.84,1.5,0.075),
	vector(0.0,1.5,0.075),
	vector(1.425,0.0,0.0),
	vector(1.425,0.798,0.0),
	vector(0.798,1.425,0.0),
	vector(0.0,1.425,0.0),
	vector(-0.84,1.5,0.15),
	vector(-1.5,0.84,0.15),
	vector(-1.5,0.0,0.15),
	vector(-0.84,1.5,0.075),
	vector(-1.5,0.84,0.075),
	vector(-1.5,0.0,0.075),
	vector(-0.798,1.425,0.0),
	vector(-1.425,0.798,0.0),
	vector(-1.425,0.0,0.0),
	vector(-1.5,-0.84,0.15),
	vector(-0.84,-1.5,0.15),
	vector(0.0,-1.5,0.15),
	vector(-1.5,-0.84,0.075),
	vector(-0.84,-1.5,0.075),
	vector(0.0,-1.5,0.075),
	vector(-1.425,-0.798,0.0),
	vector(-0.798,-1.425,0.0),
	vector(0.0,-1.425,0.0),
	vector(0.84,-1.5,0.15),
	vector(1.5,-0.84,0.15),
	vector(0.84,-1.5,0.075),
	vector(1.5,-0.84,0.075),
	vector(0.798,-1.425,0.0),
	vector(1.425,-0.798,0.0)
};

ActionModelAddTeapot::ActionModelAddTeapot(const vector &_pos, float _radius, int _samples)
{
	pos = _pos;
	radius = _radius;
	samples = _samples;
}

static float Bernstein3(int i, float t)
{
	float ti = 1 - t;
	if (i == 0)
		return ti*ti*ti;
	if (i == 1)
		return 3*t*ti*ti;
	if (i == 2)
		return 3*t*t*ti;
	return t*t*t;
}

void ActionModelAddTeapot::addBezier(int v00, int v01, int v02, int v03, int v10, int v11, int v12, int v13, int v20, int v21, int v22, int v23, int v30, int v31, int v32, int v33, DataModel *m)
{
	int nv = m->Vertex.num;
	float r = radius / 3;
	int N = samples;
	int v[4][4] = {{v00, v01, v02, v03}, {v10, v11, v12, v13}, {v20, v21, v22, v23}, {v30, v31, v32, v33}};
	for (int i=0; i<=N; i++)
		for (int j=0; j<=N; j++){
			float ti = (float)i / (float)N;
			float tj = (float)j / (float)N;
			vector p = v_0;
			for (int k=0; k<4; k++)
				for (int l=0; l<4; l++)
					p += Bernstein3(k, ti) * Bernstein3(l, tj) * tp_vert[v[k][l]];
			AddSubAction(new ActionModelAddVertex(pos + p * r), m);
		}
	for (int i=0; i<N; i++)
		for (int j=0; j<N; j++){
			Array<int> v;
			v.add(nv + i*(N+1)+j);
			v.add(nv + i*(N+1)+j+1);
			v.add(nv + (i+1)*(N+1)+j+1);
			v.add(nv + (i+1)*(N+1)+j);
			Array<vector> sv;
			sv.add(vector((float) i    / (float)N, (float) j    / (float)N, 0));
			sv.add(vector((float) i    / (float)N, (float)(j+1) / (float)N, 0));
			sv.add(vector((float)(i+1) / (float)N, (float)(j+1) / (float)N, 0));
			sv.add(vector((float)(i+1) / (float)N, (float) j    / (float)N, 0));
			AddSubAction(new ActionModelAddPolygonSingleTexture(v, m->CurrentMaterial, sv), m);
		}
}


void *ActionModelAddTeapot::compose(Data *d)
{
	DataModel *m = dynamic_cast<DataModel*>(d);
	float epsilon = radius / samples * 0.01f;

	addBezier(1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,m);
	addBezier(4,17,18,19,8,20,21,22,12,23,24,25,16,26,27,28,m);
	addBezier(19,29,30,31,22,32,33,34,25,35,36,37,28,38,39,40,m);
	addBezier(31,41,42,1,34,43,44,5,37,45,46,9,40,47,48,13,m);
	addBezier(13,14,15,16,49,50,51,52,53,54,55,56,57,58,59,60,m);
	addBezier(16,26,27,28,52,61,62,63,56,64,65,66,60,67,68,69,m);
	addBezier(28,38,39,40,63,70,71,72,66,73,74,75,69,76,77,78,m);
	addBezier(40,47,48,13,72,79,80,49,75,81,82,53,78,83,84,57,m);
	addBezier(57,58,59,60,85,86,87,88,89,90,91,92,93,94,95,96,m);
	addBezier(60,67,68,69,88,97,98,99,92,100,101,102,96,103,104,105,m);
	addBezier(69,76,77,78,99,106,107,108,102,109,110,111,105,112,113,114,m);
	addBezier(78,83,84,57,108,115,116,85,111,117,118,89,114,119,120,93,m);
	addBezier(270,270,270,270,279,280,281,282,275,276,277,278,271,272,273,274,m);
	addBezier(270,270,270,270,282,289,290,291,278,286,287,288,274,283,284,285,m);
	addBezier(270,270,270,270,291,298,299,300,288,295,296,297,285,292,293,294,m);
	addBezier(270,270,270,270,300,305,306,279,297,303,304,275,294,301,302,271,m);
	for (int i=0; i<15; i++)
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, epsilon), m);
	addBezier(121,122,123,124,125,126,127,128,129,130,131,132,133,134,135,136,m);
	addBezier(124,137,138,121,128,139,140,125,132,141,142,129,136,143,144,133,m);
	addBezier(133,134,135,136,145,146,147,148,149,150,151,152,69,153,154,155,m);
	addBezier(136,143,144,133,148,156,157,145,152,158,159,149,155,160,161,69,m);
	for (int i=0; i<3; i++)
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, epsilon), m);
	addBezier(162,163,164,165,166,167,168,169,170,171,172,173,174,175,176,177,m);
	addBezier(165,178,179,162,169,180,181,166,173,182,183,170,177,184,185,174,m);
	addBezier(174,175,176,177,186,187,188,189,190,191,192,193,194,195,196,197,m);
	addBezier(177,184,185,174,189,198,199,186,193,200,201,190,197,202,203,194,m);
	for (int i=0; i<3; i++)
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, epsilon), m);
	addBezier(204,204,204,204,207,208,209,210,211,211,211,211,212,213,214,215,m);
	addBezier(204,204,204,204,210,217,218,219,211,211,211,211,215,220,221,222,m);
	addBezier(204,204,204,204,219,224,225,226,211,211,211,211,222,227,228,229,m);
	addBezier(204,204,204,204,226,230,231,207,211,211,211,211,229,232,233,212,m);
	addBezier(212,213,214,215,234,235,236,237,238,239,240,241,242,243,244,245,m);
	addBezier(215,220,221,222,237,246,247,248,241,249,250,251,245,252,253,254,m);
	addBezier(222,227,228,229,248,255,256,257,251,258,259,260,254,261,262,263,m);
	addBezier(229,232,233,212,257,264,265,234,260,266,267,238,263,268,269,242,m);
	for (int i=0; i<7; i++)
		AddSubAction(new ActionModelSurfaceAutoWeld(m->Surface.num - 2, m->Surface.num - 1, epsilon), m);

	return &m->Surface.back();
}
