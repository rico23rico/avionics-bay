#include "plugin.hpp"

#include "data_file_reader.hpp"
#include "utilities/logger.hpp"
#include "api.hpp"

#include <cassert>
#include <cstring>

#define LOG *logger << avionicsbay::STARTL

using avionicsbay::Logger;
using avionicsbay::ENDL;
using avionicsbay::logger_level_t;
using avionicsbay::DataFileReader;
using avionicsbay::XPData;

static std::string fatal_error;
static std::shared_ptr<Logger> logger;
static std::shared_ptr<XPData> xpdata;

static std::shared_ptr<DataFileReader> dfr;

namespace avionicsbay {
    std::shared_ptr<Logger> get_logger() noexcept {
        return logger;
    }

    std::shared_ptr<XPData> get_xpdata() noexcept {
        return xpdata;
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
    
}

bool initialize(const char* xplane_path) {
    try {
        logger = std::make_shared<Logger>();
    } catch(...) {
        fatal_error = "Unable to open the log file.";
        return false;
    }

    LOG << logger_level_t::INFO << "Initializing avionicsbay..." << ENDL;

    xpdata = std::make_shared<XPData>();

    if (! avionicsbay::init_data_file_reader(xplane_path)) {
        return false;
    }

    avionicsbay::api_init();

    LOG << logger_level_t::INFO << "Initialization complete." << ENDL;
    return true;
}

const char* get_error(void) {
    return fatal_error.c_str();
}

