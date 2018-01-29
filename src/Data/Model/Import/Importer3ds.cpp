/*
 * Importer3ds.cpp
 *
 *  Created on: 07.03.2013
 *      Author: michi
 */

#include "Importer3ds.h"

Importer3ds::Importer3ds()
{
}

Importer3ds::~Importer3ds()
{
}

bool Importer3ds::Import(DataModel *m, const string &filename)
{
	m->reset();

	File *f = FileOpen(filename);

	while(true){
		int id = f->read_word();
		int length = f->read_int();
		switch (id){
			case 0x4d4d:
				msg_write("Main3DS");
				break;
			case 0x3d3d:
				msg_write("Edit3DS");
				break;
			case 0x3e3d:
				msg_write("EditConfig2");
				break;
			case 0x4000:
				msg_write("EditObject");
				while(true){
					char c=f->read_byte();
					if (c==0)
						break;
				}
				break;
			case 0x4100:
				msg_write("ObjTrimesh");
				LoadMesh(m, f, length);
				break;
			case 0x0000:
				msg_error("interner Fehler... (0x0000)");
				break;
			default:
				msg_write(format("-unbekannt-  %x - %d", id, length));
				if (length>10000000){
					msg_error("interner Fehler... (zu gro&se Abschnittsl&ange!)");
					return false;
				}
				f->seek(length - 6);
				break;
		}
		if (f->get_pos() >= f->get_size())
			break;
	}

	FileClose(f);

	m->importFromTriangleSkin(1);
	m->resetHistory();
	return true;
}

void Importer3ds::LoadMesh(DataModel *m, File *f, int _length)
{
	msg_right();
	int NumVerticesOld = m->skin[1].vertex.num;
	int end_pos = f->get_pos() + _length - 6;
	Array<vector> skin_vert;
	while(f->get_pos() < end_pos){
		int id = f->read_word();
		int length = f->read_int();
		switch (id){
			case 0x4110:
				{msg_write("TriVertexList");
				int nv=f->read_word();
				msg_write(format("\t\t\t\tNumVertices: %d",nv));
				for (int i=0;i<nv;i++){
					ModelVertex v;
					v.pos.x = f->read_float();
					v.pos.y = f->read_float();
					v.pos.z = f->read_float();
					v.normal_mode = NORMAL_MODE_ANGULAR;
					m->skin[1].vertex.add(v);
				}
				}break;
			case 0x4120:
				{msg_write("TriFaceList");
				int nt=f->read_word();
				msg_write(format("\t\t\t\tNumTriangles: %d",nt));
				for (int i=0;i<nt;i++){
					ModelTriangle t;
					t.vertex[0] = f->read_word() + NumVerticesOld;
					t.vertex[1] = f->read_word() + NumVerticesOld;
					t.vertex[2] = f->read_word() + NumVerticesOld;
					f->read_word();
					m->skin[1].sub[0].triangle.add(t);
				}
				}break;
			case 0x4140:
				{msg_write("TriMappingList");
				int nv=f->read_word();
				skin_vert.resize(nv);
				msg_write(format("\t\t\t\tNumVertices: %d",nv));
				for (int i=0;i<nv;i++){
					skin_vert[i].x = f->read_float();
					skin_vert[i].y = 1 - f->read_float();
				}
				}break;
			default:
				msg_write(format("-unbekannt-  %x - %d", id, length));
				f->seek(length - 6);
				break;
		}
	}
	for (ModelTriangle &t: m->skin[1].sub[0].triangle)
		for (int k=0;k<3;k++)
			if ((t.vertex[k] >= NumVerticesOld) && (t.vertex[k] < NumVerticesOld + skin_vert.num))
					t.skin_vertex[0][k] = skin_vert[t.vertex[k] - NumVerticesOld];
	msg_left();
}

