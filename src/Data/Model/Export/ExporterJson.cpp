/*
 * ExporterJson.cpp
 *
 *  Created on: 17.01.2015
 *      Author: michi
 */

#include "ExporterJson.h"

ExporterJson::ExporterJson()
{
}

ExporterJson::~ExporterJson()
{
}

// needs reflection
string ExporterJson::vecToJson(vector &v)
{
	return format("%f, %f, %f", -v.x, v.y, v.z);
}

string ExporterJson::col3ToJson(color &c)
{
	return format("%f, %f, %f", c.r, c.g, c.b);
}

// needs reflection
string ExporterJson::qToJson(quaternion &q)
{
	return format("%f, %f, %f, %f", -q.x, q.y, q.z, -q.w);
}

string ExporterJson::materialToJson(ModelMaterial &m)
{
	string str;
	str += "	{\n";
	str += "		'DbgColor' : 15658734,\n";
	str += "		'DbgIndex' : 0,\n";
	str += "		'DbgName' : 'Test',\n";
	str += "		'blending' : 'NormalBlending',\n";
	str += "		'colorAmbient' : [" + col3ToJson(m.ambient) + "],\n";
	str += "		'colorDiffuse' : [" + col3ToJson(m.diffuse) + "],\n";
	str += "		'colorSpecular' : [" + col3ToJson(m.specular) + "],\n";
	str += "		'depthTest' : true,\n";
	str += "		'depthWrite' : true,\n";
	str += "		'shading' : 'Phong',\n";
	str += "		'specularCoef' : " + f2s(m.shininess,6) + ",\n";
	str += "		'transparency' : 1.0,\n";
	str += "		'transparent' : false,\n";
	str += "		'vertexColors' : false\n";
	str += "	}";
	return str;
}

vector getMoveDPos(DataModel *m, ModelFrame &f, int bi)
{
	ModelBone &b = m->bone[bi];
	int r = b.parent;
	if (r < 0)
		return b.pos + f.skel_dpos[bi];
	ModelBone &pb = m->bone[r];
	quaternion q;
	QuaternionRotationV(q, f.skel_ang[r]);
	return q * (b.pos - pb.pos) + getMoveDPos(m, f, r);
}

void getMoveData(DataModel *m, ModelFrame &f, int b, quaternion &q, vector &t)
{
	QuaternionRotationV(q, f.skel_ang[b]);
	t = getMoveDPos(m, f, b);//f.skel_dpos[b];
}

string ExporterJson::moveToJson(ModelMove &m, DataModel *model)
{
	quaternion q;
	vector t;
	string str;
	str += "	{\n";
	str += "		'name': '" + m.name + "',\n";
	str += "		'fps': " + f2s(m.frames_per_sec_const,6) + ",\n";
	str += "		'length': " + f2s(m.frame.num / m.frames_per_sec_const, 6) + ",\n";
	str += "		'hierarchy': [\n";
	foreachi(ModelBone &b, model->bone, i){
		str += "			{\n";
		str += "				'parent': -1,\n"; //+ i2s(b.parent) + ",\n";
		str += "				'keys': [\n";
		foreachi(ModelFrame &f, m.frame, j){
			getMoveData(model, f, i, q, t);
			str += "					{\n";
			str += "						'time': " + f2s(j / m.frames_per_sec_const, 6) + ",\n";
			str += "						'pos': [" + vecToJson(t) + "],\n";
			str += "						'rot': [" + qToJson(q) + "],\n";
			str += "						'scl': [1,1,1]\n";
			str += "					},\n";
		}
		getMoveData(model, m.frame[0], i, q, t);
		str += "					{\n";
		str += "						'time': " + f2s(m.frame.num / m.frames_per_sec_const, 6) + ",\n";
		str += "						'pos': [" + vecToJson(t) + "],\n";
		str += "						'rot': [" + qToJson(q) + "],\n";
		str += "						'scl': [1,1,1]\n";
		str += "					}\n";
		str += "				]\n";
		if (i < model->bone.num - 1)
			str += "			},\n";
		else
			str += "			}\n";
	}
	str += "		]\n";
	str += "	}";
	return str;
}

string ExporterJson::boneToJson(ModelBone &b)
{
	string str;
	str += "	{\n";
	str += "		'parent': -1,\n";// + i2s(b.parent) + ",\n";
	str += "		'name': 'xxx',\n";
	str += "		'pos': [" + vecToJson(b.pos) + "],\n";
	str += "		'scl': [1,1,1],\n";
	str += "		'rot': [0,0,0],\n";
	str += "		'rotq': [0,0,0,1]\n";
	str += "	}";
	return str;
}

