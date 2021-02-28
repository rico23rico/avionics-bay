# API Documentation
This file contains the description of all the functions exported by the FFI library in Lua.

## XPData functions

### General
* bool **xpdata_is_ready()**
  * It returns `true` if the data from XP files have been successfully read. The return value is `false`
   until the worker thread finished its execution or if it encouters an error. This function should be
   verified to be `true` before calling any other `xpdata_` function.
* bool **xpdata_is_error()**
  * It returns `true` if an error occurred during the reading of X-Plane files. If the error is not
    critical, the `xpdata_is_ready` may still be `true` but partial data are available.
* void **xpdata_force_refresh()**
  * Force the refresh of cached data (nearest airport, ...).
  * This function is very computational intensive and should be called only when really needed.  

### NAV Aids
* struct xpdata_navaid_list_t  **xpdata_navaid_by_coords(int type, double lat, double lon, bool extended_range)**
  * It finds the nearest navaids of the given type (see NAV_ID_* constants) and the coordinates. If `extended_range` is false,
    the search is limited in a square of aprox. 180nm each side.  If `extended_range` is true, the search is performed
    in a square of <ins>at least</ins> 360nm each side, but the performance penalty is high.

* struct xpdata_navaid_list_t  **xpdata_navaid_by_freq(int type, double freq)**
  * It finds the nearest navaids of the given type (see NAV_ID_* constants) and frequency.

* struct xpdata_navaid_list_t  **xpdata_navaid_by_name(int type, const char* name)**
  * It finds the nearest navaids of the given type (see NAV_ID_* constants) and short name (e.g. 'SRN').


### Airport
* struct xpdata_airport_t  **xpdata_find_nearest_airport()**
  * It returns the nearest airport. The result is cached inside the function and updated every 5 seconds (computer time, not simulation time).
  * A forced refresh can be asked via `xpdata_force_refresh()`.
  
* struct xpdata_airport_t  **xpdata_find_airport_by_coords(double lat, double lon)**
  * It finds the nearest airport with respect to the provided latitude and longitude.
  * This is an heavy sequential operation, caching the result is encouraged.
... TODO ...

## XPData data structure
```c++
struct xpdata_coords_t {
    double lat;
    double lon;
};
```

```c++
struct xpdata_navaid_t {
    const char *id; // e.g., SRN
    const char *full_name;  // Saronno VOR
    int type;                // Constants NAV_ID_* 
    xpdata_coords_t coords;
    double altitude;
    double frequency;
};
```

```c++
struct xpdata_navaid_list_t {
    struct xpdata_navaid_t **navaid;
    int nr_elements;
};
```


```c++
struct xpdata_airport_t {
    const char *id; // e.g., LIML
    const char *full_name;
    xpdata_coords_t coords;
    double altitude;

};
```

