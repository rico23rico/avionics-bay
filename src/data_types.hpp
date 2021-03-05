#ifndef DATA_TYPES_H
#define DATA_TYPES_H

typedef int xpdata_navaid_type_t;

typedef struct xpdata_coords_t {
    double lat;
    double lon;
} xpdata_coords_t;

/******************************* NAVAIDS *******************************/
typedef struct xpdata_navaid_t {
    const char *id;         // e.g., SRN
    int id_len;
    const char *full_name;  // e.g., Saronno VOR
    int full_name_len;
    xpdata_navaid_type_t type; // Constants NAV_ID_* 
    xpdata_coords_t coords;
    int altitude;
    unsigned int frequency;
    bool is_coupled_dme;    // True if the vor is coupled with DME
} xpdata_navaid_t;

typedef struct xpdata_navaid_array_t {
    const struct xpdata_navaid_t * const * navaids;
    int len;
} xpdata_navaid_array_t;

/******************************* FIXES *******************************/
typedef struct xpdata_fix_t {
    const char *id;         // e.g., ROMEO
    int id_len;
    xpdata_coords_t coords;
} xpdata_fix_t;

typedef struct xpdata_fix_array_t {
    const struct xpdata_fix_t * const * fixes;
    int len;
} xpdata_fix_array_t;

/******************************* ARPT *******************************/

typedef struct xpdata_apt_rwy_t {
    char name[4];
    char sibl_name[4];              // On the other head of the runway

    xpdata_coords_t coords;
    xpdata_coords_t sibl_coords;    // On the other head of the runway
    
    double width;
    int surface_type;
    bool has_ctr_lights;
    
} xpdata_apt_rwy_t;

typedef struct xpdata_apt_t {
    const char *id;         // e.g., LIRF
    int id_len;
    
    const char *full_name;  // e.g., Roma Fiumicino
    int full_name_len;
    
    int altitude;

    const xpdata_apt_rwy_t *rwys;
    int rwys_len;
    
    xpdata_coords_t apt_center;
    
    long pos_seek;   // For internal use only, do not modify this value
    
} xpdata_apt_t;

typedef struct xpdata_apt_array_t {
    const struct xpdata_apt_t * const * apts;
    int len;
} xpdata_apt_array_t;


#endif // DATA_TYPES_H
