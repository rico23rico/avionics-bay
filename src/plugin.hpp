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

namespace xpfiles {

    std::shared_ptr<Logger> get_logger() noexcept;
    std::shared_ptr<XPData> get_xpdata() noexcept;
}
extern "C" {
    EXPORT_DLL bool initialize(const char* xplane_path);
    EXPORT_DLL const char* get_error(void);
}

#endif // PLUGIN_H
