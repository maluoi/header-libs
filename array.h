// Licensed under MIT or Public Domain. See bottom of file for details.
//
// array.h is a dynamic array implementation using Plain Old Data structs. 
//
// array_t handles memory allocation and resizing, but follows standard and
// predictable behavior during argument passing, construction, etc. Memory 
// must be freed explicity with .free(), as it doesn't use constructors or 
// destructors.
//
// array_view_t is a partial 'view' of a chunk of memory that contains much 
// more than just what we're interested in. For example, if we want to 
// interact with only the 'y' component of an array of vec3s. It also has 
// tools for de-interlacing data from such chunks of memory. While array_t 
// should be better for most use cases, this is particularly handy for data 
// you don't know the whole story about, or when loading data from files.

// Notes: array.h uses size_t instead of int32_t for performance. It 
// eliminates at least one ASM instruction in access and set code.

// Example usage:
/*

array_t<vec3> vertices = {};
vertices.resize(3); // resize is optional

vertices.add( vec3{1,0,0} );
vertices.add( vec3{0,1,0} );
vertices.add( vec3{0,0,1} );

for (size_t i=0; i<vertices.count; i+=1) {
	vertices[i] += vec3{1,1,1};
}

array_view_t<float> heights = array_view_create(vertices, &vec3::y);
for (size_t i=0; i<heights.count; i+=1) {
	heights[i] = 10;
}
float *arr_heights = heights.copy_deinterlace();
free(arr_heights);

heights = {};

vertices.free();

*/

#pragma once

#include <stdint.h>

//////////////////////////////////////

// Options for customizing code dependencies!

#ifndef ARRAY_MALLOC
#include <malloc.h>
#define ARRAY_MALLOC malloc
#endif
#ifndef ARRAY_FREE
#include <malloc.h>
#define ARRAY_FREE ::free
#endif

#ifndef ARRAY_MEMCPY
#include <string.h>
#define ARRAY_MEMCPY memcpy
#endif
#ifndef ARRAY_MEMMOVE
#include <string.h>
#define ARRAY_MEMMOVE memmove
#endif

#ifndef ARRAY_ASSERT
#include <assert.h>
#define ARRAY_ASSERT assert
#endif

//////////////////////////////////////
// array_t                          //
//////////////////////////////////////

template <typename T>
struct array_t {
	void  *data;
	size_t count;
	size_t capacity;

	size_t     add(const T &item)        { if (count+1 >= capacity) { resize(capacity * 2 < 4 ? 4 : capacity * 2); } ((T*)data)[count] = item; count += 1; return count - 1; }
	void       clear()                   { count = 0; }
	void       each(void (*e)(T &))      { for (size_t i=0; i<count; i++) e(((T*)data)[i]); }
	T         &last() const              { return ((T*)data)[count - 1]; }
	void       pop()                     { remove(count - 1); }
	void       resize(size_t to_capacity);
	void       free();
	array_t<T> copy() const;
	void       remove(size_t at);
	void       insert(size_t at, const T &item);
	void       reverse();

	inline void set        (size_t id, const T &val) { ((T*)data)[id] = val; }
	inline T   &get        (size_t id) const         { return ((T*)data)[id]; }
	inline T   &operator[] (size_t id) const         { return ((T*)data)[id]; }
};

//////////////////////////////////////
// array_view_t                     //
//////////////////////////////////////

template <typename T>
struct array_view_t {
	void  *data;
	size_t count;

	size_t stride;
	size_t offset;

	void each(void (*e)(T &))      { for (size_t i=0; i<count; i++) e((T&)((uint8_t*)data + i*stride + offset)); }
	T   &last()             const  { return (T&)((uint8_t*)data + (count-1)*stride + offset); }
	T   *copy_deinterlace() const;

	inline void set        (size_t id, const T &val) { *(T*)((uint8_t*)data + id*stride + offset) = val; }
	inline T   &get        (size_t id) const         { return (T&)((uint8_t*)data + id*stride + offset); }
	inline T   &operator[] (size_t id) const         { return (T&)((uint8_t*)data + id*stride + offset); }
};

