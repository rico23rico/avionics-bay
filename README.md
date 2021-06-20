[![Building](https://github.com/rico23rico/avionics-bay/actions/workflows/cmake.yml/badge.svg)](https://github.com/rico23rico/avionics-bay/actions)


This is a helper library for C/C++ and Lua plugins in X-Plane 11. Lua is exposed via FFI and it supports any Lua plugin (SASL, XLua, etc.).

(Planned) Features
==================
 - Multi-threaded parsing, transparent to the user plugin
 - Parsing of the following X-Plane files:
   - `apt.dat` :heavy_check_mark:
   - `earth_awy.dat` :construction:
   - `earth_fix.dat` :heavy_check_mark:
   - `earth_hold.dat` :heavy_check_mark:
   - `earth_mora.dat` :heavy_check_mark:
   - `earth_msa.dat` :construction:
   - `earth_nav.dat` :heavy_check_mark:
 - Parsing of CIFP files
   - SID   :heavy_check_mark:
   - STAR  :heavy_check_mark: 
   - APPCH :heavy_check_mark: 
   - RWVY  :heavy_check_mark: 
 - Polygon triangulation with holes

Current status and development
==============================

The library is currently developed to support the open source [A321Neo project](https://github.com/JonathanOrr/A321Neo-FXPL),
and the development is currently focused on the needs and integration with this project.

Following the first release of the A320 aircraft, this library will be improved (especially regarding documentation) and released separately.

License
=======
This library is released with GPL3.0 (check the [LICENSE](LICENSE) file). Be aware of the limitations and implications of this license when the
library is included in your plugin (especially for commercial products).
