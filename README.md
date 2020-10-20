## array.h
A short and sweet single header file C++ dynamic array and hashmap type done in a Plain Old Data style. I use this in a number of my own projects as a replacement for std::vector. These take a lot of inspiration from C#'s List and Dictionary types.

## ferr_hash.h
A collection of hash functions I use frequently. Contains 32 & 64 bit implementations of FNV-1a, including variations that calculate a variation of the hash at compile-time rather than runtime.

## micro_ply.h
An ASCII .ply loader done in as few lines of code as possible while still maintaining readability. Makes it easy to embed within other files! Includes functions for converting ply data into whatever format you're using, so this is should be handy for loading data that isn't necessarily a traditional mesh.

## License

MIT or public domain, pick your preference.

# Other Single Header Libraries
I've got a couple other single header libraries that have their own repository, since they're bigger projects.

## [sk_gpu.h](https://github.com/maluoi/sk_gpu)
sk_gpu.h is a mid-level cross-platform graphics library focused on Mixed Reality rendering, in an amalgamated single file header! It currently uses D3D11 on Windows, GLES on Android, and WebGL on the Web, and works very well with OpenXR.

## [warm_sock.h](https://github.com/maluoi/warm-sock)
A single header high-level socket/networking library for building server/client multi-user experiences.

## [ferr_graphnet.h](https://github.com/maluoi/FerrGraphNet)
A single header library for loading, saving and working with computational graphs in a nicely version-controllable format.