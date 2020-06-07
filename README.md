# POD-array
A short and sweet single header file C++ dynamic array type done in a Plain Old Data style. I use this in a number of my own projects as a replacement for std::vector.

`array_t` handles memory allocation and resizing, but follows standard and predictable behavior during argument passing, construction, etc. Memory must be freed explicity with `.free()`, as it doesn't use constructors or destructors.

`array_view_t` is a partial 'view' of a chunk of memory that contains much more than just what we're interested in. For example, if we want to interact with only the 'y' component of an array of vec3s. It also has tools for de-interlacing data from such chunks of memory. While `array_t` should be better for most use cases, this is particularly handy for data you don't know the whole story about, or when loading data from files!

## Example usage

```C
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
```

## License

MIT or public domain, pick your preference.