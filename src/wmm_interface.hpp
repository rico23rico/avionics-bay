#ifndef WMM_INTERFACE_H
#define WMM_INTERFACE_H

namespace avionicsbay {
    double get_declination(double lat, double lon, unsigned short year);
    bool init_wmm_interface(std::string plane_path);
}
#endif