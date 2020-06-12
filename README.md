# POD-array
A short and sweet single header file C++ dynamic array type done in a Plain Old Data style. I use this in a number of my own projects as a replacement for std::vector.

`array_t` handles memory allocation and resizing, but follows standard and predictable behavior during argument passing, construction, etc. Memory must be freed explicity with `.free()`, as it doesn't use constructors or destructors.

`array_view_t` is a partial 'view' of a chunk of memory that contains much more than just what we're interested in. For example, if we want to interact with only the 'y' component of an array of vec3s. It also has tools for de-interlacing data from such chunks of memory. While `array_t` should be better for most use cases, this is particularly handy for data you don't know the whole story about, or when loading data from files!

## API

- `array_t<T>`
  - `void  *data`
  - `size_t count`
  - `size_t capacity`
  -
  - `size_t   add         (const T &item)`
  - `void     insert      (size_t at, const T &item)`
  - `void     resize      (size_t to_capacity)`
  - `void     trim        ()`
  - `void     remove      (size_t at)`
  - `void     pop         ()`
  - `void     clear       ()`
  - `T       &last        () const`
  - `void    &set         (size_t id, const T &val)`
  - `T       &get         (size_t id) const`
  - `T       &operator[]  (size_t id)`
  - `void       reverse   ()`
  - `array_t<T> copy      () const`
  - `void       each      (void (*e)(T &))`
  - `void    free         ()`
  - `int64_t binary_search(const T &item)` - for elements with comparison operators
  - `int64_t binary_search(D T::*key, const D &item)` - call like `binary_search(&vec3::y, 0.5f)`
  - `void    sort         ()` - for elements with comparison operators
  - `void    sort_desc    ()` - descending, for elements with comparison operators
  - `void    sort     <T,D,D T::*key>()` - call like `sort<vec3, float, &vec3::y>()`
  - `void    sort_desc<T,D,D T::*key>()` - descending, call like `sort_desc<vec3, float, &vec3::y>()`
  - `void    sort         (int32_t (*compare)(const T&a, const T&b))`
- `array_view_t<T>`
  -	`void  *data`
  - `size_t count`
  - `size_t stride`
  - `size_t offset`
  -
  - `T     &last           () const`
  - `T     &get            (size_t id) const`
  - `void  &set            (size_t id, const T &val)`
  - `T     &operator[]     (size_t id)`
  - `T    *copy_deinterlace() const`
  - `void  each            (void (*e)(T &))`


## Example usage

```C
array_t<vec3> vertices = {};
vertices.resize(3); // resize is optional

vertices.add( vec3{1,0,0} );
vertices.add( vec3{0,1,0} );
vertices.add( vec3{0,0,1} );

// Sort in ascending order using the y component
vertices.sort<vec3, float, &vec3::y>();

// Binary search y components for 0.5, binary_search only works on sorted
// lists.
int32_t at = vertices.binary_search(&vec3::y, 0.5f);

// If result is positive, it's the index of the item. If negative, then
// the '~' operator gives you the insert index for that value.
if (at < 0) at = ~at;
vertices.insert(at, vec3{0, 0.5f, 0});

// standard array access
for (size_t i=0; i<vertices.count; i+=1) {
    vertices[i] += vec3{1,1,1};
}

// or with a callback
vertices.each([](vec3 &v){ v += vec3{1,1,1}; });

// Array views allow you to work with just a single component as if it
// was an array of its own.
array_view_t<float> heights = array_view_create(vertices, &vec3::y);
for (size_t i=0; i<heights.count; i+=1) {
    heights[i] = 10;
}

// copy_deinterlace will separate out the components into their very own
// chunk of memory.
float *arr_heights = heights.copy_deinterlace();
// Which you will have to free
free(arr_heights);

// Array views don't need freed, since they're just a view on some other
// array.
heights = {};

// No deconstructors here, you just have to remember to free it.
vertices.free();
```

## License

MIT or public domain, pick your preference.