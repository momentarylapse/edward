
#ifndef _MATH_VECTOR_INCLUDED_
#define _MATH_VECTOR_INCLUDED_

#include <math.h>

class matrix;
class matrix3;

struct vector
{
public:
	float x,y,z;
	vector(){};
	vector(float x,float y,float z)
	{	this->x=x;	this->y=y;	this->z=z;	}
	// assignment operators
	vector& operator += (const vector& v)
	{	x+=v.x;	y+=v.y;	z+=v.z;	return *this;	}
	vector& operator -= (const vector& v)
	{	x-=v.x;	y-=v.y;	z-=v.z;	return *this;	}
	vector& operator *= (float f)
	{	x*=f;	y*=f;	z*=f;	return *this;	}
	vector& operator /= (float f)
	{	x/=f;	y/=f;	z/=f;	return *this;	}
	// unitary operator(s)
	vector operator - () const
	{	return vector(-x,-y,-z);	}
	// binary operators
	vector operator + (const vector &v) const
	{	return vector( x+v.x , y+v.y , z+v.z );	}
	vector operator - (const vector &v) const
	{	return vector( x-v.x , y-v.y , z-v.z );	}
	vector operator * (float f) const
	{	return vector( x*f , y*f , z*f );	}
	vector operator / (float f) const
	{	return vector( x/f , y/f , z/f );	}
	friend vector operator * (float f,const vector &v)
	{	return v*f;	}
	bool operator == (const vector &v) const
	{	return ((x==v.x)&&(y==v.y)&&(z==v.z));	}
	bool operator != (const vector &v) const
	{	return !((x==v.x)&&(y==v.y)&&(z==v.z));	}
	float operator * (const vector &v) const
	{	return x*v.x + y*v.y + z*v.z;	}
	vector operator ^ (const vector &v) const
	{	return vector( y*v.z-z*v.y, z*v.x-x*v.z, x*v.y-y*v.x );	}
	string str() const
	{	return format("(%f, %f, %f)", x, y, z);	}

	float length() const;
	float length_sqr() const;
	float length_fuzzy() const;
	void normalize();
	vector ang2dir() const;
	vector dir2ang() const;
	vector dir2ang2(const vector &up) const;
	vector ortho() const;
	int important_plane() const;
	vector rotate(const vector &ang) const;
	vector transform(const matrix &m) const;
	vector transform_normal(const matrix &m) const;
	vector transform3(const matrix3 &m) const;
	void _min(const vector &test_partner);
	void _max(const vector &test_partner);
	bool between(const vector &a, const vector &b) const;
	float factor_between(const vector &a, const vector &b) const;
	bool bounding_cube(const vector &a, float r) const;
};
// vectors
float _cdecl VecDotProduct(const vector &v1, const vector &v2);
vector _cdecl VecCrossProduct(const vector &v1, const vector &v2);
vector _cdecl VecAngAdd(const vector &ang1, const vector &ang2);
vector _cdecl VecAngInterpolate(const vector &ang1, const vector &ang2, float t);
float VecLineDistance(const vector &p, const vector &l1, const vector &l2);
vector VecLineNearestPoint(const vector &p, const vector &l1, const vector &l2);

const vector v_0 = vector(0, 0, 0);
const vector e_x = vector(1, 0, 0);
const vector e_y = vector(0, 1, 0);
const vector e_z = vector(0, 0, 1);


inline float _vec_length_(const vector &v)
{	return sqrt(v*v);	}

inline float _vec_length_fuzzy_(const vector &v)
{
	float x=fabs(v.x);
	float y=fabs(v.y);
	float z=fabs(v.z);
	float xy=(x>y)?x:y;
	return (xy>z)?xy:z;
}

inline void _vec_normalize_(vector &v)
{	float inv_norm = 1.0f / sqrt(v*v); v *= inv_norm;	}

inline bool _vec_between_(const vector &v,const vector &a,const vector &b)
{
	if ((v.x>a.x)&&(v.x>b.x))	return false;
	if ((v.x<a.x)&&(v.x<b.x))	return false;
	if ((v.y>a.y)&&(v.y>b.y))	return false;
	if ((v.y<a.y)&&(v.y<b.y))	return false;
	if ((v.z>a.z)&&(v.z>b.z))	return false;
	if ((v.z<a.z)&&(v.z<b.z))	return false;
	return true;
}

inline float _vec_factor_between_(const vector &v,const vector &a,const vector &b)
{	return ((v-a)*(b-a)) / ((b-a)*(b-a));	}


#endif
