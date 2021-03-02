#include "api.hpp"

#include "plugin.hpp"

std::shared_ptr<avionicsbay::XPData> xpdata;

xpdata_navaid_array_t build_navaid_array(std::pair<const xpdata_navaid_t* const*, size_t> std_vec) {
    struct xpdata_navaid_array_t array;
    array.navaids = std_vec.first;
    array.len = std_vec.second;
    return array;
}

xpdata_navaid_array_t get_navaid_by_name(xpdata_navaid_type_t type, const char* name) {
    return build_navaid_array(xpdata->get_navaids_by_name(type, name));
}

EXPORT_DLL xpdata_navaid_array_t get_navaid_by_freq  (xpdata_navaid_type_t type, unsigned int freq) {
    return build_navaid_array(xpdata->get_navaids_by_freq(type, freq));
}

EXPORT_DLL xpdata_navaid_array_t get_navaid_by_coords(xpdata_navaid_type_t type, double lat, double lon, bool extended_range) {
    return build_navaid_array(xpdata->get_navaids_by_coords(type, lat, lon, extended_range));
}

bool xpdata_is_ready(void) {
    return xpdata->get_is_ready();
}

namespace avionicsbay {
    void api_init() {
        xpdata = get_xpdata();
    }
}
