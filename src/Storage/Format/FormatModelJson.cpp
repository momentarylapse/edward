/*
 * FormatModelJson.cpp
 *
 *  Created on: 20.01.2020
 *      Author: michi
 */

#include "FormatModelJson.h"
#include "../../Edward.h"
#include "../../Data/Model/DataModel.h"
#include "../../Data/Model/ModelMesh.h"
#include "../../Data/Model/ModelPolygon.h"

FormatModelJson::FormatModelJson() : TypedFormat<DataModel>(FD_MODEL, "json", _("Model json"), Flag::READ_WRITE) {
	f = nullptr;
	cur_token = -1;
}

static FormatModelJson::Value ValueNone;

// needs reflection
string vecToJson(const vector &v) {
	return format("%f, %f, %f", -v.x, v.y, v.z);
}

string col3ToJson(const color &c) {
	return format("%f, %f, %f", c.r, c.g, c.b);
}

// needs reflection
string qToJson(const quaternion &q) {
	return format("%f, %f, %f, %f", -q.x, q.y, q.z, -q.w);
}

string materialToJson(ModelMaterial *m)
{
	string str;
	str += "	{\n";
	str += "		'DbgColor' : 15658734,\n";
	str += "		'DbgIndex' : 0,\n";
	str += "		'DbgName' : 'Test',\n";
	str += "		'blending' : 'NormalBlending',\n";
	str += "		'colorAmbient' : [" + col3ToJson(m->col.ambient()) + "],\n";
	str += "		'colorDiffuse' : [" + col3ToJson(m->col.albedo) + "],\n";
	str += "		'colorSpecular' : [" + col3ToJson(m->col.specular()) + "],\n";
	str += "		'depthTest' : true,\n";
	str += "		'depthWrite' : true,\n";
	str += "		'shading' : 'Phong',\n";
	str += "		'specularCoef' : " + f2s(m->col.shininess(),6) + ",\n";
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
	q = quaternion::rotation_v( f.skel_ang[r]);
	return q * (b.pos - pb.pos) + getMoveDPos(m, f, r);
}

void getMoveData(DataModel *m, ModelFrame &f, int b, quaternion &q, vector &t)
{
	q = quaternion::rotation_v( f.skel_ang[b]);
	t = getMoveDPos(m, f, b);//f.skel_dpos[b];
}

string moveToJson(ModelMove &m, DataModel *model)
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

string boneToJson(ModelBone &b)
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


