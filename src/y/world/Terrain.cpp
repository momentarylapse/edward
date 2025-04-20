/*----------------------------------------------------------------------------*\
| Terrain                                                                      |
| -> terrain of a world                                                        |
|                                                                              |
| vital properties:                                                            |
|                                                                              |
| last updated: 2008.11.02 (c) by MichiSoft TM                                 |
\*----------------------------------------------------------------------------*/

#include "Terrain.h"
#include "Material.h"
#include "World.h"
#include "../y/EngineData.h"
#include "../helper/ResourceManager.h"
#include "../graphics-impl.h"
#include <lib/math/vec3.h>
#include <lib/math/plane.h>
#include <lib/os/file.h>
#include <lib/os/msg.h>
#include <lib/os/time.h>

const kaba::Class *Terrain::_class = nullptr;


#define Index(x,z)		((x)*(num_z+1)+(z))
#define Index2(t,x,z)	((x)*(t->num_z+1)+(z))
//#define max(a,b)		(((a)>(b))?(a):(b))
//#define min(a,b)		(((a)<(b))?(a):(b))

void Terrain::reset() {
	filename = "";
	error = false;
	num_x = num_z = 0;
	changed = false;
	force_redraw = true;
	vertex_shader_module = "default";
}

Terrain::Terrain() {
	reset();
}

Terrain::Terrain(ResourceManager *resource_manager, const Path &_filename_) : Terrain() {
	load(resource_manager, _filename_);
}

bool Terrain::load(ResourceManager *resource_manager, const Path &_filename_, bool deep) {
	msg_write(format("loading terrain: %s", _filename_));
	msg_right();

	reset();

	filename = _filename_;
	auto f = os::fs::open(engine.map_dir | filename.with(".map"), "rb");
	if (f) {
		//int ffv = f->read_ReadFileFormatVersion();

		char c = f->read_char();
		if (c != 'b')
			return false;
		int ffv = f->read_word();
		if (ffv == 4) {
			f->read_byte();
			// Metrics
			f->read_comment();
			num_x = f->read_int();
			num_z = f->read_int();
			int num = (num_x + 1) * (num_z + 1);
			height.resize(num);
			pattern.x = f->read_float();
			pattern.y = 0;
			pattern.z = f->read_float();
			// Textures
			f->read_comment();
			int num_textures = f->read_int();
			for (int i=0;i<num_textures;i++) {
				f->read_str(); // texture file (deprecated, using material instead)
				texture_scale[i].x = f->read_float();
				texture_scale[i].y = 0.107f + (float)i * 0.231f; // rotation
				texture_scale[i].z = f->read_float();
			}
			// Material
			material_file = f->read_str();
			if (deep) {
				material = resource_manager->load_material(material_file);

				while (num_textures > material->textures.num)
					material->textures.add(resource_manager->load_texture(""));


				// height
				for (int x=0;x<num_x+1;x++)
					for (int z=0;z<num_z+1;z++)
						height[Index(x,z)] = f->read_float();
				for (int x=0;x<num_x/TERRAIN_CHUNK_SIZE+1;x++)
					for (int z=0;z<num_z/TERRAIN_CHUNK_SIZE+1;z++)
						chunk_lod[x][z] = -1;

//#ifdef USING_VULKAN
				vertex_buffer = new VertexBuffer("3f,3f,2f");
//#else
//				vertex_buffer = new VertexBuffer("3f,3f" + string(",2f").repeat(material->textures.num));
//#endif
			}
		} else {
			msg_error(format("wrong file format: %d (4 expected)",ffv));
			error = true;
		}

		delete f;


		// generate normal vectors
		if (deep)
			update(-1, -1, -1, -1, TerrainUpdateAll);
		// bounding box
		min = vec3(0,0,0);
		max = vec3(pattern.x * num_x, 0, pattern.z * num_z);

		changed = false;
		force_redraw = true;
	} else {
		error = true;
	}
	msg_write("/terrain");
	msg_left();
	return !error;
}

Terrain::Terrain(int nx, int nz, const vec3& _pattern, Material* _material) : Terrain() {
	num_x = nx;
	num_z = nz;
	height.resize((num_x + 1) * (num_z + 1));
	vertex_buffer = new VertexBuffer("3f,3f,2f");
	pattern = _pattern;
	material = _material;
	update(-1, -1, -1, -1, TerrainUpdateAll);
}