//////////////////////////////////////
// array_t methods                  //
//////////////////////////////////////

template <typename T>
void array_t<T>::resize(size_t to_capacity) {
	if (count > to_capacity) 
		count = to_capacity;

	void  *old_memory = data;
	void  *new_memory = ARRAY_MALLOC(sizeof(T) * to_capacity); 
	memcpy(new_memory, old_memory, sizeof(T) * count);

	data = new_memory;
	ARRAY_FREE(old_memory);

	capacity = to_capacity;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::free() {
	ARRAY_FREE(data); 
	*this = {}; 
}

//////////////////////////////////////

template <typename T>
array_t<T> array_t<T>::copy() const { 
	array_t<T> result = { 
		ARRAY_MALLOC(sizeof(T) * capacity), 
		count, 
		capacity 
	}; 
	ARRAY_MEMCPY(result.data, data, sizeof(T) * count); 
	return result; 
}

//////////////////////////////////////

template <typename T>
void array_t<T>::remove(size_t aAt) {
	ARRAY_ASSERT(aAt < count);

	int64_t  at  = aAt;
	uint8_t *arr = (uint8_t*)data;
	ARRAY_MEMMOVE(arr + at * sizeof(T), arr + (at + 1)*sizeof(T), (arr + count*sizeof(T)) - (arr + (at + 1)*sizeof(T)));
	count -= 1;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::insert(size_t aAt, const T &item) {
	ARRAY_ASSERT(aAt <= count);

	if (count+1 > capacity) 
		resize(capacity<1?1:capacity*2);

	int64_t  at  = aAt;
	uint8_t *arr = (uint8_t*)data;
	ARRAY_MEMMOVE(arr + (at + 1)*sizeof(T), arr + at*sizeof(T), (arr + count*sizeof(T)) - (arr + at*sizeof(T)));
	ARRAY_MEMCPY (arr + at * sizeof(T), &item, sizeof(T));
	count += 1;
}

//////////////////////////////////////

template <typename T>
void array_t<T>::reverse() {
	for(size_t i=0; i<count/2; i+=1) {
		T tmp = this->get(i);
		this->set(i, this->get(count - i - 1));
		this->set(count - i - 1, tmp);
	}
}

//////////////////////////////////////
// array_view_t methods             //
//////////////////////////////////////

template <typename T, typename D>
inline static array_view_t<T> array_view_create(const D *data, size_t count, T D::*offset) {
	return array_view_t<T>{data, count, sizeof(D), offset};
}

//////////////////////////////////////

// can be called like this: array_view_create(arr_of_vec3, &vec3::x);
template <typename T, typename D>
inline static array_view_t<T> array_view_create(const array_t<T> &src, T D::*offset) {
	return array_view_t<T>{src.data, src.count, sizeof(D), offset};
}

//////////////////////////////////////

template <typename T>
T *array_view_t<T>::copy_deinterlace() const {
	T *result = (T*)ARRAY_MALLOC(sizeof(T) * count);

	for (size_t i=0; i<count; i++) {
		result[i] = this->get(i);
	}
	return result;
}

//////////////////////////////////////

/*
------------------------------------------------------------------------------
This software is available under 2 licenses -- choose whichever you prefer.
------------------------------------------------------------------------------
ALTERNATIVE A - MIT License
Copyright (c) 2020 Nick Klingensmith
Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
------------------------------------------------------------------------------
ALTERNATIVE B - Public Domain (www.unlicense.org)
This is free and unencumbered software released into the public domain.
Anyone is free to copy, modify, publish, use, compile, sell, or distribute this
software, either in source code form or as a compiled binary, for any purpose,
commercial or non-commercial, and by any means.
In jurisdictions that recognize copyright laws, the author or authors of this
software dedicate any and all copyright interest in the software to the public
domain. We make this dedication for the benefit of the public at large and to
the detriment of our heirs and successors. We intend this dedication to be an
overt act of relinquishment in perpetuity of all present and future rights to
this software under copyright law.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
------------------------------------------------------------------------------
*/