void FormatModelJson::_save(const Path &filename, DataModel *m) {

	File *f = FileCreateText(filename);

	int n_tria = 0;
	for (int ip=0; ip<m->mesh->polygon.num; ip++){
		ModelPolygon &p = m->mesh->polygon[ip];
		n_tria += p.side.num - 2;
	}

	string str = "{'metadata':{\n";
	str += "	'formatVersion': 3,\n";
	str += "	'generatedBy': 'Edward',\n";
	str += "	'vertices': " + i2s(m->mesh->vertex.num) + ",\n";
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
	foreachi(auto *mat, m->material, i){
		str += materialToJson(mat);
		if (i < m->material.num - 1)
			str += ",\n";
	}
	str += "],\n";
	str += "'colors': [],\n";
	str += "'vertices': [\n";
	foreachi(auto &v, m->mesh->vertex, i){
		str += "	" + vecToJson(v.pos);
		if (i < m->mesh->vertex.num - 1)
			str += ",\n";
	}
	str += "],\n";
	str += "'uvs': [[0,0]],\n";
	str += "'normals': [\n";
	foreachi(auto &p, m->mesh->polygon, ip){
		for (int k=0; k<p.side.num; k++){
			str += "	" + vecToJson(p.side[k].normal);
			if ((ip < m->mesh->polygon.num - 1) or (k < p.side.num - 1))
				str += ",\n";
		}
	}
	str += "],\n";
	str += "'faces': [\n";
	int n_normals = 0;
	foreachi(auto &p, m->mesh->polygon, ip){
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
			if ((ip < m->mesh->polygon.num - 1) or (k < p.side.num - 3))
				str += ",\n";
		}
		n_normals += p.side.num;
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
	foreachi(auto &v, m->mesh->vertex, i){
		str += i2s(v.bone_index);
		if (i < m->mesh->vertex.num - 1)
			str += ", ";
	}
	str += "],\n";
	str += "'skinWeights': [";
	foreachi(auto &v, m->mesh->vertex, i){
		str += "1";
		if (i < m->mesh->vertex.num - 1)
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
	f->write_str(str);

	delete(f);
}













inline bool is_whitespace(char c)
{
	return ((c == ' ') or (c == '\n') or (c == '\t') or (c == '\t'));
}

inline bool is_special(char c)
{
	return ((c == '[') or (c == ']') or (c == '{') or (c == '}') or (c == ':') or (c == ','));
}

string FormatModelJson::rnext()
{
	string s;
	for (int i=0; i<256; i++){
		char c = f->read_char();
		if (f->end())
			return "";
		if (is_whitespace(c))
			continue;

		s.add(c);
		if (is_special(c))
			return s;
		bool is_string = (c == '\"');
		for (int j=0; j<256; j++){
			c = f->read_char();
			if (!is_string)
				if ((is_whitespace(c)) or is_special(c)){
					f->seek(-1);
					break;
				}
			s.add(c);
			if (is_string)
				if (c == '\"')
					break;
			if (f->end())
				return s;
		}
		break;
	}
	return s;
}

string FormatModelJson::next()
{
	return tokens[cur_token ++];
}

string FormatModelJson::peek_next()
{
	return tokens[cur_token];
}

FormatModelJson::Value::~Value(){}

class ValueNumber : public FormatModelJson::Value
{
public:
	float data;
	ValueNumber(const string &str)
	{
		data = str._float();
	}
	virtual int i()
	{ return (int)data; }
	virtual float f()
	{ return data; }
};

class ValueString : public FormatModelJson::Value
{
public:
	string data;
	ValueString(const string &str)
	{
		data = str.sub(1, -1);
	}
	virtual string s()
	{ return data; }
};

class ValueArray: public FormatModelJson::Value
{
public:
	Array<Value*> data;
	virtual ~ValueArray()
	{
		for (Value *v: data)
			delete(v);
	}
	virtual int getCount()
	{ return data.num; }
	virtual FormatModelJson::Value *get(int i)
	{
		if ((i >= 0) and (i < data.num))
			return data[i];
		return &ValueNone;
	}
};

class ValueStruct: public FormatModelJson::Value
{
public:
	Array<Value*> data;
	Array<string> keys;
	virtual ~ValueStruct()
	{
		for (Value *v: data)
			delete(v);
	}
	virtual int getCount()
	{ return data.num; }
	virtual Array<string> getKeys()
	{ return keys; }
	virtual FormatModelJson::Value *get(const string &key)
	{
		for (int i=0; i<keys.num; i++)
			if (keys[i] == key)
				return data[i];
		return &ValueNone;
	}
	void add(const string &key, Value *v)
	{
		keys.add(key);
		data.add(v);
	}
};

FormatModelJson::Value *FormatModelJson::read_value()
{
	//msg_db_f("value", 0);
	string s = peek_next();
	//msg_write(s);
	if (s == "{")
		return read_struct();
	if (s == "[")
		return read_array();
	next();
	if (s[0] == '\"')
		return new ValueString(s);
	return new ValueNumber(s);
}

// after {
FormatModelJson::Value *FormatModelJson::read_struct()
{
	//msg_db_f("struct", 0);
	ValueStruct *v = new ValueStruct;
	next();
	while (true){
		string s = peek_next();
		if (s == "}"){
			next();
			break;
		}

		next();
		string key = s.sub(1, -1);
		//msg_write("key: " + key);
		next(); // :
		Value *child = read_value();
		v->add(key, child);

		s = next();
		if (s == ",")
			continue;
		if (s == "}")
			break;
		throw "invalid struct: " + s;

	}
	return v;
}

// after [
FormatModelJson::Value *FormatModelJson::read_array()
{
	//msg_db_f("array", 0);
	ValueArray *v = new ValueArray;
	next();
	while (true){
		string s = peek_next();
		if (s == "]"){
			next();
			break;
		}

		Value *child = read_value();
		v->data.add(child);

		s = next();
		if (s == ",")
			continue;
		if (s == "]")
			break;
		throw "invalid array: " + s;

	}
	return v;
}




void FormatModelJson::importMaterials(DataModel *m, Value *v)
{
	msg_write("materials");
}

void FormatModelJson::importVertices(DataModel *m, Value *v)
{
	msg_write("vertices");
	for (int i=0; i<v->getCount(); i+=3)
		m->addVertex(val2vec(v, i));
}

void FormatModelJson::importPolygons(DataModel *m, Value *v)
{
	msg_write("polys");
	int i = 0;
	while (i < v->getCount()){
		int mode = v->get(i ++)->i();
		//msg_write(mode);
		bool has_quad = (mode & 1);
		bool has_material = (mode & 2);
		bool has_uv = (mode & 4);
		bool has_vertex_uv = (mode & 8);
		bool has_normal = (mode & 16);
		bool has_vertex_normal = (mode & 32);
		bool has_color = (mode & 64);
		bool has_vertex_color = (mode & 128);
		Array<int> vert, uv, norm, col;
		vert.add(v->get(i ++)->i());
		vert.add(v->get(i ++)->i());
		vert.add(v->get(i ++)->i());
		if (has_quad)
			vert.add(v->get(i ++)->i());
		int mat = -1;
		if (has_material)
			mat = v->get(i ++)->i();
		if (has_uv)
			uv.add(v->get(i ++)->i());
		if (has_vertex_uv){
			for (int j=0; j<vert.num; j++)
				uv.add(v->get(i ++)->i());
		}
		if (has_normal)
			norm.add(v->get(i ++)->i());
		if (has_vertex_normal){
			for (int j=0; j<vert.num; j++)
				norm.add(v->get(i ++)->i());
		}
		if (has_color)
			col.add(v->get(i ++)->i());
		if (has_vertex_color){
			for (int j=0; j<vert.num; j++)
				col.add(v->get(i ++)->i());
		}
		m->addPolygon(vert, mat);
		//msg_write(ia2s(vert));
	}
}

void FormatModelJson::importBones(DataModel *m, Value *v)
{
	msg_write("bones");
	ModelBone b;
	b.model = NULL;
	for (int i=0; i<v->getCount(); i++){
		Value *vb = v->get(i);
		b.parent = vb->get("parent")->i();
		b.pos = val2vec(vb->get("pos"));
		if (b.parent >= 0)
			b.pos += m->bone[b.parent].pos;
		m->bone.add(b);
	}
}

void undo_rot(DataModel *m, ModelFrame &f, int b)
{
	int r = m->bone[b].parent;
	while (r >= 0){
		quaternion q, qr, qri;
		q = quaternion::rotation_v( f.skel_ang[b]);
		qr = quaternion::rotation_v( f.skel_ang[r]);
		qri = qr.inverse();
		f.skel_ang[b] = (q * qr).get_angles();
		r = m->bone[r].parent;
		break;
	}
}

void FormatModelJson::importMoves(DataModel *m, Value *v)
{
	msg_write("moves");
	for (int i=0; i<v->getCount(); i++){
		Value *va = v->get(i);
		ModelMove move;
		move.type = MOVE_TYPE_SKELETAL;
		move.name = va->get("name")->s();
		move.frames_per_sec_const = va->get("fps")->f();
		move.frames_per_sec_factor = 0;
		Value *vh = va->get("hierarchy");
		int num_frames = 0;
		for (int j=0; j<vh->getCount(); j++){
			num_frames = max(num_frames, vh->get(j)->get("keys")->getCount());
		}
		msg_write(num_frames);
		move.frame.resize(num_frames);
		foreachi(ModelFrame &f, move.frame, fi){
			f.duration = 1;
			f.skel_ang.resize(m->bone.num);
			f.skel_dpos.resize(m->bone.num);
			for (int b=0; b<m->bone.num; b++){
				f.skel_ang[b] = val2quat(vh->get(b)->get("keys")->get(fi)->get("rot")).get_angles();
				undo_rot(m, f, b);
			}
		}

		m->move.add(move);
	}
}

void FormatModelJson::importBoneIndices(DataModel *m, Value *v, int num_influences)
{
	msg_write("boneIndices");
	foreachi(auto &vert, m->mesh->vertex, i)
		vert.bone_index = v->get(i * num_influences)->i();
}

vector FormatModelJson::val2vec(Value* v, int offset)
{
	return vector(v->get(offset)->f(), v->get(offset+1)->f(), v->get(offset+2)->f());
}

quaternion FormatModelJson::val2quat(Value* v, int offset)
{
	return quaternion(v->get(offset+3)->f(), val2vec(v, offset));
}

color FormatModelJson::val2col3(Value* v, int offset)
{
	return color(1, v->get(offset)->f(), v->get(offset+1)->f(), v->get(offset+2)->f());
}

void FormatModelJson::_load(const Path &filename, DataModel *m, bool deep) {

	m->reset();
	m->action_manager->enable(false);

	try{
	f = FileOpenText(filename);

	msg_write("lexical");
	while (!f->end())
		tokens.add(rnext());
	cur_token = 0;

	delete(f);


	msg_write("parse");
	Value *tree;
	try{
		tree = read_value();
	}catch(string &s){
		msg_error(s);
		for (int i=max(0,cur_token-5); i<min(tokens.num,cur_token+5); i++)
			msg_write(tokens[i]);
	}


	msg_write("model");
	int num_influences = min(tree->get("influencesPerVertex")->i(), 1);
	importMaterials(m, tree->get("materials"));
	importVertices(m, tree->get("vertices"));
	importPolygons(m, tree->get("faces"));
	importBones(m, tree->get("bones"));
	importBoneIndices(m, tree->get("skinIndices"), num_influences);
	importMoves(m, tree->get("animations"));


	delete(tree);

	}catch(...){}

	m->action_manager->enable(true);
}
