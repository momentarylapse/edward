/*
 * ImporterJson.cpp
 *
 *  Created on: 17.01.2015
 *      Author: michi
 */

#include "ImporterJson.h"

static ImporterJson::Value ValueNone;

ImporterJson::ImporterJson() {
}

ImporterJson::~ImporterJson() {
}

inline bool is_whitespace(char c)
{
	return ((c == ' ') or (c == '\n') or (c == '\t') or (c == '\t'));
}

inline bool is_special(char c)
{
	return ((c == '[') or (c == ']') or (c == '{') or (c == '}') or (c == ':') or (c == ','));
}

string ImporterJson::rnext()
{
	string s;
	for (int i=0; i<256; i++){
		char c = f->ReadChar();
		if (f->Eof)
			return "";
		if (is_whitespace(c))
			continue;

		s.add(c);
		if (is_special(c))
			return s;
		bool is_string = (c == '\"');
		for (int j=0; j<256; j++){
			c = f->ReadChar();
			if (!is_string)
				if ((is_whitespace(c)) or is_special(c)){
					f->SetPos(-1, false);
					break;
				}
			s.add(c);
			if (is_string)
				if (c == '\"')
					break;
			if (f->Eof)
				return s;
		}
		break;
	}
	return s;
}

string ImporterJson::next()
{
	return tokens[cur_token ++];
}

string ImporterJson::peek_next()
{
	return tokens[cur_token];
}

ImporterJson::Value::~Value(){}

class ValueNumber : public ImporterJson::Value
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

class ValueString : public ImporterJson::Value
{
public:
	string data;
	ValueString(const string &str)
	{
		data = str.substr(1, str.num - 2);
	}
	virtual string s()
	{ return data; }
};

class ValueArray: public ImporterJson::Value
{
public:
	Array<Value*> data;
	virtual ~ValueArray()
	{
		foreach (Value *v, data)
			delete(v);
	}
	virtual int getCount()
	{ return data.num; }
	virtual ImporterJson::Value *get(int i)
	{
		if ((i >= 0) and (i < data.num))
			return data[i];
		return &ValueNone;
	}
};

class ValueStruct: public ImporterJson::Value
{
public:
	Array<Value*> data;
	Array<string> keys;
	virtual ~ValueStruct()
	{
		foreach (Value *v, data)
			delete(v);
	}
	virtual int getCount()
	{ return data.num; }
	virtual Array<string> getKeys()
	{ return keys; }
	virtual ImporterJson::Value *get(const string &key)
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

ImporterJson::Value *ImporterJson::read_value()
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
ImporterJson::Value *ImporterJson::read_struct()
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
		string key = s.substr(1, s.num - 2);
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
ImporterJson::Value *ImporterJson::read_array()
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

bool ImporterJson::Import(DataModel *m, const string &filename)
{
	m->reset();
	m->action_manager->enable(false);

	f = FileOpen(filename);
	if (!f)
		return false;

	msg_write("lexical");
	while (!f->Eof)
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


	m->action_manager->enable(true);
	return true;
}

void ImporterJson::importMaterials(DataModel *m, Value *v)
{
	msg_write("materials");
}

void ImporterJson::importVertices(DataModel *m, Value *v)
{
	msg_write("vertices");
	for (int i=0; i<v->getCount(); i+=3)
		m->addVertex(val2vec(v, i));
}

void ImporterJson::importPolygons(DataModel *m, Value *v)
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

void ImporterJson::importBones(DataModel *m, Value *v)
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
		QuaternionRotationV(q, f.skel_ang[b]);
		QuaternionRotationV(qr, f.skel_ang[r]);
		qri = qr;
		qri.invert();
		f.skel_ang[b] = (q * qr).get_angles();
		r = m->bone[r].parent;
		break;
	}
}

void ImporterJson::importMoves(DataModel *m, Value *v)
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

void ImporterJson::importBoneIndices(DataModel *m, Value *v, int num_influences)
{
	foreachi(ModelVertex &vert, m->vertex, i)
		vert.bone_index = v->get(i * num_influences)->i();
}

vector ImporterJson::val2vec(Value* v, int offset)
{
	return vector(v->get(offset)->f(), v->get(offset+1)->f(), v->get(offset+2)->f());
}

quaternion ImporterJson::val2quat(Value* v, int offset)
{
	return quaternion(v->get(offset+3)->f(), val2vec(v, offset));
}

color ImporterJson::val2col3(Value* v, int offset)
{
	return color(1, v->get(offset)->f(), v->get(offset+1)->f(), v->get(offset+2)->f());
}