bool ExporterJson::Export(DataModel *m, const string &filename)
{
	CFile *f = FileCreate(filename);

	int n_tria = 0;
	for (int is=0; is<m->surface.num; is++){
		ModelSurface &s = m->surface[is];
		for (int ip=0; ip<s.polygon.num; ip++){
			ModelPolygon &p = s.polygon[ip];
			n_tria += p.side.num - 2;
		}
	}

	string str = "{'metadata':{\n";
	str += "	'formatVersion': 3,\n";
	str += "	'generatedBy': 'Edward',\n";
	str += "	'vertices': " + i2s(m->vertex.num) + ",\n";
	str += "	'faces': " + i2s(n_tria) + ",\n";
	str += "	'normals': " + i2s(n_tria*3) + ",\n";
	str += "	'colors': 0,\n";
	str += "	'uvs': 1,\n";
	str += "	'morphTargets': 0,\n";
	str += "	'bones': " + i2s(m->bone.num) + "\n";
	str += "},\n";
	str += "'influencesPerVertex': 1,\n";
	str += "'scale': 1.000,\n";
	/*str += "'materials': [\n"
	str += "	{\n"
	str += "		'DbgColor' : 15658734,\n"
	str += "		'DbgIndex' : 0,\n"
	str += "		'DbgName' : 'Michi 1',\n"
	str += "		'blending' : 'NormalBlending',\n"
	str += "		'colorAmbient' : [0.75, 0.75, 0.75],\n"
	str += "		'colorDiffuse' : [1.0, 1.0, 1.0],\n"
	str += "		'colorSpecular' : [0.5, 0.5, 0.5],\n"
	str += "		'depthTest' : true,\n"
	str += "		'depthWrite' : true,\n"
	str += "		'shading' : 'Phong',\n"
	str += "		'specularCoef' : 50,\n"
	str += "		'transparency' : 1.0,\n"
	str += "		'transparent' : false,\n"
	str += "		'vertexColors' : false\n"
	str += "	}],\n"*/
	str += "'materials': [\n";
	foreachi(ModelMaterial &mat, m->material, i){
		str += materialToJson(mat);
		if (i < m->material.num - 1)
			str += ",\n";
	}
	str += "],\n";
	str += "'colors': [],\n";
	str += "'vertices': [\n";
	foreachi(ModelVertex &v, m->vertex, i){
		str += "	" + vecToJson(v.pos);
		if (i < m->vertex.num - 1)
			str += ",\n";
	}
	str += "],\n";
	str += "'uvs': [[0,0]],\n";
	str += "'normals': [\n";
	foreachi(ModelSurface &s, m->surface, is){
		foreachi(ModelPolygon &p, s.polygon, ip){
			for (int k=0; k<p.side.num; k++){
				str += "	" + vecToJson(p.side[k].normal);
				if ((is < m->surface.num - 1) or (ip < s.polygon.num - 1) or (k < p.side.num - 1))
					str += ",\n";
			}
		}
	}
	str += "],\n";
	str += "'faces': [\n";
	int n_normals = 0;
	foreachi(ModelSurface &s, m->surface, is){
		foreachi(ModelPolygon &p, s.polygon, ip){
			for (int k=0; k<p.side.num-2; k++){
				// reflected coordinates!
				int a = p.side[k].triangulation[0];
				int b = p.side[k].triangulation[1];
				int c = p.side[k].triangulation[2];
				int va = p.side[a].vertex;
				int vb = p.side[b].vertex;
				int vc = p.side[c].vertex;
				int na = n_normals + a;
				int nb = n_normals + b;
				int nc = n_normals + c;
				vector n = p.temp_normal;
				str += format("	42, %d, %d, %d, %d, 0, 0, 0, %d, %d, %d", va, vc, vb, p.material, na, nc, nb);
				if ((is < m->surface.num - 1) or (ip < s.polygon.num - 1) or (k < p.side.num - 3))
					str += ",\n";
			}
			n_normals += p.side.num;
		}
	}
	str += "],\n";
	/*str += "'morphTargets': [\n"
	for int i, 0, data_model.Move.num
		str += moveToJson(data_model.Move[i])
		if i < data_model.Move.num - 1
			str += ",\n"
	str += "],\n"*/
	str += "'bones': [\n";
	foreachi(ModelBone &b, m->bone, i){
		str += boneToJson(b);
		if (i < m->bone.num - 1)
			str += ",\n";
	}
	str += "],\n";
	str += "'skinIndices': [";
	foreachi(ModelVertex &v, m->vertex, i){
		str += i2s(v.bone_index);
		if (i < m->vertex.num - 1)
			str += ", ";
	}
	str += "],\n";
	str += "'skinWeights': [";
	foreachi(ModelVertex &v, m->vertex, i){
		str += "1";
		if (i < m->vertex.num - 1)
			str += ", ";
	}
	str += "],\n";
	str += "'animations': [\n";
	foreachi(ModelMove &move, m->move, i){
		str += moveToJson(move, m);
		if (i < m->move.num - 1)
			str += ",\n";
	}
	str += "]\n";
	str += "}";
	str = str.replace("'", "\"");
	f->WriteStr(str);

	delete(f);
	return true;
}