Terrain::~Terrain() = default;

// die Normalen-Vektoren in einem bestimmten Abschnitt der Karte neu berechnen
void Terrain::update(int x1,int x2,int z1,int z2,int mode) {
	if (x1<0)		x1=0;
	if (x2<0)		x2=num_x;
	if (x2>=num_x)	x2=num_x;
	if (z1<0)		z1=0;
	if (z2<0)		z2=num_z;
	if (z2>=num_z)	z2=num_z;
	bool un=((mode & TerrainUpdateNormals)>0);
	bool uv=((mode & TerrainUpdateVertices)>0);
	bool up=((mode & TerrainUpdatePlanes)>0);
	float dhx,dhz;

	if (un)
		normal.resize(height.num);
	if (uv)
		vertex.resize(height.num);

	pl.resize(num_x * num_z * 2);

	// create (soft) normal vectors and vertices
	for (int i=x1;i<=x2;i++)
		for (int j=z1;j<=z2;j++){
			int n=Index(i,j);
			if (un){
				if ((i>1) and (i<num_x-1))
					dhx=height[Index(i+2,j)]-height[Index(i-2,j)];
				else
					dhx=0;
				if ((j>1) and (j<num_z-1))
					dhz=height[Index(i,j+2)]-height[Index(i,j-2)];
				else
					dhz=0;
				normal[n]=vec3(-dhx/pattern.x,4,-dhz/pattern.z);
				normal[n].normalize();
			}
			if (uv)
				vertex[n]=vec3(pattern.x*(float)i,height[n],pattern.z*(float)j);
		}
	if (uv){
		int j = z2;
		for (int i=x1;i<=x2;i++)
			vertex[Index(i,j)]=vec3(pattern.x*(float)i,height[Index(i,j)],pattern.z*(float)j);
		int i = x2;
		for (j=z1;j<=z2;j++)
			vertex[Index(i,j)]=vec3(pattern.x*(float)i,height[Index(i,j)],pattern.z*(float)j);
	}

	// update planes (for collision detection)
	if (up)
		for (int i=x1;i<x2;i++)
			for (int j=z1;j<z2;j++){
				int nt=(i*num_z+j)*2;
				pl[nt  ] = plane::from_points(vertex[Index(i  ,j  )],vertex[Index(i  ,j+1)],vertex[Index(i+1,j+1)]);
				pl[nt+1] = plane::from_points(vertex[Index(i  ,j  )],vertex[Index(i+1,j+1)],vertex[Index(i+1,j  )]);
			}

	force_redraw = true;
}

float Terrain::gimme_height(const vec3 &p) // liefert die interpolierte Hoehe zu einer Position
{
	auto o = owner;
	float x = p.x - o->pos.x;
	float z = p.z - o->pos.z;
	if ((x<=min.x)||(z<=min.z)||(x>=max.x)||(z>=max.z))
		return p.y;
	bool SPI_OB=false;
	float he=0;
	int dr_z=(int)(z/pattern.z); // welches Dreieck?
	int dr_x=(int)(x/pattern.x);
	if (x-dr_x*pattern.x<z-dr_z*pattern.z)
		SPI_OB=true;
	int j=dr_z;
	int i=dr_x;
	if (SPI_OB){ // genaue Position auf dem Dreieck
		dhx=height[Index(i+1,j+1)]-height[Index(i,j+1)];
		dhz=height[Index(i,j)]-height[Index(i,j+1)];
		he=height[Index(dr_x,dr_z+1)] + dhx/pattern.x*(x-dr_x*pattern.x) + dhz/pattern.z*(dr_z*pattern.z+pattern.z-z);
	}else{
		dhx=height[Index(i+1,j  )]-height[Index(i  ,j  )];
		dhz=height[Index(i+1,j  )]-height[Index(i+1,j+1)];
		he=height[Index(dr_x+1,dr_z)]-dhx/pattern.x*(dr_x*pattern.x+pattern.x-x)-dhz/pattern.z*(z-dr_z*pattern.z);
	}

	return he+o->pos.y;
}

float Terrain::gimme_height_n(const vec3 &p, vec3 &n) {
	float he = gimme_height(p);
	vec3 vdx = vec3(pattern.x, dhx,0        );
	vec3 vdz = vec3(0        ,-dhz,pattern.z);
	n = vec3::cross(vdz, vdx).normalized();
	return he;
}

