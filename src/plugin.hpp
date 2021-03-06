#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef _WIN32
#define EXPORT_DLL __declspec( dllexport )
#else
#define EXPORT_DLL
#endif

#include "xpdata.hpp"
#include "utilities/logger.hpp"

#include <memory>

namespace avionicsbay {

    std::shared_ptr<Logger> get_logger() noexcept;
    std::shared_ptr<XPData> get_xpdata() noexcept;
    
    void set_acf_cur_pos(double lat, double lon) noexcept;
    std::pair<double, double> get_acf_cur_pos() noexcept;
}
extern "C" {
    EXPORT_DLL bool initialize(const char* xplane_path);
    EXPORT_DLL const char* get_error(void);
    EXPORT_DLL void terminate(void);
    xpdata_navaid_array_t get_navaid_by_name(xpdata_navaid_type_t, const char*);
    bool xpdata_is_ready(void);
}

#endif // PLUGIN_H
