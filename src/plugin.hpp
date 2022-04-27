#ifndef PLUGIN_H
#define PLUGIN_H

#ifdef _WIN32
#define EXPORT_DLL __declspec( dllexport )
#else
#define EXPORT_DLL
#endif

#include "xpdata.hpp"
#include "cifp_parser.hpp"
#include "data_file_reader.hpp"
#include "utilities/logger.hpp"

#ifndef GIT_COMMIT_HASH
#define GIT_COMMIT_HASH "Unknown"
#endif
#define AVIONICSBAY_VERSION "1.0.1"

#include <memory>

namespace avionicsbay {

    std::shared_ptr<Logger> get_logger() noexcept;
    std::shared_ptr<XPData> get_xpdata() noexcept;
    std::shared_ptr<DataFileReader> get_dfr() noexcept;
    std::shared_ptr<CIFPParser> get_cifp() noexcept;
    
    void set_acf_cur_pos(double lat, double lon) noexcept;
    std::pair<double, double> get_acf_cur_pos() noexcept;
}
extern "C" {
    EXPORT_DLL bool initialize(const char* xplane_path, const char* plane_path);
    EXPORT_DLL const char* get_error(void);
    EXPORT_DLL void terminate(void);

}

#endif // PLUGIN_H