// Daten fuer das Darstellen des Bodens
void Terrain::calc_detail(const vec3 &cam_pos) {
	vec3 dpos = cam_pos;
	if (owner)
		dpos -= owner->pos;

	for (int x1=0;x1<(num_x-1)/TERRAIN_CHUNK_SIZE+1;x1++)
		for (int z1=0;z1<(num_z-1)/TERRAIN_CHUNK_SIZE+1;z1++) {
			int lx=(x1*TERRAIN_CHUNK_SIZE>num_x-TERRAIN_CHUNK_SIZE)?(num_x%TERRAIN_CHUNK_SIZE):TERRAIN_CHUNK_SIZE;
			int lz=(z1*TERRAIN_CHUNK_SIZE>num_z-TERRAIN_CHUNK_SIZE)?(num_z%TERRAIN_CHUNK_SIZE):TERRAIN_CHUNK_SIZE;
			int x0=x1*TERRAIN_CHUNK_SIZE;
			int z0=z1*TERRAIN_CHUNK_SIZE;
			float depth = (dpos - vertex[Index(x0+lx/2,z0+lz/2)]).length() / pattern.x;

			/*int e=32;
			if (depth<500)	e=32;
			if (depth<320)	e=16;
			if (depth<160)	e=8;
			if (depth<100)	e=4;
			if (depth<70)	e=2;
			if (depth<40)	e=1;*/
			//msg_write(format("%.1f   %f", depth, log(depth / 40)));
			chunk_lod[x1][z1] = clamp((int)log(depth / 20), 0, TERRAIN_LOG_CHUNK_SIZE);
		}
}

static int TempVertexIndex[2048*4];
static int TempTriangleIndex[2048*12];
static int TempEdgeIndex[2048*4];
static plane TempPlaneList[2048*4];

inline void add_edge(int &num, int e0, int e1)
{
	if (e0 > e1){
		int t = e0;
		e0 = e1;
		e1 = t;
	}
	/*for (int i=0;i<num;i++)
		if ((TempEdgeIndex[i*2] == e0) and (TempEdgeIndex[i*2+1] == e1))
			return;*/
	TempEdgeIndex[num * 2    ] = e0;
	TempEdgeIndex[num * 2 + 1] = e1;
	num ++;
}

// for collision detection:
//    get a part of the terrain
void Terrain::get_triangle_hull(TriangleHull *h, vec3 &_pos_, float _radius_)
{
	auto o = owner;

	h->p = &vertex[0];
	h->index = TempVertexIndex;
	h->triangle_index = TempTriangleIndex;
	h->edge_index = TempEdgeIndex;
	h->pl = TempPlaneList;

	h->num_vertices = 0;
	h->num_triangles = 0;
	h->num_edges = 0;

	// how much do we need
	vec3 _min_ = _pos_ - vec3(1,1,1) * _radius_ - o->pos;
	vec3 _max_ = _pos_ + vec3(1,1,1) * _radius_ - o->pos;

	int x0=int(_min_.x/pattern.x);	if (x0<0)	x0=0;
	int z0=int(_min_.z/pattern.z);	if (z0<0)	z0=0;
	int x1=int(_max_.x/pattern.x+1);	if (x1>num_x)	x1=num_x;
	int z1=int(_max_.z/pattern.z+1);	if (z1>num_z)	z1=num_z;

	if ((x0>=x1)||(z0>=z1))
		return;

	//printf("%d %d %d %d\n", x0, x1, z0, z1);
	// c d
	// a b
	// (acd),(adb)
	h->num_vertices = (x1-x0) * (z1-z0);
	h->num_triangles = (x1-x0) * (z1-z0) * 2;
	h->num_edges = 0;
	int n = 0, n6 = 0;
	if (h->num_vertices > 2048)
		msg_error("Terrain.GetTriangleHull: too much data");
	for (int x=x0;x<x1;x++)
		for (int z=z0;z<z1;z++){
			// vertex
			TempVertexIndex[n]=Index(x,z);
			// triangle 1
			TempTriangleIndex[n6  ]=Index(x  ,z  );
			TempTriangleIndex[n6+1]=Index(x  ,z+1);
			TempTriangleIndex[n6+2]=Index(x+1,z+1);
			TempPlaneList[n*2  ]=pl[(x*num_z+z)*2  ];
			// triangle 2
			TempTriangleIndex[n6+3]=Index(x  ,z  );
			TempTriangleIndex[n6+4]=Index(x+1,z+1);
			TempTriangleIndex[n6+5]=Index(x+1,z  );
			TempPlaneList[n*2+1]=pl[(x*num_z+z)*2+1];
			// edges
			add_edge(h->num_edges, Index(x  ,z  ), Index(x+1,z  ));
			add_edge(h->num_edges, Index(x  ,z  ), Index(x  ,z+1));
			add_edge(h->num_edges, Index(x  ,z  ), Index(x+1,z+1));
			n ++;
			n6 += 6;
		}
}

