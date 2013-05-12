
#ifndef _MATH_COMPLEX_INCLUDED_
#define _MATH_COMPLEX__INCLUDED_

struct complex
{
	float x, y;
	complex(){};
	complex(float x, float y);
	// assignment operators
	complex& operator += (const complex& v);
	complex& operator -= (const complex& v);
	complex& operator *= (float f);
	complex& operator /= (float f);
	complex& operator *= (const complex &v);
	complex& operator /= (const complex &v);
	// unitary operator(s)
	complex operator - ();
	// binary operators
	complex operator + (const complex &v) const;
	complex operator - (const complex &v) const;
	complex operator * (float f) const;
	complex operator / (float f) const;
	complex operator * (const complex &v) const;
	complex operator / (const complex &v) const;
	friend complex operator * (float f,const complex &v)
	{	return v*f;	}
	bool operator == (const complex &v) const;
	bool operator != (const complex &v) const;
	float abs() const;
	float abs_sqr() const;
	string str() const;
};

// complex
const complex c_i = complex(0, 1);

#endif
