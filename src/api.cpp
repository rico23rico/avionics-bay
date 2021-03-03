#include "api.hpp"

#include "plugin.hpp"

std::shared_ptr<avionicsbay::XPData> xpdata;

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


EXPORT_DLL xpdata_navaid_array_t get_navaid_by_name(xpdata_navaid_type_t type, const char* name) {
    return build_navaid_array(xpdata->get_navaids_by_name(type, name));
}

EXPORT_DLL xpdata_navaid_array_t get_navaid_by_freq  (xpdata_navaid_type_t type, unsigned int freq) {
    return build_navaid_array(xpdata->get_navaids_by_freq(type, freq));
}

EXPORT_DLL xpdata_navaid_array_t get_navaid_by_coords(xpdata_navaid_type_t type, double lat, double lon) {
    return build_navaid_array(xpdata->get_navaids_by_coords(type, lat, lon));
}

EXPORT_DLL xpdata_fix_array_t get_fixes_by_name(const char* name) {
    return build_fix_array(xpdata->get_fixes_by_name(name));
}

EXPORT_DLL xpdata_fix_array_t get_fixes_by_coords(double lat, double lon) {
    return build_fix_array(xpdata->get_fixes_by_coords(lat, lon));
}

bool xpdata_is_ready(void) {
    return xpdata->get_is_ready();
}

namespace avionicsbay {
    void api_init() {
        xpdata = get_xpdata();
    }
}
