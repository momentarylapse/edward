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
	m->Reset();

	CFile *f = OpenFile(filename);
	f->SetBinaryMode(true);

	while(true){
		int id = f->ReadWord();
		int length = f->ReadInt();
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
					char c=f->ReadByte();
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
				f->SetPos(length - 6, false);
				break;
		}
		if (f->GetPos() >= f->GetSize())
			break;
	}

	FileClose(f);

	m->ImportFromTriangleSkin(1);
	m->ResetHistory();
	return true;
}

void Importer3ds::LoadMesh(DataModel *m, CFile *f, int _length)
{
	msg_right();
	int NumVerticesOld = m->Skin[1].Vertex.num;
	int end_pos = f->GetPos() + _length - 6;
	Array<vector> skin_vert;
	while(f->GetPos() < end_pos){
		int id = f->ReadWord();
		int length = f->ReadInt();
		switch (id){
			case 0x4110:
				{msg_write("TriVertexList");
				int nv=f->ReadWord();
				msg_write(format("\t\t\t\tNumVertices: %d",nv));
				for (int i=0;i<nv;i++){
					ModelVertex v;
					v.pos.x = f->ReadFloat();
					v.pos.y = f->ReadFloat();
					v.pos.z = f->ReadFloat();
					v.NormalMode = m->Skin[1].NormalModeAll;
					m->Skin[1].Vertex.add(v);
				}
				}break;
			case 0x4120:
				{msg_write("TriFaceList");
				int nt=f->ReadWord();
				msg_write(format("\t\t\t\tNumTriangles: %d",nt));
				for (int i=0;i<nt;i++){
					ModelTriangle t;
					t.Vertex[0] = f->ReadWord() + NumVerticesOld;
					t.Vertex[1] = f->ReadWord() + NumVerticesOld;
					t.Vertex[2] = f->ReadWord() + NumVerticesOld;
					f->ReadWord();
					m->Skin[1].Sub[0].Triangle.add(t);
				}
				}break;
			case 0x4140:
				{msg_write("TriMappingList");
				int nv=f->ReadWord();
				skin_vert.resize(nv);
				msg_write(format("\t\t\t\tNumVertices: %d",nv));
				for (int i=0;i<nv;i++){
					skin_vert[i].x = f->ReadFloat();
					skin_vert[i].y = 1 - f->ReadFloat();
				}
				}break;
			default:
				msg_write(format("-unbekannt-  %x - %d", id, length));
				f->SetPos(length - 6, false);
				break;
		}
	}
	foreach(ModelTriangle &t, m->Skin[1].Sub[0].Triangle)
		for (int k=0;k<3;k++)
			if ((t.Vertex[k] >= NumVerticesOld) && (t.Vertex[k] < NumVerticesOld + skin_vert.num))
					t.SkinVertex[0][k] = skin_vert[t.Vertex[k] - NumVerticesOld];
	msg_left();
}
