# etcpak 2.1 #

## The fastest ETC compressor on the planet ##

etcpak is an extremely fast [Ericsson Texture Compression](http://en.wikipedia.org/wiki/Ericsson_Texture_Compression) and [Block Compression](https://en.wikipedia.org/wiki/S3_Texture_Compression) utility. Currently it's best suited for rapid assets preparation during development, when graphics quality is not a concern, but it's also used in production builds of applications used by millions of people.

## Compression times ##

Benchmark performed on an AMD Ryzen 9 7950X, using a real-life RGBA 16K × 16K atlas.

ETC1: ST: **797 Mpx/s**, MT: **9613 Mpx/s**  
ETC2: ST: **356 Mpx/s**, MT: **6378 Mpx/s**  
BC1: ST: **5730 Mpx/s**, MT: **10174 Mpx/s**  
BC3: ST: **2437 Mpx/s**, MT: **8568 Mpx/s**  
BC7: ST: **12.1 Mpx/s**, MT: **225 Mpx/s**

The same benchmark performed on Intel i7 1185G7:

ETC1: ST: **516 Mpx/s**, MT: **2223 Mpx/s**  
ETC2: ST: **238 Mpx/s**, MT: **984 Mpx/s**  
BC1: ST: **2969 Mpx/s**, MT: **9199 Mpx/s**  
BC3: ST: **1351 Mpx/s**, MT: **5503 Mpx/s**  
BC7: ST: **7.8 Mpx/s**, MT: **21.3 Mpx/s**

Apple M3 Max benchmark:

ETC1: ST: **204 Mpx/s**, MT: **2455 Mpx/s**  
ETC2: ST: **101 Mpx/s**, MT: **1231 Mpx/s**  
BC1: ST: **3255 Mpx/s**, MT: **30366 Mpx/s**  
BC3: ST: **1147 Mpx/s**, MT: **13238 Mpx/s**  
BC7: ST: **5.47 Mpx/s**, MT: **65.8 Mpx/s**

[Why there's no image quality metrics? / Quality comparison.](http://i.imgur.com/FxlmUOF.png)

## Decompression times ##

etcpak can also decompress textures. Timings on Ryzen 7950X (all single-threaded):

ETC1: **604 Mpx/s**  
ETC2: **599 Mpx/s**  
BC1: **1634 Mpx/s**  
BC3: **1172 Mpx/s**  
BC7: **201 Mpx/s**

i7 1185G7:

ETC1: **383 Mpx/s**  
ETC2: **378 Mpx/s**  
BC1: **1056 Mpx/s**  
BC3: **788 Mpx/s**  
BC7: **76 Mpx/s**

M3 Max:

ETC1: **690 Mpx/s**  
ETC2: **786 Mpx/s**  
BC1: **2434 Mpx/s**  
BC3: **1414 Mpx/s**  
BC7: **406 Mpx/s**

To give some perspective here, Nvidia in-driver ETC2 decoder can do only 42.5 Mpx/s.

## Quality comparison ##

Original image:

![](examples/kodim23.png)

Compressed image:

ETC1:

![](examples/etc1.png "ETC1 mode")

ETC2:

![](examples/etc2.png "ETC2 mode")

BC1:

![](examples/dxtc.png "BC1 mode")

BC7:

![](examples/bc7.png "BC7 mode")
