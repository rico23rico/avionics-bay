#include "plugin.hpp"
#include "utilities/logger.hpp"

#include <cassert>
#include <cstring>
#include <iostream>
#include <memory>

#define LOG *logger

using xpfiles::Logger;
using xpfiles::ENDL;
using xpfiles::logger_level_t;

static std::string fatal_error;
static std::shared_ptr<Logger> logger;

bool initialize(void) {
    try {
        logger = std::make_shared<Logger>();
    } catch(...) {
        fatal_error = "Unable to open the log file.";
        return false;
    }

    LOG << logger_level_t::INFO << "Initializing xpfiles..." << ENDL;

    

    LOG << logger_level_t::INFO << "Initialization complete." << ENDL;
    return true;
}

const char* get_error(void) {
    return fatal_error.c_str();
}