inline bool TracePattern(Terrain *t, const vec3 &pos, const vec3 &p1,const vec3 &p2, CollisionData &data, int x, int z, float y_min, int dir, float range)
{
	// trace beam too high above this pattern?
	if ( (t->height[Index2(t,x,z)]<y_min) and (t->height[Index2(t,x,z+1)]<y_min) and (t->height[Index2(t,x+1,z)]<y_min) and (t->height[Index2(t,x+1,z+1)]<y_min) )
		return false;

	// 4 vertices for 2 triangles
	vec3 a = pos + t->vertex[Index2(t,x  ,z  )];
	vec3 b = pos + t->vertex[Index2(t,x+1,z  )];
	vec3 c = pos + t->vertex[Index2(t,x  ,z+1)];
	vec3 d = pos + t->vertex[Index2(t,x+1,z+1)];

	float dmin1 = range, dmin2 = range;
	vec3 ttp;
	vec3 v;

	// scan both triangles
	vec3 tp;
	if (line_intersects_triangle(a,b,d,p1,p2,tp))
		dmin1=(tp-p1).length();
	if (line_intersects_triangle(a,c,d,p1,p2,ttp)){
		dmin2=(tp-p1).length();
		if (dmin2<dmin1){ // better than the first one?
			dmin1=dmin2;
			tp=ttp;
		}
	}
	if (dmin1 >= range)
		return false;

	// don't scan backwards...
	if ((dir==0) and (tp.x<p1.x))	return false;
	if ((dir==1) and (tp.z<p1.z))	return false;
	if ((dir==2) and (tp.x>p1.x))	return false;
	if ((dir==3) and (tp.z>p1.z))	return false;

	data.pos= tp;
	data.entity = t->owner;
	return true;
}

