#ifndef API_H
#define API_H

#ifdef _WIN32
#define EXPORT_DLL __declspec( dllexport )
#else
#define EXPORT_DLL
#endif

#include "data_types.hpp"

extern "C" {
    EXPORT_DLL xpdata_navaid_array_t get_navaid_by_name  (xpdata_navaid_type_t, const char*);
    EXPORT_DLL xpdata_navaid_array_t get_navaid_by_freq  (xpdata_navaid_type_t, unsigned int);
    EXPORT_DLL xpdata_navaid_array_t get_navaid_by_coords(xpdata_navaid_type_t, double, double);

    EXPORT_DLL xpdata_fix_array_t get_fixes_by_name  (const char*);
    EXPORT_DLL xpdata_fix_array_t get_fixes_by_coords(double, double);

    EXPORT_DLL xpdata_apt_array_t get_apts_by_name  (const char*);
    EXPORT_DLL xpdata_apt_array_t get_apts_by_coords(double, double);
    EXPORT_DLL const xpdata_apt_t* get_nearest_apt();
    EXPORT_DLL void request_apts_details(const char* arpt_id);

    EXPORT_DLL int get_mora(double lat, double lon);

    EXPORT_DLL void set_acf_coords(double lat, double lon);
    
    EXPORT_DLL xpdata_coords_t get_route_pos(const xpdata_apt_t *apt, int route_id);

    EXPORT_DLL xpdata_hold_array_t get_hold_by_id(const char* id);
    EXPORT_DLL xpdata_hold_array_t get_hold_by_apt_id(const char* apt_id);

    EXPORT_DLL xpdata_awy_array_t get_awy_by_id(const char* id);
    EXPORT_DLL xpdata_awy_array_t get_awy_by_start_wpt(const char* wpt_id);
    EXPORT_DLL xpdata_awy_array_t get_awy_by_end_wpt(const char* wpt_id);

    EXPORT_DLL xpdata_triangulation_t triangulate(const xpdata_apt_node_array_t* array);

    EXPORT_DLL xpdata_cifp_t get_cifp(const char* airport_id);
    EXPORT_DLL void load_cifp(const char* airport_id);
    EXPORT_DLL bool is_cifp_ready();

    EXPORT_DLL bool xpdata_is_ready(void);
}

namespace avionicsbay {
    void api_init() noexcept;
}

#endif // API_H
