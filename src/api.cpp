#include "api.hpp"

#include "plugin.hpp"
#include "triangulator.hpp"

static avionicsbay::XPData* xpdata;
static avionicsbay::Triangulator t;

#if __GNUC__
    #define likely(x)    __builtin_expect (!!(x), 1)
    #define unlikely(x)  __builtin_expect (!!(x), 0)
#else
    #define likely(x)    (x)
    #define unlikely(x)  (x)
#endif

#define SANITY_CHECK_ARRAY() if (unlikely(xpdata == nullptr)) { return {nullptr, 0}; }
#define SANITY_CHECK_COORDS() if (unlikely(xpdata == nullptr)) { return {0, 0}; }
#define SANITY_CHECK_PTR() if (unlikely(xpdata == nullptr)) { return nullptr; }
#define SANITY_CHECK_VOID() if (unlikely(xpdata == nullptr)) { return; }
#define SANITY_CHECK_BOOL() if (unlikely(xpdata == nullptr)) { return false; }
#define SANITY_CHECK_INT() if (unlikely(xpdata == nullptr)) { return 0; }

#define SANITY_CHECK_DFR_VOID() if (unlikely(avionicsbay::get_dfr() == nullptr)) { return; }
#define SANITY_CHECK_CIFP_VOID() if (unlikely(avionicsbay::get_cifp() == nullptr)) { return; }
#define SANITY_CHECK_CIFP_BOOL() if (unlikely(avionicsbay::get_cifp() == nullptr)) { return false; }

/**************************************************************************************************/
/** Helpers functions **/
/**************************************************************************************************/

static xpdata_navaid_array_t build_navaid_array(std::pair<const xpdata_navaid_t* const*, size_t> std_vec) {
    xpdata_navaid_array_t array;
    array.navaids = std_vec.first;
    array.len = std_vec.second;
    return array;
}

static xpdata_fix_array_t build_fix_array(std::pair<const xpdata_fix_t* const*, size_t> std_vec) {
    xpdata_fix_array_t array;
    array.fixes = std_vec.first;
    array.len = std_vec.second;
    return array;
}

static xpdata_apt_array_t build_apt_array(std::pair<const xpdata_apt_t* const*, size_t> std_vec) {
    xpdata_apt_array_t array;
    array.apts = std_vec.first;
    array.len = std_vec.second;
    return array;
}

static xpdata_hold_array_t build_hold_array(std::pair<const xpdata_hold_t* const*, size_t> std_vec) {
    xpdata_hold_array_t array;
    array.holds = std_vec.first;
    array.len = std_vec.second;
    return array;
}

static xpdata_awy_array_t build_awy_array(std::pair<const xpdata_awy_t* const*, size_t> std_vec) {
    xpdata_awy_array_t array;
    array.awys = std_vec.first;
    array.len = std_vec.second;
    return array;
}

/**************************************************************************************************/
/** NAVAIDS **/
/**************************************************************************************************/
EXPORT_DLL xpdata_navaid_array_t get_navaid_by_name(xpdata_navaid_type_t type, const char* name) {
    SANITY_CHECK_ARRAY();
    return build_navaid_array(xpdata->get_navaids_by_name(type, name));
}

EXPORT_DLL xpdata_navaid_array_t get_navaid_by_freq  (xpdata_navaid_type_t type, unsigned int freq) {
    SANITY_CHECK_ARRAY();
    return build_navaid_array(xpdata->get_navaids_by_freq(type, freq));
}

EXPORT_DLL xpdata_navaid_array_t get_navaid_by_coords(xpdata_navaid_type_t type, double lat, double lon) {
    SANITY_CHECK_ARRAY();
    return build_navaid_array(xpdata->get_navaids_by_coords(type, lat, lon));
}

/**************************************************************************************************/
/** FIXES **/
/**************************************************************************************************/
EXPORT_DLL xpdata_fix_array_t get_fixes_by_name(const char* name) {
    SANITY_CHECK_ARRAY();
    return build_fix_array(xpdata->get_fixes_by_name(name));
}

EXPORT_DLL xpdata_fix_array_t get_fixes_by_coords(double lat, double lon) {
    SANITY_CHECK_ARRAY();
    return build_fix_array(xpdata->get_fixes_by_coords(lat, lon));
}