bool Terrain::trace(const vec3 &p1, const vec3 &p2, const vec3 &dir, float range, CollisionData &data, bool simple_test) {
	vec3 pr1 = p1;
	vec3 pr2 = p2;
	vec3 pos = v_0;
	if (owner) {
		auto o = owner;
		pr1 -= o->pos;
		pr2 -= o->pos;
		pos = o->pos;
	}
	[[maybe_unused]] float dmin = range + 1;
	vec3 c;

	if ((p2.x==p1.x) and (p2.z==p1.z) and (p2.y<p1.y)){
		float h=gimme_height(p1);
		if (p2.y < h){
			data.pos = vec3(p1.x,h,p1.z);
			data.entity = owner;
			return true;
		}
	}

	float w=(float)atan2(p2.z-p1.z,p2.x-p1.x);
	int x,z;
	float y_rel;

// scanning directions

	// positive x (to the right)
	if ((w>=-pi/4) and (w<pi/4)){
		int x0 = ::max( int(pr1.x/pattern.x), 0 );
		int x1 = ::min( int(pr2.x/pattern.x), num_x );
		for (x=x0;x<x1;x++){
			z = int( ( ( pattern.x*((float)x+0.5f) - pr1.x ) * (pr2.z-pr1.z) / (pr2.x-pr1.x) + pr1.z ) / pattern.z - 0.5f );
			if ( ( z > num_z - 2) || ( z < 0 ) )
				break;
			if ( p1.y > p2.y )	y_rel = pr1.y + ( ( x + 1 ) * pattern.x - pr1.x ) * ( pr2.y - pr1.y ) / ( pr2.x - pr1.x );
			else				y_rel = pr1.y + (   x       * pattern.x - pr1.x ) * ( pr2.y - pr1.y ) / ( pr2.x - pr1.x );
			if (TracePattern(this,pos,p1,p2,data,x,z  ,y_rel,0,range))	return true;
			if (TracePattern(this,pos,p1,p2,data,x,z+1,y_rel,0,range))	return true;
		}
	}

	// positive z (forward)
	if ((w>=pi/4) and (w<pi*3/4)){
		int z0 = ::max( int(pr1.z/pattern.z), 0 );
		int z1 = ::min( int(pr2.z/pattern.z), num_z );
		for (z=z0;z<z1;z++){
			x = int( ( ( pattern.z*((float)z+0.5f) - pr1.z ) * (pr2.x-pr1.x) / (pr2.z-pr1.z) + pr1.x ) / pattern.x - 0.5f );
			if ( ( x > num_x - 2) || ( x < 0 ) )
				break;
			if ( p1.y > p2.y )	y_rel = pr1.y + ( ( z + 1 ) * pattern.z - pr1.z ) * ( pr2.y - pr1.y ) / ( pr2.z - pr1.z );
			else				y_rel = pr1.y + (   z       * pattern.z - pr1.z ) * ( pr2.y - pr1.y ) / ( pr2.z - pr1.z );
			if (TracePattern(this,pos,p1,p2,data,x  ,z,y_rel,1,range))	return true;
			if (TracePattern(this,pos,p1,p2,data,x+1,z,y_rel,1,range))	return true;
		}
	}

	// negative x (to the left)
	if ((w>=pi*3/4)||(w<-pi*3/4)){
		int x0 = ::min( int(pr1.x/pattern.x), num_x );
		int x1 = ::max( int(pr2.x/pattern.x), 0 );
		for (x=x0;x>x1;x--){
			z = int( ( ( pattern.x*((float)x+0.5f) - pr1.x ) * (pr2.z-pr1.z) / (pr2.x-pr1.x) + pr1.z ) / pattern.z - 0.5f );
			if ( ( z > num_z - 2) || ( z < 0 ) )
				break;
			if ( p1.y < p2.y )	y_rel = pr1.y + ( ( x + 1 ) * pattern.x - pr1.x ) * ( pr2.y - pr1.y ) / ( pr2.x - pr1.x );
			else				y_rel = pr1.y + (   x       * pattern.x - pr1.x ) * ( pr2.y - pr1.y ) / ( pr2.x - pr1.x );
			if (TracePattern(this,pos,p1,p2,data,x,z  ,y_rel,2,range))	return true;
			if (TracePattern(this,pos,p1,p2,data,x,z+1,y_rel,2,range))	return true;
		}
	}

	// negative z (backward)
	if ((w>=-pi*3/4) and (w<-pi/4)){
		int z0 = ::min( int(pr1.z/pattern.z), num_z );
		int z1 = ::max( int(pr2.z/pattern.z), 0 );
		for (z=z0;z>z1;z--){
			x = int( ( ( pattern.z*((float)z+0.5f) - pr1.z ) * (pr2.x-pr1.x) / (pr2.z-pr1.z) + pr1.x ) / pattern.x - 0.5f );
			if ( ( x > num_x - 2) || ( x < 0 ) )
				break;
			if ( p1.y < p2.y )	y_rel = pr1.y + ( ( z + 1 ) * pattern.z - pr1.z ) * ( pr2.y - pr1.y ) / ( pr2.z - pr1.z );
			else				y_rel = pr1.y + (   z       * pattern.z - pr1.z ) * ( pr2.y - pr1.y ) / ( pr2.z - pr1.z );
			if (TracePattern(this,pos,p1,p2,data,x  ,z,y_rel,3,range))	return true;
			if (TracePattern(this,pos,p1,p2,data,x+1,z,y_rel,3,range))	return true;
		}
	}
	return false;
}

