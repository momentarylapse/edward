
#ifndef __MAP_INCLUDED__
#define __MAP_INCLUDED__

#include "set.h"

template<class T1, class T2>
struct MapEntry
{
	T1 key;
	T2 value;
	bool operator == (const MapEntry<T1, T2> &e) const
	{	return key == e.key;	}
	bool operator > (const MapEntry<T1, T2> &e) const
	{	return key > e.key;	}
};

template<class T1, class T2>
class Map : public Set<MapEntry<T1, T2> >
{
	T2 dummy;
public:
	typedef MapEntry<T1, T2> Entry;
	using DynamicArray::num;
	using DynamicArray::data;
	int add(const T1 &key, const T2 &value)
	{
		MapEntry<T1, T2> e = {key, value};
		return Set<MapEntry<T1, T2> >::add(e);
	}
	const T2 &operator[] (const T1 &key) const
	{
		//msg_write("const[]");
		for (int i=0;i<num;i++)
			if (((Entry*)data)[i].key == key)
				return ((Entry*)data)[i].value;
		return dummy;
	}
	T2 &operator[] (const T1 &key)
	{
		/*HashEntry e = {"", hash, EmptyVar};
		int n = find(e);
		if (n >= 0)
			return ((HashEntry*)data)[n].value;
		n = add(key, EmptyVar);
		return ((HashEntry*)data)[n].value;*/

		for (int i=0;i<num;i++)
			if (((Entry*)data)[i].key == key)
				return ((Entry*)data)[i].value;
		resize(num + 1);
		int n = num - 1;
		((Entry*)data)[n].key = key;
		return ((Entry*)data)[n].value;
	}
};

#endif
