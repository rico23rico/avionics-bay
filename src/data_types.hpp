#ifndef DATA_TYPES_H
#define DATA_TYPES_H

typedef int xpdata_navaid_type_t;

typedef struct xpdata_coords_t {
    double lat;
    double lon;
} xpdata_coords_t;

typedef struct xpdata_navaid_t {
    const char *id;         // e.g., SRN
    int id_len;
    const char *full_name;  // e.g., Saronno VOR
    int full_name_len;
    xpdata_navaid_type_t type; // Constants NAV_ID_* 
    xpdata_coords_t coords;
    int altitude;
    unsigned int frequency;
} xpdata_navaid_t;

typedef struct xpdata_navaid_array_t {
    const struct xpdata_navaid_t * const * navaids;
    int len;
} xpdata_navaid_array_t;

#endif // DATA_TYPES_H
