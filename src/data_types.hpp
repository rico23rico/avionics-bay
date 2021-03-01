#ifndef DATA_TYPES_H
#define DATA_TYPES_H


struct xpdata_coords_t {
    double lat;
    double lon;
};


struct xpdata_navaid_t {
    const char *id;         // e.g., SRN
    const char *full_name;  // e.g., Saronno VOR
    int type;                // Constants NAV_ID_* 
    xpdata_coords_t coords;
    double altitude;
    double frequency;
};


#endif // DATA_TYPES_H