bool XTerrainVBUpdater::build_chunk(int chunk_no) {
	int num_x = terrain->num_x;
	int num_z = terrain->num_z;
	auto& chunk_lod = terrain->chunk_lod;
	auto& vertex = terrain->vertex;
	auto& normal = terrain->normal;

	// number of blocks (including the partially filled ones)
	int nx = (num_x - 1) / TERRAIN_CHUNK_SIZE + 1;
	int nz = (num_z - 1) / TERRAIN_CHUNK_SIZE + 1;
	/*nx=num_x/32;
	nz=num_z/32;*/

	int x1 = chunk_no / nz;
	int z1 = chunk_no % nz;

	// loop through the blocks
	/*for (int x1=0; x1<nx; x1++)
		for (int z1=0; z1<nz; z1++) {*/
	for (int _i=0; _i<1; _i++) {

			// block size?    (32 or cut off...)
			int lx = (x1 * TERRAIN_CHUNK_SIZE > num_x - TERRAIN_CHUNK_SIZE) ? (num_x % TERRAIN_CHUNK_SIZE) : TERRAIN_CHUNK_SIZE;
			int lz = (z1 * TERRAIN_CHUNK_SIZE > num_z - TERRAIN_CHUNK_SIZE) ? (num_z % TERRAIN_CHUNK_SIZE) : TERRAIN_CHUNK_SIZE;

			// start
			int x0 = x1 * TERRAIN_CHUNK_SIZE;
			int z0 = z1 * TERRAIN_CHUNK_SIZE;
			int l = chunk_lod[x1][z1];
			if (l < 0)
				continue;
			int e = 1<<l;

			// loop through the squares
			for (int x=x0;x<=x0+lx-e;x+=e)
				for (int z=z0;z<=z0+lz-e;z+=e) {
					// x,z "real" pattern indices

					// vertices
					vec3 va=vertex[Index(x  ,z  )];
					vec3 vb=vertex[Index(x+e,z  )];
					vec3 vc=vertex[Index(x  ,z+e)];
					vec3 vd=vertex[Index(x+e,z+e)];

					// normal vectors
					vec3 na=normal[Index(x  ,z  )];
					vec3 nb=normal[Index(x+e,z  )];
					vec3 nc=normal[Index(x  ,z+e)];
					vec3 nd=normal[Index(x+e,z+e)];

					// left border correction
					if (x==x0 and x1>0) {
						int ll = chunk_lod[x1-1][z1];
						int p = 1 << ll;
						if (p>0 and e<p) {
							int a0=p*int(z/p);
							if (a0+p<=num_z) {
								float t0=float(z%p)/(float)p;
								float t1=t0+float(e)/float(p);
								va=vertex[Index(x,a0)]*(1-t0)+vertex[Index(x,a0+p)]*t0;
								na=normal[Index(x,a0)]*(1-t0)+normal[Index(x,a0+p)]*t0;
								vc=vertex[Index(x,a0)]*(1-t1)+vertex[Index(x,a0+p)]*t1;
								nc=normal[Index(x,a0)]*(1-t1)+normal[Index(x,a0+p)]*t1;
							}
						}
					}

					// right border correction
					if (x==x0+TERRAIN_CHUNK_SIZE-e and x1<nx-1) {
						[[maybe_unused]] int ll = chunk_lod[x1+1][z1];
						int p = 1 << l;
						if (p>0 and e<p) {
							int a0=p*int(z/p);
							if (a0+p<num_x) {
								float t0=float(z%p)/(float)p;
								float t1=t0+float(e)/float(p);
								vb=vertex[Index(x+e,a0)]*(1-t0)+vertex[Index(x+e,a0+p)]*t0;
								nb=normal[Index(x+e,a0)]*(1-t0)+normal[Index(x+e,a0+p)]*t0;
								vd=vertex[Index(x+e,a0)]*(1-t1)+vertex[Index(x+e,a0+p)]*t1;
								nd=normal[Index(x+e,a0)]*(1-t1)+normal[Index(x+e,a0+p)]*t1;
							}
						}
					}

					// bottom border correction
					if (z==z0 and z1>0) {
						int l = chunk_lod[x1][z1-1];
						int p = 1 << l;
						if (p>0 and e<p) {
							int a0=p*int(x/p);
							if (a0+p<=num_x) {
								float t0=float(x%p)/(float)p;
								float t1=t0+float(e)/float(p);
								va=vertex[Index(a0,z)]*(1-t0)+vertex[Index(a0+p,z)]*t0;
								na=normal[Index(a0,z)]*(1-t0)+normal[Index(a0+p,z)]*t0;
								vb=vertex[Index(a0,z)]*(1-t1)+vertex[Index(a0+p,z)]*t1;
								nb=normal[Index(a0,z)]*(1-t1)+normal[Index(a0+p,z)]*t1;
							}
						}
					}

					// top border correction
					if (z==z0+TERRAIN_CHUNK_SIZE-e and z1<nz-1) {
						int l = chunk_lod[x1][z1+1];
						int p = 1 << l;
						if (p>0 and e<p) {
							int a0=p*int(x/p);
							if (a0+p<num_z) {
								float t0=float(x%p)/(float)p;
								float t1=t0+float(e)/float(p);
								vc=vertex[Index(a0,z+e)]*(1-t0)+vertex[Index(a0+p,z+e)]*t0;
								nc=normal[Index(a0,z+e)]*(1-t0)+normal[Index(a0+p,z+e)]*t0;
								vd=vertex[Index(a0,z+e)]*(1-t1)+vertex[Index(a0+p,z+e)]*t1;
								nd=normal[Index(a0,z+e)]*(1-t1)+normal[Index(a0+p,z+e)]*t1;
							}
						}
					}

					float u1 = (float) x    / (float)num_x;//*texture_scale[i].x;
					float u2 = (float)(x+e) / (float)num_x;//*texture_scale[i].x;
					float v1 = (float) z    / (float)num_z;//*texture_scale[i].z;
					float v2 = (float)(z+e) / (float)num_z;//*texture_scale[i].z;

					uv.add(u1);	uv.add(v1);
					uv.add(u1);	uv.add(v2);
					uv.add(u2);	uv.add(v2);
					uv.add(u1);	uv.add(v1);
					uv.add(u2);	uv.add(v2);
					uv.add(u2);	uv.add(v1);

					/*vertices.add({va,na,ta[0],ta[1]});
					vertices.add({vc,nc,tc[0],tc[1]});
					vertices.add({vd,nd,td[0],td[1]});
					vertices.add({va,na,ta[0],ta[1]});
					vertices.add({vd,nd,td[0],td[1]});
					vertices.add({vb,nb,tb[0],tb[1]});*/
					p.add(va);	n.add(na);
					p.add(vc);	n.add(nc);
					p.add(vd);	n.add(nd);
					p.add(va);	n.add(na);
					p.add(vd);	n.add(nd);
					p.add(vb);	n.add(nb);

				}
		}
	return chunk_no >= nx*nz-1;
}

