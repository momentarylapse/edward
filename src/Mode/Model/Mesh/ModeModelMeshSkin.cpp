/*
 * ModeModelMeshSkin.cpp
 *
 *  Created on: 06.03.2012
 *      Author: michi
 */

#include "../../../Edward.h"
#include "../../../MultiView.h"
#include "ModeModelMeshSkin.h"



ModeModelMeshSkin *mode_model_mesh_skin = NULL;

ModeModelMeshSkin::ModeModelMeshSkin(Mode *_parent, DataModel *_data)
{
	name = "ModelMeshSkin";
	parent = _parent;
	data = _data;
	menu = HuiCreateResourceMenu("menu_model");
	multi_view = ed->multi_view_3d;
	Subscribe(data);

	// vertex buffers
	VBMarked = NixCreateVB(65536);
	VBModel = NixCreateVB(65536);
	VBModel2 = -1;
	VBModel3 = -1;
	VBModel4 = -1;
	VBMouseOver = NixCreateVB(1024);
	VBCreation = NixCreateVB(1024);
}

ModeModelMeshSkin::~ModeModelMeshSkin()
{
}

#define GetVertex(v)	data->Vertex[v].pos

void ModeModelMeshSkin::DrawTrias()
{
	msg_db_r("ModelSkin.DrawTrias",2);

	// draw all materials separately
	foreachi(data->Material, m, mi){
		m.ApplyForRendering();

		// single texture
		if (m.NumTextures == 1){
			NixVBClear(VBModel);

			foreach(data->Surface, surf)
				foreach(surf.Triangle, t)
					if ((t.view_stage >= data->ViewStage) && (t.Material == mi)){

						vector tv1 = t.SkinVertex[0][0];
						vector tv2 = t.SkinVertex[0][1];
						vector tv3 = t.SkinVertex[0][2];
						NixVBAddTria(	VBModel,
										GetVertex(t.Vertex[0]), t.Normal[0], tv1.x, tv1.y,
										GetVertex(t.Vertex[1]), t.Normal[1], tv2.x, tv2.y,
										GetVertex(t.Vertex[2]), t.Normal[2], tv3.x, tv3.y);
					}

			// draw
			NixDraw3D(m.Texture[0], VBModel, m_id); // TODO:  alle Texturen, nicht nur die erste....

		// multi texture
		}else{
			int num_tex = m.NumTextures;
			if (num_tex > 4)
				num_tex = 4;
			int *vb = &VBModel2;
			if (num_tex == 3)
				vb = &VBModel3;
			if (num_tex == 4)
				vb = &VBModel4;
			if (*vb < 0)
				*vb = NixCreateVBM(65536, num_tex);

			NixVBClear(*vb);

			foreach(data->Surface, surf)
				foreach(surf.Triangle, t)
					if ((t.view_stage >= data->ViewStage) && (t.Material == mi)){
						float t1[8], t2[8], t3[8];
						for (int tl=0;tl<num_tex;tl++){
							t1[tl*2  ] = t.SkinVertex[tl][0].x;
							t1[tl*2+1] = t.SkinVertex[tl][0].y;
							t2[tl*2  ] = t.SkinVertex[tl][1].x;
							t2[tl*2+1] = t.SkinVertex[tl][1].y;
							t3[tl*2  ] = t.SkinVertex[tl][2].x;
							t3[tl*2+1] = t.SkinVertex[tl][2].y;
						}
						NixVBAddTriaM(	*vb,
										GetVertex(t.Vertex[0]), t.Normal[0], t1,
										GetVertex(t.Vertex[1]), t.Normal[1], t2,
										GetVertex(t.Vertex[2]), t.Normal[2], t3);
					}

			// draw
			NixDraw3DM(m.Texture, *vb, m_id);
		}
	}
	msg_db_l(2);
}

void SetMaterialMarked()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,Red);
}

void SetMaterialMouseOver()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0,0,0),Black,0,White);
}

void SetMaterialCreation()
{
	NixSetAlpha(AlphaMaterial);
	NixSetMaterial(Black,color(0.3f,0.3f,1,0.3f),Black,0,color(1,0.1f,0.4f,0.1f));
}

void ModeModelMeshSkin::DrawWin(int win, irect dest)
{
	msg_db_r("skin.DrawWin",4);
	/*NixSetWire(MVWireMode);
	NixEnableLighting(MVLightEnabled);*/

	/*if (Detail==DetailPhysical){
		SetMaterialPhysical();
		NixDraw3D(-1,VBModel,m_id);
	}else*/
		DrawTrias();
	NixSetShader(-1);
	NixSetWire(false);
	NixSetZ(true,true);
	NixSetAlpha(AlphaNone);
	NixEnableLighting(true);
	msg_db_m("----a",4);

	SetMaterialMarked();
	NixDraw3D(-1,VBMarked,m_id);
	SetMaterialMouseOver();
	NixDraw3D(-1,VBMouseOver,m_id);
	SetMaterialCreation();
	NixDraw3D(-1,VBCreation,m_id);
	NixSetMaterial(White,White,Black,0,Black);
	NixSetAlpha(AlphaNone);



	/*if ((CreationMode == CMTria) || (CreationMode == CMTriaU)){
		for (int i=0;i<SelVertex.num-1;i++)
			NixDrawLine3D(GetVertex(SelVertex[i]), GetVertex(SelVertex[i + 1]), Green);
	}*/
	msg_db_l(4);
}



void ModeModelMeshSkin::OnMiddleButtonDown()
{
}



void ModeModelMeshSkin::End()
{
}



void ModeModelMeshSkin::OnKeyDown()
{
}



void ModeModelMeshSkin::Start()
{
	OnUpdate(data);
}



void ModeModelMeshSkin::OnUpdate(Observable *o)
{
	multi_view->ResetData();
	multi_view->MVRectable = true;
}



void ModeModelMeshSkin::OnRightButtonDown()
{
}



void ModeModelMeshSkin::OnRightButtonUp()
{
}



void ModeModelMeshSkin::OnCommand(const string & id)
{
}



void ModeModelMeshSkin::Draw()
{
}



void ModeModelMeshSkin::OnMiddleButtonUp()
{
}



void ModeModelMeshSkin::OnKeyUp()
{
}



void ModeModelMeshSkin::OnLeftButtonUp()
{
}



void ModeModelMeshSkin::OnLeftButtonDown()
{
}



void ModeModelMeshSkin::OnMouseMove()
{
}


