#include "cifp_parser.hpp"

#include "constants.hpp"
#include "data_types.hpp"
#include "plugin.hpp"
#include "utilities/filesystem.hpp"

#include <cassert>
#include <chrono>

#define LOG *this->logger << STARTL

#define CIFP_FILE_DIR  "Resources/default data/CIFP/"

namespace avionicsbay {

CIFPParser::CIFPParser(const std::string &xplane_directory) : xplane_directory(xplane_directory) {

    this->logger = get_logger();
    this->xpdata = get_xpdata();
    
    assert(this->logger && this->xpdata);

    LOG << logger_level_t::DEBUG << "Initializing CIFP Parser..." << ENDL;

}


void CIFPParser::perform_init_checks() {
    if (!is_a_directory(xplane_directory)) {
        throw std::runtime_error("Directory " + xplane_directory + " is not a directory or is not accessible.");
    }

    if (!is_a_directory(xplane_directory + '/' + CIFP_FILE_DIR)) {
        throw std::runtime_error("Directory " + xplane_directory + " is not a directory or is not accessible.");
    }

}

void CIFPParser::load_airport(const std::string &arpt_id) {
    this->ap_future = std::async(std::launch::async, &CIFPParser::task, this, arpt_id);
}

bool CIFPParser::is_ready() noexcept {
    if (! this->ap_future.valid()) {
        return true;  // If it's not valid, the thread has been never called or finished running
    }
    if (this->ap_future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
        // Loaded
        this->ap_future.get();
        return true;
    }
    return false;
}

void CIFPParser::task(const std::string &arpt_id) noexcept {
    // Do something...
}


} // namespace avionicsbay
