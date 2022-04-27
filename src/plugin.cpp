#include "plugin.hpp"

#include "utilities/logger.hpp"
#include "api.hpp"
#include "wmm_interface.hpp"

#include <cassert>
#include <cstring>
#include <mutex>

#define LOG *logger << avionicsbay::STARTL

using avionicsbay::CIFPParser;
using avionicsbay::Logger;
using avionicsbay::ENDL;
using avionicsbay::logger_level_t;
using avionicsbay::DataFileReader;
using avionicsbay::XPData;

static std::string fatal_error;

static double acf_lat, acf_lon;
static std::mutex mx_acf_lat_lon;


static std::shared_ptr<Logger> logger;
static std::shared_ptr<XPData> xpdata;

static std::shared_ptr<DataFileReader> dfr;
static std::shared_ptr<CIFPParser> cifp;

namespace avionicsbay {
    std::shared_ptr<Logger> get_logger() noexcept {
        return logger;
    }

    std::shared_ptr<XPData> get_xpdata() noexcept {
        return xpdata;
    }

    std::shared_ptr<DataFileReader> get_dfr() noexcept {
        return dfr;
    }

    std::shared_ptr<CIFPParser> get_cifp() noexcept {
        return cifp;
    }

    void set_acf_cur_pos(double lat, double lon) noexcept {
        std::lock_guard<std::mutex> lk(mx_acf_lat_lon);
        acf_lat = lat;
        acf_lon = lon;
    }
    std::pair<double, double> get_acf_cur_pos() noexcept {
        std::lock_guard<std::mutex> lk(mx_acf_lat_lon);
        return std::make_pair(acf_lat, acf_lon);
    }

    bool init_data_file_reader(const char* xplane_path) {
        try {
            dfr = std::make_shared<DataFileReader>(xplane_path);
        } catch (const std::runtime_error &err) {
            LOG << logger_level_t::ERROR << "DataFileReader Error: " << err.what() << ENDL;
            return false;
        } catch (...) {
            LOG << logger_level_t::CRIT << "DataFileReader Unexpected error." << ENDL;
            return false;
        }
        return true;
    }

    bool init_cifp_parser(const char* xplane_path) {
        try {
            cifp = std::make_shared<CIFPParser>(xplane_path);
        } catch (const std::runtime_error &err) {
            LOG << logger_level_t::ERROR << "CIFPParser Error: " << err.what() << ENDL;
            return false;
        } catch (...) {
            LOG << logger_level_t::CRIT << "CIFPParser Unexpected error." << ENDL;
            return false;
        }
        return true;
    }
}

bool initialize(const char* xplane_path, const char* plane_path) {
    try {
        logger = std::make_shared<Logger>(plane_path);
    } catch(...) {
        fatal_error = "Unable to open the log file.";
        return false;
    } 

    LOG << logger_level_t::INFO << "Initializing avionicsbay..." << ENDL;
    LOG << logger_level_t::INFO << "Version: " << AVIONICSBAY_VERSION << " - Commit Hash: " << GIT_COMMIT_HASH << ENDL;
    
    xpdata = std::make_shared<XPData>();

    if (! avionicsbay::init_data_file_reader(xplane_path)) {
        return false;
    }
    
    if (! avionicsbay::init_cifp_parser(xplane_path)) {
        return false;
    }

    if (! avionicsbay::init_wmm_interface(plane_path)) {
        return false;
    }

    avionicsbay::api_init();

    LOG << logger_level_t::INFO << "Initialization complete." << ENDL;
    return true;
}

const char* get_error(void) {
    return fatal_error.c_str();
}


void terminate(void) {
    if (!dfr) {
        return;
    }
    LOG << logger_level_t::NOTICE << "Termination request..." << ENDL;
    dfr->worker_stop();
    while (dfr->is_worker_running()) {
        std::this_thread::yield();
    }
    dfr.reset();    // This will join()
    LOG << logger_level_t::DEBUG << "DFR Terminated." << ENDL;
}