void XTerrainVBUpdater::condense() {
	for (int i=0; i<p.num; i++) {
		vertices.add({p[i], n[i], uv[i*2], uv[i*2+1]});
	}
	p.clear();
	n.clear();
	uv.clear();
}

void XTerrainVBUpdater::upload() {
	vb->update(vertices);
	vertices.clear();
}

int XTerrainVBUpdater::iterate(const vec3 &cam_pos) {
	if (mode == 0) {
		terrain->calc_detail(cam_pos);

		// do we have to recreate the terrain?
		bool redraw = false;
		if (terrain->force_redraw) {
			redraw = true;
		} else {
			for (int x=0; x<terrain->num_x/TERRAIN_CHUNK_SIZE; x++)
				for (int z=0; z<terrain->num_z/TERRAIN_CHUNK_SIZE; z++)
					if (terrain->chunk_lod_old[x][z] != terrain->chunk_lod[x][z])
						redraw = true;
		}
		if (!redraw)
			return 0; // no update needed

		terrain->force_redraw = false;
		for (int x=0; x<terrain->num_x/TERRAIN_CHUNK_SIZE; x++)
			for (int z=0; z<terrain->num_z/TERRAIN_CHUNK_SIZE; z++)
				terrain->chunk_lod_old[x][z] = terrain->chunk_lod[x][z];
		mode = 10;
	} else if (mode == 10) {
		if (build_chunk(counter ++)) {
			mode = 11;
			counter = 0;
		}
	} else if (mode == 11) {
		condense();
		mode = 12;
	} else if (mode == 12) {
		upload();
		mode = 0;
		return 2; // swap vb and restart!
	}
	return 1; // keep iterating!
}

void Terrain::prepare_draw(const vec3 &cam_pos) {
	// c d
	// a b
	// (acd),(adb)

	XTerrainVBUpdater u;
	u.terrain = this;
	u.vb = vertex_buffer.get();
	while (int r = u.iterate(cam_pos) == 1) {}
}