/**************************************************************************************************/
/** ARPTS **/
/**************************************************************************************************/
EXPORT_DLL xpdata_apt_array_t get_apts_by_name(const char* name) {
    SANITY_CHECK_ARRAY();
    return build_apt_array(xpdata->get_apts_by_name(name));
}

EXPORT_DLL xpdata_apt_array_t get_apts_by_coords(double lat, double lon) {
    SANITY_CHECK_ARRAY();
    return build_apt_array(xpdata->get_apts_by_coords(lat, lon));
}

EXPORT_DLL const xpdata_apt_t* get_nearest_apt() {
    SANITY_CHECK_PTR();
    return xpdata->get_nearest_airport();
}

EXPORT_DLL void request_apts_details(const char* arpt_id) {
    SANITY_CHECK_DFR_VOID();
    avionicsbay::get_dfr()->request_apts_details(arpt_id);
}

EXPORT_DLL xpdata_coords_t get_route_pos(const xpdata_apt_t *apt, int route_id) {
    SANITY_CHECK_COORDS();
    try {
        return xpdata->get_route_point(apt->pos_seek, route_id);
    } catch(...) {
        return {0,0};   // If it doesn't exist
    }
}

EXPORT_DLL xpdata_triangulation_t triangulate(const xpdata_apt_node_array_t* array) {

    const auto &result = t.triangulate(array);

    xpdata_triangulation_t triang = {
        .points = result.data(),
        .points_len = static_cast<int>(result.size())
    };

    return triang;
}

/**************************************************************************************************/
/** MISc **/
/**************************************************************************************************/

EXPORT_DLL void set_acf_coords(double lat, double lon) {
    avionicsbay::set_acf_cur_pos(lat, lon); 
}

EXPORT_DLL bool xpdata_is_ready(void) {
    SANITY_CHECK_BOOL();
    return xpdata->get_is_ready();
}

/**************************************************************************************************/
/** MORA **/
/**************************************************************************************************/
EXPORT_DLL int get_mora(double lat, double lon) {
    SANITY_CHECK_INT()
    try {
        return xpdata->get_mora(lat, lon);
    } catch(...) {
        return 0;   // If it doesn't exist
    }
}

/**************************************************************************************************/
/** HOLDs **/
/**************************************************************************************************/
EXPORT_DLL xpdata_hold_array_t get_hold_by_id(const char* id) {
    SANITY_CHECK_ARRAY();
    return build_hold_array(xpdata->get_holds_by_id(id));
}

EXPORT_DLL xpdata_hold_array_t get_hold_by_apt_id(const char* apt_id) {
    SANITY_CHECK_ARRAY();
    return build_hold_array(xpdata->get_holds_by_apt_id(apt_id));
}

/**************************************************************************************************/
/** AWYs **/
/**************************************************************************************************/
EXPORT_DLL xpdata_awy_array_t get_awy_by_id(const char* id) {
    SANITY_CHECK_ARRAY();
    return build_awy_array(xpdata->get_awys_by_id(id));
}

EXPORT_DLL xpdata_awy_array_t get_awy_by_start_wpt(const char* wpt_id) {
    SANITY_CHECK_ARRAY();
    return build_awy_array(xpdata->get_awys_by_start_wpt(wpt_id));
}

EXPORT_DLL xpdata_awy_array_t get_awy_by_end_wpt(const char* wpt_id) {
    SANITY_CHECK_ARRAY();
    return build_awy_array(xpdata->get_awys_by_end_wpt(wpt_id));
}


/**************************************************************************************************/
/** CFP **/
/**************************************************************************************************/
EXPORT_DLL xpdata_cifp_t get_cifp(const char* airport_id) {
    if (!avionicsbay::get_cifp()->is_ready()) {
        return {};
    }
    
    return avionicsbay::get_cifp()->get_full_cifp(airport_id);

}

EXPORT_DLL void load_cifp(const char* airport_id) {
    SANITY_CHECK_CIFP_VOID();
    avionicsbay::get_cifp()->load_airport(airport_id);

}

EXPORT_DLL bool is_cifp_ready() {
    return avionicsbay::get_cifp()->is_ready();
}

namespace avionicsbay {
    void api_init() noexcept {
        xpdata = get_xpdata().get();
    }
}
